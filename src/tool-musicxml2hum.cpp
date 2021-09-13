//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Jan 19 08:07:04 PST 2020
// Filename:      musicxml2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/musicxml2hum.cpp
// Syntax:        C++11; humlib
// vim:           ts=3:noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "tool-autobeam.h"
#include "tool-chord.h"
#include "tool-musicxml2hum.h"
#include "tool-ruthfix.h"
#include "tool-transpose.h"
#include "tool-tremolo.h"
#include "tool-trillspell.h"

#include "Convert.h"
#include "HumGrid.h"
#include "HumRegex.h"

#include <string.h>
#include <stdlib.h>

#include <cctype>
#include <algorithm>
#include <regex>
// #include <vector>

using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_musicxml2hum::Tool_musicxml2hum --
//

Tool_musicxml2hum::Tool_musicxml2hum(void) {
	// Options& options = m_options;
	// options.define("k|kern=b","display corresponding **kern data");

	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");

	VoiceDebugQ = false;
	DebugQ = false;
}



//////////////////////////////
//
// Tool_musicxml2hum::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool Tool_musicxml2hum::convertFile(ostream& out, const char* filename) {
	xml_document doc;
	auto result = doc.load_file(filename);
	if (!result) {
		cerr << "\nXML file [" << filename << "] has syntax errors\n";
		cerr << "Error description:\t" << result.description() << "\n";
		cerr << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}


bool Tool_musicxml2hum::convert(ostream& out, istream& input) {
	string s(istreambuf_iterator<char>(input), {});
	return convert(out, s.c_str());
}


