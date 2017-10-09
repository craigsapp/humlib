//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 14:16:18 PDT 2016
// Filename:      musicxml2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/musicxml2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "tool-musicxml2hum.h"
#include "tool-ruthfix.h"
#include "HumGrid.h"
#include "HumRegex.h"

#include <string.h>
#include <stdlib.h>

#include <algorithm>

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
	auto result = doc.load(input);
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
	getPartContent(partcontent, partids, doc);
	vector<MxmlPart> partdata;
	partdata.resize(partids.size());
	fillPartData(partdata, partids, partinfo, partcontent);

	// for debugging:
	//printPartInfo(partids, partinfo, partcontent, partdata);

	// check the voice info
	for (int i=0; i<(int)partdata.size(); i++) {
		partdata[i].prepareVoiceMapping();
		// for debugging:
		if (VoiceDebugQ) {
			partdata[i].printStaffVoiceInfo();
		}
	}

	// re-index voices to disallow empty intermediate voices.
	reindexVoices(partdata);

	HumGrid outdata;
	if (m_recipQ) {
		outdata.enableRecipSpine();
	}
	status &= stitchParts(outdata, partids, partinfo, partcontent, partdata);

	outdata.removeRedundantClefChanges();
	outdata.removeSibeliusIncipit();

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
	for (int p=0; p<(int)partdata.size(); p++) {
		bool dynstate = partdata[p].hasDynamics();
		if (dynstate) {
			outdata.setDynamicsPresent(p);
			break;
		}
	}

	// set the duration of the last slice

	HumdrumFile outfile;

	outdata.transferTokens(outfile);

	addHeaderRecords(outfile, doc);
	addFooterRecords(outfile, doc);

	Tool_ruthfix ruthfix;
	ruthfix.run(outfile);

	for (int i=0; i<outfile.getLineCount(); i++) {
		outfile[i].createLineFromTokens();
	}
	out << outfile;

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

	return status;
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
// Tool_musicxml2hum::cleanSpaces --
//

string Tool_musicxml2hum::cleanSpaces(string& input) {
	for (int i=0; i<(int)input.size(); i++) {
		if (std::isspace(input[i])) {
			input[i] = ' ';
		}
	}
	return input;
}



//////////////////////////////
//
// Tool_musicxml2hum::addHeaderRecords -- Inserted in reverse order
//      (last record inserted first).
//

void Tool_musicxml2hum::addHeaderRecords(HumdrumFile& outfile, xml_document& doc) {
	string xpath;
	HumRegex hre;

	// OTL: title //////////////////////////////////////////////////////////

	// Sibelius method
	xpath = "/score-partwise/work/work-title";
	string worktitle = cleanSpaces(doc.select_single_node(xpath.c_str()).node().child_value());
	bool worktitleQ = false;
	if (worktitle != "") {
		string otl_record = "!!!OTL:\t";
		otl_record += worktitle;
		outfile.insertLine(0, otl_record);
		worktitleQ = true;
	}

	xpath = "/score-partwise/movement-title";
	string mtitle = cleanSpaces(doc.select_single_node(xpath.c_str()).node().child_value());
	if (mtitle != "") {
		string otl_record = "!!!OTL:\t";
		if (worktitleQ) {
			otl_record = "!!!OMV:\t";
		}
		otl_record += mtitle;
		outfile.insertLine(0, otl_record);
	}

	// COM: composer /////////////////////////////////////////////////////////
	// CDT: composer's dates
	xpath = "/score-partwise/identification/creator[@type='composer']";
	string composer = cleanSpaces(doc.select_single_node(xpath.c_str()).node().child_value());
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
					cdt_record = "!!!CDT:\t";
					cdt_record += dates;
				}
			}
		}
	}

	if (cdt_record != "") {
		outfile.insertLine(0, cdt_record);
	}

	if (composer != "") {
		string com_record = "!!!COM:\t";
		com_record += composer;
		outfile.insertLine(0, com_record);
	}

}



//////////////////////////////
//
// Tool_musicxml2hum::addFooterRecords --
//