bool Tool_musicxml2hum::convert(ostream& out, const char* input) {
	xml_document doc;
	auto result = doc.load_string(input);
	if (!result) {
		cout << "\nXML content has syntax errors\n";
		cout << "Error description:\t" << result.description() << "\n";
		cout << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}



bool Tool_musicxml2hum::convert(ostream& out, xml_document& doc) {
	initialize();

	bool status = true; // for keeping track of problems in conversion process.

	setSoftwareInfo(doc);
	vector<string> partids;            // list of part IDs
	map<string, xml_node> partinfo;    // mapping if IDs to score-part elements
	map<string, xml_node> partcontent; // mapping of IDs to part elements

	getPartInfo(partinfo, partids, doc);
	m_used_hairpins.resize(partinfo.size());

	m_current_dynamic.resize(partids.size());
	m_current_brackets.resize(partids.size());
	m_stop_char.resize(partids.size(), "[");

	getPartContent(partcontent, partids, doc);
	vector<MxmlPart> partdata;
	partdata.resize(partids.size());
	m_last_ottava_direction.resize(partids.size());

	fillPartData(partdata, partids, partinfo, partcontent);

	// for debugging:
	//printPartInfo(partids, partinfo, partcontent, partdata);

	m_maxstaff = 0;
	// check the voice info
	for (int i=0; i<(int)partdata.size(); i++) {
		partdata[i].prepareVoiceMapping();
		m_maxstaff += partdata[i].getStaffCount();
		// for debugging:
		if (VoiceDebugQ) {
			partdata[i].printStaffVoiceInfo();
		}
	}

	// re-index voices to disallow empty intermediate voices.
	reindexVoices(partdata);

	HumGrid outdata;
	status &= stitchParts(outdata, partids, partinfo, partcontent, partdata);

	if (outdata.size() > 2) {
		if (outdata.at(0)->getDuration() == 0) {
			while (!outdata.at(0)->empty()) {
				outdata.at(1)->push_front(outdata.at(0)->back());
				outdata.at(0)->pop_back();
			}
			outdata.deleteMeasure(0);
		}
	}

	for (int i=0; i<(int)partdata.size(); i++) {
		m_hasOrnamentsQ |= partdata[i].hasOrnaments();
	}

	outdata.removeRedundantClefChanges();
	outdata.removeSibeliusIncipit();
	m_systemDecoration = getSystemDecoration(doc, outdata, partids);

	// tranfer verse counts from parts/staves to HumGrid:
	// should also do part verse counts here (-1 staffindex).
	int versecount;
	for (int p=0; p<(int)partdata.size(); p++) {
		for (int s=0; s<partdata[p].getStaffCount(); s++) {
			versecount = partdata[p].getVerseCount(s);
			outdata.setVerseCount(p, s, versecount);
		}
	}

	// transfer harmony counts from parts to HumGrid:
	for (int p=0; p<(int)partdata.size(); p++) {
		int harmonyCount = partdata[p].getHarmonyCount();
		outdata.setHarmonyCount(p, harmonyCount);
	}

	// transfer dynamics boolean for part to HumGrid
	for (int p = 0; p<(int)partdata.size(); p++) {
		bool dynstate = partdata[p].hasDynamics();
		if (dynstate) {
			outdata.setDynamicsPresent(p);
		}
	}

	// transfer figured bass boolean for part to HumGrid
	for (int p=0; p<(int)partdata.size(); p++) {
		bool fbstate = partdata[p].hasFiguredBass();
		if (fbstate) {
			outdata.setFiguredBassPresent(p);
			break;
		}
	}

	if (m_recipQ || m_forceRecipQ) {
		outdata.enableRecipSpine();
	}

	outdata.buildSingleList();
	outdata.expandLocalCommentLayers();

	// set the duration of the last slice

	HumdrumFile outfile;
	outdata.transferTokens(outfile);

	addHeaderRecords(outfile, doc);
	addFooterRecords(outfile, doc);

	Tool_ruthfix ruthfix;
	ruthfix.run(outfile);

	addMeasureOneNumber(outfile);

	// Maybe implement barnum tool and apply here based on options.

	Tool_chord chord;
	chord.run(outfile);

	if (m_hasOrnamentsQ) {
		Tool_trillspell trillspell;
		trillspell.run(outfile);
	}

	if (m_hasTremoloQ) {
		Tool_tremolo tremolo;
		tremolo.run(outfile);
	}

	if (m_software == "sibelius") {
		// Needed at least for Sibelius 19.5/Dolet 6.6 for Sibelius
		// where grace note groups are not beamed in the MusicXML export.
		Tool_autobeam gracebeam;
		vector<string> argv;
		argv.push_back("autobeam"); // name of program (placeholder)
		argv.push_back("-g");       // beam adjacent grace notes
		gracebeam.process(argv);
		// Need to force a reparsing of the files contents to
		// analyze strands.  For now just create a temporary
		// Humdrum file to force the analysis of the strands.
		stringstream sstream;
		sstream << outfile;
		HumdrumFile outfile2;
		outfile2.readString(sstream.str());
		gracebeam.run(outfile2);
		outfile = outfile2;
	}

	if (m_hasTransposition) {
		Tool_transpose transpose;
		vector<string> argv;
		argv.push_back("transpose"); // name of program (placeholder)
		argv.push_back("-C");        // transpose to concert pitch
		transpose.process(argv);
		transpose.run(outfile);
		if (transpose.hasHumdrumText()) {
			stringstream ss;
			transpose.getHumdrumText(ss);
			outfile.readString(ss.str());
			printResult(out, outfile);
		}
	} else {
		for (int i=0; i<outfile.getLineCount(); i++) {
			outfile[i].createLineFromTokens();
		}
		printResult(out, outfile);
	}

	// add RDFs
	if (m_slurabove) {
		out << "!!!RDF**kern: > = above" << endl;
	}
	if (m_slurbelow) {
		out << "!!!RDF**kern: < = below" << endl;
	}

	for (int i=0; i<(int)partdata.size(); i++) {
		if (partdata[i].hasEditorialAccidental()) {
			out << "!!!RDF**kern: i = editorial accidental" << endl;
			break;
		}
	}

	// put the above code in here some time:
	prepareRdfs(partdata);
	printRdfs(out);

	return status;
}



//////////////////////////////
//
// Tool_musicxml2hum::addMeasureOneNumber -- For the first measure if it occurs before
//    the first data, change = to =1.  Maybe check next measure for a number and
//    addd one less than that number instead of 1.
//

void Tool_musicxml2hum::addMeasureOneNumber(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isBarline()) {
			continue;
		}
		HTp token = infile.token(i, 0);
		string value = *token;
		bool hasdigit = false;
		for (int j=0; j<(int)value.size(); j++) {
			if (isdigit(value[j])) {
				hasdigit = true;
				break;
			}
		}
		if (hasdigit) {
			break;
		}
		// there is no digit on barline, so add one.

		string newvalue = "=";
		if (value.size() < 2) {
			newvalue += "1";
		} else if (value[1] != '=') {
			newvalue += "1";
			newvalue += value.substr(1);
		}
		token->setText(newvalue);

		// Add "1" to other spines here:
		for (int j=1; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			tok->setText(newvalue);
		}
		break;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::printResult -- filter out
//      some item if not necessary:
//
// MuseScore calls everything "Piano" by default, so suppress
// this instrument name if there is only one **kern spine in
// the file.
//

void Tool_musicxml2hum::printResult(ostream& out, HumdrumFile& outfile) {
	vector<HTp> kernspines = outfile.getKernSpineStartList();
	if (kernspines.size() > 1) {
		out << outfile;
	} else {
		for (int i=0; i<outfile.getLineCount(); i++) {
			bool isPianoLabel = false;
			bool isPianoAbbr  = false;
			bool isPartNum    = false;
			bool isStaffNum   = false;
			if (!outfile[i].isInterpretation()) {
				out << outfile[i] << "\n";
				continue;
			}
			for (int j=0; j<outfile[i].getFieldCount(); j++) {
				if (*outfile.token(i, j) == "*I\"Piano") {
					isPianoLabel = true;
				} else if (*outfile.token(i, j) == "*I'Pno.") {
					isPianoAbbr = true;
				} else if (*outfile.token(i, j) == "*staff1") {
					isStaffNum = true;
				} else if (*outfile.token(i, j) == "*part1") {
					isPartNum = true;
				}
			}
			if (isPianoLabel || isPianoAbbr || isStaffNum || isPartNum) {
				continue;
			}
			out << outfile[i] << "\n";
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::printRdfs --
//

void Tool_musicxml2hum::printRdfs(ostream& out) {
	if (!m_caesura_rdf.empty()) {
		out << m_caesura_rdf << "\n";
	}
}



//////////////////////////////
//
// Tool_muisicxml2hum::setSoftwareInfo -- Store which software program generated the
//    MusicXML data to handle locale variants.  There can be more than one
//    <software> entry, so desired information is not necessarily in the first one.
//

void Tool_musicxml2hum::setSoftwareInfo(xml_document& doc) {
	string xpath = "/score-partwise/identification/encoding/software";
	string software = doc.select_node(xpath.c_str()).node().child_value();
	HumRegex hre;
	if (hre.search(software, "sibelius", "i")) {
		m_software = "sibelius";
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::cleanSpaces -- Converts newlines and tabs to spaces, and removes
//     trailing spaces from the string.  Does not remove leading spaces, but this could
//     be added.  Another variation would be to use \n to encode newlines if they need
//     to be preserved, but for now converting them to spaces.
//

string& Tool_musicxml2hum::cleanSpaces(string& input) {
	for (int i=0; i<(int)input.size(); i++) {
		if (std::isspace(input[i])) {
			input[i] = ' ';
		}
	}
	while ((!input.empty()) && std::isspace(input.back())) {
		input.resize(input.size() - 1);
	}
	return input;
}



//////////////////////////////
//
// Tool_musicxml2hum::cleanSpacesAndColons -- Converts newlines and
//     tabs to spaces, and removes leading and trailing spaces from the
//     string.  Another variation would be to use \n to encode newlines
//     if they need to be preserved, but for now converting them to spaces.
//     Colons (:) are also converted to &colon;.

string Tool_musicxml2hum::cleanSpacesAndColons(const string& input) {
	string output;
	bool foundnonspace = false;
	for (int i=0; i<(int)input.size(); i++) {
		if (std::isspace(input[i])) {
			if (!foundnonspace) {
				output += ' ';
			}
		}
		if (input[i] == ':') {
			foundnonspace = true;
			output += "&colon;";
		} else {
			output += input[i];
			foundnonspace = true;
		}
	}
	while ((!output.empty()) && std::isspace(output.back())) {
		output.resize(output.size() - 1);
	}
	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::addHeaderRecords -- Inserted in reverse order
//      (last record inserted first).
//

void Tool_musicxml2hum::addHeaderRecords(HumdrumFile& outfile, xml_document& doc) {
	string xpath;
	HumRegex hre;

	if (!m_systemDecoration.empty()) {
		// outfile.insertLine(0, "!!!system-decoration: " + m_systemDecoration);
		if (m_systemDecoration != "s1") {
			outfile.appendLine("!!!system-decoration: " + m_systemDecoration);
		}
	}

	xpath = "/score-partwise/credit/credit-words";
   pugi::xpath_node_set credits = doc.select_nodes(xpath.c_str());
	map<string, int> keys;
	vector<string> refs;
	vector<int> positions; // +1 = above, -1 = below;
	for (auto it = credits.begin(); it != credits.end(); it++) {
		string contents = cleanSpaces(it->node().child_value());
		if (contents.empty()) {
			continue;
		}
		if ((contents[0] != '@') && (contents[0] != '!')) {
			continue;
		}

		if (contents.size() >= 3) {
			// If line starts with "@@" then place at end of score.
			if ((contents[0] == '@') && (contents[1] == '@')) {
				positions.push_back(-1);
			} else {
				positions.push_back(1);
			}
		} else {
			positions.push_back(1);
		}

		if (hre.search(contents, "^[@!]+([^\\s]+):")) {
			// reference record
			string key = hre.getMatch(1);
			keys[key] = 1;
			hre.replaceDestructive(contents, "!!!", "^[!@]+");
			refs.push_back(contents);
		} else {
			// global comment
			hre.replaceDestructive(contents, "!!", "^[!@]+");
			refs.push_back(contents);
		}
	}

	// OTL: title //////////////////////////////////////////////////////////

	// Sibelius method
	xpath = "/score-partwise/work/work-title";
	string worktitle = cleanSpaces(doc.select_node(xpath.c_str()).node().child_value());
	string otl_record;
	string omv_record;
	bool worktitleQ = false;
	if ((worktitle != "") && (worktitle != "Title")) {
		otl_record = "!!!OTL: ";
		otl_record += worktitle;
		worktitleQ = true;
	}

	xpath = "/score-partwise/movement-title";
	string mtitle = cleanSpaces(doc.select_node(xpath.c_str()).node().child_value());
	if (mtitle != "") {
		if (worktitleQ) {
			omv_record = "!!!OMV: ";
			omv_record += mtitle;
		} else {
			otl_record = "!!!OTL: ";
			otl_record += mtitle;
		}
	}

	// COM: composer /////////////////////////////////////////////////////////
	// CDT: composer's dates
	xpath = "/score-partwise/identification/creator[@type='composer']";
	string composer = cleanSpaces(doc.select_node(xpath.c_str()).node().child_value());
	string cdt_record;
	if (composer != "") {
		if (hre.search(composer, R"(\((.*?\d.*?)\))")) {
			string dates = hre.getMatch(1);
			// hre.replaceDestructive(composer, "", R"(\()" + dates + R"(\))");
			auto loc = composer.find(dates);
			if (loc != std::string::npos) {
				composer.replace(loc-1, dates.size()+2, "");
			}
			hre.replaceDestructive(composer, "", R"(^\s+)");
			hre.replaceDestructive(composer, "", R"(\s+$)");
			if (hre.search(composer, R"(([^\s]+) +([^\s]+))")) {
				composer = hre.getMatch(2) + ", " + hre.getMatch(1);
			}
			if (dates != "") {
				if (hre.search(dates, R"(\b(\d{4})\?)")) {
					string replacement = "~";
					replacement += hre.getMatch(1);
					hre.replaceDestructive(dates, replacement, R"(\b\d{4}\?)");
					cdt_record = "!!!CDT: ";
					cdt_record += dates;
				}
			}
		}
	}


	for (int i=(int)refs.size()-1; i>=0; i--) {
		if (positions.at(i) > 0) {
			// place at start of file
			outfile.insertLine(0, refs[i]);
		}
	}

	for (int i=0; i<(int)refs.size(); i++) {
		if (positions.at(i) < 0) {
			// place at end of file
			outfile.appendLine(refs[i]);
		}
	}

	if ((!omv_record.empty()) && (!keys["OMV"])) {
		outfile.insertLine(0, omv_record);
	}

	if ((!otl_record.empty()) && (!keys["OTL"])) {
		outfile.insertLine(0, otl_record);
	}

	if ((!cdt_record.empty()) && (!keys["CDT"])) {
		outfile.insertLine(0, cdt_record);
	}

	if ((!composer.empty()) && (!keys["COM"])) {
		// Don't print composer name if it is "Composer".
		if (composer != "Composer") {
			string com_record = "!!!COM: " + composer;
			outfile.insertLine(0, com_record);
		}
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::addFooterRecords --
//

void Tool_musicxml2hum::addFooterRecords(HumdrumFile& outfile, xml_document& doc) {

	// YEM: copyright
	string copy = doc.select_node("/score-partwise/identification/rights").node().child_value();
	bool validcopy = true;
	if (copy == "") {
		validcopy = false;
	}
	if ((copy.length() == 2) && ((unsigned char)copy[0] == 0xc2) && ((unsigned char)copy[1] == 0xa9)) {
		validcopy = false;
	}
	if ((copy.find("opyright") != std::string::npos) && (copy.size() < 15)) {
		validcopy = false;
	}

	if (validcopy) {
		string yem_record = "!!!YEM: ";
		yem_record += cleanSpaces(copy);
		outfile.appendLine(yem_record);
	}

	// RDF:
	if (m_hasEditorial) {
		string rdf_record = "!!!RDF**kern: i = editorial accidental";
		outfile.appendLine(rdf_record);
	}
}



//////////////////////////////
//
// initialize --
//

void Tool_musicxml2hum::initialize(void) {
	m_recipQ = getBoolean("recip");
	m_stemsQ = getBoolean("stems");
	m_hasOrnamentsQ = false;
}



//////////////////////////////
//
// Tool_musicxml2hum::reindexVoices --
//

void Tool_musicxml2hum::reindexVoices(vector<MxmlPart>& partdata) {
	for (int p=0; p<(int)partdata.size(); p++) {
		for (int m=0; m<(int)partdata[p].getMeasureCount(); m++) {
			MxmlMeasure* measure = partdata[p].getMeasure(m);
			if (!measure) {
				continue;
			}
			reindexMeasure(measure);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::prepareRdfs --
//

void Tool_musicxml2hum::prepareRdfs(vector<MxmlPart>& partdata) {
	string caesura;
	for (int i=0; i<(int)partdata.size(); i++) {
		caesura = partdata[i].getCaesura();
		if (!caesura.empty()) {
		}
	}

	if (!caesura.empty()) {
		m_caesura_rdf = "!!!RDF**kern: " + caesura + " = caesura";
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::reindexMeasure --
//

void Tool_musicxml2hum::reindexMeasure(MxmlMeasure* measure) {
	if (!measure) {
		return;
	}

	vector<vector<int> > staffVoiceCounts;
	vector<MxmlEvent*>& elist = measure->getEventList();

	for (int i=0; i<(int)elist.size(); i++) {
		int staff = elist[i]->getStaffIndex();
		int voice = elist[i]->getVoiceIndex();

		if ((voice >= 0) && (staff >= 0)) {
			if (staff >= (int)staffVoiceCounts.size()) {
				int newsize = staff + 1;
				staffVoiceCounts.resize(newsize);
			}
			if (voice >= (int)staffVoiceCounts[staff].size()) {
				int oldsize = (int)staffVoiceCounts[staff].size();
				int newsize = voice + 1;
				staffVoiceCounts[staff].resize(newsize);
				for (int i=oldsize; i<newsize; i++) {
					staffVoiceCounts[staff][voice] = 0;
				}
			}
			staffVoiceCounts[staff][voice]++;
		}
	}

	bool needreindexing = false;

	for (int i=0; i<(int)staffVoiceCounts.size(); i++) {
		if (staffVoiceCounts[i].size() < 2) {
			continue;
		}
		for (int j=1; j<(int)staffVoiceCounts[i].size(); j++) {
			if (staffVoiceCounts[i][j] == 0) {
				needreindexing = true;
				break;
			}
		}
		if (needreindexing) {
			break;
		}
	}

	if (!needreindexing) {
		return;
	}

	vector<vector<int> > remapping;
	remapping.resize(staffVoiceCounts.size());
	int reindex;
	for (int i=0; i<(int)staffVoiceCounts.size(); i++) {
		remapping[i].resize(staffVoiceCounts[i].size());
		reindex = 0;
		for (int j=0; j<(int)remapping[i].size(); j++) {
			if (remapping[i].size() == 1) {
				remapping[i][j] = 0;
				continue;
			}
			if (staffVoiceCounts[i][j]) {
				remapping[i][j] = reindex++;
			} else {
				remapping[i][j] = -1;  // invalidate voice
			}
		}
	}

	// Go back and remap the voice indexes of elements.
	// Presuming that the staff does not need to be reindex.
	for (int i=0; i<(int)elist.size(); i++) {
		int oldvoice = elist[i]->getVoiceIndex();
		int staff = elist[i]->getStaffIndex();
		if (oldvoice < 0) {
			continue;
		}
		int newvoice = remapping[staff][oldvoice];
		if (newvoice == oldvoice) {
			continue;
		}
		elist[i]->setVoiceIndex(newvoice);
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::setOptions --
//

void Tool_musicxml2hum::setOptions(int argc, char** argv) {
	m_options.process(argc, argv);
}


void Tool_musicxml2hum::setOptions(const vector<string>& argvlist) {
    m_options.process(argvlist);
}



//////////////////////////////
//
// Tool_musicxml2hum::getOptionDefinitions -- Used to avoid
//     duplicating the definitions in the test main() function.
//

Options Tool_musicxml2hum::getOptionDefinitions(void) {
	return m_options;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// Tool_musicxml2hum::fillPartData --
//

bool Tool_musicxml2hum::fillPartData(vector<MxmlPart>& partdata,
		const vector<string>& partids, map<string, xml_node>& partinfo,
		map<string, xml_node>& partcontent) {

	bool output = true;
	for (int i=0; i<(int)partinfo.size(); i++) {
		partdata[i].setPartNumber(i+1);
		output &= fillPartData(partdata[i], partids[i], partinfo[partids[i]],
				partcontent[partids[i]]);
	}
	return output;
}


bool Tool_musicxml2hum::fillPartData(MxmlPart& partdata,
		const string& id, xml_node partdeclaration, xml_node partcontent) {
	if (m_stemsQ) {
		partdata.enableStems();
	}

	partdata.parsePartInfo(partdeclaration);
	// m_last_ottava_direction.at(partdata.getPartIndex()).resize(partdata.getStaffCount());
	// staff count is incorrect at this point? Just assume 32 staves in the part, which should
	// be 28-30 staffs too many.
	m_last_ottava_direction.at(partdata.getPartIndex()).resize(32);

	int count;
	auto measures = partcontent.select_nodes("./measure");
	for (int i=0; i<(int)measures.size(); i++) {
		partdata.addMeasure(measures[i].node());
		count = partdata.getMeasureCount();
		if (count > 1) {
			HumNum dur = partdata.getMeasure(count-1)->getTimeSigDur();
			if (dur == 0) {
				HumNum dur = partdata.getMeasure(count-2)
						->getTimeSigDur();
				if (dur > 0) {
					partdata.getMeasure(count - 1)->setTimeSigDur(dur);
				}
			}
		}

	}
	return true;
}



//////////////////////////////
//
// Tool_musicxml2hum::printPartInfo -- Debug information.
//

void Tool_musicxml2hum::printPartInfo(vector<string>& partids,
		map<string, xml_node>& partinfo, map<string, xml_node>& partcontent,
		vector<MxmlPart>& partdata) {
	cout << "\nPart information in the file:" << endl;
	int maxmeasure = 0;
	for (int i=0; i<(int)partids.size(); i++) {
		cout << "\tPART " << i+1 << " id = " << partids[i] << endl;
		cout << "\tMAXSTAFF " << partdata[i].getStaffCount() << endl;
		cout << "\t\tpart name:\t"
		     << getChildElementText(partinfo[partids[i]], "part-name") << endl;
		cout << "\t\tpart abbr:\t"
		     << getChildElementText(partinfo[partids[i]], "part-abbreviation")
		     << endl;
		auto node = partcontent[partids[i]];
		auto measures = node.select_nodes("./measure");
		cout << "\t\tMeasure count:\t" << measures.size() << endl;
		if (maxmeasure < (int)measures.size()) {
			maxmeasure = (int)measures.size();
		}
		cout << "\t\tTotal duration:\t" << partdata[i].getDuration() << endl;
	}

	MxmlMeasure* measure;
	for (int i=0; i<maxmeasure; i++) {
		cout << "m" << i+1 << "\t";
		for (int j=0; j<(int)partdata.size(); j++) {
			measure = partdata[j].getMeasure(i);
			if (measure) {
				cout << measure->getDuration();
			}
			if (j < (int)partdata.size() - 1) {
				cout << "\t";
			}
		}
		cout << endl;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartNames --
//

void Tool_musicxml2hum::insertPartNames(HumGrid& outdata, vector<MxmlPart>& partdata) {

	bool hasname = false;
	bool hasabbr = false;

	for (int i=0; i<(int)partdata.size(); i++) {
		string value;
		value = partdata[i].getPartName();
		if (!value.empty()) {
			hasname = true;
			break;
		}
	}

	for (int i=0; i<(int)partdata.size(); i++) {
		string value;
		value = partdata[i].getPartAbbr();
		if (!value.empty()) {
			hasabbr = true;
			break;
		}
	}

	if (!(hasabbr || hasname)) {
		return;
	}

	GridMeasure* gm;
	if (outdata.empty()) {
		gm = new GridMeasure(&outdata);
		outdata.push_back(gm);
	} else {
		gm = outdata[0];
	}

	int maxstaff;

	if (hasabbr) {
		for (int i=0; i<(int)partdata.size(); i++) {
			string partabbr = partdata[i].getPartAbbr();
			if (partabbr.empty()) {
				continue;
			}
			string abbr = "*I'" + partabbr;
			maxstaff = outdata.getStaffCount(i);
			gm->addLabelAbbrToken(abbr, 0, i, maxstaff-1, 0, (int)partdata.size(), maxstaff);
		}
	}

	if (hasname) {
		for (int i=0; i<(int)partdata.size(); i++) {
			string partname = partdata[i].getPartName();
			if (partname.empty()) {
				continue;
			}
			if (partname.find("MusicXML") != string::npos) {
				// ignore Finale dummy part names
				continue;
			}
			if (partname.find("Part_") != string::npos) {
				// ignore SharpEye dummy part names
				continue;
			}
			if (partname.find("Unnamed") != string::npos) {
				// ignore Sibelius dummy part names
				continue;
			}
			string name = "*I\"" + partname;
			maxstaff = outdata.getStaffCount(i);
			gm->addLabelToken(name, 0, i, maxstaff-1, 0, (int)partdata.size(), maxstaff);
		}
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::stitchParts -- Merge individual parts into a
//     single score sequence.
//

bool Tool_musicxml2hum::stitchParts(HumGrid& outdata,
		vector<string>& partids, map<string, xml_node>& partinfo,
		map<string, xml_node>& partcontent, vector<MxmlPart>& partdata) {
	if (partdata.size() == 0) {
		return false;
	}

	int i;
	int measurecount = partdata[0].getMeasureCount();
	// i used to start at 1 for some strange reason.
	for (i=0; i<(int)partdata.size(); i++) {
		if (measurecount != partdata[i].getMeasureCount()) {
			cerr << "ERROR: cannot handle parts with different measure\n";
			cerr << "counts yet. Compare MM" << measurecount << " to MM";
			cerr << partdata[i].getMeasureCount() << endl;
			exit(1);
		}
	}

	vector<int> partstaves(partdata.size(), 0);
	for (i=0; i<(int)partstaves.size(); i++) {
		partstaves[i] = partdata[i].getStaffCount();
	}

	bool status = true;
	int m;
	for (m=0; m<partdata[0].getMeasureCount(); m++) {
		status &= insertMeasure(outdata, m, partdata, partstaves);
		// a hack for now:
		// insertSingleMeasure(outfile);
		// measures.push_back(&outfile[outfile.getLineCount()-1]);
	}

	moveBreaksToEndOfPreviousMeasure(outdata);

	insertPartNames(outdata, partdata);

	return status;
}



//////////////////////////////
//
// moveBreaksToEndOfPreviousMeasure --
//

void Tool_musicxml2hum::moveBreaksToEndOfPreviousMeasure(HumGrid& outdata) {
	for (int i=1; i<(int)outdata.size(); i++) {
		GridMeasure* gm = outdata[i];
		GridMeasure* gmlast = outdata[i-1];
		if (!gm || !gmlast) {
			continue;
		}
		if (gm->begin() == gm->end()) {
			// empty measure
			return;
		}
		GridSlice *firstit = *(gm->begin());
		HumNum starttime = firstit->getTimestamp();
		for (auto it = gm->begin(); it != gm->end(); it++) {
			HumNum time2 = (*it)->getTimestamp();
			if (time2 > starttime) {
				break;
			}
			if (!(*it)->isGlobalComment()) {
				continue;
			}
			HTp token = (*it)->at(0)->at(0)->at(0)->getToken();
			if (!token) {
				continue;
			}
			if ((*token == "!!linebreak:original") ||
			    (*token == "!!pagebreak:original")) {
				GridSlice *swapper = *it;
				gm->erase(it);
				gmlast->push_back(swapper);
				// there can be only one break, so quit the loop now.
				break;
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::cleanupMeasures --
//     Also add barlines here (keeping track of the
//     duration of each measure).
//

void Tool_musicxml2hum::cleanupMeasures(HumdrumFile& outfile,
		vector<HLp> measures) {

   HumdrumToken* token;
	for (int i=0; i<outfile.getLineCount(); i++) {
		if (!outfile[i].isBarline()) {
			continue;
		}
		if (!outfile[i+1].isInterpretation()) {
			int fieldcount = outfile[i+1].getFieldCount();
			for (int j=1; j<fieldcount; j++) {
				token = new HumdrumToken("=");
				outfile[i].appendToken(token);
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertSingleMeasure --
//

void Tool_musicxml2hum::insertSingleMeasure(HumdrumFile& outfile) {
	HLp line = new HumdrumLine;
	HumdrumToken* token;
	token = new HumdrumToken("=");
	line->appendToken(token);
	line->createLineFromTokens();
	outfile.appendLine(line);
}



//////////////////////////////
//
// Tool_musicxml2hum::insertAllToken --
//

void Tool_musicxml2hum::insertAllToken(HumdrumFile& outfile,
		vector<MxmlPart>& partdata, const string& common) {

	HLp line = new HumdrumLine;
	HumdrumToken* token;

	int i, j;
	for (i=0; i<(int)partdata.size(); i++) {
		for (j=0; j<(int)partdata[i].getStaffCount(); j++) {
			token = new HumdrumToken(common);
			line->appendToken(token);
		}
		for (j=0; j<(int)partdata[i].getVerseCount(); j++) {
			token = new HumdrumToken(common);
			line->appendToken(token);
		}
	}
	outfile.appendLine(line);
}



//////////////////////////////
//
// Tool_musicxml2hum::insertMeasure --
//

bool Tool_musicxml2hum::insertMeasure(HumGrid& outdata, int mnum,
		vector<MxmlPart>& partdata, vector<int> partstaves) {

	GridMeasure* gm = outdata.addMeasureToBack();

	MxmlMeasure* xmeasure;
	vector<MxmlMeasure*> measuredata;
	vector<vector<SimultaneousEvents>* > sevents;
	int i;

	for (i=0; i<(int)partdata.size(); i++) {
		xmeasure = partdata[i].getMeasure(mnum);
		measuredata.push_back(xmeasure);
		if (i==0) {
			gm->setDuration(partdata[i].getMeasure(mnum)->getDuration());
			gm->setTimestamp(partdata[i].getMeasure(mnum)->getTimestamp());
			gm->setTimeSigDur(partdata[i].getMeasure(mnum)->getTimeSigDur());
		}
		checkForDummyRests(xmeasure);
		sevents.push_back(xmeasure->getSortedEvents());
		if (i == 0) {
			// only checking measure style of first barline
			gm->setBarStyle(xmeasure->getBarStyle());
		}
	}

	vector<HumNum> curtime(partdata.size());
	vector<HumNum> measuredurs(partdata.size());
	vector<int> curindex(partdata.size(), 0); // assuming data in a measure...
	HumNum nexttime = -1;

	vector<vector<MxmlEvent*>> endingDirections(partdata.size());

	HumNum tsdur;
	for (i=0; i<(int)curtime.size(); i++) {
		tsdur = measuredata[i]->getTimeSigDur();
		if ((tsdur == 0) && (i > 0)) {
			tsdur = measuredata[i-1]->getTimeSigDur();
			measuredata[i]->setTimeSigDur(tsdur);
		}

		// Keep track of hairpin endings that should be attached
		// the the previous note (and doubling the ending marker
		// to indicate that the timestamp of the ending is at the
		// end rather than the start of the note.
		vector<MxmlEvent*>& events = measuredata[i]->getEventList();
		xml_node hairpin = xml_node(NULL);
		for (int j=(int)events.size() - 1; j >= 0; j--) {
			if (events[j]->getElementName() == "note") {
				if (hairpin) {
					events[j]->setHairpinEnding(hairpin);
					hairpin = xml_node(NULL);
				}
				break;
			} else if (events[j]->getElementName() == "direction") {
				stringstream ss;
				ss.str("");
				events[j]->getNode().print(ss);
				if (ss.str().find("wedge") != string::npos) {
					if (ss.str().find("stop") != string::npos) {
						hairpin = events[j]->getNode();
					}
				}
			}
		}

		if (VoiceDebugQ) {
			for (int j=0; j<(int)events.size(); j++) {
				cerr << "!!ELEMENT: ";
				cerr << "\tTIME:  " << events[j]->getStartTime();
				cerr << "\tSTi:   " << events[j]->getStaffIndex();
				cerr << "\tVi:    " << events[j]->getVoiceIndex();
				cerr << "\tTS:    " << events[j]->getStartTime();
				cerr << "\tDUR:   " << events[j]->getDuration();
				cerr << "\tPITCH: " << events[j]->getKernPitch();
				cerr << "\tNAME:  " << events[j]->getElementName();
				cerr << endl;
			}
			cerr << "======================================" << endl;
		}
		if (!(*sevents[i]).empty()) {
			curtime[i] = (*sevents[i])[curindex[i]].starttime;
		} else {
			curtime[i] = tsdur;
		}
		if (nexttime < 0) {
			nexttime = curtime[i];
		} else if (curtime[i] < nexttime) {
			nexttime = curtime[i];
		}
		measuredurs[i] = measuredata[i]->getDuration();
	}

	bool allend = false;
	vector<SimultaneousEvents*> nowevents;
	vector<int> nowparts;
	bool status = true;

	HumNum processtime = nexttime;
	while (!allend) {
		nowevents.resize(0);
		nowparts.resize(0);
		allend = true;
		processtime = nexttime;
		nexttime = -1;
		for (i = (int)partdata.size()-1; i >= 0; i--) {
			if (curindex[i] >= (int)(*sevents[i]).size()) {
				continue;
			}

			if ((*sevents[i])[curindex[i]].starttime == processtime) {
				SimultaneousEvents* thing = &(*sevents[i])[curindex[i]];
				nowevents.push_back(thing);
				nowparts.push_back(i);
				curindex[i]++;
			}

			if (curindex[i] < (int)(*sevents[i]).size()) {
				allend = false;
				if ((nexttime < 0) ||
						((*sevents[i])[curindex[i]].starttime < nexttime)) {
					nexttime = (*sevents[i])[curindex[i]].starttime;
				}
			}
		}
		status &= convertNowEvents(outdata.back(),
				nowevents, nowparts, processtime, partdata, partstaves);
	}

	if (offsetHarmony.size() > 0) {
		insertOffsetHarmonyIntoMeasure(outdata.back());
	}
	if (offsetFiguredBass.size() > 0) {
		insertOffsetFiguredBassIntoMeasure(outdata.back());
	}
	return status;
}



//////////////////////////////
//
// Tool_musicxml2hum::insertOffsetFiguredBassIntoMeasure --
//

void Tool_musicxml2hum::insertOffsetFiguredBassIntoMeasure(GridMeasure* gm) {
	if (offsetFiguredBass.empty()) {
		return;
	}

	bool beginQ = true;
	for (auto it = gm->begin(); it != gm->end(); ++it) {
		GridSlice* gs = *it;
		if (!gs->isNoteSlice()) {
			// Only attached harmony to data lines.
			continue;
		}
		HumNum timestamp = gs->getTimestamp();
		for (int i=0; i<(int)offsetFiguredBass.size(); i++) {
			if (offsetFiguredBass[i].token == NULL) {
				continue;
 			}
			if (offsetFiguredBass[i].timestamp == timestamp) {
				// this is the slice to insert the harmony
				gs->at(offsetFiguredBass[i].partindex)->setFiguredBass(offsetFiguredBass[i].token);
				offsetFiguredBass[i].token = NULL;
			} else if (offsetFiguredBass[i].timestamp < timestamp) {
				if (beginQ) {
					cerr << "Error: Cannot insert harmony " << offsetFiguredBass[i].token
					     << " at timestamp " << offsetFiguredBass[i].timestamp
					     << " since first timestamp in measure is " << timestamp << endl;
				} else {
					m_forceRecipQ = true;
					// go back to previous note line and insert
					// new slice to store the harmony token
					auto tempit = it;
					tempit--;
					while (tempit != gm->end()) {
						if ((*tempit)->getTimestamp() == (*it)->getTimestamp()) {
							tempit--;
							continue;
						}
						int partcount = (int)(*tempit)->size();
						tempit++;
						GridSlice* newgs = new GridSlice(gm, offsetFiguredBass[i].timestamp,
								SliceType::Notes, partcount);
						newgs->at(offsetFiguredBass[i].partindex)->setFiguredBass(offsetFiguredBass[i].token);
						gm->insert(tempit, newgs);
						offsetFiguredBass[i].token = NULL;
						break;
					}
				}
			}
		}
		beginQ = false;
	}
	// If there are still valid harmonies in the input list, apppend
	// them to the end of the measure.
	for (int i=0; i<(int)offsetFiguredBass.size(); i++) {
		if (offsetFiguredBass[i].token == NULL) {
			continue;
 		}
		m_forceRecipQ = true;
		int partcount = (int)gm->back()->size();
		GridSlice* newgs = new GridSlice(gm, offsetFiguredBass[i].timestamp,
				SliceType::Notes, partcount);
		newgs->at(offsetFiguredBass[i].partindex)->setFiguredBass(offsetFiguredBass[i].token);
		gm->insert(gm->end(), newgs);
		offsetFiguredBass[i].token = NULL;
	}
	offsetFiguredBass.clear();
}



//////////////////////////////
//
// Tool_musicxml2hum::insertOffsetHarmonyIntoMeasure --
//

void Tool_musicxml2hum::insertOffsetHarmonyIntoMeasure(GridMeasure* gm) {
	if (offsetHarmony.empty()) {
		return;
	}
	// the offsetHarmony list should probably be time sorted first, and then
	// iterate through the slices once.  But there should not be many offset
	bool beginQ = true;
	for (auto it = gm->begin(); it != gm->end(); ++it) {
		GridSlice* gs = *it;
		if (!gs->isNoteSlice()) {
			// Only attached harmony to data lines.
			continue;
		}
		HumNum timestamp = gs->getTimestamp();
		for (int i=0; i<(int)offsetHarmony.size(); i++) {
			if (offsetHarmony[i].token == NULL) {
				continue;
 			}
			if (offsetHarmony[i].timestamp == timestamp) {
				// this is the slice to insert the harmony
				gs->at(offsetHarmony[i].partindex)->setHarmony(offsetHarmony[i].token);
				offsetHarmony[i].token = NULL;
			} else if (offsetHarmony[i].timestamp < timestamp) {
				if (beginQ) {
					cerr << "Error: Cannot insert harmony " << offsetHarmony[i].token
					     << " at timestamp " << offsetHarmony[i].timestamp
					     << " since first timestamp in measure is " << timestamp << endl;
				} else {
					m_forceRecipQ = true;
					// go back to previous note line and insert
					// new slice to store the harmony token
					auto tempit = it;
					tempit--;
					while (tempit != gm->end()) {
						if ((*tempit)->getTimestamp() == (*it)->getTimestamp()) {
							tempit--;
							continue;
						}
						int partcount = (int)(*tempit)->size();
						tempit++;
						GridSlice* newgs = new GridSlice(gm, offsetHarmony[i].timestamp,
								SliceType::Notes, partcount);
						newgs->at(offsetHarmony[i].partindex)->setHarmony(offsetHarmony[i].token);
						gm->insert(tempit, newgs);
						offsetHarmony[i].token = NULL;
						break;
					}
				}
			}
		}
		beginQ = false;
	}
	// If there are still valid harmonies in the input list, apppend
	// them to the end of the measure.
	for (int i=0; i<(int)offsetHarmony.size(); i++) {
		if (offsetHarmony[i].token == NULL) {
			continue;
 		}
		m_forceRecipQ = true;
		int partcount = (int)gm->back()->size();
		GridSlice* newgs = new GridSlice(gm, offsetHarmony[i].timestamp,
				SliceType::Notes, partcount);
		newgs->at(offsetHarmony[i].partindex)->setHarmony(offsetHarmony[i].token);
		gm->insert(gm->end(), newgs);
		offsetHarmony[i].token = NULL;
	}
	offsetHarmony.clear();
}



//////////////////////////////
//
// Tool_musicxml2hum::checkForDummyRests --
//

void Tool_musicxml2hum::checkForDummyRests(MxmlMeasure* measure) {
	vector<MxmlEvent*>& events = measure->getEventList();

	MxmlPart* owner = measure->getOwner();
	int maxstaff = owner->getStaffCount();
	vector<vector<int> > itemcounts(maxstaff);
	for (int i=0; i<(int)itemcounts.size(); i++) {
		itemcounts[i].resize(1);
		itemcounts[i][0] = 0;
	}

	for (int i=0; i<(int)events.size(); i++) {
		if (!nodeType(events[i]->getNode(), "note")) {
			// only counting notes/(rests) for now.  <forward> may
			// need to be counted.
			continue;
		}
     	int voiceindex = events[i]->getVoiceIndex();
		int staffindex = events[i]->getStaffIndex();

		if (voiceindex < 0) {
			continue;
		}
		if (staffindex < 0) {
			continue;
		}

		if (staffindex >= (int)itemcounts.size()) {
			itemcounts.resize(staffindex+1);
		}

		if (voiceindex >= (int)itemcounts[staffindex].size()) {
			int oldsize = (int)itemcounts[staffindex].size();
			int newsize = voiceindex + 1;
			itemcounts[staffindex].resize(newsize);
			for (int j=oldsize; j<newsize; j++) {
					  itemcounts[staffindex][j] = 0;
			}
		}
		itemcounts[staffindex][voiceindex]++;
  	}

	bool dummy = false;
	for (int i=0; i<(int)itemcounts.size(); i++) {
		for (int j=0; j<(int)itemcounts[i].size(); j++) {
			if (itemcounts[i][j]) {
				continue;
			}
			HumNum mdur = measure->getDuration();
			HumNum starttime = measure->getStartTime();
      	measure->addDummyRest(starttime, mdur, i, j);
			measure->forceLastInvisible();
			dummy = true;
		}
	}

	if (dummy) {
		measure->sortEvents();
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::convertNowEvents --
//

bool Tool_musicxml2hum::convertNowEvents(GridMeasure* outdata,
		vector<SimultaneousEvents*>& nowevents, vector<int>& nowparts,
		HumNum nowtime, vector<MxmlPart>& partdata, vector<int>& partstaves) {

	if (nowevents.size() == 0) {
		// cout << "NOW EVENTS ARE EMPTY" << endl;
		return true;
	}

	//if (0 && VoiceDebugQ) {
	//	for (int j=0; j<(int)nowevents.size(); j++) {
	//		vector<MxmlEvent*> nz = nowevents[j]->nonzerodur;
	//		for (int i=0; i<(int)nz.size(); i++) {
	//			cerr << "NOWEVENT NZ NAME: " << nz[i]->getElementName()
	//			     << "<\t" << nz[i]->getKernPitch() << endl;
	//		}
	//	}
	//}

	appendZeroEvents(outdata, nowevents, nowtime, partdata);

	if (nowevents[0]->nonzerodur.size() == 0) {
		// no duration events (should be a terminal barline)
		// ignore and deal with in calling function.
		return true;
	}

	appendNonZeroEvents(outdata, nowevents, nowtime, partdata);

	return true;
}



/////////////////////////////
//
// Tool_musicxml2hum::appendNonZeroEvents --
//

void Tool_musicxml2hum::appendNonZeroEvents(GridMeasure* outdata,
		vector<SimultaneousEvents*>& nowevents, HumNum nowtime,
		vector<MxmlPart>& partdata) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
			SliceType::Notes);
	if (outdata->empty()) {
		outdata->push_back(slice);
	} else {
		HumNum lasttime = outdata->back()->getTimestamp();
		if (nowtime >= lasttime) {
			outdata->push_back(slice);
		} else {
			// travel backwards in the measure until the correct
			// time position is found.
			auto it = outdata->rbegin();
			while (it != outdata->rend()) {
				lasttime = (*it)->getTimestamp();
				if (nowtime >= lasttime) {
					outdata->insert(it.base(), slice);
					break;
				}
				it++;
			}
		}
	}
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)nowevents.size(); i++) {
		vector<MxmlEvent*>& events = nowevents[i]->nonzerodur;
		for (int j=0; j<(int)events.size(); j++) {
			addEvent(slice, outdata, events[j], nowtime);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addEvent -- Add a note or rest.
//

void Tool_musicxml2hum::addEvent(GridSlice* slice, GridMeasure* outdata, MxmlEvent* event,
		HumNum nowtime) {
	int partindex;  // which part the event occurs in
	int staffindex; // which staff the event occurs in (need to fix)
	int voiceindex; // which voice the event occurs in (use for staff)

	partindex  = event->getPartIndex();
	staffindex = event->getStaffIndex();
	voiceindex = event->getVoiceIndex();

	string recip;
	string pitch;
	string prefix;
	string postfix;
	bool invisible = false;
	bool primarynote = true;
	vector<int> slurdirs;

	if (!event->isFloating()) {
		recip     = event->getRecip();
		HumRegex hre;
		if (hre.search(recip, "(\\d+)%(\\d+)(\\.*)")) {
			int first = hre.getMatchInt(1);
			int second = hre.getMatchInt(2);
			string dots = hre.getMatch(3);
			if (dots.empty()) {
				if ((first == 1) && (second == 2)) {
					hre.replaceDestructive(recip, "0", "1%2");
				}
				if ((first == 1) && (second == 4)) {
					hre.replaceDestructive(recip, "00", "1%4");
				}
				if ((first == 1) && (second == 3)) {
					hre.replaceDestructive(recip, "0.", "1%3");
				}
			} else {
				if ((first == 1) && (second == 2)) {
					string original = "1%2" + dots;
					string replacement = "0" + dots;
					hre.replaceDestructive(recip, replacement, original);
				}
			}
		}

		pitch     = event->getKernPitch();
		prefix    = event->getPrefixNoteInfo();
		postfix   = event->getPostfixNoteInfo(primarynote, recip);
		if (postfix.find("@") != string::npos) {
			m_hasTremoloQ = true;
		}
		bool grace     = event->isGrace();
		int slurstarts = event->hasSlurStart(slurdirs);
		int slurstops = event->hasSlurStop();

		if (pitch.find('r') != std::string::npos) {
			string restpitch =  event->getRestPitch();
			pitch += restpitch;
		}

		for (int i=0; i<slurstarts; i++) {
			prefix.insert(0, "(");
		// Ignoring slur direction for now.
		// if (slurstart) {
		// 	prefix.insert(0, "(");
		// 	if (slurdir) {
		// 		if (slurdir > 0) {
		// 			prefix.insert(1, ">");
		// 			m_slurabove++;
		// 		} else if (slurdir < 0) {
		// 			prefix.insert(1, "<");
		// 			m_slurbelow++;
		// 		}
		// 	}
		// }
		}
		for (int i=0; i<slurstops; i++) {
			postfix.push_back(')');
		}
		//if (slurstop) {
		//	postfix.push_back(')');
		//}

		invisible = isInvisible(event);
		if (event->isInvisible()) {
			invisible = true;
		}

		if (grace) {
			HumNum modification;
			HumNum dur = event->getEmbeddedDuration(modification, event->getNode()) / 4;
			if (dur.getNumerator() == 1) {
				recip = to_string(dur.getDenominator()) + "q";
			} else {
				recip = "q";
			}
			if (!event->hasGraceSlash()) {
				recip += "q";
			}
		}
	}

	stringstream ss;
	if (event->isFloating()) {
		ss << ".";
		HTp token = new HumdrumToken(ss.str());
		slice->at(partindex)->at(staffindex)->setTokenLayer(voiceindex, token,
			event->getDuration());
	} else {
		ss << prefix << recip << pitch << postfix;
		if (invisible) {
			ss << "yy";
		}

		// check for chord notes.
		HTp token;
		if (event->isChord()) {
			addSecondaryChordNotes(ss, event, recip);
			token = new HumdrumToken(ss.str());
			slice->at(partindex)->at(staffindex)->setTokenLayer(voiceindex, token,
				event->getDuration());
		} else {
			token = new HumdrumToken(ss.str());
			slice->at(partindex)->at(staffindex)->setTokenLayer(voiceindex, token,
				event->getDuration());
		}
	}

	if (DebugQ) {
		cerr << "!!TOKEN: " << ss.str();
		cerr << "\tTS: "    << event->getStartTime();
		cerr << "\tDUR: "   << event->getDuration();
		cerr << "\tSTi: "   << event->getStaffNumber();
		cerr << "\tVn: "    << event->getVoiceNumber();
		cerr << "\tSTi: "   << event->getStaffIndex();
		cerr << "\tVi: "    << event->getVoiceIndex();
		cerr << "\teNAME: " << event->getElementName();
		cerr << endl;
	}

	int vcount = addLyrics(slice->at(partindex)->at(staffindex), event);

	if (vcount > 0) {
		event->reportVerseCountToOwner(staffindex, vcount);
	}

	int hcount = addHarmony(slice->at(partindex), event, nowtime, partindex);
	if (hcount > 0) {
		event->reportHarmonyCountToOwner(hcount);
	}

	int fcount = addFiguredBass(slice->at(partindex), event, nowtime, partindex);
	if (fcount > 0) {
		event->reportFiguredBassToOwner();
	}

	if (m_current_brackets[partindex].size() > 0) {
		for (int i=0; i<(int)m_current_brackets[partindex].size(); i++) {
			event->setBracket(m_current_brackets[partindex].at(i));
		}
		m_current_brackets[partindex].clear();
		addBrackets(slice, outdata, event, nowtime, partindex);
	}

	if (m_current_text.size() > 0) {
		event->setTexts(m_current_text);
		m_current_text.clear();
		addTexts(slice, outdata, event->getPartIndex(), staffindex, voiceindex, event);
	}

	if (m_current_tempo.size() > 0) {
		event->setTempos(m_current_tempo);
		m_current_tempo.clear();
		addTempos(slice, outdata, event->getPartIndex(), staffindex, voiceindex, event);
	}

	if (m_current_dynamic[partindex].size()) {
		// only processing the first dynamic at the current time point for now.
		// Fix later so that multiple dynamics are handleded in the part at the
		// same time.  The LO parameters for multiple dynamics will need to be
		// qualified with "n=#".
		for (int i=0; i<(int)m_current_dynamic[partindex].size(); i++) {
			event->setDynamics(m_current_dynamic[partindex][i]);
			string dparam = getDynamicsParameters(m_current_dynamic[partindex][i]);

			event->reportDynamicToOwner();
			addDynamic(slice->at(partindex), event, partindex);
			if (dparam != "") {
				// deal with multiple layout entries here...
				GridMeasure *gm = slice->getMeasure();
				string fullparam = "!LO:DY" + dparam;
				if (gm) {
					gm->addDynamicsLayoutParameters(slice, partindex, fullparam);
				}
			}
		}
		m_current_dynamic[partindex].clear();
	}

	// see if a hairpin ending needs to be added before end of measure:
	xml_node enode = event->getHairpinEnding();
	if (enode) {
		event->reportDynamicToOwner();  // shouldn't be necessary
		addHairpinEnding(slice->at(partindex), event, partindex);
		// shouldn't need dynamics layout parameter
	}

	if (m_post_note_text.empty()) {
		return;
	}

	// check the text buffer for text which needs to be moved
	// after the current note.
	string index;
	index = to_string(partindex);
	index += ' ';
	index += to_string(staffindex);
	index += ' ';
	index += to_string(voiceindex);

	auto it = m_post_note_text.find(index);
	if (it == m_post_note_text.end()) {
		// There is text waiting, but not for this note
		// (for some strange reason).
		return;
	}
	vector<xml_node>& tnodes = it->second;
	for (int i=0; i<(int)tnodes.size(); i++) {
		addText(slice, outdata, partindex, staffindex, voiceindex, tnodes[i], true);
	}
	m_post_note_text.erase(it);
}



//////////////////////////////
//
// Tool_musicxml2hum::addTexts -- Add all text direction for a note.
//

void Tool_musicxml2hum::addTexts(GridSlice* slice, GridMeasure* measure, int partindex,
		int staffindex, int voiceindex, MxmlEvent* event) {
	vector<pair<int, xml_node>>& nodes = event->getTexts();
	for (auto item : nodes) {
		int newpartindex = item.first;
		int newstaffindex = 0; // Not allowing addressing text by layer (could be changed).
		addText(slice, measure, newpartindex, newstaffindex, voiceindex, item.second, false);
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addTempos -- Add all text direction for a note.
//

void Tool_musicxml2hum::addTempos(GridSlice* slice, GridMeasure* measure, int partindex,
		int staffindex, int voiceindex, MxmlEvent* event) {
	vector<pair<int, xml_node>>& nodes = event->getTempos();
	for (auto item : nodes) {
		int newpartindex = item.first;
		int newstaffindex = 0; // Not allowing addressing text by layer (could be changed).
		addTempo(slice, measure, newpartindex, newstaffindex, voiceindex, item.second);
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addBrackets --
//
//
//    <direction placement="above">
//      <direction-type>
//        <bracket default-y="12" line-end="down" line-type="dashed" number="1" type="start"/>
//      </direction-type>
//      <offset>4</offset>
//    </direction>
//
//    <direction placement="above">
//      <direction-type>
//        <bracket line-end="down" number="1" type="stop"/>
//      </direction-type>
//      <offset>5</offset>
//    </direction>
//

void Tool_musicxml2hum::addBrackets(GridSlice* slice, GridMeasure* measure, MxmlEvent* event,
	HumNum nowtime, int partindex) {
	int staffindex = 0;
	int voiceindex = 0;
	string token;
	HumNum timestamp;
	vector<xml_node> brackets = event->getBrackets();
	for (int i=0; i<(int)brackets.size(); i++) {
		xml_node bracket = brackets[i].child("direction-type").child("bracket");
		if (!bracket) {
			continue;
		}
		string linetype = bracket.attribute("line-type").as_string();
		string endtype = bracket.attribute("type").as_string();
		int number = bracket.attribute("number").as_int();
		if (endtype == "stop") {
			linetype = m_bracket_type_buffer[number];
		} else {
			m_bracket_type_buffer[number] = linetype;
		}

		if (linetype == "solid") {
			if (endtype == "start") {
				token = "*lig";
				measure->addInterpretationBefore(slice, partindex, staffindex, voiceindex, token);
			} else if (endtype == "stop") {
				token = "*Xlig";
				timestamp = nowtime + event->getDuration();
				measure->addInterpretationAfter(slice, partindex, staffindex, voiceindex, token, timestamp);
			}
		} else if (linetype == "dashed") {
			if (endtype == "start") {
				token = "*col";
				measure->addInterpretationBefore(slice, partindex, staffindex, voiceindex, token);
			} else if (endtype == "stop") {
				token = "*Xcol";
				timestamp = nowtime + event->getDuration();
				measure->addInterpretationAfter(slice, partindex, staffindex, voiceindex, token, timestamp);
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addText -- Add a text direction to the grid.
//
//      <direction placement="below">
//        <direction-type>
//          <words font-style="italic">Some Text</words>
//        </direction-type>
//      </direction>
//
// Multi-line example:
//
// <direction placement="above">
//         <direction-type>
//           <words default-y="40.00" relative-x="-9.47" relative-y="2.71">note</words>
//           <words>with newline</words>
//         </direction-type>
//       <staff>2</staff>
//       </direction>
//

void Tool_musicxml2hum::addText(GridSlice* slice, GridMeasure* measure, int partindex,
		int staffindex, int voiceindex, xml_node node, bool force) {
	string placementstring;
	xml_attribute placement = node.attribute("placement");
	if (placement) {
		string value = placement.value();
		if (value == "above") {
			placementstring = ":a";
		} else if (value == "below") {
			placementstring = ":b";
		}
	}

	xml_node child = node.first_child();
	if (!child) {
		return;
	}
	if (!nodeType(child, "direction-type")) {
		return;
	}

	xml_node grandchild = child.first_child();
	if (!grandchild) {
		return;
	}

	xml_node sibling = grandchild;

	bool dyQ = false;
	xml_attribute defaulty;

	string text;
	while (sibling) {
		if (nodeType(sibling, "words")) {
			text += sibling.child_value();
			if (!dyQ) {
				defaulty = sibling.attribute("default-y");
				if (defaulty) {
					dyQ = true;
					double number = std::stod(defaulty.value());
					if (number >= 0.0) {
						placementstring = ":a";
					} else if (number < 0.0) {
						placementstring = ":b";
					}
				}
			}
		}
		sibling = sibling.next_sibling();
	}

	if (text == "") {
		// Don't insert an empty text
		return;
	}

	// Mapping \n (0x0a) to newline (ignoring \r, (0x0d))
	string newtext;
	for (int i=0; i<(int)text.size(); i++) {
		switch (text[i]) {
			case 0x0a:
				newtext += "\\n";
			case 0x0d:
				break;
			default:
				newtext += text[i];
		}
	}
	text = newtext;

	// Remove newlines encodings at end of text.
	HumRegex hre;
	hre.replaceDestructive(text, "", "(\\\\n)+\\s*$");

	/* Problem: these are also possibly signs for figured bass
	if (text == "#") {
		// interpret as an editorial sharp marker
		setEditorialAccidental(+1, slice, partindex, staffindex, voiceindex);
		return;
	} else if (text == "b") {
		// interpret as an editorial flat marker
		setEditorialAccidental(-1, slice, partindex, staffindex, voiceindex);
		return;
	// } else if (text == u8"§") {
	} else if (text == "\xc2\xa7") {
		// interpret as an editorial natural marker
		setEditorialAccidental(0, slice, partindex, staffindex, voiceindex);
		return;
	}
	*/

	//
	// The following code should be merged into the loop to apply
	// font changes within the text.  Internal formatting code for
	// the string would need to be developed if so.  For now, just
	// the first word's style will be processed.
	//

	string stylestring;
	bool italic = false;
	bool bold = false;

	xml_attribute fontstyle = grandchild.attribute("font-style");
	if (fontstyle) {
		string value = fontstyle.value();
		if (value == "italic") {
			italic = true;
		}
	}

	xml_attribute fontweight = grandchild.attribute("font-weight");
	if (fontweight) {
		string value = fontweight.value();
		if (value == "bold") {
			bold = true;
		}
	}

	if (italic && bold) {
		stylestring = ":Bi";
	} else if (italic) {
		stylestring = ":i";
	} else if (bold) {
		stylestring = ":B";
	}

	bool interpQ = false;
	bool specialQ = false;
	bool globalQ = false;
	bool afterQ = false;
	string output;
	if (text == "!") {
		// null local comment
		output = text;
		specialQ = true;
	} else if (text == "*") {
		// null interpretation
		output = text;
		specialQ = true;
		interpQ = true;
	} else if ((text.size() > 1) && (text[0] == '*') && (text[1] != '*')) {
		// regular tandem interpretation, but disallow manipulators:
		if (text == "*^") {
			specialQ = false;
		} else if (text == "*+") {
			specialQ = false;
		} else if (text == "*-") {
			specialQ = false;
		} else if (text == "*v") {
			specialQ = false;
		} else {
			specialQ = true;
			interpQ = true;
			output = text;
		}
	} else if ((text.size() > 2) && (text[0] == '*') && (text[1] == '*')) {
		hre.replaceDestructive(text, "*", "^\\*+");
		output = text;
		specialQ = true;
		afterQ = true;
		interpQ = true;
		if (force == false) {
			// store text for later processing after the next note in the data.
			string index;
			index += to_string(partindex);
			index += ' ';
			index += to_string(staffindex);
			index += ' ';
			index += to_string(voiceindex);
			m_post_note_text[index].push_back(node);
			return;
		}
	} else if ((text.size() > 1) && (text[0] == '!') && (text[1] != '!')) {
		// embedding a local comment
		output = text;
		specialQ = true;
	} else if ((text.size() >= 2) && (text[0] == '!') && (text[1] == '!')) {
		// embedding a global comment (or bibliographic record, etc.).
		output = text;
		globalQ = true;
		specialQ = true;
	} else if (hre.search(text, "\\s*problem\\s*:\\s*(.*)\\s*$")) {
		specialQ = true;
		output = "!LO:TX:t=P:problem:";
		output += hre.getMatch(1);
		hre.replaceDestructive(output, "\\n", "\n", "g");
		hre.replaceDestructive(output, " ", "\t", "g");
	}

	if (!specialQ) {
		text = cleanSpacesAndColons(text);
		if (text.empty()) {
			// no text to display after removing whitespace
			return;
		}

		if (placementstring.empty()) {
			// force above if no placement specified
			placementstring = ":a";
		}

		output = "!LO:TX";
		output += placementstring;
		output += stylestring;
		output += ":t=";
		output += text;
	}

	// The text direction needs to be added before the last line
	// in the measure object.  If there is already an empty layout
	// slice before the current one (with no spine manipulators
	// in between), then insert onto the existing layout slice;
	// otherwise create a new layout slice.

	if (interpQ) {
		if (afterQ) {
			int voicecount = (int)slice->at(partindex)->at(staffindex)->size();
			if (voiceindex >= voicecount) {
				// Adding voices in the new slice.  It might be
				// better to first check for a previous text line
				// at the current timestamp that is empty (because there
				// is text at the same time in another spine).
				GridStaff* gs = slice->at(partindex)->at(staffindex);
				gs->resize(voiceindex+1);
				string null = slice->getNullTokenForSlice();
				for (int m=voicecount; m<voiceindex+1; m++) {
					gs->at(m) = new GridVoice(null, 0);
				}
			}
			HTp token = slice->at(partindex)->at(staffindex)->at(voiceindex)->getToken();
			HumNum tokdur = Convert::recipToDuration(token);
			HumNum timestamp = slice->getTimestamp() + tokdur;
			measure->addInterpretationAfter(slice, partindex, staffindex, voiceindex, output, timestamp);
		} else {
			measure->addInterpretationBefore(slice, partindex, staffindex, voiceindex, output);
		}
	} else if (globalQ) {
		HumNum timestamp = slice->getTimestamp();
		measure->addGlobalComment(text, timestamp);
	} else {
		// adding local comment that is not a layout parameter also goes here:
		measure->addLayoutParameter(slice, partindex, output);
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addTempo -- Add a tempo direction to the grid.
//
// <direction placement="above">
//    <direction-type>
//       <metronome parentheses="no" default-x="-35.96" relative-y="20.00">
//          <beat-unit>half</beat-unit>
//          <per-minute>80</per-minute>
//       </metronome>
//    </direction-type>
//    <sound tempo="160"/>
// </direction>
//
// Dotted tempo example:
//
// <direction placement="above">
//    <direction-type>
//       <metronome parentheses="no" default-x="-39.10" relative-y="20.00">
//          <beat-unit>quarter</beat-unit>
//          <beat-unit-dot/>
//          <per-minute>80</per-minute>
//       </metronome>
//    </direction-type>
//    <sound tempo="120"/>
// </direction>
//
//

void Tool_musicxml2hum::addTempo(GridSlice* slice, GridMeasure* measure, int partindex,
		int staffindex, int voiceindex, xml_node node) {
	string placementstring;
	xml_attribute placement = node.attribute("placement");
	if (placement) {
		string value = placement.value();
		if (value == "above") {
			placementstring = ":a";
		} else if (value == "below") {
			placementstring = ":b";
		} else {
			// force above if no explicit placement:
			placementstring = ":a";
		}
	}

	xml_node child = node.first_child();
	if (!child) {
		return;
	}
	if (!nodeType(child, "direction-type")) {
		return;
	}

	xml_node sound(NULL);
	xml_node sibling = child;
	while (sibling) {
		if (nodeType(sibling, "sound")) {
			sound = sibling;
			break;
		}
		sibling = sibling.next_sibling();
	}

	// grandchild should be <metronome> (containing textual display)
	// and <sound @tempo> which gives *MM data.
	xml_node metronome(NULL);

	xml_node grandchild = child.first_child();
	if (!grandchild) {
		return;
	}
	sibling = grandchild;

	while (sibling) {
		if (nodeType(sibling, "metronome")) {
			metronome = sibling;
		}
		sibling = sibling.next_sibling();
	}

	// get metronome parameters

	xml_node beatunit(NULL);
	xml_node beatunitdot(NULL);
	xml_node perminute(NULL);

	if (metronome) {
		sibling = metronome.first_child();
		while (sibling) {
			if (nodeType(sibling, "beat-unit")) {
				beatunit = sibling;
			} else if (nodeType(sibling, "beat-unit-dot")) {
				beatunitdot = sibling;
			} else if (nodeType(sibling, "per-minute")) {
				perminute = sibling;
			}
			sibling = sibling.next_sibling();
		}
	}

	string mmvalue;
	if (sound) {
		mmvalue = getAttributeValue(sound, "tempo");
	}

	if (!beatunit) {
		cerr << "Warning: missing beat-unit in tempo setting" << endl;
		return;
	}
	if (!perminute) {
		cerr << "Warning: missing per-minute in tempo setting" << endl;
		return;
	}

	int staff = 0;
	int voice = 0;

	if (sound) {
		string mmtok = "*MM";
		double mmv = stod(mmvalue);
		double mmi = int(mmv + 0.001);
		if (fabs(mmv - mmi) < 0.01) {
			stringstream sstream;
			sstream << mmi;
			mmtok += sstream.str();
		} else {
			mmtok += mmvalue;
		}
		HumNum timestamp = slice->getTimestamp();
		measure->addTempoToken(mmtok, timestamp, partindex, staff, voice, m_maxstaff);
	}

	string butext = beatunit.child_value();
	string pmtext = perminute.child_value();
	string stylestring;

	// create textual tempo marking
	string text;
	text = "[";
	text += butext;
	if (beatunitdot) {
		text += "-dot";
	}
	text += "]";
	text += "=";
	text += pmtext;

	string output = "!LO:TX";
	output += placementstring;
	output += stylestring;
	output += ":t=";
	output += text;

	// The text direction needs to be added before the last line in the measure object.
	// If there is already an empty layout slice before the current one (with no spine manipulators
	// in between), then insert onto the existing layout slice; otherwise create a new layout slice.
	measure->addTempoToken(slice, partindex, output);
}



//////////////////////////////
//
// setEditorialAccidental --
//

void Tool_musicxml2hum::setEditorialAccidental(int accidental, GridSlice* slice,
		int partindex, int staffindex, int voiceindex) {

	HTp tok = slice->at(partindex)->at(staffindex)->at(voiceindex)->getToken();

	if ((accidental < 0) && (tok->find("-") == string::npos))  {
		cerr << "Editorial error for " << tok << ": no flat to mark" << endl;
		return;
	}
	if ((accidental > 0) && (tok->find("#") == string::npos))  {
		cerr << "Editorial error for " << tok << ": no sharp to mark" << endl;
		return;
	}
	if ((accidental == 0) &&
			((tok->find("#") != string::npos) || (tok->find("-") != string::npos)))  {
		cerr << "Editorial error for " << tok << ": requesting a natural accidental" << endl;
		return;
	}

	string newtok = *tok;

	if (accidental == -1) {
		auto loc = newtok.find("-");
		if (loc < newtok.size()) {
			if (newtok[loc+1] == 'X') {
				// replace explicit accidental with editorial accidental
				newtok[loc+1] = 'i';
				tok->setText(newtok);
				m_hasEditorial = 'i';
			} else {
				// append i after -:
				newtok.insert(loc+1, "i");
				tok->setText(newtok);
				m_hasEditorial = 'i';
			}
		}
		return;
	}

	if (accidental == +1) {
		auto loc = newtok.find("#");
		if (loc < newtok.size()) {
			if (newtok[loc+1] == 'X') {
				// replace explicit accidental with editorial accidental
				newtok[loc+1] = 'i';
				tok->setText(newtok);
				m_hasEditorial = 'i';
			} else {
				// append i after -:
				newtok.insert(loc+1, "i");
				tok->setText(newtok);
				m_hasEditorial = 'i';
			}
		}
		return;
	}

	if (accidental == 0) {
		auto loc = newtok.find("n");
		if (loc < newtok.size()) {
			if (newtok[loc+1] == 'X') {
				// replace explicit accidental with editorial accidental
				newtok[loc+1] = 'i';
				tok->setText(newtok);
				m_hasEditorial = 'i';
			} else {
				// append i after -:
				newtok.insert(loc+1, "i");
				tok->setText(newtok);
				m_hasEditorial = 'i';
			}
		} else {
			// no natural sign, so add it after any pitch classes.
			HumRegex hre;
			hre.search(newtok, R"(([a-gA-G]+))");
			string diatonic = hre.getMatch(1);
			string newacc = diatonic + "i";
			hre.replaceDestructive(newtok, newacc, diatonic);
			tok->setText(newtok);
			m_hasEditorial = 'i';
		}
		return;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addDynamic -- extract any dynamics for the event
//
// Such as:
//    <direction placement="below">
//      <direction-type>
//        <dynamics>
//          <fff/>
//          </dynamics>
//        </direction-type>
//      <sound dynamics="140.00"/>
//      </direction>
//
// Hairpins:
//      <direction placement="below">
//        <direction-type>
//          <wedge default-y="-75" number="2" spread="15" type="diminuendo"/>
//        </direction-type>
//      </direction>
//
//      <direction>
//        <direction-type>
//          <wedge spread="15" type="stop"/>
//        </direction-type>
//      </direction>
//

void Tool_musicxml2hum::addDynamic(GridPart* part, MxmlEvent* event, int partindex) {
	vector<xml_node> directions = event->getDynamics();
	if (directions.empty()) {
		return;
	}

	HTp tok = NULL;

	for (int i=0; i<(int)directions.size(); i++) {
		xml_node direction = directions[i];
		xml_attribute placement = direction.attribute("placement");
		bool above = false;
		if (placement) {
			string value = placement.value();
			if (value == "above") {
				above = true;
			}
		}
		xml_node child = direction.first_child();
		if (!child) {
			continue;
		}
		if (!nodeType(child, "direction-type")) {
			continue;
		}
		xml_node grandchild = child.first_child();
		if (!grandchild) {
			continue;
		}

		if (!(nodeType(grandchild, "dynamics") || nodeType(grandchild, "wedge"))) {
			continue;
		}

		if (nodeType(grandchild, "dynamics")) {
			xml_node dynamic = grandchild.first_child();
			if (!dynamic) {
				continue;
			}
			string dstring = getDynamicString(dynamic);
			if (!tok) {
				tok = new HumdrumToken(dstring);
			} else {
				string oldtext = tok->getText();
				string newtext = oldtext + " " + dstring;
				tok->setText(newtext);
			}
		} else if ( nodeType(grandchild, "wedge")) {
			xml_node hairpin = grandchild;

			if (isUsedHairpin(hairpin, partindex)) {
				// need to suppress wedge ending if already used in [[ or ]]
				continue;
			}
			if (!hairpin) {
				cerr << "Warning: Expecting a hairpin, but found nothing" << endl;
				continue;
			}
			string hstring = getHairpinString(hairpin, partindex);
			if (!tok) {
				tok = new HumdrumToken(hstring);
			} else {
				string oldtext = tok->getText();
				string newtext = oldtext + " " + hstring;
				tok->setText(newtext);
			}

			// Deal here with adding an index if there is more than one hairpin.
			if ((hstring != "[") && (hstring != "]") && above) {
				tok->setValue("LO", "HP", "a", "true");
			}
		}
	}
	if (tok) {
		part->setDynamics(tok);
	}
}



//////////////////////////////
//
// Tool_musicxml::isUsedHairpin --  Needed to avoid double-insertion
//    of hairpins which were stored before a barline so that they
//    are not also repeated on the first beat of the next barline.
//    This fuction will remove the hairpin from the used array
//    when it is checked.  The used array is only for storing
//    hairpins that end on measures, so in theory there should not
//    be too many, and they will be removed fairly quickly.
//

bool Tool_musicxml2hum::isUsedHairpin(xml_node hairpin, int partindex) {
	for (int i=0; i<(int)m_used_hairpins.at(partindex).size(); i++) {
		if (hairpin == m_used_hairpins.at(partindex).at(i)) {
			// Cannot delete yet: the hairpin endings are being double accessed somewhere.
			//m_used_hairpins[partindex].erase(m_used_hairpins[partindex].begin() + i);
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_musicxml2hum::addHairpinEnding -- extract any hairpin ending
//   at the end of a measure.
//
// Hairpins:
//      <direction>
//        <direction-type>
//          <wedge spread="15" type="stop"/>
//        </direction-type>
//      </direction>
//

void Tool_musicxml2hum::addHairpinEnding(GridPart* part, MxmlEvent* event, int partindex) {

	xml_node direction = event->getHairpinEnding();
	if (!direction) {
		return;
	}

	xml_node child = direction.first_child();
	if (!child) {
		return;
	}
	if (!nodeType(child, "direction-type")) {
		return;
	}
	xml_node grandchild = child.first_child();
	if (!grandchild) {
		return;
	}

	if (!nodeType(grandchild, "wedge")) {
		return;
	}

	if (nodeType(grandchild, "wedge")) {
		xml_node hairpin = grandchild;
		if (!hairpin) {
			return;
		}
		string hstring = getHairpinString(hairpin, partindex);
		if (hstring == "[") {
			hstring = "[[";
		} else if (hstring == "]") {
			hstring = "]]";
		}
		m_used_hairpins.at(partindex).push_back(hairpin);
		HTp current = part->getDynamics();
		if (!current) {
			HTp htok = new HumdrumToken(hstring);
			part->setDynamics(htok);
		} else {
			string text = current->getText();
			text += " ";
			text += hstring;
			// Set single-note crescendos
			if (text == "< [[") {
				text = "<[";
			} else if (text == "> ]]") {
				text = ">]";
			} else if (text == "< [") {
				text = "<[";
			} else if (text == "> ]") {
				text = ">]";
			}
			current->setText(text);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::convertFiguredBassNumber --
//

string Tool_musicxml2hum::convertFiguredBassNumber(const xml_node& figure) {
	string output;
	xml_node fnum = figure.select_node("figure-number").node();
	// assuming one each of prefix/suffix:
	xml_node prefixelement = figure.select_node("prefix").node();
	xml_node suffixelement = figure.select_node("suffix").node();

	string prefix;
	if (prefixelement) {
		prefix = prefixelement.child_value();
	}

	string suffix;
	if (suffixelement) {
		suffix = suffixelement.child_value();
	}

	string number;
	if (fnum) {
		number = fnum.child_value();
	}

	string accidental;
	string slash;

	if (prefix == "flat-flat") {
		accidental = "--";
	} else if (prefix == "flat") {
		accidental = "-";
	} else if (prefix == "double-sharp") {
		accidental = "##";
	} else if (prefix == "sharp") {
		accidental = "#";
	} else if (prefix == "natural") {
		accidental = "n";
	} else if (suffix == "flat-flat") {
		accidental = "--r";
	} else if (suffix == "flat") {
		accidental = "-r";
	} else if (suffix == "double-sharp") {
		accidental = "##r";
	} else if (suffix == "sharp") {
		accidental = "#r";
	} else if (suffix == "natural") {
		accidental = "nr";
	}

	// If suffix is "cross", "slash" or "backslash",  then an accidental
	// should be given (probably either a natural or a sharp in general, but
	// could be a flat).  At the moment do not assign the accidental, but
	// in the future assign an accidental to the slashed figure, probably
	// with a post-processing tool.
	if (suffix == "cross" || prefix == "cross") {
		slash = "|";
		if (accidental.empty()) {
			accidental = "#";
		}
	} else if ((suffix == "backslash") || (prefix == "backslash")) {
		slash = "\\";
		if (accidental.empty()) {
			accidental = "#";
		}
	} else if ((suffix == "slash") || (prefix == "slash")) {
		slash = "/";
		if (accidental.empty()) {
			accidental = "-";
		}
	}

	string editorial;
	string extension;

	xml_node extendelement = figure.select_node("extend").node();
	if (extendelement) {
		string typestring = extendelement.attribute("type").value();
		if (typestring == "start") {
			extension = "_";
		}
	}

	output += accidental + number + slash + editorial + extension;

	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::getDynanmicsParameters --  Already presumed to be
//     a dynamic.
//

string Tool_musicxml2hum::getDynamicsParameters(xml_node element) {
	string output;
	if (!nodeType(element, "direction")) {
		return output;
	}

	xml_attribute placement = element.attribute("placement");
	if (!placement) {
		return output;
	}
	string value = placement.value();
	if (value == "above") {
		output = ":a";
	}
	xml_node child = element.first_child();
	if (!child) {
		return output;
	}
	if (!nodeType(child, "direction-type")) {
		return output;
	}
	xml_node grandchild = child.first_child();
	if (!grandchild) {
		return output;
	}
	if (!nodeType(grandchild, "wedge")) {
		return output;
	}

	xml_attribute wtype = grandchild.attribute("type");
	if (!wtype) {
		return output;
	}
	string value2 = wtype.value();
	if (value2 == "stop") {
		// don't apply parameters to ends of hairpins.
		output = "";
	}

	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::getFiguredBassParameters --  Already presumed to be
//     figured bass.
//

string Tool_musicxml2hum::getFiguredBassParameters(xml_node element) {
	string output;
	if (!nodeType(element, "figured-bass")) {
		return output;
	}
	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::getHairpinString --
//
// Hairpins:
//      <direction placement="below">
//        <direction-type>
//          <wedge default-y="-75" number="2" spread="15" type="diminuendo"/>
//        </direction-type>
//      </direction>
//
//      <direction>
//        <direction-type>
//          <wedge spread="15" type="stop"/>
//        </direction-type>
//      </direction>
//

string Tool_musicxml2hum::getHairpinString(xml_node element, int partindex) {
	if (nodeType(element, "wedge")) {
		xml_attribute wtype = element.attribute("type");
		if (!wtype) {
			return "???";
		}
		string output;
		string wstring = wtype.value();
		if (wstring == "diminuendo") {
			m_stop_char.at(partindex) = "]";
			output = ">";
		} else if (wstring == "crescendo") {
			m_stop_char.at(partindex) = "[";
			output = "<";
		} else if (wstring == "stop") {
			output = m_stop_char.at(partindex);
		} else {
			output = "???";
		}
		return output;
	}

	return "???";
}



//////////////////////////////
//
// Tool_musicxml2hum::getDynamicString --
//

string Tool_musicxml2hum::getDynamicString(xml_node element) {

	if (nodeType(element, "f")) {
		return "f";
	} else if (nodeType(element, "p")) {
		return "p";
	} else if (nodeType(element, "mf")) {
		return "mf";
	} else if (nodeType(element, "mp")) {
		return "mp";
	} else if (nodeType(element, "ff")) {
		return "ff";
	} else if (nodeType(element, "pp")) {
		return "pp";
	} else if (nodeType(element, "sf")) {
		return "sf";
	} else if (nodeType(element, "sfp")) {
		return "sfp";
	} else if (nodeType(element, "sfpp")) {
		return "sfpp";
	} else if (nodeType(element, "fp")) {
		return "fp";
	} else if (nodeType(element, "rf")) {
		return "rfz";
	} else if (nodeType(element, "rfz")) {
		return "rfz";
	} else if (nodeType(element, "sfz")) {
		return "sfz";
	} else if (nodeType(element, "sffz")) {
		return "sffz";
	} else if (nodeType(element, "fz")) {
		return "fz";
	} else if (nodeType(element, "fff")) {
		return "fff";
	} else if (nodeType(element, "ppp")) {
		return "ppp";
	} else if (nodeType(element, "ffff")) {
		return "ffff";
	} else if (nodeType(element, "pppp")) {
		return "pppp";
	} else {
		return "???";
	}
}


//////////////////////////////
//
// Tool_musicxml2hum::addFiguredBass --
//
// Such as:
//
//      <figured-bass>
//        <figure>
//          <figure-number>0</figure-number>
//        </figure>
//      </figured-bass>
// or:
//      <figured-bass>
//        <figure>
//          <figure-number>5</figure-number>
//          <suffix>backslash</suffix>
//        </figure>
//        <figure>
//          <figure-number>2</figure-number>
//          <suffix>cross</suffix>
//        </figure>
//      </figured-bass>
//
//      <figured-bass parentheses="yes">
//        <figure>
//          <prefix>flat</prefix>
//        </figure>
//      </figured-bass>
//
// Case where there is more than one figure attached to a note:
// (notice <duration> element)
//
//      <figured-bass>
//        <figure>
//          <figure-number>6</figure-number>
//          <extend type="start" />
//        </figure>
//        <duration>2</duration>
//      <figured-bass>
//
//

int Tool_musicxml2hum::addFiguredBass(GridPart* part, MxmlEvent* event, HumNum nowtime, int partindex) {
	if (m_current_figured_bass.empty()) {
		return 0;
	}

	int dursum = 0;
	for (int i=0; i<(int)m_current_figured_bass.size(); i++) {
		xml_node fnode = m_current_figured_bass.at(i);
		if (!fnode) {
			// strange problem
			continue;
		}
		string fstring = getFiguredBassString(fnode);

		HTp ftok = new HumdrumToken(fstring);
		if (i == 0) {
			part->setFiguredBass(ftok);
		} else {
			// store the figured bass for later handling at end of
			// measure processing.
			MusicXmlFiguredBassInfo finfo;
			finfo.timestamp = dursum;
			finfo.timestamp /= (int)event->getQTicks();
			finfo.timestamp += nowtime;
			finfo.partindex = partindex;
			finfo.token = ftok;
			offsetFiguredBass.push_back(finfo);
		}
		if (i < (int)m_current_figured_bass.size() - 1) {
			dursum += getFiguredBassDuration(fnode);
		}
	}
	m_current_figured_bass.clear();

	return 1;

/* deal with figured bass layout parameters?:
			string fparam = getFiguredBassParameters(fnode);
			if (fparam != "") {
				GridMeasure *gm = slice->getMeasure();
				string fullparam = "!LO:FB" + fparam;
				if (gm) {
					gm->addFiguredBassLayoutParameters(slice, partindex, fullparam);
				}
			}
		}
*/

}



//////////////////////////////
//
// Tool_musicxml2hum::getFiguredBassString -- extract any figured bass string
//   from XML node.
//

string Tool_musicxml2hum::getFiguredBassString(xml_node fnode) {
	string output;

	// Parentheses can only enclose an entire figure stack, not
	// individual numbers or accidentals on numbers in MusicXML,
	// so apply an editorial mark for parentheses.
	string editorial;
	xml_attribute pattr = fnode.attribute("parentheses");
	if (pattr) {
		string pval = pattr.value();
		if (pval == "yes") {
			editorial = "i";
		}
	}
	// There is no bracket for FB in musicxml (3.0).

	auto children = fnode.select_nodes("figure");
	for (int i=0; i<(int)children.size(); i++) {
		output += convertFiguredBassNumber(children[i].node());
		output += editorial;
		if (i < (int)children.size() - 1) {
			output += " ";
		}
	}

	return output;

	// HTp fbtok = new HumdrumToken(fbstring);
	// part->setFiguredBass(fbtok);
}



//////////////////////////////
//
// Tool_musicxml2hum::addHarmony --
//

int Tool_musicxml2hum::addHarmony(GridPart* part, MxmlEvent* event, HumNum nowtime,
		int partindex) {
	xml_node hnode = event->getHNode();
	if (!hnode) {
		return 0;
	}

	// fill in X with the harmony values from the <harmony> node
	string hstring = getHarmonyString(hnode);
	int offset = getHarmonyOffset(hnode);
	HTp htok = new HumdrumToken(hstring);
	if (offset == 0) {
		part->setHarmony(htok);
	} else {
		MusicXmlHarmonyInfo hinfo;
		hinfo.timestamp = offset;
		hinfo.timestamp /= (int)event->getQTicks();
		hinfo.timestamp += nowtime;
		hinfo.partindex = partindex;
		hinfo.token = htok;
		offsetHarmony.push_back(hinfo);
	}

	return 1;
}



//////////////////////////////
//
// Tool_musicxml2hum::getHarmonyOffset --
//   <harmony default-y="40">
//       <root>
//           <root-step>C</root-step>
//       </root>
//       <kind>major-ninth</kind>
//       <bass>
//           <bass-step>E</bass-step>
//       </bass>
//       <offset>-8</offset>
//   </harmony>
//

int Tool_musicxml2hum::getHarmonyOffset(xml_node hnode) {
	if (!hnode) {
		return 0;
	}
	xml_node child = hnode.first_child();
	if (!child) {
		return 0;
	}
	while (child) {
		if (nodeType(child, "offset")) {
			return atoi(child.child_value());
		}
		child = child.next_sibling();
	}

	return 0;
}



//////////////////////////////
//
// Tool_musicxml2hum::getFiguredBaseDuration -- Needed for cases where there is more
//   than one figure attached to a note.  Return value is the integer of the duration
//   element.  If will need to be converted to quarter notes later.
//
//   <figured-bass>
//      <figure>
//         <figure-number>5</figure-number>
//      </figure>
//      <figure>
//         <figure-number>3</figure-number>
//      </figure>
//      <duration>2</duration>    <-- get this field if it exists.
//   </figured-bass>
//

int Tool_musicxml2hum::getFiguredBassDuration(xml_node fnode) {
	if (!fnode) {
		return 0;
	}
	xml_node child = fnode.first_child();
	if (!child) {
		return 0;
	}
	while (child) {
		if (nodeType(child, "duration")) {
			return atoi(child.child_value());
		}
		child = child.next_sibling();
	}

	return 0;
}



//////////////////////////////
//
// Tool_musicxml2hum::getHarmonyString --
//   <harmony default-y="40">
//       <root>
//           <root-step>C</root-step>
//       </root>
//       <kind>major-ninth</kind>
//       <bass>
//           <bass-step>E</bass-step>
//       </bass>
//       <offset>-8</offset>
//   </harmony>
//
// For harmony labels from Musescore:
//
//    <harmony print-frame="no">
//      <root>
//        <root-step text="">C</root-step>
//        </root>
//      <kind text="V43">none</kind>
//      </harmony>
//
// Converts to: "V43" ignoring the root-step and kind contents
// if they are both "C" and "none".
//

//retrieved from https://www.geeksforgeeks.org/how-to-find-index-of-a-given-element-in-a-vector-in-cpp/
int getIndex(vector<string> v, string K) {
    auto it = find(v.begin(), v.end(), K);
	int index = -1;
    // If element was found
    if (it != v.end())
    {
        // calculating the index of K
        int index = it - v.begin();

    }
	return index;
}

string getInterval(string bottomNote, string topNote, int bottomAcc = 0, int topAcc = 0){
	map<string, int> notes = {
    	{"C", 0}, {"D",1}, {"E",2}, {"F",3},  {"G",4}, {"A",5}, {"B",6}
	};
	int bottomInd = notes[bottomNote];
	int topInd = notes[topNote];
	int noteInterval = topInd - bottomInd + 1;
	if (noteInterval < 2){
		noteInterval += 7;
	}
	map<string, int> clockVals{
		{"C", 0},
		{"D", 2},
		{"E", 4},
		{"F", 5},
		{"G", 7},
		{"A", 9},
		{"B", 11}
	};

	map<int, int> intervalToClock {
		{1, 0},
		{2, 2},
		{3, 4},
		{4, 5},
		{5, 7},
		{6, 9},
		{7, 11}
	};

	int bottomClock = clockVals[bottomNote] + bottomAcc;
	int topClock = clockVals[topNote] + topAcc;
	int clockValue = topClock - bottomClock;
	if (clockValue < 0){
		clockValue += 12;
	}
	int noteToClock = intervalToClock[noteInterval];
	int diff = clockValue - noteToClock;

	stringstream ss;
	if (diff > 0) {
		for (int i=0; i<diff; i++) {
			ss << "#";
		}
	} else if (diff < 0) {
		for (int i=0; i<-diff; i++) {
			ss << "b";
		}
	}
	string interval =  ss.str() + std::to_string(noteInterval);
	return interval;

}

string decipherHarte(vector<int> degrees){
	map<int, string> altStrings{
		{0, "*"},
		{1, "bbbb"},
		{2, "bbb"},
		{3, "bb"},
		{4, "b"},
		{5, ""},
		{6, "#"},
		{7, "##"},
		{8, "###"},
		{9, "####"}
	};
	stringstream hartess;
	hartess << "(";
	for (int i=0; i < degrees.size(); i++){
		int harteDegree = degrees[i] / 10;
		int harteAlter = degrees[i] % 10;
		hartess << altStrings[harteAlter];
		hartess << std::to_string(harteDegree);
		if (i < degrees.size()-1){
			hartess << ",";
		}
	}
	hartess << ")";
	return hartess.str();
}

string alterRoot(int rootalter){
	stringstream ss;
	ss << "";
	if (rootalter > 0) {
		for (int i=0; i<rootalter; i++) {
			ss << "#";
		}
	} else if (rootalter < 0) {
		for (int i=0; i<-rootalter; i++) {
			ss << "b";
		}
	}
	return ss.str();
}

string Tool_musicxml2hum::getHarmonyString(xml_node hnode) {
	if (!hnode) {
		return "";
	}
	xml_node child = hnode.first_child();
	if (!child) {
		return "";
	}
	string root;
	string kind;
	string kindtext;
	string bass;
	int rootalter = 0;
	int bassalter = 0;
	//Vectors to store altered degree information
	vector<int> degreeValue;
	vector<int> degreeAlter;
	vector<string> degreeType;
	xml_node grandchild;
	while (child) {
		if (nodeType(child, "root")) {
			grandchild = child.first_child();
			while (grandchild) {
				if (nodeType(grandchild, "root-step")) {
					root = grandchild.child_value();
				} if (nodeType(grandchild, "root-alter")) {
					rootalter = atoi(grandchild.child_value());
				}
				grandchild = grandchild.next_sibling();
			}
		} else if (nodeType(child, "kind")) {
			kindtext = getAttributeValue(child, "text");
			kind = child.child_value();
			if (kind == "") {
				kind = child.attribute("text").value();
				transform(kind.begin(), kind.end(), kind.begin(), ::tolower);
			}
		} else if (nodeType(child, "bass")) {
			grandchild = child.first_child();
			while (grandchild) {
				if (nodeType(grandchild, "bass-step")) {
					bass = grandchild.child_value();
				} if (nodeType(grandchild, "bass-alter")) {
					bassalter = atoi(grandchild.child_value());
				}
				grandchild = grandchild.next_sibling();
			}
		} else if (nodeType(child, "degree")){
			grandchild = child.first_child();
			while(grandchild){
				if (nodeType(grandchild, "degree-value")){
					degreeValue.push_back(atoi(grandchild.child_value()));
				} if (nodeType(grandchild, "degree-alter")){
					degreeAlter.push_back(atoi(grandchild.child_value()));
				} if (nodeType(grandchild, "degree-type")){
					degreeType.push_back(grandchild.child_value());
				}
				grandchild = grandchild.next_sibling();
			}
		}
		child = child.next_sibling();
	}
	stringstream ss;

	if ((kind == "none") && (root == "C") && !kindtext.empty()) {
		ss << kindtext;
		string output = cleanSpaces(ss.str());
		return output;
	}
	string rootacc = alterRoot(rootalter);
	ss << root << rootacc;

	if (root.size() && kind.size()) {
		ss << " ";
	}
	ss << kind;
	if (bass.size()) {
		ss << "/";

	}
	ss << bass;

	if (bassalter > 0) {
		for (int i=0; i<bassalter; i++) {
			ss << "#";
		}
	} else if (bassalter < 0) {
		for (int i=0; i<-bassalter; i++) {
			ss << "-";
		}
	}

	//convert degree-alter integers into sharps and flats
	map<int, string> alterations{ 
		{-2, "double-flat "},
		{-1, "flat "},
		{0, ""},
		{1, "sharp "},
		{2, "double-sharp "}
	};

	//map between musicxml kindtext and Harte syntax chord abbreviations
	map<string, string> kindMappingsShort {
		{"augmented", "aug"},
		{"augmented-seventh", "aug7"},
		{"diminished", "dim"},
		{"diminished-seventh", "dim7"},
		{"dominant", "7"},
		{"dominant-11th", "11"},
		{"dominant-13th", "13"},
		{"dominant-ninth", "9"},
		{"French", "fr6"},
		{"German", "ge6"},
		{"half-diminished", "hdim7"},
		{"Italian", "it6"},
		{"major", "maj"},
		{"major-11th", "maj11"},
		{"major-13th", "maj13"},
		{"major-ninth", "maj9"},
		{"major-minor", "minmaj7"},
		{"major-seventh", "maj7"},
		{"major-sixth", "maj6"},
		{"minor", "min"},
		{"minor-11th", "min11"},
		{"minor-13th", "min13"},
		{"minor-ninth", "min9"},
		{"minor-seventh", "min7"},
		{"minor-sixth", "min6"},
		{"Neapolitan", "N"},
		{"none", "none"},
		{"other", "other"},
		{"pedal", "ped"},
		{"power", "pow"},
		{"suspended-fourth", "sus4"},
		{"suspended-second", "sus2"},
		{"Tristan", "tris"}

	};

	//map between musicxml kindtext and long-form Harte syntax
	map<string, vector<int>> kindMappingsLong{
		{"augmented", {35, 56}},
		{"augmented-seventh", {35, 56, 74}},
		{"diminished", {34, 54}},
		{"diminished-seventh", {34, 54, 73}},
		{"dominant", {35, 55, 74}},
		{"dominant-11th", {35, 55, 74, 95, 115}},
		{"dominant-13th", {35, 55, 74, 95, 115, 135}},
		{"dominant-ninth", {35, 55, 74, 95}},
		{"French", {}},
		{"German", {}},
		{"half-diminished", {34, 55, 74}},
		{"Italian", {}},
		{"major", {35, 55}},
		{"major-11th", {35, 55, 75, 95, 115}},
		{"major-13th", {35, 55, 75, 95, 115 ,135}},
		{"major-ninth", {35, 55, 75, 95}},
		{"major-minor", {34, 55, 75}},
		{"major-seventh", {35, 55, 75}},
		{"major-sixth", {35, 55, 65}},
		{"minor", {34, 55}},
		{"minor-11th", {34, 55, 74, 95, 115}},
		{"minor-13th", {34, 55, 74, 95, 115, 135}},
		{"minor-ninth", {34, 55, 74, 95}},
		{"minor-seventh", {34, 55, 74}},
		{"minor-sixth", {34, 55, 65}},
		{"Neapolitan", {}},
		{"none", {}},
		{"other", {}},
		{"pedal", {}},
		{"power", {55}},
		{"suspended-fourth", {45, 55}},
		{"suspended-second", {25, 55}},
		{"Tristan", {46, 66, 96}}
	};
	
	//Degree information for long Harte notation (i.e. just root and scale degrees, no chord kind)
	vector<int> harteDegrees = kindMappingsLong[kind];
	string shortHarteChord = kindMappingsShort[kind];
	vector<int> shortHarteDegrees = {};

	for (int i = 0; i < degreeValue.size(); i++){
		ss << " ";
		//Remove elements of the specified degree
		if(degreeType[i] == "subtract" || degreeType[i] == "alter"){
			vector<int>::iterator it = remove_if(harteDegrees.begin(), harteDegrees.end(), [&](int k){
				return((k/10)==degreeValue[i]);
			});

			if(degreeType[i] == "subtract"){
				shortHarteDegrees.push_back(10*degreeValue[i]);
			}
		}

		//Add in added or altered degrees
		if(degreeType[i] == "add" || degreeType[i] == "alter"){
			int addedDegree = 10*degreeValue[i]+5+degreeAlter[i];
			harteDegrees.push_back(addedDegree);
			shortHarteDegrees.push_back(addedDegree);
		}
		if(degreeType[i]!="alter"){
			ss << degreeType[i];
		}
		ss << " ";
		ss << alterations[degreeAlter[i]];
		ss << degreeValue[i];
	}

	//Put the notes in the correct order.
	sort(harteDegrees.begin(), harteDegrees.end());
	harteDegrees.erase( unique( harteDegrees.begin(), harteDegrees.end() ), harteDegrees.end() );

	sort(shortHarteDegrees.begin(), shortHarteDegrees.end());
	shortHarteDegrees.erase( unique( shortHarteDegrees.begin(), shortHarteDegrees.end() ), shortHarteDegrees.end() );

	stringstream hartess;
	hartess << root << rootacc << ":" << decipherHarte(harteDegrees);

	if(bass.size()){
		hartess << "/" << getInterval(root, bass, rootalter, bassalter);
	}
	ss << " " << hartess.str();

	stringstream shortHartess;
	shortHartess << root << rootacc;
	if (shortHarteDegrees.size()){
		shortHartess << ":" << shortHarteChord << decipherHarte(shortHarteDegrees);
	} else if (shortHarteChord != "maj"){
		shortHartess << ":" << shortHarteChord;
	}
	if(bass.size()){
		shortHartess << "/" << getInterval(root, bass, rootalter, bassalter);
	}
	ss << " " << shortHartess.str();


	string output = cleanSpaces(ss.str());
	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::addLyrics --
//

int Tool_musicxml2hum::addLyrics(GridStaff* staff, MxmlEvent* event) {
	xml_node node = event->getNode();
	if (!node) {
		return 0;
	}
	HumRegex hre;
	xml_node child = node.first_child();
	xml_node grandchild;
	// int max;
	int number = 0;
	vector<xml_node> verses;
	string syllabic;
	string text;
	while (child) {
		if (!nodeType(child, "lyric")) {
			child = child.next_sibling();
			continue;
		}
		string value = child.attribute("number").value();
		if (hre.search(value, R"(verse(\d+))")) {
			// Fix for Sibelius which uses number="part8verse5" format.
			number = stoi(hre.getMatch(1));
		} else {
			number = atoi(child.attribute("number").value());
		}
		if (number > 100) {
			cerr << "Error: verse number is too large: number" << endl;
			return 0;
		}
		if (number == (int)verses.size() + 1) {
			verses.push_back(child);
		} else if ((number > 0) && (number < (int)verses.size())) {
			// replace a verse for some reason.
			verses[number-1] = child;
		} else if (number > 0) {
			int oldsize = (int)verses.size();
			int newsize = number;
			verses.resize(newsize);
			for (int i=oldsize; i<newsize; i++) {
				verses[i] = xml_node(NULL);
			}
			verses[number-1] = child;
		}
		child = child.next_sibling();
	}

	string finaltext;
	string fontstyle;
	HTp token;
	for (int i=0; i<(int)verses.size(); i++) {
		if (!verses[i]) {
			// no verse so doing an empty slot.
		} else {
			child = verses[i].first_child();
			finaltext = "";
			while (child) {
				if (nodeType(child, "syllabic")) {
					syllabic = child.child_value();
					child = child.next_sibling();
					continue;
				} else if (nodeType(child, "text")) {
					fontstyle = child.attribute("font-style").value();
					text = cleanSpaces(child.child_value());
					if (fontstyle == "italic") {
						text = "<i>" + text + "</i>";
					}
				} else if (nodeType(child, "elision")) {
					finaltext += " ";
					child = child.next_sibling();
					continue;
				} else {
					// such as <extend>
					child = child.next_sibling();
					continue;
				}
				// escape text which would otherwise be reinterpreated
				// as Humdrum syntax.
				if (!text.empty()) {
					if (text[0] == '!') {
						text.insert(0, 1, '\\');
					} else if (text[0] == '*') {
						text.insert(0, 1, '\\');
					}
				}
				child = child.next_sibling();
				if (syllabic == "middle" ) {
					finaltext += "-";
					finaltext += text;
					finaltext += "-";
				} else if (syllabic == "end") {
					finaltext += "-";
					finaltext += text;
				} else if (syllabic == "begin") {
					finaltext += text;
					finaltext += "-";
				} else {
					finaltext += text;
				}
				syllabic.clear();
			}
		}

		if (finaltext.empty()) {
			continue;
		}
		if (m_software == "sibelius") {
			hre.replaceDestructive(finaltext, " ", "_", "g");
		}

		if (verses[i]) {
			token = new HumdrumToken(finaltext);
			staff->setVerse(i,token);
		} else {
			token = new HumdrumToken(".");
			staff->setVerse(i,token);
		}
	}

	return (int)staff->getVerseCount();
}



//////////////////////////////
//
// cleanSpaces -- remove trailing and leading spaces from text.
//    Also removed doubled spaces, and converts tabs and newlines
//    into spaces.
//

string Tool_musicxml2hum::cleanSpaces(const string& input) {
	int endi = (int)input.size() - 1;
	while (endi >= 0) {
		if (isspace(input[endi])) {
			endi--;
			continue;
		}
		break;
	}
	int starti = 0;
	while (starti <= endi) {
		if (isspace(input[starti])) {
			starti++;
			continue;
		}
		break;

	}
	string output;
   for (int i=starti; i<=endi; i++) {
		if (!isspace(input[i])) {
			output += input[i];
			continue;
		}
		output += " ";
		i++;
		while ((i < endi) && isspace(input[i])) {
			i++;
		}
		i--;
	}
	if ((output.size() == 3) && ((unsigned char)output[0] == 0xee) &&
			((unsigned char)output[1] == 0x95) && ((unsigned char)output[2] == 0x91)) {
		// MuseScore elision character:
		// <text font-family="MScore Text"></text>
		output = " ";
	}

	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::isInvisible --
//

bool Tool_musicxml2hum::isInvisible(MxmlEvent* event) {
	xml_node node = event->getNode();
	if (!node) {
		return false;
	}
	if (strcmp(node.attribute("print-object").value(), "no") == 0) {
		return true;
	}

	return false;
}



//////////////////////////////
//
// Tool_musicxml2hum::addSecondaryChordNotes --
//

void Tool_musicxml2hum::addSecondaryChordNotes(ostream& output,
		MxmlEvent* head, const string& recip) {
	vector<MxmlEvent*> links = head->getLinkedNotes();
	MxmlEvent* note;
	string pitch;
	string prefix;
	string postfix;
	int slurstarts = 0;
	int slurstops  = 0;
	vector<int> slurdirs;

	bool primarynote = false;
	for (int i=0; i<(int)links.size(); i++) {
		note       = links.at(i);
		pitch      = note->getKernPitch();
		prefix     = note->getPrefixNoteInfo();
		postfix    = note->getPostfixNoteInfo(primarynote, recip);
		slurstarts = note->hasSlurStart(slurdirs);
		slurstops  = note->hasSlurStop();

		// or maybe walk backwards in the following loop?
		for (int i=0; i<slurstarts; i++) {
			prefix.insert(0, "(");
			if (slurdirs[i] > 0) {
				prefix.insert(1, ">");
				m_slurabove++;
			} else if (slurdirs[i] < 0) {
				prefix.insert(1, "<");
				m_slurbelow++;
			}
		}
		for (int i=0; i<slurstops; i++) {
			postfix.push_back(')');
		}
		//if (slurstart) {
		//	prefix.insert(0, "(");
		//	if (slurdir) {
		//		if (slurdir > 0) {
		//			prefix.insert(1, ">");
		//			m_slurabove++;
		//		} else if (slurdir < 0) {
		//			prefix.insert(1, "<");
		//			m_slurbelow++;
		//		}
		//	}
		//}
		//if (slurstop) {
		//	postfix.push_back(')');
		//}

		output << " " << prefix << recip << pitch << postfix;
	}
}



/////////////////////////////
//
// Tool_musicxml2hum::appendZeroEvents --
//

void Tool_musicxml2hum::appendZeroEvents(GridMeasure* outdata,
		vector<SimultaneousEvents*>& nowevents, HumNum nowtime,
		vector<MxmlPart>& partdata) {

	bool hasclef           = false;
	bool haskeysig         = false;
	bool haskeydesignation = false;
	bool hastransposition  = false;
	bool hastimesig        = false;
	bool hasottava         = false;
	bool hasstafflines     = false;

	vector<vector<xml_node>> clefs(partdata.size());
	vector<vector<xml_node>> keysigs(partdata.size());
	vector<vector<xml_node>> transpositions(partdata.size());
	vector<vector<xml_node>> timesigs(partdata.size());
	vector<vector<vector<xml_node>>> ottavas(partdata.size());
	vector<vector<xml_node>> hairpins(partdata.size());
	vector<vector<xml_node>> stafflines(partdata.size());

	vector<vector<vector<vector<MxmlEvent*>>>> gracebefore(partdata.size());
	vector<vector<vector<vector<MxmlEvent*>>>> graceafter(partdata.size());
	bool foundnongrace = false;

	int pindex = 0;
	xml_node child;
	xml_node grandchild;

	for (int i=0; i<(int)nowevents.size(); i++) {
		for (int j=0; j<(int)nowevents[i]->zerodur.size(); j++) {
			xml_node element = nowevents[i]->zerodur[j]->getNode();
			pindex = nowevents[i]->zerodur[j]->getPartIndex();

			if (nodeType(element, "attributes")) {
				child = element.first_child();
				while (child) {
					if (nodeType(child, "clef")) {
						clefs[pindex].push_back(child);
						hasclef = true;
						foundnongrace = true;
					}

					if (nodeType(child, "key")) {
						keysigs[pindex].push_back(child);
						haskeysig = true;
						string xpath = "mode";
						string mode = child.select_node(xpath.c_str()).node().child_value();
						if (mode != "") {
							haskeydesignation = true;
						}
						foundnongrace = true;
					}

					if (nodeType(child, "transpose")) {
						transpositions[pindex].push_back(child);
						hastransposition = true;
						foundnongrace = true;
					}

					if (nodeType(child, "staff-details")) {
						grandchild = child.first_child();
						while (grandchild) {
							if (nodeType(grandchild, "staff-lines")) {
								stafflines[pindex].push_back(grandchild);
								hasstafflines = true;
							}
							grandchild = grandchild.next_sibling();
						}
					}

					if (nodeType(child, "time")) {
						timesigs[pindex].push_back(child);
						hastimesig = true;
						foundnongrace = true;
					}
					child = child.next_sibling();
				}
			} else if (nodeType(element, "direction")) {
				// direction -> direction-type -> words
				// direction -> direction-type -> dynamics
				// direction -> direction-type -> octave-shift
				child = element.first_child();
				if (nodeType(child, "direction-type")) {
					grandchild = child.first_child();
					if (nodeType(grandchild, "words")) {
						m_current_text.emplace_back(std::make_pair(pindex, element));
					} else if (nodeType(grandchild, "metronome")) {
						m_current_tempo.emplace_back(std::make_pair(pindex, element));
					} else if (nodeType(grandchild, "dynamics")) {
						m_current_dynamic[pindex].push_back(element);
					} else if (nodeType(grandchild, "octave-shift")) {
						storeOttava(pindex, grandchild, element, ottavas);
						hasottava = true;
					} else if (nodeType(grandchild, "wedge")) {
						m_current_dynamic[pindex].push_back(element);
					} else if (nodeType(grandchild, "bracket")) {
						m_current_brackets[pindex].push_back(element);
					}
				}
			} else if (nodeType(element, "figured-bass")) {
				m_current_figured_bass.push_back(element);
			} else if (nodeType(element, "note")) {
				if (foundnongrace) {
					addEventToList(graceafter, nowevents[i]->zerodur[j]);
				} else {
					addEventToList(gracebefore, nowevents[i]->zerodur[j]);
				}
			} else if (nodeType(element, "print")) {
				processPrintElement(outdata, element, nowtime);
			}
		}
	}

	addGraceLines(outdata, gracebefore, partdata, nowtime);

	if (hasstafflines) {
		addStriaLine(outdata, stafflines, partdata, nowtime);
	}

	if (hasclef) {
		addClefLine(outdata, clefs, partdata, nowtime);
	}

	if (hastransposition) {
		addTranspositionLine(outdata, transpositions, partdata, nowtime);
	}

	if (haskeysig) {
		addKeySigLine(outdata, keysigs, partdata, nowtime);
	}

	if (haskeydesignation) {
		addKeyDesignationLine(outdata, keysigs, partdata, nowtime);
	}

	if (hastimesig) {
		addTimeSigLine(outdata, timesigs, partdata, nowtime);
	}

	if (hasottava) {
		addOttavaLine(outdata, ottavas, partdata, nowtime);
	}

	addGraceLines(outdata, graceafter, partdata, nowtime);
}



//////////////////////////////
//
// Tool_musicxml2hum::storeOcttava -- store an ottava mark which has this structure:
//
//  octaveShift:
//     <octave-shift type="down" size="8" number="1" default-y="30.00"/>
//
//  For grand staff or multi-staff parts, the staff number needs to be extracted from an uncle element:
//       <direction placement="below">
//        <direction-type>
//          <octave-shift type="up" size="8" number="1" default-y="-83.10"/>
//        </direction-type>
//        <staff>2</staff>
//      </direction>
//
// ottavas array has three dimensions: (1) is the part, (2) is the staff, and (3) is the list of ottavas.
//

void Tool_musicxml2hum::storeOttava(int pindex, xml_node octaveShift, xml_node direction,
	vector<vector<vector<xml_node>>>& ottavas) {
	int staffindex = 0;
	xml_node staffnode = direction.select_node("staff").node();
	if (staffnode && staffnode.text()) {
		int staffnum = staffnode.text().as_int();
		if (staffnum > 0) {
			staffindex = staffnum - 1;
		}
	}
	// ottavas presumed to be allocated by part, but not by staff.
	if ((int)ottavas[pindex].size() <= staffindex) {
		ottavas[pindex].resize(staffindex+1);
	}
	ottavas[pindex][staffindex].push_back(octaveShift);
}



//////////////////////////////
//
// Tool_musicxml2hum::processPrintElement --
//      <print new-page="yes">
//      <print new-system="yes">
//

void Tool_musicxml2hum::processPrintElement(GridMeasure* outdata, xml_node element,
		HumNum timestamp) {
	bool isPageBreak = false;
	bool isSystemBreak = false;
	string pageparam = element.attribute("new-page").value();
	string systemparam = element.attribute("new-system").value();
	if (pageparam == "yes") {
		isPageBreak = true;
	}
	if (systemparam == "yes") {
		isSystemBreak = true;
	}

	if (!(isPageBreak || isSystemBreak)) {
		return;
	}
	GridSlice* gs = outdata->back();

	HTp token = NULL;
	if (gs && gs->size() > 0) {
		if (gs->at(0)->size() > 0) {
			if (gs->at(0)->at(0)->size() > 0) {
				token = gs->at(0)->at(0)->at(0)->getToken();
			}
		}
	}

	if (isPageBreak) {
		if (!token || *token != "!!pagebreak:original")  {
			outdata->addGlobalComment("!!pagebreak:original", timestamp);
		}
	} else if (isSystemBreak) {
		if (!token || *token != "!!linebreak:original")  {
			outdata->addGlobalComment("!!linebreak:original", timestamp);
		}
	}
}



///////////////////////////////
//
// Tool_musicxml2hum::addEventToList --
//

void Tool_musicxml2hum::addEventToList(vector<vector<vector<vector<MxmlEvent*> > > >& list,
		MxmlEvent* event) {
	int pindex = event->getPartIndex();
	int staffindex = event->getStaffIndex();
	int voiceindex = event->getVoiceIndex();
	if (pindex >= (int)list.size()) {
		list.resize(pindex+1);
	}
	if (staffindex >= (int)list[pindex].size()) {
		list[pindex].resize(staffindex+1);
	}
	if (voiceindex >= (int)list[pindex][staffindex].size()) {
		list[pindex][staffindex].resize(voiceindex+1);
	}
	list[pindex][staffindex][voiceindex].push_back(event);
}



///////////////////////////////
//
// Tool_musicxml2hum::addGraceLines -- Add grace note lines.  The number of
//     lines is equal to the maximum number of successive grace notes in
//     any part.  Grace notes are filled in reverse sequence.
//

void Tool_musicxml2hum::addGraceLines(GridMeasure* outdata,
		vector<vector<vector<vector<MxmlEvent*> > > >& notes,
		vector<MxmlPart>& partdata, HumNum nowtime) {

	int maxcount = 0;

	for (int i=0; i<(int)notes.size(); i++) {
		for (int j=0; j<(int)notes.at(i).size(); j++) {
			for (int k=0; k<(int)notes.at(i).at(j).size(); k++) {
				if (maxcount < (int)notes.at(i).at(j).at(k).size()) {
					maxcount = (int)notes.at(i).at(j).at(k).size();
				}
			}
		}
	}

	if (maxcount == 0) {
		return;
	}

	vector<GridSlice*> slices(maxcount);
	for (int i=0; i<(int)slices.size(); i++) {
		slices[i] = new GridSlice(outdata, nowtime, SliceType::GraceNotes);
		outdata->push_back(slices[i]);
		slices[i]->initializePartStaves(partdata);
	}

	for (int i=0; i<(int)notes.size(); i++) {
		for (int j=0; j<(int)notes[i].size(); j++) {
			for (int k=0; k<(int)notes[i][j].size(); k++) {
				int startm = maxcount - (int)notes[i][j][k].size();
				for (int m=0; m<(int)notes[i][j][k].size(); m++) {
					addEvent(slices.at(startm+m), outdata, notes[i][j][k][m], nowtime);
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addClefLine --
//

void Tool_musicxml2hum::addClefLine(GridMeasure* outdata,
		vector<vector<xml_node> >& clefs, vector<MxmlPart>& partdata,
		HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::Clefs);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)clefs[i].size(); j++) {
			if (clefs[i][j]) {
				insertPartClefs(clefs[i][j], *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addStriaLine --
//

void Tool_musicxml2hum::addStriaLine(GridMeasure* outdata,
		vector<vector<xml_node> >& stafflines, vector<MxmlPart>& partdata,
		HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::Stria);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)stafflines[i].size(); j++) {
			if (stafflines[i][j]) {
				string lines = stafflines[i][j].child_value();
				int linecount = stoi(lines);
				insertPartStria(linecount, *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addTimeSigLine --
//

void Tool_musicxml2hum::addTimeSigLine(GridMeasure* outdata,
		vector<vector<xml_node> >& timesigs, vector<MxmlPart>& partdata,
		HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime, SliceType::TimeSigs);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	bool status = false;

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)timesigs[i].size(); j++) {
			if (timesigs[i][j]) {
				status |= insertPartTimeSigs(timesigs[i][j], *slice->at(i));
			}
		}
	}

	if (!status) {
		return;
	}

	// Add mensurations related to time signatures

	slice = new GridSlice(outdata, nowtime, SliceType::MeterSigs);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	// now add mensuration symbols associated with time signatures
	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)timesigs[i].size(); j++) {
			if (timesigs[i][j]) {
				insertPartMensurations(timesigs[i][j], *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addOttavaLine -- Probably there will be a problem if
//    an ottava line ends and another one starts at the same timestamp.
//    Maybe may OttavaStart and OttavaEnd be separate categories?
//

void Tool_musicxml2hum::addOttavaLine(GridMeasure* outdata,
		vector<vector<vector<xml_node>>>& ottavas, vector<MxmlPart>& partdata,
		HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::Ottavas);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int p=0; p<(int)ottavas.size(); p++) { // part loop
		for (int s=0; s<(int)ottavas[p].size(); s++) { // staff loop
			for (int j=0; j<(int)ottavas[p][s].size(); j++) { // ottava loop
				if (ottavas[p][s][j]) {
					// int scount = partdata[p].getStaffCount();
					// int ss = scount - s - 1;
					insertPartOttavas(ottavas[p][s][j], *slice->at(p), p, s, partdata[p].getStaffCount());
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addKeySigLine -- Only adding one key signature
//   for each part for now.
//

void Tool_musicxml2hum::addKeySigLine(GridMeasure* outdata,
		vector<vector<xml_node> >& keysigs,
		vector<MxmlPart>& partdata, HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::KeySigs);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)keysigs[i].size(); j++) {
			if (keysigs[i][j]) {
				insertPartKeySigs(keysigs[i][j], *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addKeyDesignationLine -- Only adding one key designation line
//   for each part for now.
//

void Tool_musicxml2hum::addKeyDesignationLine(GridMeasure* outdata,
		vector<vector<xml_node> >& keydesigs,
		vector<MxmlPart>& partdata, HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::KeyDesignations);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)keydesigs[i].size(); j++) {
			if (keydesigs[i][j]) {
				insertPartKeyDesignations(keydesigs[i][j], *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addTranspositionLine -- Transposition codes to
//   produce written parts.
//

void Tool_musicxml2hum::addTranspositionLine(GridMeasure* outdata,
		vector<vector<xml_node> >& transpositions,
		vector<MxmlPart>& partdata, HumNum nowtime) {

	GridSlice* slice = new GridSlice(outdata, nowtime,
		SliceType::Transpositions);
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		for (int j=0; j<(int)transpositions[i].size(); j++) {
			if (transpositions[i][j]) {
				insertPartTranspositions(transpositions[i][j], *slice->at(i));
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartClefs --
//

void Tool_musicxml2hum::insertPartClefs(xml_node clef, GridPart& part) {
	if (!clef) {
		// no clef for some reason.
		return;
	}

	HTp token;
	int staffnum = 0;
	while (clef) {
		clef = convertClefToHumdrum(clef, token, staffnum);
		part[staffnum]->setTokenLayer(0, token, 0);
	}

	// go back and fill in all NULL pointers with null interpretations
	fillEmpties(&part, "*");
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartStria --
//

void Tool_musicxml2hum::insertPartStria(int lines, GridPart& part) {
	HTp token = new HumdrumToken;
	string value = "*stria" + to_string(lines);
	token->setText(value);
	part[0]->setTokenLayer(0, token, 0);

	// go back and fill in all NULL pointers with null interpretations
	fillEmpties(&part, "*");
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartOttavas --
//

void Tool_musicxml2hum::insertPartOttavas(xml_node ottava, GridPart& part, int partindex,
		int partstaffindex, int staffcount) {
	if (!ottava) {
		// no ottava for some reason.
		return;
	}

	HTp token = NULL;
	while (ottava) {
		ottava = convertOttavaToHumdrum(ottava, token, partstaffindex, partindex, partstaffindex, staffcount);
		part[partstaffindex]->setTokenLayer(0, token, 0);
	}

	// go back and fill in all NULL pointers with null interpretations
	fillEmpties(&part, "*");
}



//////////////////////////////
//
// Tool_musicxml2hum::fillEmpties --
//

void Tool_musicxml2hum::fillEmpties(GridPart* part, const char* string) {
	int staffcount = (int)part->size();
	GridVoice* gv;
	int vcount;

 	for (int s=0; s<staffcount; s++) {
		GridStaff* staff = part->at(s);
		if (staff == NULL) {
			cerr << "Strange error here" << endl;
			continue;
		}
		vcount = (int)staff->size();
		if (vcount == 0) {
			gv = new GridVoice(string, 0);
			staff->push_back(gv);
		} else {
			for (int v=0; v<vcount; v++) {
				gv = staff->at(v);
				if (gv == NULL) {
					gv = new GridVoice(string, 0);
					staff->at(v) = gv;
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartKeySigs --
//

void Tool_musicxml2hum::insertPartKeySigs(xml_node keysig, GridPart& part) {
	if (!keysig) {
		return;
	}

	HTp token;
	int staffnum = 0;
	while (keysig) {
		keysig = convertKeySigToHumdrum(keysig, token, staffnum);
		if (staffnum < 0) {
			// key signature applies to all staves in part (most common case)
			for (int s=0; s<(int)part.size(); s++) {
				if (s==0) {
					part[s]->setTokenLayer(0, token, 0);
				} else {
					HTp token2 = new HumdrumToken(*token);
					part[s]->setTokenLayer(0, token2, 0);
				}
			}
		} else {
			part[staffnum]->setTokenLayer(0, token, 0);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartKeyDesignations --
//

void Tool_musicxml2hum::insertPartKeyDesignations(xml_node keydesig, GridPart& part) {
	if (!keydesig) {
		return;
	}

	HTp token;
	int staffnum = 0;
	while (keydesig) {
		token = NULL;
		keydesig = convertKeySigToHumdrumKeyDesignation(keydesig, token, staffnum);
		if (token == NULL) {
			return;
		}
		if (staffnum < 0) {
			// key signature applies to all staves in part (most common case)
			for (int s=0; s<(int)part.size(); s++) {
				if (s==0) {
					part[s]->setTokenLayer(0, token, 0);
				} else {
					string value = *token;
					HTp token2 = new HumdrumToken(value);
					part[s]->setTokenLayer(0, token2, 0);
				}
			}
		} else {
			part[staffnum]->setTokenLayer(0, token, 0);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartTranspositions --
//

void Tool_musicxml2hum::insertPartTranspositions(xml_node transposition, GridPart& part) {
	if (!transposition) {
		return;
	}

	HTp token;
	int staffnum = 0;
	while (transposition) {
		transposition = convertTranspositionToHumdrum(transposition, token, staffnum);
		if (staffnum < 0) {
			// Transposition applies to all staves in part (most common case)
			for (int s=0; s<(int)part.size(); s++) {
				if (s==0) {
					part[s]->setTokenLayer(0, token, 0);
				} else {
					HTp token2 = new HumdrumToken(*token);
					part[s]->setTokenLayer(0, token2, 0);
				}
			}
		} else {
			part[staffnum]->setTokenLayer(0, token, 0);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartTimeSigs -- Only allowing one
//		time signature per part for now.
//

bool Tool_musicxml2hum::insertPartTimeSigs(xml_node timesig, GridPart& part) {
	if (!timesig) {
		// no timesig
		return false;
	}

	bool hasmensuration = false;
	HTp token;
	int staffnum = 0;

	while (timesig) {
		hasmensuration |= checkForMensuration(timesig);
		timesig = convertTimeSigToHumdrum(timesig, token, staffnum);
		if (token && (staffnum < 0)) {
			// time signature applies to all staves in part (most common case)
			for (int s=0; s<(int)part.size(); s++) {
				if (s==0) {
					part[s]->setTokenLayer(0, token, 0);
				} else {
					HTp token2 = new HumdrumToken(*token);
					part[s]->setTokenLayer(0, token2, 0);
				}
			}
		} else if (token) {
			part[staffnum]->setTokenLayer(0, token, 0);
		}
	}

	return hasmensuration;
}



//////////////////////////////
//
// Tool_musicxml2hum::insertPartMensurations --
//

void Tool_musicxml2hum::insertPartMensurations(xml_node timesig,
		GridPart& part) {
	if (!timesig) {
		// no timesig
		return;
	}

	HTp token = NULL;
	int staffnum = 0;

	while (timesig) {
		timesig = convertMensurationToHumdrum(timesig, token, staffnum);
		if (staffnum < 0) {
			// time signature applies to all staves in part (most common case)
			for (int s=0; s<(int)part.size(); s++) {
				if (s==0) {
					part[s]->setTokenLayer(0, token, 0);
				} else {
					HTp token2 = new HumdrumToken(*token);
					part[s]->setTokenLayer(0, token2, 0);
				}
			}
		} else {
			part[staffnum]->setTokenLayer(0, token, 0);
		}
	}

}


//////////////////////////////
//
// Tool_musicxml::checkForMensuration --
//    Examples:
//        <time symbol="common">
//        <time symbol="cut">
//

bool Tool_musicxml2hum::checkForMensuration(xml_node timesig) {
	if (!timesig) {
		return false;
	}

	xml_attribute mens = timesig.attribute("symbol");
	if (mens) {
		return true;
	} else {
		return false;
	}
}


//////////////////////////////
//
//	Tool_musicxml2hum::convertTranspositionToHumdrum --
//
//  <transpose>
//     <diatonic>-1</diatonic>
//     <chromatic>-2</chromatic>
//

xml_node Tool_musicxml2hum::convertTranspositionToHumdrum(xml_node transpose,
		HTp& token, int& staffindex) {

	if (!transpose) {
		return transpose;
	}

	staffindex = -1;
	xml_attribute sn = transpose.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	int diatonic = 0;
	int chromatic = 0;

	xml_node child = transpose.first_child();
	while (child) {
		if (nodeType(child, "diatonic")) {
			diatonic = atoi(child.child_value());
		} else if (nodeType(child, "chromatic")) {
			chromatic = atoi(child.child_value());
		}
		child = child.next_sibling();
	}


	// Switching to sounding viewpoint: transposition to get written pitch:
	diatonic = -diatonic;
	chromatic = -chromatic;

	stringstream ss;
	ss << "*Trd" << diatonic << "c" << chromatic;

	token = new HumdrumToken(ss.str());

	int base40 = -Convert::transToBase40(ss.str());
	if (base40 != 0) {
		m_hasTransposition = true;
	}

	transpose = transpose.next_sibling();
	if (!transpose) {
		return transpose;
	}
	if (nodeType(transpose, "transpose")) {
		return transpose;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertKeySigToHumdrumKeyDesignation --
//
//  <key>
//     <fifths>4</fifths>
// and sometimes:
//     <mode>major</mode>
// or
//     <mode>minor</mode>
//

xml_node Tool_musicxml2hum::convertKeySigToHumdrumKeyDesignation(xml_node keysig,
		HTp& token, int& staffindex) {

	if (!keysig) {
		token = new HumdrumToken("*");
		return keysig;
	}

	staffindex = -1;
	xml_attribute sn = keysig.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	int fifths = 0;
	int mode = -1;

	xml_node child = keysig.first_child();
	while (child) {
		if (nodeType(child, "fifths")) {
			fifths = atoi(child.child_value());
		}
		if (nodeType(child, "mode")) {
			string value = child.child_value();
			if (value == "major") {
				mode = 0;
			} else if (value == "minor") {
				mode = 1;
			}
		}
		child = child.next_sibling();
	}

	if (mode < 0) {
		token = new HumdrumToken("*");
		return xml_node(NULL);
	}

	stringstream ss;
	ss << "*";

	if (mode == 0) { // major:
		switch (fifths) {
			case +7: ss << "C#"; break;
			case +6: ss << "F#"; break;
			case +5: ss << "B"; break;
			case +4: ss << "E"; break;
			case +3: ss << "A"; break;
			case +2: ss << "D"; break;
			case +1: ss << "G"; break;
			case  0: ss << "C"; break;
			case -1: ss << "F"; break;
			case -2: ss << "B-"; break;
			case -3: ss << "E-"; break;
			case -4: ss << "A-"; break;
			case -5: ss << "D-"; break;
			case -6: ss << "G-"; break;
			case -7: ss << "C-"; break;
			default:
				token = new HumdrumToken("*");
				return xml_node(NULL);
		}
	} else if (mode == 1) { // minor:
		switch (fifths) {
			case +7: ss << "a#"; break;
			case +6: ss << "d#"; break;
			case +5: ss << "g#"; break;
			case +4: ss << "c#"; break;
			case +3: ss << "f#"; break;
			case +2: ss << "b"; break;
			case +1: ss << "e"; break;
			case  0: ss << "a"; break;
			case -1: ss << "d"; break;
			case -2: ss << "g"; break;
			case -3: ss << "c"; break;
			case -4: ss << "f"; break;
			case -5: ss << "b-"; break;
			case -6: ss << "e-"; break;
			case -7: ss << "a-"; break;
			default:
				token = new HumdrumToken("*");
				return xml_node(NULL);
		}
	}
	ss << ":";

	token = new HumdrumToken(ss.str());

	keysig = keysig.next_sibling();
	if (!keysig) {
		return keysig;
	}
	if (nodeType(keysig, "key")) {
		return keysig;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertKeySigToHumdrum --
//
//  <key>
//     <fifths>4</fifths>
// and sometimes:
//     <mode>major</mode>
// or
//     <mode>minor</mode>
//

xml_node Tool_musicxml2hum::convertKeySigToHumdrum(xml_node keysig,
		HTp& token, int& staffindex) {

	if (!keysig) {
		return keysig;
	}

	staffindex = -1;
	xml_attribute sn = keysig.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	int fifths = 0;
	//int mode = -1;

	xml_node child = keysig.first_child();
	while (child) {
		if (nodeType(child, "fifths")) {
			fifths = atoi(child.child_value());
		}
		if (nodeType(child, "mode")) {
			string value = child.child_value();
			if (value == "major") {
				// mode = 0;
			} else if (value == "minor") {
				// mode = 1;
			}
		}
		child = child.next_sibling();
	}

	stringstream ss;
	ss << "*k[";
	if (fifths > 0) {
		if (fifths > 0) { ss << "f#"; }
		if (fifths > 1) { ss << "c#"; }
		if (fifths > 2) { ss << "g#"; }
		if (fifths > 3) { ss << "d#"; }
		if (fifths > 4) { ss << "a#"; }
		if (fifths > 5) { ss << "e#"; }
		if (fifths > 6) { ss << "b#"; }
	} else if (fifths < 0) {
		if (fifths < 0)  { ss << "b-"; }
		if (fifths < -1) { ss << "e-"; }
		if (fifths < -2) { ss << "a-"; }
		if (fifths < -3) { ss << "d-"; }
		if (fifths < -4) { ss << "g-"; }
		if (fifths < -5) { ss << "c-"; }
		if (fifths < -6) { ss << "f-"; }
	}
	ss << "]";

	token = new HumdrumToken(ss.str());

	keysig = keysig.next_sibling();
	if (!keysig) {
		return keysig;
	}
	if (nodeType(keysig, "key")) {
		return keysig;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertTimeSigToHumdrum --
//
//  <time symbol="common">
//     <beats>4</beats>
//     <beat-type>4</beat-type>
//
// also:
//  <time symbol="common">
//

xml_node Tool_musicxml2hum::convertTimeSigToHumdrum(xml_node timesig,
		HTp& token, int& staffindex) {

	token = NULL;

	if (!timesig) {
		return xml_node(NULL);
	}

	staffindex = -1;
	xml_attribute sn = timesig.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	int beats = -1;
	int beattype = -1;

	xml_node child = timesig.first_child();
	while (child) {
		if (nodeType(child, "beats")) {
			beats = atoi(child.child_value());
		} else if (nodeType(child, "beat-type")) {
			beattype = atoi(child.child_value());
		}
		child = child.next_sibling();
	}

	if ((beats == -1) && (beattype == -1)) {
		// No time signature, such as:
		// <time print-object="no">
		//   <senza-misura/>
		// </time>
		return xml_node(NULL);
	}

	stringstream ss;
	ss << "*M" << beats<< "/" << beattype;
	token = new HumdrumToken(ss.str());

	timesig = timesig.next_sibling();
	if (!timesig) {
		return timesig;
	}
	if (nodeType(timesig, "time")) {
		return timesig;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertMensurationToHumdrum --
//
//  <time symbol="common">
//     <beats>4</beats>
//     <beat-type>4</beat-type>
//
// also:
//  <time symbol="common">
//

xml_node Tool_musicxml2hum::convertMensurationToHumdrum(xml_node timesig,
		HTp& token, int& staffindex) {

	if (!timesig) {
		return timesig;
	}

	staffindex = -1;
	xml_attribute mens = timesig.attribute("symbol");
	if (!mens) {
		token = new HumdrumToken("*");
	} else {
		string text = mens.value();
		if (text == "cut") {
			token = new HumdrumToken("*met(c|)");
		} else if (text == "common") {
			token = new HumdrumToken("*met(c)");
		} else {
			token = new HumdrumToken("*");
		}
	}

	timesig = timesig.next_sibling();
	if (!timesig) {
		return timesig;
	}
	if (nodeType(timesig, "time")) {
		return timesig;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertClefToHumdrum --
//

xml_node Tool_musicxml2hum::convertClefToHumdrum(xml_node clef,
		HTp& token, int& staffindex) {

	if (!clef) {
		// no clef for some reason.
		return clef;
	}

	staffindex = 0;
	xml_attribute sn = clef.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	string sign;
	int line = -1000;
	int octadjust = 0;

	xml_node child = clef.first_child();
	while (child) {
		if (nodeType(child, "sign")) {
			sign = child.child_value();
		} else if (nodeType(child, "line")) {
			line = atoi(child.child_value());
		} else if (nodeType(child, "clef-octave-change")) {
			octadjust = atoi(child.child_value());
		}
		child = child.next_sibling();
	}

	// Check for percussion clefs, etc., here.
	if (sign == "percussion") {
		sign = "X";
		// ignore line of percussion clef (assume it is centered on staff).
		line = -1000;
	}
	stringstream ss;
	ss << "*clef" << sign;
	if (octadjust < 0) {
		for (int i=0; i < -octadjust; i++) {
			ss << "v";
		}
	} else if (octadjust > 0) {
		for (int i=0; i<octadjust; i++) {
			ss << "^";
		}
	}
	if (line > 0) {
		ss << line;
	}
	token = new HumdrumToken(ss.str());

	clef = clef.next_sibling();
	if (!clef) {
		return clef;
	}
	if (nodeType(clef, "clef")) {
		return clef;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
//	Tool_musicxml2hum::convertOttavaToHumdrum --
//    Example:
//      <direction placement="above">
//        <direction-type>
//          <octave-shift type="down" size="8" number="1"/>
//        </direction-type>
//      </direction>
//      ...
//      <direction placement="above">
//        <direction-type>
//          <octave-shift type="stop" size="8" number="1"/>
//        </direction-type>
//      </direction>
//
//

xml_node Tool_musicxml2hum::convertOttavaToHumdrum(xml_node ottava,
		HTp& token, int& staffindex, int partindex, int partstaffindex, int staffcount) {

	// partstaffindex is useless or incorrect? At least for grand staff parts.
	// The staffindex calculated below is the one to used.

	if (!ottava) {
		// no clef for some reason.
		return ottava;
	}


	// Don't use "number" to set the staff index, now use the input parameter
	// which comes from the direction/staff element that is an uncle of the ottava node.
	//staffindex = 0;
	//xml_attribute sn = ottava.attribute("number");
	//if (sn) {
	//	staffindex = atoi(sn.value()) - 1;
	//}
	//staffindex = staffcount - staffindex - 1;

	int interval = 0;

	interval = ottava.attribute("size").as_int();
	string otype = ottava.attribute("type").as_string();
	string lastotype = m_last_ottava_direction.at(partindex).at(staffindex);

	string ss;
	ss = "*";
	if (otype == "stop") {
		ss += "X";
	} else {
	   m_last_ottava_direction.at(partindex).at(staffindex) = otype;
   }
	if (interval == 15) {
		ss += "15";
		if (otype == "down") {
			ss += "ma";
		} else if (otype == "up") {
			ss += "ba";
		} else if (otype == "stop") {
			if (m_last_ottava_direction.at(partindex).at(staffindex) == "up") {
				ss += "ba";
			} else if (m_last_ottava_direction.at(partindex).at(staffindex) == "down") {
				ss += "ma";
			}
		}
	} else if (interval == 8) {
		ss += "8";
		if (otype == "down") {
			ss += "va";
		} else if (otype == "up") {
			ss += "ba";
		} else if (otype == "stop") {
			if (m_last_ottava_direction.at(partindex).at(staffindex) == "up") {
				ss += "ba";
			} else if (m_last_ottava_direction.at(partindex).at(staffindex) == "down") {
				ss += "va";
			}
		}
	} else {
		ss += "*8";
		if (otype == "down") {
			ss += "va";
		} else if (otype == "up") {
			ss += "ba";
		} else if (otype == "stop") {
			if (m_last_ottava_direction.at(partindex).at(staffindex) == "up") {
				ss += "ba";
			} else if (m_last_ottava_direction.at(partindex).at(staffindex) == "down") {
				ss += "va";
			}
		}
	}
	token = new HumdrumToken(ss);

	ottava = ottava.next_sibling();
	if (!ottava) {
		return ottava;
	}
	if (nodeType(ottava, "octave-shift")) {
		return ottava;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::nodeType -- return true if node type matches
//     string.
//

bool Tool_musicxml2hum::nodeType(xml_node node, const char* testname) {
	if (strcmp(node.name(), testname) == 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::appendNullTokens --
//

void Tool_musicxml2hum::appendNullTokens(HLp line,
		MxmlPart& part) {
	int i;
	int staffcount = part.getStaffCount();
	int versecount = part.getVerseCount();
	for (i=staffcount-1; i>=0; i--) {
		line->appendToken(".");
	}
	for (i=0; i<versecount; i++) {
		line->appendToken(".");
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::getPartContent -- Extract the part elements in
//     the file indexed by part ID.
//

bool Tool_musicxml2hum::getPartContent(
		map<string, xml_node>& partcontent,
		vector<string>& partids, xml_document& doc) {

	auto parts = doc.select_nodes("/score-partwise/part");
	int count = (int)parts.size();
	if (count != (int)partids.size()) {
		cerr << "Warning: part element count does not match part IDs count: "
		     << parts.size() << " compared to " << partids.size() << endl;
	}

	string partid;
	for (int i=0; i<(int)parts.size(); i++) {
		partid = getAttributeValue(parts[i], "id");
		if (partid.size() == 0) {
			cerr << "Warning: Part " << i << " has no ID" << endl;
		}
		auto status = partcontent.insert(make_pair(partid, parts[i].node()));
		if (status.second == false) {
			cerr << "Error: ID " << partids.back()
			     << " is duplicated and secondary part will be ignored" << endl;
		}
		if (find(partids.begin(), partids.end(), partid) == partids.end()) {
			cerr << "Error: Part ID " << partid
			     << " is not present in part-list element list" << endl;
			continue;
		}
	}

	if (partcontent.size() != partids.size()) {
		cerr << "Error: part-list count does not match part count "
		     << partcontent.size() << " compared to " << partids.size() << endl;
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::getPartInfo -- Extract a list of the part ids,
//    and a reverse mapping to the <score-part> element to which is refers.
//
//	   part-list structure:
//        <part-list>
//          <score-part id="P1"/>
//          <score-part id="P2"/>
//          etc.
//        </part-list>
//

bool Tool_musicxml2hum::getPartInfo(map<string, xml_node>& partinfo,
		vector<string>& partids, xml_document& doc) {
	auto scoreparts = doc.select_nodes("/score-partwise/part-list/score-part");
	partids.reserve(scoreparts.size());
	bool output = true;
	for (auto el : scoreparts) {
		partids.emplace_back(getAttributeValue(el.node(), "id"));
		auto status = partinfo.insert(make_pair(partids.back(), el.node()));
		if (status.second == false) {
			cerr << "Error: ID " << partids.back()
			     << " is duplicated and secondary part will be ignored" << endl;
		}
		output &= status.second;
		partinfo[partids.back()] = el.node();
	}
	return output;
}



//////////////////////////////
//
// Tool_musicxml2hum::getChildElementText -- Return the (first)
//    matching child element's text content.
//

string Tool_musicxml2hum::getChildElementText(xml_node root,
		const char* xpath) {
	return root.select_node(xpath).node().child_value();
}

string Tool_musicxml2hum::getChildElementText(xpath_node root,
		const char* xpath) {
	return root.node().select_node(xpath).node().child_value();
}



//////////////////////////////
//
// Tool_musicxml2hum::getAttributeValue -- For an xml_node, return
//     the value for the given attribute name.
//

string Tool_musicxml2hum::getAttributeValue(xml_node xnode,
		const string& target) {
	for (auto at = xnode.first_attribute(); at; at = at.next_attribute()) {
		if (target == at.name()) {
			return at.value();
		}
	}
	return "";
}


string Tool_musicxml2hum::getAttributeValue(xpath_node xnode,
		const string& target) {
	auto node = xnode.node();
	for (auto at = node.first_attribute(); at; at = at.next_attribute()) {
		if (target == at.name()) {
			return at.value();
		}
	}
	return "";
}



//////////////////////////////
//
// Tool_musicxml2hum::printAttributes -- Print list of all attributes
//     for an xml_node.
//

void Tool_musicxml2hum::printAttributes(xml_node node) {
	int counter = 1;
	for (auto at = node.first_attribute(); at; at = at.next_attribute()) {
		cout << "\tattribute " << counter++
		     << "\tname  = " << at.name()
		     << "\tvalue = " << at.value()
		     << endl;
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::getSystemDecoration --
//
// Example:  [1,2]{(3,4)}
//
//  <part-list>
//    <part-group type="start" number="1">
//      <group-symbol>bracket</group-symbol>
//    </part-group>
//
//    <score-part id="P1">
//      <part-name>S A</part-name>
//      <score-instrument id="P1-I1">
//        <instrument-name>Soprano/Alto</instrument-name>
//      </score-instrument>
//      <midi-device id="P1-I1" port="1"></midi-device>
//      <midi-instrument id="P1-I1">
//        <midi-channel>1</midi-channel>
//        <midi-program>53</midi-program>
//        <volume>78.7402</volume>
//        <pan>0</pan>
//      </midi-instrument>
//    </score-part>
//
//    <score-part id="P2">
//      <part-name>T B</part-name>
//      <score-instrument id="P2-I1">
//        <instrument-name>Tenor/Bass</instrument-name>
//      </score-instrument>
//      <midi-device id="P2-I1" port="1"></midi-device>
//      <midi-instrument id="P2-I1">
//        <midi-channel>2</midi-channel>
//        <midi-program>53</midi-program>
//        <volume>78.7402</volume>
//        <pan>0</pan>
//      </midi-instrument>
//    </score-part>
//
//    <part-group type="stop" number="1"/>
//
//    <score-part id="P3">
//      <part-name>Organ</part-name>
//      <part-abbreviation>Org.</part-abbreviation>
//      <score-instrument id="P3-I1">
//        <instrument-name>Pipe Organ</instrument-name>
//      </score-instrument>
//      <midi-device id="P3-I1" port="1"></midi-device>
//      <midi-instrument id="P3-I1">
//        <midi-channel>3</midi-channel>
//        <midi-program>76</midi-program>
//        <volume>78.7402</volume>
//        <pan>0</pan>
//      </midi-instrument>
//    </score-part>
//
//  </part-list>
//

string Tool_musicxml2hum::getSystemDecoration(xml_document& doc, HumGrid& grid,
	vector<string>& partids) {

	xml_node partlist = doc.select_node("/score-partwise/part-list").node();
	if (!partlist) {
		cerr << "Error: cannot find partlist\n";
		return "";
	}
	vector<xml_node> children;
	getChildrenVector(children, partlist);

	vector<vector<int>> staffnumbers;
	int pcount = grid.getPartCount();
	staffnumbers.resize(pcount);

	int scounter = 1;
	for (int i=0; i<pcount; i++) {
		int staffcount = grid.getStaffCount(i);
		for (int j=0; j<staffcount; j++) {
			staffnumbers[i].push_back(scounter++);
		}
	}

	string output;

	// part-group @type=start @number=1
   //   <group-symbol>bracket</group-symbol>
	// score-part
	// score-part
	// part-group @type=stop @number=1
	// score-part
	int pcounter = 0;
	scounter = 1;
	vector<string> typeendings(100);
	for (int i=0; i<(int)children.size(); i++) {
		string name = children[i].name();
		if (name == "part-group") {
			string grouptype = children[i].attribute("type").value();
			string gsymbol = "";
			int number = children[i].attribute("number").as_int();
			if (grouptype == "start") {
				string g = children[i].select_node("//group-symbol").node().child_value();
				if (g == "bracket") {
					output += "[(";
					typeendings[number] = ")]";
				} else if (g == "brace") {
					output += "[(";
					typeendings[number] = ")]";
				} else {
					cerr << "Unknown part grouping symbol: " << g << endl;
				}
			} else if (grouptype == "stop") {
				output += typeendings[number];
				typeendings[number].clear();
			}
		} else if (name == "score-part") {
			pcounter++;
			int staffcount = grid.getStaffCount(pcounter-1);
			if (staffcount == 1) {
				output += "s" + to_string(scounter++);
			} else if (staffcount > 1) {
				output += "{(";
				for (int k=0; k<staffcount; k++) {
					output += "s" + to_string(scounter++);
				}
				output += ")}";
			}
		}
	}

	string newoutput;
	for (int i=0; i<(int)output.size(); i++) {
		if ((i>0) && (output[i] == 's') && isdigit(output[i-1])) {
			newoutput += ',';
		}
		newoutput += output[i];
	}

	return newoutput;
}



//////////////////////////////
//
// Tool_musicxml2hum::getChildrenVector -- Return a list of all children
//   elements of a given element.  Pugixml does not allow random access,
//   but storing them in a vector allows that possibility.
//

void Tool_musicxml2hum::getChildrenVector(vector<xml_node>& children,
		xml_node parent) {
	children.clear();
	for (xml_node child : parent.children()) {
		children.push_back(child);
	}
}


// END_MERGE

} // end namespace hum