void Tool_musicxml2hum::addFooterRecords(HumdrumFile& outfile, xml_document& doc) {

	// YEM: copyright
	string copy = doc.select_single_node("/score-partwise/identification/rights").node().child_value();
	bool validcopy = true;
	if (copy == "") {
		validcopy = false;
	}
	if ((copy.find("opyright") != std::string::npos) && (copy.size() < 15)) {
		validcopy = false;
	}

	if (validcopy) {
		string yem_record = "!!!YEM:\t";
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
// stitchParts -- Merge individual parts into a single score sequence.
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

	return status;
}



//////////////////////////////
//
// Tool_musicxml2hum::cleanupMeasures --
//     Also add barlines here (keeping track of the
//     duration of each measure).
//

void Tool_musicxml2hum::cleanupMeasures(HumdrumFile& outfile,
		vector<HumdrumLine*> measures) {

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
	HumdrumLine* line = new HumdrumLine;
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

	HumdrumLine* line = new HumdrumLine;
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

	HumNum tsdur;
	for (i=0; i<(int)curtime.size(); i++) {
		tsdur = measuredata[i]->getTimeSigDur();
		if ((tsdur == 0) && (i > 0)) {
			tsdur = measuredata[i-1]->getTimeSigDur();
			measuredata[i]->setTimeSigDur(tsdur);
		}
		if (VoiceDebugQ) {
			vector<MxmlEvent*>& events = measuredata[i]->getEventList();
			for (int j=0; j<(int)events.size(); j++) {
				cerr << "!!ELEMENT: ";
				cerr << "\tSTi:   " << events[j]->getStaffIndex();
				cerr << "\tVi:    " << events[j]->getVoiceIndex();
				cerr << "\tTS:    " << events[j]->getStartTime();
				cerr << "\tDUR:   " << events[j]->getDuration();
				cerr << "\tPITCH: " << events[j]->getKernPitch();
				cerr << "\tNAME:  " << events[j]->getElementName();
				cerr << endl;
			}
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
	while (!allend) {
		nowevents.resize(0);
		nowparts.resize(0);
		allend = true;
		HumNum processtime = nexttime;
		nexttime = -1;
		for (i = (int)partdata.size()-1; i >= 0; i--) {
			if (curindex[i] >= (int)(*sevents[i]).size()) {
				continue;
			}

			if ((*sevents[i])[curindex[i]].starttime == processtime) {
				auto thing = &(*sevents[i])[curindex[i]];
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

	return status;
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
	outdata->push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)nowevents.size(); i++) {
		vector<MxmlEvent*>& events = nowevents[i]->nonzerodur;
		for (int j=0; j<(int)events.size(); j++) {
			addEvent(slice, outdata, events[j]);
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addEvent -- Add a note or rest.
//

void Tool_musicxml2hum::addEvent(GridSlice* slice, GridMeasure* outdata, MxmlEvent* event) {

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
	bool grace = false;
	bool invisible = false;
	bool primarynote = true;
	bool slurstart = false;
	bool slurstop = false;
	int slurdir = 0;

	if (!event->isFloating()) {
		recip     = event->getRecip();
		// will need to fix for exotic tuplest such as 11%2 or 1%23
		auto loc = recip.find("1%2");
		if (loc != string::npos) {
			recip.replace(loc, 3, "0");
		}
		// will need to fix for exotic tuplest such as 11%4 or 1%42
		loc = recip.find("1%4");
		if (loc != string::npos) {
			recip.replace(loc, 3, "00");
		}
		pitch     = event->getKernPitch();
		prefix    = event->getPrefixNoteInfo();
		postfix   = event->getPostfixNoteInfo(primarynote);
		grace     = event->isGrace();
		slurstart = event->hasSlurStart(slurdir);
		slurstop  = event->hasSlurStop();

		if (slurstart) {
			prefix.insert(0, "(");
			if (slurdir) {
				if (slurdir > 0) {
					prefix.insert(1, ">");
					m_slurabove++;
				} else if (slurdir < 0) {
					prefix.insert(1, "<");
					m_slurbelow++;
				}
			}
		}
		if (slurstop) {
			postfix.push_back(')');
		}

		invisible = isInvisible(event);
		if (event->isInvisible()) {
			invisible = true;
		}

		if (grace) {
			HumNum dur = event->getEmbeddedDuration(event->getNode()) / 4;
			if (dur.getNumerator() == 1) {
				recip = to_string(dur.getDenominator()) + "q";
			} else {
				recip = "q";
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

	int hcount = addHarmony(slice->at(partindex), event);
	if (hcount > 0) {
		event->reportHarmonyCountToOwner(hcount);
	}

	if (m_current_text.size() > 0) {
		event->setTexts(m_current_text);
		m_current_text.clear();
		addTexts(slice, outdata, partindex, staffindex, voiceindex, event);
	}

	if (m_current_dynamic) {
		event->setDynamics(m_current_dynamic);
		string dparam = getDynamicsParameters(m_current_dynamic);
		m_current_dynamic = xml_node(NULL);
		event->reportDynamicToOwner();
		addDynamic(slice->at(partindex), event);
		if (dparam != "") {
			GridMeasure *gm = slice->getMeasure();
			string fullparam = "!LO:DY" + dparam;
			if (gm) {
				gm->addDynamicsLayoutParameters(slice, partindex, fullparam);
			}
		}
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::addTexts -- Add all text direction for a note.
//

void Tool_musicxml2hum::addTexts(GridSlice* slice, GridMeasure* measure, int partindex,
		int staffindex, int voiceindex, MxmlEvent* event) {
	vector<xml_node>& nodes = event->getTexts();
	for (xml_node item : nodes) {
		addText(slice, measure, partindex, staffindex, voiceindex, item);
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

void Tool_musicxml2hum::addText(GridSlice* slice, GridMeasure* measure, int partindex, 
		int staffindex, int voiceindex, xml_node node) {
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
	if (!nodeType(grandchild, "words")) {
		return;
	}
	string text = grandchild.child_value();
	if (text == "") {
		return;
	}

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

	// maybe check for text only containing spaces and exclude here
	string output = "!LO:TX";
	output += placementstring;
	output += stylestring;
	output += ":t=";
	output += text;

	// The text direction needs to be added before the last line in the measure object.
	// If there is already an empty layout slice before the current one (with no spine manipulators
	// in between), then insert onto the existing layout slice; otherwise create a new layout slice.
	measure->addLayoutParameter(slice, partindex, output);

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

void Tool_musicxml2hum::addDynamic(GridPart* part, MxmlEvent* event) {
	xml_node direction = event->getDynamics();
	if (!direction) {
		return;
	}
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
		return;
	}
	if (!nodeType(child, "direction-type")) {
		return;
	}
	xml_node grandchild = child.first_child();
	if (!grandchild) {
		return;
	}

	if (!(nodeType(grandchild, "dynamics") || nodeType(grandchild, "wedge"))) {
		return;
	}

	if (nodeType(grandchild, "dynamics")) {
		xml_node dynamic = grandchild.first_child();
		if (!dynamic) {
			return;
		}
		string dstring = getDynamicString(dynamic);
		HTp dtok = new HumdrumToken(dstring);
		part->setDynamics(dtok);
	} else if (nodeType(grandchild, "wedge")) {
		xml_node hairpin = grandchild;
		if (!hairpin) {
			return;
		}
		string hstring = getHairpinString(hairpin);
		HTp htok = new HumdrumToken(hstring);
		if ((hstring != "[") && (hstring != "]") && above) {
			htok->setValue("LO", "HP", "a", "true");
		}
		part->setDynamics(htok);
	}
}



//////////////////////////////
//
// Tool_musicxml2hum::getDynanmicsParameters --  Already presumed to be a dynamic.
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

string Tool_musicxml2hum::getHairpinString(xml_node element) {
	static char stopchar = '[';
	if (nodeType(element, "wedge")) {
		xml_attribute wtype = element.attribute("type");
		if (!wtype) {
			return "???";
		}
		string output;
		string wstring = wtype.value();
		if (wstring == "diminuendo") {
			stopchar = ']';
			output = '>';
		} else if (wstring == "crescendo") {
			stopchar = '[';
			output = '<';
		} else if (wstring == "stop") {
			output = stopchar;
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
// Tool_musicxml2hum::addHarmony --
//

int Tool_musicxml2hum::addHarmony(GridPart* part, MxmlEvent* event) {
	xml_node hnode = event->getHNode();
	if (!hnode) {
		return 0;
	}

	// fill in X with the harmony values from the <harmony> node
	string hstring = getHarmonyString(hnode);
	HTp htok = new HumdrumToken(hstring);
	part->setHarmony(htok);

	return 1;
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
//   </harmony>
//

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
	string bass;
	int rootalter = 0;
	int bassalter = 0;
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
		}
		child = child.next_sibling();
	}
	stringstream ss;
	ss << root;

	if (rootalter > 0) {
		for (int i=0; i<rootalter; i++) {
			ss << "#";
		}
	} else if (rootalter < 0) {
		for (int i=0; i<-rootalter; i++) {
			ss << "-";
		}
	}

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
					text = cleanSpaces(child.child_value());
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
	bool slurstart = false;
	bool slurstop  = false;
	int  slurdir = 0;

	bool primarynote = false;
	for (int i=0; i<(int)links.size(); i++) {
		note = links.at(i);
		pitch   = note->getKernPitch();
		prefix  = note->getPrefixNoteInfo();
		postfix = note->getPostfixNoteInfo(primarynote);
		slurstart = note->hasSlurStart(slurdir);
		slurstop  = note->hasSlurStop();

		if (slurstart) {
			prefix.insert(0, "(");
			if (slurdir) {
				if (slurdir > 0) {
					prefix.insert(1, ">");
					m_slurabove++;
				} else if (slurdir < 0) {
					prefix.insert(1, "<");
					m_slurbelow++;
				}
			}
		}
		if (slurstop) {
			postfix.push_back(')');
		}

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

	bool hasclef    = false;
	bool haskeysig  = false;
	bool hastimesig = false;

	vector<vector<xml_node> > clefs(partdata.size());
	vector<vector<xml_node> > keysigs(partdata.size());
	vector<vector<xml_node> > timesigs(partdata.size());

	vector<vector<vector<vector<MxmlEvent*> > > > gracebefore(partdata.size());
	vector<vector<vector<vector<MxmlEvent*> > > > graceafter(partdata.size());
	bool foundnongrace = false;

	int pindex = 0;
	xml_node child;
	xml_node grandchild;

	for (int i=0; i<(int)nowevents.size(); i++) {
		for (int j=0; j<(int)nowevents[i]->zerodur.size(); j++) {
			xml_node element = nowevents[i]->zerodur[j]->getNode();

			if (nodeType(element, "attributes")) {
				child = element.first_child();
				while (child) {
					pindex = nowevents[i]->zerodur[j]->getPartIndex();
					if (nodeType(child, "clef")) {
						clefs[pindex].push_back(child);
						hasclef = true;
						foundnongrace = true;
					}

					if (nodeType(child, "key")) {
						keysigs[pindex].push_back(child);
						haskeysig = true;
						foundnongrace = true;
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
				child = element.first_child();
				if (nodeType(child, "direction-type")) {
					grandchild = child.first_child();
					if (nodeType(grandchild, "words")) {
						m_current_text.push_back(element);
					} else if (nodeType(grandchild, "dynamics")) {
						m_current_dynamic = element;
					} else if (nodeType(grandchild, "wedge")) {
						m_current_dynamic = element;
					}
				}

			} else if (nodeType(element, "note")) {
				if (foundnongrace) {
					addEventToList(graceafter, nowevents[i]->zerodur[j]);
				} else {
					addEventToList(gracebefore, nowevents[i]->zerodur[j]);
				}
			}
		}
	}

	addGraceLines(outdata, gracebefore, partdata, nowtime);

	if (hasclef) {
		addClefLine(outdata, clefs, partdata, nowtime);
	}

	if (haskeysig) {
		addKeySigLine(outdata, keysigs, partdata, nowtime);
	}

	if (hastimesig) {
		addTimeSigLine(outdata, timesigs, partdata, nowtime);
	}

	addGraceLines(outdata, graceafter, partdata, nowtime);
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
					addEvent(slices.at(startm+m), outdata, notes[i][j][k][m]);
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

	HTp token;
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
//	Tool_musicxml2hum::convertKeySigToHumdrum --
//
//  <key>
//     <fifths>4</fifths>
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

	xml_node child = keysig.first_child();
	while (child) {
		if (nodeType(child, "fifths")) {
			fifths = atoi(child.child_value());
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

	if (!timesig) {
		return timesig;
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
	int line = 0;
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
	ss << line;
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

void Tool_musicxml2hum::appendNullTokens(HumdrumLine* line,
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
	return root.select_single_node(xpath).node().child_value();
}

string Tool_musicxml2hum::getChildElementText(xpath_node root,
		const char* xpath) {
	return root.node().select_single_node(xpath).node().child_value();
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


// END_MERGE

} // end namespace hum


