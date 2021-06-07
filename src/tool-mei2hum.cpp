//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:58:26 PDT 2017
// Last Modified: Tue Mar  9 22:03:56 PST 2021
// Filename:      mei2hum.cpp
// URL:           https://github.com/craigsapp/mei2hum/blob/master/src/mei2hum.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert an MEI file into a Humdrum file.
//

#include "tool-mei2hum.h"
#include "HumGrid.h"
#include "HumRegex.h"
#include "Convert.h"

#include <string.h>
#include <stdlib.h>

#include <algorithm>
#include <cctype>

using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE


#define QUARTER_CONVERT * 4
#define ELEMENT_DEBUG_STATEMENT(X)
//#define ELEMENT_DEBUG_STATEMENT(X)  cerr << #X << endl;

#define NODE_VERIFY(ELEMENT, RETURNVALUE)        \
	if (!ELEMENT) {                               \
		return RETURNVALUE;                        \
	}                                             \
	if (strcmp(ELEMENT.name(), #ELEMENT) != 0) {  \
		return RETURNVALUE;                        \
	}                                             \
	ELEMENT_DEBUG_STATEMENT(ELEMENT)

#define MAKE_CHILD_LIST(VARNAME, ELEMENT)        \
	vector<xml_node> VARNAME;                     \
	getChildrenVector(VARNAME, ELEMENT);


#define DKHTP "Don't know how to process "

#define CURRLOC " in measure " << m_currentMeasure


//////////////////////////////
//
// Tool_mei2hum::Tool_mei2hum --
//

Tool_mei2hum::Tool_mei2hum(void) {
	define("app|app-label=s", "app label to follow");
	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");
	define("x|xmlids=b", "include xmlids in output");
	define("P|no-place=b", "Do not convert placement attribute");

	m_maxverse.resize(m_maxstaff);
	fill(m_maxverse.begin(), m_maxverse.end(), 0);

	m_measureDuration.resize(m_maxstaff);
	fill(m_measureDuration.begin(), m_measureDuration.end(), 0);

	m_currentMeterUnit.resize(m_maxstaff);
	fill(m_currentMeterUnit.begin(), m_currentMeterUnit.end(), 4);

	m_hasDynamics.resize(m_maxstaff);
	fill(m_hasDynamics.begin(), m_hasDynamics.end(), false);

	m_hasXmlids.resize(m_maxstaff);
	fill(m_hasXmlids.begin(), m_hasXmlids.end(), false);

	m_hasHarm.resize(m_maxstaff);
	fill(m_hasHarm.begin(), m_hasHarm.end(), false);
}



//////////////////////////////
//
// Tool_mei2hum::convert -- Convert an MEI file into
//     Humdrum content.
//

bool Tool_mei2hum::convertFile(ostream& out, const char* filename) {
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


bool Tool_mei2hum::convert(ostream& out, istream& input) {
	string s(istreambuf_iterator<char>(input), {});
	return convert(out, s.c_str());
}


bool Tool_mei2hum::convert(ostream& out, const char* input) {
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



bool Tool_mei2hum::convert(ostream& out, xml_document& doc) {
	initialize();

	bool status = true; // for keeping track of problems in conversion process.

	buildIdLinkMap(doc);

	auto score = doc.select_node("/mei/music/body/mdiv/score").node();

	if (!score) {
		cerr << "Cannot find score, so cannot convert MEI file to Humdrum";
		cerr << endl;
		cerr << "Perhaps there is a problem in the XML structure of the file.";
		cerr << endl;
		return false;
	}

	m_staffcount = extractStaffCountByFirstMeasure(score);
	if (m_staffcount == 0) {
		// probably mensural music
		m_staffcount = extractStaffCountByScoreDef(score);
		if (m_staffcount == 0) {
			cerr << "error: no music detected in <score>" << endl;
		}
	}

	if (m_recipQ) {
		m_outdata.enableRecipSpine();
	}

	HumNum systemstamp = 0;  // timestamp for music.
	systemstamp = parseScore(score, systemstamp);

	m_outdata.removeRedundantClefChanges();

	processHairpins();

	// set the duration of the last slice

	HumdrumFile outfile;

	// Report verse counts for each staff to HumGrid:
	for (int i=0; i<(int)m_maxverse.size(); i++) {
		if (m_maxverse[i] == 0) {
			continue;
		}
		m_outdata.setVerseCount(i, 0, m_maxverse[i]);
	}

	// Report dynamic presence for each staff to HumGrid:
	for (int i=0; i<(int)m_hasDynamics.size(); i++) {
		if (m_hasDynamics[i] == false) {
			continue;
		}
		m_outdata.setDynamicsPresent(i);
	}

	// Report <harm> presence for each staff to HumGrid:
	for (int i=0; i<(int)m_hasHarm.size(); i++) {
		if (m_hasHarm[i] == false) {
			continue;
		}
		m_outdata.setHarmonyPresent(i);
	}

	// Report xmlid presence for each staff to HumGrid:
	for (int i=0; i<(int)m_hasXmlids.size(); i++) {
		if (m_hasXmlids[i] == false) {
			continue;
		}
		m_outdata.setXmlidsPresent(i);
	}

	auto measure = doc.select_node("/mei/music/body/mdiv/score/section/measure").node();
	auto number = measure.attribute("n");
	int measurenumber = 0;

	if (number) {
		measurenumber = number.as_int();
	} else {
		measurenumber = 0;
	}

	string interp = "**kern";
	if (m_mensuralQ) {
		interp = "**mens";
	}
	if (measurenumber > 1) {
		m_outdata.transferTokens(outfile, measurenumber, interp);
	} else {
		m_outdata.transferTokens(outfile, 0, interp);
	}

	addHeaderRecords(outfile, doc);
	addExtMetaRecords(outfile, doc);
	addFooterRecords(outfile, doc);

	for (int i=0; i<outfile.getLineCount(); i++) {
		outfile[i].createLineFromTokens();
	}
	out << outfile;

	return status;
}



//////////////////////////////
//
// Tool_mei2hum::processHairpins --
//

void Tool_mei2hum::processHairpins(void) {
	for (int i=0; i<(int)m_hairpins.size(); i++) {
		processHairpin(m_hairpins[i]);
	}
}



//////////////////////////////
//
// Tool_mei2hum::processHairpin -- Currently does not create lines to refine position of hairpin.
//
//    <hairpin tstamp="1" form="dim" place="below" staff="8" opening="0" endto="4" tstamp2="0m+4.667"/>
//

void Tool_mei2hum::processHairpin(hairpin_info& info) {
	xml_node hairpin = info.hairpin;
	GridMeasure *gm = info.gm;
	int mindex = info.mindex;

	string tstamp  = hairpin.attribute("tstamp").value();
	string tstamp2 = hairpin.attribute("tstamp2").value();
	string form    = hairpin.attribute("form").value();
	string staff   = hairpin.attribute("staff").value();
	if (staff == "") {
		cerr << "Error: hairpin requires a staff number" << endl;
		return;
	}

	auto myit = m_outdata.begin();
	while (myit != m_outdata.end()) {
		if (*myit == gm) {
			break;
		}
		myit++;
	}

	int staffnum = stoi(staff);
	string hairopen = "<";
	string hairclose = "[";
	if (form == "dim") {
		hairopen = ">";
		hairclose = "]";
	}
	double starttime = stod(tstamp) - 1.0;
	double measure = 0.0;
	auto loc = tstamp2.find("m+");
	if (loc != string::npos) {
		string mnum = tstamp2.substr(0, loc);
		measure = stod(mnum);
		tstamp2 = tstamp2.substr(loc+2, string::npos);
	}
	double endtime = stod(tstamp2) - 1;

	HumNum measurestart = gm->getTimestamp();
	HumNum timestamp;   // timestamp of event in measure
	HumNum mtimestamp;  // starttime of the measure
	double threshold = 0.001;
	auto it = gm->begin();
	GridSlice *lastgs = NULL;
	// bool found = false;

	while (it != gm->end()) {
		if (!(*it)->isDataSlice()) {
			it++;
			continue;
		}
		timestamp = (*it)->getTimestamp();
		mtimestamp = (timestamp - measurestart) * 4;
      mtimestamp /= m_currentMeterUnit[mindex];
		double diff = starttime - mtimestamp.getFloat();
		if (diff < threshold) {
			// found = true;
			lastgs = *it;
			break;
		} else if (diff < 0.0) {
			// found = true;
			lastgs = *it;
			break;
		}
		lastgs = *it;
		it++;
	}

	if (lastgs) {
		GridPart* part = lastgs->at(staffnum-1);
		part->setDynamics(hairopen);
		m_outdata.setDynamicsPresent(staffnum-1);
	}

	myit += (int)measure;
	mindex += (int)measure;
	gm = *myit;
	it = gm->begin();
	lastgs = NULL;
	// found = false;
	while (it != gm->end()) {
		if (!(*it)->isDataSlice()) {
			it++;
			continue;
		}
		timestamp = (*it)->getTimestamp();
		mtimestamp = (timestamp - measurestart) * 4;
		mtimestamp /=  m_currentMeterUnit[mindex];
		double diff = endtime - mtimestamp.getFloat();
		if (diff < threshold) {
			// found = true;
			lastgs = *it;
			break;
		} else if (diff < 0.0) {
			// found = true;
			lastgs = *it;
			break;
		}
		lastgs = *it;
		it++;
	}
	if (lastgs) {
		GridPart* part = lastgs->at(staffnum-1);
		part->setDynamics(hairclose);
		m_outdata.setDynamicsPresent(staffnum-1);
	}

// ggg
}



//////////////////////////////
//
// Tool_mei2hum::addExtMetaRecords --
//

void Tool_mei2hum::addExtMetaRecords(HumdrumFile& outfile, xml_document& doc) {
	pugi::xpath_node_set metaframes = doc.select_nodes("/mei/meiHead/extMeta/frames/metaFrame");
	double starttime;
	string starttimevalue;
	string token;
	xml_node node;
	xml_node timenode;

	// place header reference records, assumed to be time sorted
	for (int i=(int)metaframes.size()-1; i>=0; i--) {
		node = metaframes[i].node();
		timenode = node.select_node("./frameInfo/startTime").node();
		starttimevalue = timenode.attribute("float").value();
		if (starttimevalue == "") {
			starttime = 0.0;
		} else {
			starttime = stof(starttimevalue);
		}
		if (starttime > 0.0) {
			continue;
		}
		token = node.attribute("token").value();
		if (token.empty()) {
			continue;
		}
		outfile.insertLine(0, token);
		if (token.find("!!!RDF**kern: < = below") != string::npos) {
			m_belowQ = false;
		}
		if (token.find("!!!RDF**kern: > = above") != string::npos) {
			m_aboveQ = false;
		}
	}

	// place footer reference records, assumed to be time sorted
	for (int i=0; i<(int)metaframes.size(); i++) {
		node = metaframes[i].node();
		timenode = node.select_node("./frameInfo/startTime").node();
		starttimevalue = timenode.attribute("float").value();
		if (starttimevalue == "") {
			starttime = 0.0;
		} else {
			starttime = stof(starttimevalue);
		}
		if (starttime == 0.0) {
			continue;
		}
		token = node.attribute("token").value();
		if (token.empty()) {
			continue;
		}
		outfile.appendLine(token);
		if (token.find("!!!RDF**kern: < = below") != string::npos) {
			m_belowQ = false;
		}
		if (token.find("!!!RDF**kern: > = above") != string::npos) {
			m_aboveQ = false;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::addHeaderRecords --
//

void Tool_mei2hum::addHeaderRecords(HumdrumFile& outfile, xml_document& doc) {

	// title is at /mei/meiHead/fileDesc/titleStmt/title
	string title = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/title").node().child_value());

	// composer is at /mei/meiHead/fileDesc/titleStmt/respStmt/persName@role="creator"
	string composer = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/respStmt/persName[@role='creator']").node().child_value());

	// lyricist is at /mei/meiHead/fileDesc/titleStmt/respStmt/persName@role="lyricist"
	string lyricist = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/respStmt/persName[@role='lyricist']").node().child_value());

	if (!m_systemDecoration.empty()) {
		outfile.insertLine(0, "!!!system-decoration: " + m_systemDecoration);
	}

	if (!title.empty()) {
		outfile.insertLine(0, "!!!OTL: " + title);
	}
	if (!lyricist.empty()) {
		outfile.insertLine(0, "!!!LYR: " + lyricist);
	}
	if (!composer.empty()) {
		outfile.insertLine(0, "!!!COM: " + composer);
	}

}



//////////////////////////////
//
// Tool_mei2hum::addFooterRecords --
//

void Tool_mei2hum::addFooterRecords(HumdrumFile& outfile, xml_document& doc) {
	if (m_aboveQ) {
		outfile.appendLine("!!!RDF**kern: > = above");
	}
	if (m_belowQ) {
		outfile.appendLine("!!!RDF**kern: < = below");
	}
	if (m_editorialAccidentalQ) {
		outfile.appendLine("!!!RDF**kern: i = editorial accidental");
	}
}



//////////////////////////////
//
// Tool_mei2hum::extractStaffCountByFirstMeasure -- Count the number of staves
//    in the score by looking at the first measure of the score.  Input is the
//    <score> element.
//

int Tool_mei2hum::extractStaffCountByFirstMeasure(xml_node element) {
	auto measure = element.select_node("//measure").node();
	if (!measure) {
		return 0;
	}

	int count = 0;
	for (xml_node child : measure.children()) {
		string nodename = child.name();
		if (nodename == "staff") {
			count++;
		}
	}
	return count;
}



//////////////////////////////
//
// Tool_mei2hum::extractStaffCountByScoreDef -- Count the number of staves
//    in the score by counting <staffDef> entries in the first <scoreDef>.
//     Input is the <score> element.
//

int Tool_mei2hum::extractStaffCountByScoreDef(xml_node element) {
	xml_node scoredef = element.select_node("//scoreDef").node();
	if (!scoredef) {
		return 0;
	}

	pugi::xpath_node_set staffdefs = element.select_nodes(".//staffDef");
	return (int)staffdefs.size();
}



///////////////////////////////////
//
// Tool_mei2hum::parseScore -- Convert an MEI <score> element into Humdrum data.
//

HumNum Tool_mei2hum::parseScore(xml_node score, HumNum starttime) {
	NODE_VERIFY(score, starttime)
	MAKE_CHILD_LIST(children, score);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "scoreDef") {
			parseScoreDef(item, starttime);
		} else if (nodename == "section") {
			starttime = parseSection(item, starttime);
		} else {
			cerr << DKHTP << score.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseScoreDef -- Process a <scoreDef> element in an MEI file.
//

void Tool_mei2hum::parseScoreDef(xml_node scoreDef, HumNum starttime) {
	NODE_VERIFY(scoreDef, )
	MAKE_CHILD_LIST(children, scoreDef);

	if (m_scoreDef.global.timestamp == starttime QUARTER_CONVERT) {
		m_scoreDef.clear();
	}
	m_scoreDef.global.timestamp = starttime QUARTER_CONVERT;

	fillWithStaffDefAttributes(m_scoreDef.global, scoreDef);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    parseStaffGrp(item, starttime);
		} else if (nodename == "staffDef") {
		    parseStaffDef(item, starttime);
		} else if (nodename == "pgHead") {
		    processPgHead(item, starttime);
		} else if (nodename == "pgFoot") {
		   processPgFoot(item, starttime);
		} else if (nodename == "keySig") { // drizo
			processKeySig(m_scoreDef.global, item, starttime); // drizo
		} else {
			cerr << DKHTP << scoreDef.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (!children.empty()) {
		string deco = prepareSystemDecoration(scoreDef);
		if (!deco.empty()) {
			m_systemDecoration = prepareSystemDecoration(scoreDef);
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::prepareSystemDecoration --
//

string Tool_mei2hum::prepareSystemDecoration(xml_node scoreDef) {
	NODE_VERIFY(scoreDef, "")
	MAKE_CHILD_LIST(children, scoreDef);

	if (children.empty()) {
		return "";
	}

	string output;
	for (int i=0; i<(int)children.size(); i++) {
		getRecursiveSDString(output, children[i]);
	}
	string newoutput;
	int counter = 0;
	for (int i=0; i<(int)output.size(); i++) {
		newoutput += output[i];
		if (i < (int)output.size() - 1) {
			if (std::isdigit(output[i]) && (output[i+1] == 's')) {
				newoutput += ',';
				counter++;
			}
		}
	}
	if (counter <= 1) {
		return "";
	} else {
		return newoutput;
	}
}



//////////////////////////////
//
// Tool_mei2hum::getRecursiveSDString --
//    <scoreDef key.sig="0" key.mode="minor" meter.count="4" meter.unit="4" meter.sym="common">
//       <staffGrp n="1" barthru="false" symbol="bracket">
//          <staffDef n="1" lines="5" clef.line="2" clef.shape="G"/>
//          <staffDef n="2" lines="5" clef.line="4" clef.shape="F"/>
//       </staffGrp>
//    </scoreDef>
//

void Tool_mei2hum::getRecursiveSDString(string& output, xml_node current) {
	string name = current.name();

	if (name == "staffDef") {
		xml_attribute natt = current.attribute("n");
		if (!natt) {
			cerr << "Error: unknown staff number for staffDef" << endl;
			return;
		}
		int n = natt.as_int();
		if (n < 1) {
			cerr << "Staff number " << n << " must be positive" << endl;
			return;
		}
		output += "s" + to_string(n);
		return;
	} else if (name == "staffGrp") {
		vector<xml_node> children;
		getChildrenVector(children, current);
		if (children.empty()) {
			// strange: no children in a staffGrp...
			return;
		}

		bool barthru = true;
		xml_attribute barthruatt = current.attribute("barthru");
		if (barthruatt) {
			string value = barthruatt.value();
			if (value == "false")  {
				barthru = false;
			}
		}
		string prestring = "";
		string poststring = "";
		xml_attribute symbolattr = current.attribute("symbol");
		if (symbolattr) {
			string value = symbolattr.value();
			if (value == "bracket") {
				prestring = "[";
				poststring = "]";
			} else if (value == "brace") {
				prestring = "{";
				poststring = "}";
			}
		}
		if (barthru) {
			prestring += "(";
			poststring.insert(0, ")");
		}
		output += prestring;
		for (int i=0; i<(int)children.size(); i++) {
			getRecursiveSDString(output, children[i]);
		}
		output += poststring;
	} else if (name == "pgHead") {
		return;
	} else if (name == "pgFoot") {
		return;
	} else if (name == "keySig") { // drizo
		return;
	} else {
		cerr << "Unknown element in scoreDef descendant: " << name << endl;
	}
}



//////////////////////////////
//
// Tool_mei2hum::processPgFoot -- Dummy function since scoreDef/pgFoot is currently ignored.
//

void Tool_mei2hum::processPgFoot(xml_node pgFoot, HumNum starttime) {
	NODE_VERIFY(pgFoot, )
	return;
}



//////////////////////////////
//
// Tool_mei2hum::processPgHead -- Dummy function since scoreDef/pgHead is currently ignored.
//

void Tool_mei2hum::processPgHead(xml_node pgHead, HumNum starttime) {
	NODE_VERIFY(pgHead, )
	return;
}
 


//////////////////////////////
//
// Tool_mei2hum::processKeySig -- Convert MEI key signature to Humdrum.
//

void Tool_mei2hum::processKeySig(mei_staffDef& staffinfo, xml_node keysig, HumNum starttime) {
	MAKE_CHILD_LIST(children, keysig);
	string token = "*k[";
	for (xml_node item : children) {
		string pname = item.attribute("pname").value(); 
		string accid = item.attribute("accid").value();
		if (pname.empty()) {
			continue;
		}
		token += pname;
		if (accid == "s") {
			token += "#";
		} else if (accid == "f") {
			token += "-";
		} else if (accid.empty() || accid == "n") {
			token += "n";
		} else if (accid == "ss") {
			token += "##";
		} else if (accid == "x") {
			token += "##";
		} else if (accid == "ff") {
			token += "--";
		} else {
			token += "?";
		}
	}
	token += "]";

	staffinfo.keysig = token;
}



//////////////////////////////
//
// Tool_mei2hum::parseStaffGrp -- Process a <staffGrp> element in an MEI file.
//

void Tool_mei2hum::parseStaffGrp(xml_node staffGrp, HumNum starttime) {
	NODE_VERIFY(staffGrp, )
	MAKE_CHILD_LIST(children, staffGrp);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    parseStaffGrp(item, starttime);
		} else if (nodename == "staffDef") {
		    parseStaffDef(item, starttime);
		} else {
			cerr << DKHTP << staffGrp.name() << "/" << nodename << CURRLOC << endl;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseStaffDef -- Process a <staffDef> element in an MEI file.
//
// Also see instrument abbreviations:
//     https://www.loc.gov/standards/valuelist/marcmusperf.html
//
// Stored in mei/meiHead/workDesc/work/perfMedium/perfResList/perfRes@codeval:
//
// <mei>
//     <meiHead>
//         <workDesc>
//             <work>
//                <perfMedium>
//                    <perfResList>
//                        <perfRes codedval="wa">Flute</perfRes>
//                        <perfRes codedval="wb">Oboe 1</perfRes>
//                        <perfRes codedval="wb">Oboe 2</perfRes>
//                        <perfRes codedval="wc">Clarinet in B flat 1</perfRes>
//                        <perfRes codedval="wc">Clarinet in B flat 2</perfRes>
//                        <perfRes codedval="bc">Horn in F 1</perfRes>
//                        <perfRes codedval="bc">Horn F 2</perfRes>
//                        <perfRes codedval="bb" solo="true">Solo Trumpet in C</perfRes>
//                        <perfRes codedval="pa">Timpani</perfRes>
//                        <perfRes codedval="sa">Violin I</perfRes>
//                        <perfRes codedval="sa">Violin II</perfRes>
//                        <perfRes codedval="sb">Viola</perfRes>
//                        <perfRes codedval="sc">Violoncello</perfRes>
//                        <perfRes codedval="sd">Contrabass</perfRes>
//                    </perfResList>
//                </perfMedium>
//

void Tool_mei2hum::parseStaffDef(xml_node staffDef, HumNum starttime) {
	NODE_VERIFY(staffDef, )

	string staffnum = staffDef.attribute("n").value();
	if (staffnum.empty()) {
		// no staffDef@n so cannot process.
		return;
	}

	int num = stoi(staffnum);
	if (num < 1) {
		// to small
		return;
	}
	if (num > 1000) {
		// too large
		return;
	}

	m_scoreDef.minresize(num);
	m_scoreDef.staves.at(num-1).clear();
	m_scoreDef.staves.at(num-1) = m_scoreDef.global;

	fillWithStaffDefAttributes(m_scoreDef.staves.at(num-1), staffDef);

	// see leaky memory note below for why there are separate
	// variables for clef, keysig, etc.
	mei_staffDef& staffdef = m_scoreDef.staves.at(num-1);
	string clef         = staffdef.clef;
	string keysig       = staffdef.keysig;
	string timesig      = staffdef.timesig;
	string midibpm      = staffdef.midibpm;
	string transpose    = staffdef.transpose;
	string label        = staffdef.label;
	string labelabbr    = staffdef.labelabbr;
	int maximodus       = staffdef.maximodus;
	int modus           = staffdef.modus;
	int tempus          = staffdef.tempus;
	int prolatio        = staffdef.prolatio;
	int hasMensuration  = maximodus | modus | tempus | prolatio;

	// Incorporate label into HumGrid:
	if (label.empty()) {
		label = m_scoreDef.global.label;
	}
	if (!label.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		// m_scoreDef.staves[num-1].keysig disappears after this line, so some
		// leaky memory is likey to happen here.
		m_outdata.back()->addLabelToken(label, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount, m_staffcount);
	}

	// Incorporate labelabbr into HumGrid:
	if (labelabbr.empty()) {
		labelabbr = m_scoreDef.global.labelabbr;
	}
	if (!labelabbr.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		// m_scoreDef.staves[num-1].keysig disappears after this line, so some
		// leaky memory is likey to happen here.
		m_outdata.back()->addLabelAbbrToken(labelabbr, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount, m_staffcount);
	}

	// Incorporate clef into HumGrid:
	if (clef.empty()) {
		clef = m_scoreDef.global.clef;
	}
	if (!clef.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		// m_scoreDef.staves[num-1].keysig disappears after this line, so some
		// leaky memory is likey to happen here.
		m_outdata.back()->addClefToken(clef, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate transposition into HumGrid:
	if (transpose.empty()) {
		transpose = m_scoreDef.global.transpose;
	}
	if (!transpose.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addTransposeToken(transpose, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate key signature into HumGrid:
	if (keysig.empty()) {
		keysig = m_scoreDef.global.keysig;
	}
	if (!keysig.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addKeySigToken(keysig, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate time signature into HumGrid:
	if (timesig.empty()) {
		timesig = m_scoreDef.global.timesig;
	}
	if (!timesig.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addTimeSigToken(timesig, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Add metric signature/mensuration sign here.

	if (hasMensuration) {
		if (m_outdata.empty()) {
			m_outdata.addMeasureToBack();
		}
		string metsigtok = "*";
		metsigtok += "met()_";
		metsigtok += to_string(maximodus);
		metsigtok += to_string(modus);
		metsigtok += to_string(tempus);
		metsigtok += to_string(prolatio);

		m_outdata.back()->addMeterSigToken(metsigtok, starttime QUARTER_CONVERT,
		      num-1, 0, 0, m_staffcount);
	}

	// Incorporate tempo into HumGrid:
	if (midibpm.empty()) {
		midibpm = m_scoreDef.global.midibpm;
	}
	if (!midibpm.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addTempoToken(midibpm, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

}



//////////////////////////////
//
// Tool_mei2hum::fillWithStaffDefAttributes --
//

void Tool_mei2hum::fillWithStaffDefAttributes(mei_staffDef& staffinfo, xml_node element) {

	string clefshape;
	string clefline;
	string clefdis;
	string clefdisplace;
	string metercount;
	string meterunit;
	string staffnum;
	string keysig;
	string midibpm;
	string label;
	string labelabbr;
	string notationtype;
	int maximodus = 0;
	int modus     = 0;
	int tempus    = 0;
	int prolatio  = 0;
	int transsemi = 0;
	int transdiat = 0;

	string nodename = element.name();

	int nnum = 0;  // For staffnumber of element is staffDef.

	for (auto atti = element.attributes_begin(); atti != element.attributes_end();
				atti++) {
		string attname = atti->name();
		if (attname == "clef.shape") {
			clefshape = atti->value();
		} else if (attname == "clef.line") {
			clefline = atti->value();
		} else if (attname == "clef.dis") {
			clefdis = atti->value();
		} else if (attname == "clef.displace") {
			clefdisplace = atti->value();
		} else if (attname == "meter.count") {
			metercount = atti->value();
		} else if (attname == "meter.unit") {
			meterunit = atti->value();
		} else if (attname == "key.sig") {
			keysig = atti->value();
		} else if (attname == "label") {
			label = atti->value();
		} else if (attname == "label.abbr") {
			labelabbr = atti->value();
		} else if (attname == "midi.bpm") {
			midibpm = atti->value();
		} else if (attname == "trans.semi") {
			transsemi = atti->as_int();
		} else if (attname == "trans.diat") {
			transdiat = atti->as_int();
		} else if (attname == "notationtype") {
			notationtype = atti->value();
		} else if (attname == "prolatio") {
			prolatio = atti->as_int();
		} else if (attname == "tempus") {
			tempus = atti->as_int();
		} else if (attname == "modusminor") {
			modus = atti->as_int();
		} else if (attname == "modusmaior") {
			maximodus = atti->as_int();
		} else if (attname == "n") {
			nnum = atoi(atti->value());
		}
	}
	if (nnum < 1) {
		nnum = 1;
	}

	if ((transsemi != 0) || (transdiat != 0)) {
		// Fix octave transposition problems:
		if ((transsemi ==  12) && (transdiat ==  0)) { transdiat =   7; }
		if ((transsemi == -12) && (transdiat ==  0)) { transdiat =  -7; }
		if ((transsemi ==   0) && (transdiat ==  7)) { transsemi =  12; }
		if ((transsemi ==   0) && (transdiat == -7)) { transsemi = -12; }
		// transposition needed to get to transposed score:
		staffinfo.transpose = "*ITrd" + to_string(-transdiat) + "c" + to_string(-transsemi);
		// transposition needed to get to C score:
		staffinfo.base40 = -Convert::transToBase40(staffinfo.transpose);
	}

	if ((!clefshape.empty()) && (!clefline.empty())) {
		staffinfo.clef = makeHumdrumClef(clefshape, clefline, clefdis, clefdisplace);
	}
	if ((!metercount.empty()) && (!meterunit.empty())) {
		HumNum meterduration = stoi(metercount) * 4 / stoi(meterunit);
		if (nodename == "scoreDef") {
			for (int i=0; i<(int)m_measureDuration.size(); i++) {
				m_measureDuration.at(i) = meterduration;
				m_currentMeterUnit.at(i) = stoi(meterunit);
			}
		} else if (nodename == "staffDef") {
			if (nnum > 0) {
				m_measureDuration.at(nnum-1) = meterduration;
				m_currentMeterUnit.at(nnum-1) = stoi(meterunit);
			}
		} else {
			cerr << DKHTP << element.name() << "@meter.count/@meter.unit" << CURRLOC << endl;
		}

		staffinfo.timesig = "*M" + metercount + "/" + meterunit;
	}
	if (!keysig.empty()) {
		int count = stoi(keysig);
		int accid = 0;
		if (keysig.find("s") != string::npos) {
			accid = +1;
		} else if (keysig.find("f") != string::npos) {
			accid = -1;
		}


		// Also check for enharmonic transpositions...
		int adjust = 0;
		if (staffinfo.base40 != 0) {
			adjust = Convert::base40IntervalToLineOfFifths(staffinfo.base40);
		}
		count += adjust;

		// Adjust for transposition to C score here.

		if (accid > 0) {
			switch (count) {
				case 1: staffinfo.keysig = "*k[f#]";             break;
				case 2: staffinfo.keysig = "*k[f#c#]";           break;
				case 3: staffinfo.keysig = "*k[f#c#g#]";         break;
				case 4: staffinfo.keysig = "*k[f#c#g#d#]";       break;
				case 5: staffinfo.keysig = "*k[f#c#g#d#a#]";     break;
				case 6: staffinfo.keysig = "*k[f#c#g#d#a#e#]";   break;
				case 7: staffinfo.keysig = "*k[f#c#g#d#a#e#b#]"; break;
			}
		} else if (accid < 0) {
			switch (count) {
				case 1: staffinfo.keysig = "*k[b-]";             break;
				case 2: staffinfo.keysig = "*k[b-e-]";           break;
				case 3: staffinfo.keysig = "*k[b-e-a-]";         break;
				case 4: staffinfo.keysig = "*k[b-e-a-d-]";       break;
				case 5: staffinfo.keysig = "*k[b-e-a-d-g-]";     break;
				case 6: staffinfo.keysig = "*k[b-e-a-d-g-c-]";   break;
				case 7: staffinfo.keysig = "*k[b-e-a-d-g-c-f-]"; break;
			}
		}
	}
	if (!midibpm.empty()) {
		staffinfo.midibpm = "*MM" + midibpm;
	}
	if (!label.empty()) {
		staffinfo.label = "*I\"" + label;
	}
	if (!labelabbr.empty()) {
		staffinfo.labelabbr = "*I'" + labelabbr;
	}
	if (notationtype.empty()) {
		staffinfo.mensural = false;
		staffinfo.black = false;
	} else {
		if (notationtype == "mensural") {
			staffinfo.mensural = true;
			staffinfo.black = false;
		} else if (notationtype == "mensural.white") {
			staffinfo.mensural = true;
			staffinfo.black = false;
		} else if (notationtype == "mensural.black") {
			staffinfo.mensural = true;
			staffinfo.black = true;
		}
		if (staffinfo.mensural) {
			m_mensuralQ = true; // used to print **mens later
			if (maximodus > 0) { staffinfo.maximodus = maximodus; }
			if (modus > 0)     { staffinfo.modus = modus; }
			if (tempus > 0)    { staffinfo.tempus = tempus; }
			if (prolatio > 0)  { staffinfo.prolatio = prolatio; }
		}
		// deal with mensuration sign, or calculate from maximodus/modus/tempus/prolatio
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseSection -- Process a <section> element in an MEI file.
//

HumNum Tool_mei2hum::parseSection(xml_node section, HumNum starttime) {
	NODE_VERIFY(section, starttime);
	MAKE_CHILD_LIST(children, section);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else if (nodename == "app") {
			starttime = parseApp(children[i], starttime);
		} else if (nodename == "sb") {   // system break
			parseSb(children[i], starttime);
		} else if (nodename == "pb") {   // page break;
			parseSb(children[i], starttime);
		} else if (nodename == "scoreDef") {   // usually page size information
			parseScoreDef(children[i], starttime);
		} else if (nodename == "staffDef") {   // will this have any useful info?
		   // ignore for now
		} else if (nodename == "staff") {
			// section/staff is possible in mensural music.
			parseStaff_mensural(children[i], starttime);
		} else {
			cerr << DKHTP << section.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseSectionScoreDef --  This is used for page layout information.  Have to look
//   later at if ever has musical content.
//
// Example:
// 	<scoreDef page.height="1973" page.width="1524" page.leftmar="179"
//                 page.rightmar="90" page.topmar="118" page.botmar="112"/>
//

void Tool_mei2hum::parseSectionScoreDef(xml_node scoreDef, HumNum starttime) {
	NODE_VERIFY(scoreDef, );
	MAKE_CHILD_LIST(children, scoreDef);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << DKHTP << scoreDef.name() << "/" << nodename << CURRLOC << endl;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parsePb -- Page break in the music.  Currently treating the same
//   as <sb>.
//

void Tool_mei2hum::parsePb(xml_node pb, HumNum starttime) {
	NODE_VERIFY(pb, );
	MAKE_CHILD_LIST(children, pb);

	// There should be no children of pb (at least any that are currently known)
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << DKHTP << pb.name() << "/" << nodename << CURRLOC << endl;
	}

	m_outdata.back()->appendGlobalLayout("!!LO:LB", starttime QUARTER_CONVERT);
}



//////////////////////////////
//
// Tool_mei2hum::parseSb -- System (line) break in the music.
//

void Tool_mei2hum::parseSb(xml_node sb, HumNum starttime) {
	NODE_VERIFY(sb, );
	MAKE_CHILD_LIST(children, sb);

	// There should be no children of sb (at least any that are currently known)
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << DKHTP << sb.name() << "/" << nodename << CURRLOC << endl;
	}

	m_outdata.back()->appendGlobalLayout("!!LO:LB", starttime QUARTER_CONVERT);
}



//////////////////////////////
//
// Tool_mei2hum::parseApp --
//

HumNum Tool_mei2hum::parseApp(xml_node app, HumNum starttime) {
	NODE_VERIFY(app, starttime);
	MAKE_CHILD_LIST(children, app);

	if (children.empty()) {
		return starttime;
	}

	xml_node target = children[0];
	if (!m_appLabel.empty()) {
		string testlabel;
		for (int i=0; i<(int)children.size(); i++) {
			testlabel = children[i].attribute("label").value();
			if (testlabel == m_appLabel) {
				target = children[i];
				break;
			}
		}
	}

	// Only following the first element in app list for now.
	string nodename = target.name();
	if (nodename == "lem") {
		starttime = parseLem(target, starttime);
	} else if (nodename == "rdg") {
		starttime = parseRdg(target, starttime);
	} else {
		cerr << DKHTP << app.name() << "/" << nodename << CURRLOC << endl;
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseLem -- Process a <lem> element in an MEI file.
//

HumNum Tool_mei2hum::parseLem(xml_node lem, HumNum starttime) {
	NODE_VERIFY(lem, starttime);
	MAKE_CHILD_LIST(children, lem);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else {
			cerr << DKHTP << lem.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseRdg -- Process a <rdg> element in an MEI file.
//

HumNum Tool_mei2hum::parseRdg(xml_node rdg, HumNum starttime) {
	NODE_VERIFY(rdg, starttime);
	MAKE_CHILD_LIST(children, rdg);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else {
			cerr << DKHTP << rdg.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseMeasure -- Process an MEI measure element.
//

HumNum Tool_mei2hum::parseMeasure(xml_node measure, HumNum starttime) {
	NODE_VERIFY(measure, starttime);
	MAKE_CHILD_LIST(children, measure);

	string n = measure.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		// cerr << "Warning: no measure number on measure element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 0) {
		cerr << "Error: invalid measure number: " << nnum << endl;
	}
	m_currentMeasure = nnum;

	GridMeasure* gm = m_outdata.addMeasureToBack();
	gm->setTimestamp(starttime QUARTER_CONVERT);

	vector<HumNum> durations;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "staff") {
			durations.push_back(parseStaff(children[i], starttime) - starttime);
		} else if (nodename == "fermata") {
			// handled in process processNodeStartLinks()
		} else if (nodename == "slur") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "tie") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "arpeg") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "tupletSpan") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "trill") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "dynam") {
			parseDynam(children[i], starttime);
		} else if (nodename == "hairpin") {
			parseHairpin(children[i], starttime);
		} else if (nodename == "harm") {
			parseHarm(children[i], starttime);
		} else if (nodename == "tempo") {
			parseTempo(children[i], starttime);
		} else if (nodename == "dir") {
			parseDir(children[i], starttime);
		} else if (nodename == "reh") {
			parseReh(children[i], starttime);
		} else {
			cerr << DKHTP << measure.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	// Check that the duration of each layer is the same here.

	if (durations.empty()) {
		return starttime;
	}

	bool allequal = true;
	for (int i=1; i<(int)durations.size(); i++) {
		if (durations[i] != durations[0]) {
			allequal = false;
			break;
		}
	}

	HumNum measuredur = durations[0];
	HumNum targetDur = m_measureDuration.at(0) / 4;
	HumNum maxdur = 0;
	bool overfilledQ = false;
	if (!allequal) {
		measuredur = targetDur;
		for (int i=0; i<(int)durations.size(); i++) {
			if (durations[i] > maxdur) {
				maxdur = durations[i];
			}
			if (durations[i] == targetDur) {
				continue;
			}
			if (durations[i] < targetDur) {

				HumNum difference = targetDur - durations[i];
				string spacer = Convert::durationToRecip(difference QUARTER_CONVERT);
				spacer += "ryy";

				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << ", staff " << i+1 << " is underfilled: ";
				message << "adding token " << spacer;
				message << " at end of measure to complete its duration.";
				cerr << message.str() << endl;

				// Add an invisible rest to fill in the problem spot.
				// staff with multiple layers will have to be addressed as well...
				m_outdata.back()->addDataToken(spacer, starttime QUARTER_CONVERT +
						durations[i] QUARTER_CONVERT, i, 0, 0, m_staffcount);

				// put an error message at the start of the measure warning about being underfilled
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);

			} else if (durations[i] > targetDur) {
				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << " staff " << i+1 << " is overfilled: ";
				message << (durations[i] QUARTER_CONVERT).getFloat();
				message << " quarter notes instead of ";
				message << targetDur.getFloat() * 4 << ".";
				cerr << message.str() << endl;
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);

				overfilledQ = true;
			}
		}
	}

	if (overfilledQ) {
		// pad measures that are not under filled so that all
		// parts have the same maximum overfilling.
		for (int i=0; i<(int)durations.size(); i++) {
			if (durations[i] == maxdur) {
				continue;
			}
			HumNum difference = maxdur - durations[i];
			string spacer = Convert::durationToRecip(difference QUARTER_CONVERT);
			spacer += "ryy";

			std::ostringstream message;
			message << "Warning: measure " << m_currentMeasure;
			message << ", staff " << i+1 << " padded. ";
			message << "adding token " << spacer;
			message << " at end ot measure to extend its duration.";
			// cerr << message.str() << endl;

			// Add an invisible rest to fill in the problem spot.
			// staff with multiple layers will have to be addressed as well...
			m_outdata.back()->addDataToken(spacer, starttime QUARTER_CONVERT +
					durations[i] QUARTER_CONVERT, i, 0, 0, m_staffcount);

			// put an error message at the start of the measure warning about being underfilled
			m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);

		}
	}

	gm->setTimestamp(starttime QUARTER_CONVERT);
	gm->setDuration(measuredur QUARTER_CONVERT);
	gm->setTimeSigDur(m_measureDuration[0]);

	string rightstyle = measure.attribute("right").value();
	if (rightstyle == "") {
		// do nothing
	} else if (rightstyle == "end") {
		gm->setFinalBarlineStyle();
	} else if (rightstyle == "rptend") {
		gm->setRepeatBackwardStyle();
	} else if (rightstyle == "invis") {
		gm->setInvisibleBarline();
	}

	if (overfilledQ) {
		return starttime + maxdur;
	} else {
		return starttime + measuredur;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseHairpin -- Process crescendo or dimuendo.
//
//    <hairpin tstamp="1" form="dim" place="below" staff="8" opening="0" endto="4" tstamp2="0m+4.667"/>
//

void Tool_mei2hum::parseHairpin(xml_node hairpin, HumNum starttime) {
	NODE_VERIFY(hairpin, );
	MAKE_CHILD_LIST(children, hairpin);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << DKHTP << hairpin.name() << "/" << nodename << CURRLOC << endl;
	}

	// Store the hairpin for later parsing when more of the
	// score is known:
	auto it = m_outdata.end();
	it--;
	if (it != m_outdata.end()) {
		m_hairpins.resize(m_hairpins.size() + 1);
		m_hairpins.back().hairpin = hairpin;
		m_hairpins.back().gm = *it;
		m_hairpins.back().mindex = ((int)m_currentMeterUnit.size()) - 1;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseReh -- Rehearsal markings (ignored for now)
//

void Tool_mei2hum::parseReh(xml_node reh, HumNum starttime) {
	NODE_VERIFY(reh, );
	MAKE_CHILD_LIST(children, reh);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "rend") {
			// deal with reh/rend here.
		} else {
			cerr << DKHTP << reh.name() << "/" << nodename << CURRLOC << endl;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseStaff -- Process an MEI staff element.
//

HumNum Tool_mei2hum::parseStaff(xml_node staff, HumNum starttime) {
	NODE_VERIFY(staff, starttime);
	MAKE_CHILD_LIST(children, staff);

	string n = staff.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		cerr << "Warning: no staff number on staff element in measure " << m_currentMeasure << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid staff number: " << nnum << endl;
		m_currentStaff++;
	} else {
		m_currentStaff = nnum;
	}

	if (m_maxStaffInFile < m_currentStaff) {
		m_maxStaffInFile = m_currentStaff;
	}

	vector<bool> layerPresent;
	vector<HumNum> durations;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "layer") {
			durations.push_back(parseLayer(children[i], starttime, layerPresent) - starttime);
		} else {
			cerr << DKHTP << staff.name() << "/" << nodename << CURRLOC << endl;
		}
	}

// ggg
// if ((m_currentMeasure == 12) && (m_currentStaff == 6)) {
// cerr << "============= LAYER COUNT " << layerPresent.size() << endl;
// }

	bool complete = true;
	for (int i=0; i<(int)layerPresent.size(); i++) {
// ggg
// if ((m_currentMeasure == 12) && (m_currentStaff == 6)) {
// cerr << "============= LAYER " << i+1 << " : " << layerPresent[i] << endl;
// }
		complete &= layerPresent[i];
	}
	if (!complete) {
		// need to add invisible rests in un-specified layers.
		cerr << "INCOMPLETE LAYERS IN STAFF" << endl;
	}

	// Check that the duration of each layer is the same here.

	if (durations.empty()) {
		return starttime;
	}

	// bool allequal = true;
	for (int i=1; i<(int)durations.size(); i++) {
		if (durations[i] != durations[0]) {
			// allequal = false;
			break;
		}
	}

	HumNum staffdur = durations[0];
	m_currentStaff = 0;

	return starttime + staffdur;
}




//////////////////////////////
//
// Tool_mei2hum::parseStaff_mensural -- Process an MEI staff element for mensural
//   music.
//

HumNum Tool_mei2hum::parseStaff_mensural(xml_node staff, HumNum starttime) {
	NODE_VERIFY(staff, starttime);
	MAKE_CHILD_LIST(children, staff);

	string n = staff.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		cerr << "Warning: no staff number on staff element in measure " << m_currentMeasure << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid staff number: " << nnum << endl;
		m_currentStaff++;
	} else {
		m_currentStaff = nnum;
	}

	if (m_maxStaffInFile < m_currentStaff) {
		m_maxStaffInFile = m_currentStaff;
	}

	vector<bool> layerPresent;
	vector<HumNum> durations;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "layer") {
			durations.push_back(parseLayer_mensural(children[i], starttime, layerPresent) - starttime);
		} else {
			cerr << DKHTP << staff.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	bool complete = true;
	for (int i=0; i<(int)layerPresent.size(); i++) {
		complete &= layerPresent[i];
	}
	if (!complete) {
		// need to add invisible rests in un-specified layers.
		cerr << "INCOMPLETE LAYERS IN STAFF" << endl;
	}

	// Check that the duration of each layer is the same here.

	if (durations.empty()) {
		return starttime;
	}

	// bool allequal = true;
	for (int i=1; i<(int)durations.size(); i++) {
		if (durations[i] != durations[0]) {
			// allequal = false;
			break;
		}
	}

	HumNum staffdur = durations[0];
	m_currentStaff = 0;

	return starttime + staffdur;
}



//////////////////////////////
//
// Tool_mei2hum::parseLayer --
//

HumNum Tool_mei2hum::parseLayer(xml_node layer, HumNum starttime, vector<bool>& layerPresent) {
	NODE_VERIFY(layer, starttime)
	MAKE_CHILD_LIST(children, layer);

	int nnum = 0;
	xml_attribute nattr = layer.attribute("n");
	if (!nattr) {
		// No number on layer, assuming next available number should be used.
		m_currentLayer++;
		nnum = m_currentLayer;
	} else {
		nnum = nattr.as_int();
	}
	if (nnum < 1) {
		cerr << "Error: Ignoring layer with invalid number: " << nnum
		     << " in measure " << m_currentMeasure
		     << ", staff " << m_currentStaff << endl;
		return starttime;
	}
	if (nnum > 8) {
		cerr << "Error: Ignoring layer with ridiculous number: " << nnum
		     << " in measure " << m_currentMeasure
		     << ", staff " << m_currentStaff << endl;
		return starttime;
	}
	m_currentLayer = nnum;

// ggg
// if ((m_currentMeasure == 12) && (m_currentStaff == 6)) {
// cerr << "CURRENT LAYER " << m_currentLayer << endl;
// }

	// grow Layer array if necessary:
	if ((int)layerPresent.size() < m_currentLayer) {
		int oldsize = (int)layerPresent.size();
		layerPresent.resize(m_currentLayer);
		for (int i=oldsize; i<m_currentLayer; i++) {
			layerPresent.at(i) = false;
		}
   }
// ggg
// if ((m_currentMeasure == 12) && (m_currentStaff == 6)) {
// cerr << "LAYER CHECKER SIZE IS " << layerPresent.size() << endl;
// }


	if (layerPresent.at(m_currentLayer - 1)) {
		cerr << "Error: measure " << m_currentMeasure
		     << ", staff " << m_currentStaff
		     << ": layer " << m_currentLayer << " is duplicated on staff: "
		     << m_currentStaff << ". Ignoring duplicate layer." << endl;
		return starttime;
	} else {
		layerPresent.at(m_currentLayer - 1) = true;
	}

	HumNum  starting = starttime;
	string dummy;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime, 0);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime, 0);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "space") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "mRest") {
			starttime = parseMRest(children[i], starttime);
		} else if (nodename == "beam") {
			starttime = parseBeam(children[i], starttime);
		} else if (nodename == "tuplet") {
			starttime = parseTuplet(children[i], starttime);
		} else if (nodename == "clef") {
			parseClef(children[i], starttime);
		} else {
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (!m_gracenotes.empty()) {
		processGraceNotes(starttime);
	}

	m_currentLayer = 0;
	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseLayer_mensural --
//

HumNum Tool_mei2hum::parseLayer_mensural(xml_node layer, HumNum starttime, vector<bool>& layerPresent) {
	NODE_VERIFY(layer, starttime)
	MAKE_CHILD_LIST(children, layer);

	int nnum = 0;
	xml_attribute nattr = layer.attribute("n");
	if (!nattr) {
		// No number on layer, assuming next available number should be used.
		m_currentLayer++;
		nnum = m_currentLayer;
	} else {
		nnum = nattr.as_int();
	}
	if (nnum < 1) {
		cerr << "Error: Ignoring layer with invalid number: " << nnum
		     << " in measure " << m_currentMeasure
		     << ", staff " << m_currentStaff << endl;
		return starttime;
	}
	if (nnum > 8) {
		cerr << "Error: Ignoring layer with ridiculous number: " << nnum
		     << " in measure " << m_currentMeasure
		     << ", staff " << m_currentStaff << endl;
		return starttime;
	}
	m_currentLayer = nnum;

	// grow Layer array if necessary:
	if ((int)layerPresent.size() < m_currentLayer) {
		int oldsize = (int)layerPresent.size();
		layerPresent.resize(m_currentLayer);
		for (int i=oldsize; i<m_currentLayer; i++) {
			layerPresent.at(i) = false;
		}
   }

	if (layerPresent.at(m_currentLayer - 1)) {
		cerr << "Error: measure " << m_currentMeasure
		     << ", staff " << m_currentStaff
		     << ": layer " << m_currentLayer << " is duplicated on staff: "
		     << m_currentStaff << ". Ignoring duplicate layer." << endl;
		return starttime;
	} else {
		layerPresent.at(m_currentLayer - 1) = true;
	}

	HumNum  starting = starttime;
	string dummy;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote_mensural(children[i], xml_node(NULL), dummy, starttime, 0);
		} else if (nodename == "chord") {
			// starttime = parseChord(children[i], starttime, 0);
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		} else if (nodename == "rest") {
			starttime = parseRest_mensural(children[i], starttime);
		} else if (nodename == "space") {
			starttime = parseRest_mensural(children[i], starttime);
		} else if (nodename == "mRest") {
			// starttime = parseMRest(children[i], starttime);
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		} else if (nodename == "beam") {
			// starttime = parseBeam(children[i], starttime);
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		} else if (nodename == "tuplet") {
			// starttime = parseTuplet(children[i], starttime);
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		} else if (nodename == "clef") {
			parseClef(children[i], starttime);
		} else if (nodename == "barLine") {
			parseBarline(children[i], starttime);
		} else if (nodename == "dot") {
			// dot is processed in parseNote_mensural;
		} else {
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	//if (!m_gracenotes.empty()) {
	//	processGraceNotes(starttime);
	//}

	m_currentLayer = 0;
	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseBarline --
//

void Tool_mei2hum::parseBarline(xml_node barLine, HumNum starttime) {
	NODE_VERIFY(barLine, )

	// m_outdata.back()->addBarlineToken("=", starttime QUARTER_CONVERT,
	// 		m_currentStaff-1, 0, 0, m_staffcount);
}



//////////////////////////////
//
// Tool_mei2hum::parseTuplet --
//

HumNum Tool_mei2hum::parseTuplet(xml_node tuplet, HumNum starttime) {
	NODE_VERIFY(tuplet, starttime)
	MAKE_CHILD_LIST(children, tuplet);

	string num = tuplet.attribute("num").value();
	string numbase = tuplet.attribute("numbase").value();

	HumNum newfactor = 1;

	if (numbase == "") {
		cerr << "Warning: tuplet@numbase is empty" << endl;
	} else {
		newfactor = stoi(numbase);
	}

	if (num == "") {
		cerr << "Warning: tuplet@num is empty" << endl;
	} else {
		newfactor /= stoi(num);
	}

	m_tupletfactor *= newfactor;

	string stored_beamPostfix;
	if (m_beamPostfix != "") {
		stored_beamPostfix = m_beamPostfix;
		m_beamPostfix.clear();
	}

	xml_node lastnoterestchord;
	for (int i=(int)children.size() - 1; i>=0; i--) {
		string nodename = children[i].name();
		if (nodename == "note") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "rest") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "chord") {
			lastnoterestchord = children[i];
			break;
		}
	}

	string dummy;
	for (int i=0; i<(int)children.size(); i++) {
		if (children[i] == lastnoterestchord) {
			m_beamPostfix = stored_beamPostfix;
		}
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime, 0);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime, 0);
		} else if (nodename == "beam") {
			starttime = parseBeam(children[i], starttime);
		} else {
			cerr << DKHTP << tuplet.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	m_tupletfactor /= newfactor;

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseBeam --
//

HumNum Tool_mei2hum::parseBeam(xml_node beam, HumNum starttime) {
	NODE_VERIFY(beam, starttime)
	MAKE_CHILD_LIST(children, beam);

	bool isvalid = beamIsValid(children);
	// bool isgrace = beamIsGrace(children);

	if (isvalid) {
		m_beamPrefix = "L";
	}

	xml_node lastnoterestchord;
	for (int i=(int)children.size()-1; i>=0; i--) {
		string nodename = children[i].name();
		if (nodename == "note") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "rest") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "chord") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "tuplet") {
			lastnoterestchord = children[i];
			break;
		}
	}

	string dummy;
	for (int i=0; i<(int)children.size(); i++) {
		if (children[i] == lastnoterestchord) {
			if (isvalid) {
				m_beamPostfix = "J";
			}
		}
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime, 0);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime, 0);
		} else if (nodename == "tuplet") {
			starttime = parseTuplet(children[i], starttime);
		} else if (nodename == "clef") { //drizo
			parseClef(children[i], starttime);
		} else {
			cerr << DKHTP << beam.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::beamIsGrace -- beam only contains grace notes.
//

bool Tool_mei2hum::beamIsGrace(vector<xml_node>& beamlist) {
	for (int i=0; i<(int)beamlist.size(); i++) {
		string nodename = beamlist[i].name();
		if (nodename != "note") {
			continue;
		}
		string grace = beamlist[i].attribute("grace").value();
		if (grace.empty()) {
			return false;
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_mei2hum::beamIsValid --
//

bool Tool_mei2hum::beamIsValid(vector<xml_node>& beamlist) {
	for (int i=0; i<(int)beamlist.size(); i++) {
		string nodename = beamlist[i].name();
		if (nodename != "note") {
			continue;
		}
		string grace = beamlist[i].attribute("grace").value();
		if (!grace.empty()) {
			continue;
		}
		string dur = beamlist[i].attribute("dur").value();
		if (dur.empty()) {
			// strange, but skip
			continue;
		}
		if (isdigit(dur[0])) {
			if (stoi(dur) <= 4) {
				return false;
			}
		} else {
			// "breve", "long", "maxima", junk, etc.
			return false;
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_mei2hum::processGraceNotes -- write the grace notes into the score
//   in reverse order before any note at the given timestamp.
//

void Tool_mei2hum::processGraceNotes(HumNum timestamp) {
	int size = (int)m_gracenotes.size();
	int counter = 1;
	string output;
	for (int i=size-1; i>=0; i--) {
		string nodename = m_gracenotes[i].node.name();
		if (nodename == "note") {
			m_beamPrefix = m_gracenotes[i].beamprefix;
			m_beamPostfix = m_gracenotes[i].beampostfix;
			parseNote(m_gracenotes[i].node, xml_node(NULL), output, m_gracetime, counter);
		} else if (nodename == "chord") {
			m_beamPrefix = m_gracenotes[i].beamprefix;
			m_beamPostfix = m_gracenotes[i].beampostfix;
			parseChord(m_gracenotes[i].node, m_gracetime, counter);
		} else {
			cerr << "STRANGE THING HAPPENED HERE, node name is " << nodename << endl;
		}
		counter++;
	}

	m_gracenotes.clear();
}



//////////////////////////////
//
// Tool_mei2hum::parseNote --
//

HumNum Tool_mei2hum::parseNote(xml_node note, xml_node chord, string& output,
		HumNum starttime, int gracenumber) {
	NODE_VERIFY(note, starttime)
	MAKE_CHILD_LIST(children, note);

	HumNum duration;
	int dotcount;

	string grace = note.attribute("grace").value();
	bool graceQ = !grace.empty();

	if (gracenumber == 0) {
		if (graceQ) {
			if (!m_gracenotes.empty()) {
				if (starttime != m_gracetime) {
					// Grace notes at end of previous measure which
					// should have been processed before coming into the
					// next measure.
					cerr << "STRANGE ERROR IN GRACE NOTE PARSING" << endl;
					cerr << "\tSTARTTIME: " << starttime << endl;
					cerr << "\tGRACETIME: " << m_gracetime << endl;
				}
			} else {
				m_gracetime = starttime;
			}
			if (chord) {
				grace_info ginfo;
				ginfo.node = chord;
				ginfo.beamprefix = m_beamPrefix;
				ginfo.beampostfix = m_beamPostfix;
				m_beamPrefix.clear();
				m_beamPostfix.clear();
				m_gracenotes.push_back(ginfo);
			} else {
				grace_info ginfo;
				ginfo.node = note;
				ginfo.beamprefix = m_beamPrefix;
				ginfo.beampostfix = m_beamPostfix;
				m_beamPrefix.clear();
				m_beamPostfix.clear();
				m_gracenotes.push_back(ginfo);
			}
			// grace notes processed after knowing how many of them
			// are before a real note (or at the end of the measure).
 			return starttime;
		}

	}

	processPreliminaryLinkedNodes(note);

	if (chord) {
		duration = getDuration(chord);
		dotcount = getDotCount(chord);
		// maybe allow different durations on notes in a chord if the
		// first one is the same as the duration of the chord.
	} else {
		duration = getDuration(note);
		dotcount = getDotCount(note);
	}

	string recip = getHumdrumRecip(duration, dotcount);
	string humpitch = getHumdrumPitch(note, children);
	string editorial = getEditorialAccidental(children);
	string cautionary = getCautionaryAccidental(children);
	if (!editorial.empty()) {
		humpitch += editorial;
	}
	if (!cautionary.empty()) {
		humpitch += cautionary;
	}

	string articulations = getNoteArticulations(note, chord);

	string stemdir = note.attribute("stem.dir").value();

	if (!m_stemsQ) {
		// suppress note stems
		stemdir = "";
	}

	if (stemdir == "up") {
		stemdir = "/";
	} else if (stemdir == "down") {
		stemdir = "\\";
	} else {
		stemdir = "";
	}
	string gracelabel = "";
	if (graceQ) {
		gracelabel = "q";
	}

	string tok = recip + gracelabel + humpitch + articulations + stemdir
			+ m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	m_fermata = false;
	processLinkedNodes(tok, note);
	if (!m_fermata) {
		processFermataAttribute(tok, note);
	}

	GridSlice* dataslice = NULL;

	if (!chord) {
		if (gracenumber == 0) {
			dataslice = m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT,
					m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);
		} else {
			dataslice = m_outdata.back()->addGraceToken(tok, starttime QUARTER_CONVERT,
					m_currentStaff-1, 0, m_currentLayer-1, m_staffcount, gracenumber);
		}
	} else {
		output += tok;
	}

	if (m_xmlidQ) {
		GridStaff* staff = dataslice->at(m_currentStaff-1)->at(0);
		// not keeping track of overwriting ids at the moment:
		string xmlid = note.attribute("xml:id").value();
		if (!xmlid.empty()) {
			staff->setXmlid(xmlid);
			m_outdata.setXmlidsPresent(m_currentStaff-1);
		}
	}

	bool hasverse = false;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if ((nodename == "verse") && (dataslice != NULL)) {
			hasverse = true;
			parseVerse(children[i], dataslice->at(m_currentStaff-1)->at(0));
		} else if ((nodename == "syl") && (dataslice != NULL)) {
			hasverse = true;
			parseBareSyl(children[i], dataslice->at(m_currentStaff-1)->at(0));
		} else if (nodename == "artic") {
			// handled elsewhere: don't do anything
		} else if (nodename == "accid") {
			// handled elsewhere: don't do anything
		} else {
			cerr << DKHTP << note.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (!hasverse) {
		string attsyl = note.attribute("syl").value();
		if (!attsyl.empty()) {
			parseSylAttribute(attsyl, dataslice->at(m_currentStaff-1)->at(0));
		}
	}

	if ((!graceQ) && (!m_gracenotes.empty())) {
		processGraceNotes(starttime);
	}

	if (graceQ) {
		return starttime;
	} else {
		return starttime + duration;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseNote_mensural --
//

HumNum Tool_mei2hum::parseNote_mensural(xml_node note, xml_node chord, string& output,
		HumNum starttime, int gracenumber) {
	NODE_VERIFY(note, starttime)
	MAKE_CHILD_LIST(children, note);

	HumNum duration;
	int dotcount;

	string grace = note.attribute("grace").value();
	bool graceQ = !grace.empty();
	if (graceQ) {
		// cannot have grace notes in mensural music
		return starttime;
	}

	processPreliminaryLinkedNodes(note);

	if (chord) {
		duration = getDuration_mensural(chord, dotcount);
		// maybe allow different durations on notes in a chord if the
		// first one is the same as the duration of the chord.
	} else {
		duration = getDuration_mensural(note, dotcount);
	}

	string meidur = note.attribute("dur").value();
	string mensrhy;
	if      (meidur == "maxima")      { mensrhy = "X"; }
	else if (meidur == "longa")       { mensrhy = "L"; }
	else if (meidur == "brevis")      { mensrhy = "S"; }
	else if (meidur == "semibrevis")  { mensrhy = "s"; }
	else if (meidur == "minima")      { mensrhy = "M"; }
	else if (meidur == "semiminima")  { mensrhy = "m"; }
	else if (meidur == "fusa")        { mensrhy = "U"; }
	else if (meidur == "semifusa")    { mensrhy = "u"; }
	else { mensrhy = "?"; }

	string recip      = getHumdrumRecip(duration/4, dotcount);
	string humpitch   = getHumdrumPitch(note, children);
	string editorial  = getEditorialAccidental(children);
	string cautionary = getCautionaryAccidental(children);

	xml_attribute dur_qual = note.attribute("dur.quality");
	string durquality = dur_qual.value();
	string quality;
	if (durquality == "perfecta") {
		quality = "p";
	} else if (durquality == "imperfecta") {
		quality = "i";
	} else if (durquality == "altera") {
		quality = "+";
	}

	humpitch = mensrhy + quality + humpitch;

	if (!editorial.empty()) {
		humpitch += editorial;
	}
	if (!cautionary.empty()) {
		humpitch += cautionary;
	}

	// string articulations = getNoteArticulations(note, chord);
	string articulations;

	string stemdir = note.attribute("stem.dir").value();

	// if (!m_stemsQ) {
	// 	// suppress note stems
	// 	stemdir = "";
	// }

	if (stemdir == "up") {
		stemdir = "/";
	} else if (stemdir == "down") {
		stemdir = "\\";
	} else {
		stemdir = "";
	}
	string gracelabel = "";
	if (graceQ) {
		gracelabel = "q";
	}

	string mensdot = "";
	xml_node nextsibling = note.next_sibling();
	if (strcmp(nextsibling.name(), "barLine") == 0) {
		nextsibling = nextsibling.next_sibling();
	}
	if (strcmp(nextsibling.name(), "dot") == 0) {
		mensdot = ":";
	}
	string tok = /* recip + */ gracelabel + humpitch + articulations + stemdir
			+ m_beamPrefix + m_beamPostfix + mensdot;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	m_fermata = false;
	processLinkedNodes(tok, note);
	if (!m_fermata) {
		processFermataAttribute(tok, note);
	}

	GridSlice* dataslice = NULL;

	if (!chord) {
		if (gracenumber == 0) {
			dataslice = m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT,
					m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);
		} else {
			dataslice = m_outdata.back()->addGraceToken(tok, starttime QUARTER_CONVERT,
					m_currentStaff-1, 0, m_currentLayer-1, m_staffcount, gracenumber);
		}
	} else {
		output += tok;
	}

	if (m_xmlidQ) {
		GridStaff* staff = dataslice->at(m_currentStaff-1)->at(0);
		// not keeping track of overwriting ids at the moment:
		string xmlid = note.attribute("xml:id").value();
		if (!xmlid.empty()) {
			staff->setXmlid(xmlid);
			m_outdata.setXmlidsPresent(m_currentStaff-1);
		}
	}

	bool hasverse = false;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if ((nodename == "verse") && (dataslice != NULL)) {
			hasverse = true;
			parseVerse(children[i], dataslice->at(m_currentStaff-1)->at(0));
		} else if ((nodename == "syl") && (dataslice != NULL)) {
			hasverse = true;
			parseBareSyl(children[i], dataslice->at(m_currentStaff-1)->at(0));
		} else if (nodename == "artic") {
			// handled elsewhere: don't do anything
		} else if (nodename == "accid") {
			// handled elsewhere: don't do anything
		} else {
			cerr << DKHTP << note.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (!hasverse) {
		string attsyl = note.attribute("syl").value();
		if (!attsyl.empty()) {
			parseSylAttribute(attsyl, dataslice->at(m_currentStaff-1)->at(0));
		}
	}

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::parseSylAttribute --
//

void Tool_mei2hum::parseSylAttribute(const string& attsyl, GridStaff* staff) {
	vector<string> pieces(1);
	int length = (int)attsyl.size();
	if (length == 0) {
		return;
	}
	if (length == 1) {
		pieces[0] += attsyl;
	} else {
		for (int i=0; i<length-2; i++) {
			if ((attsyl[i] == '/') && (attsyl[i+1] == '/')) {
				pieces.emplace_back("");
				i++;
			} else {
				pieces.back() += attsyl[i];
			}
		}
		if ((attsyl[length-1] != '/') && (attsyl[length-2] != '/')) {
			pieces.back() += attsyl[length-2];
			pieces.back() += attsyl.back();
		}
	}

	if ((pieces.size() == 1) && (pieces[0].empty())) {
		return;
	}

	for (int i=0; i<(int)pieces.size(); i++) {
		pieces[i] = cleanVerseText(pieces[i]);
	}

	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].empty()) {
			continue;
		}
		staff->setVerse(i, pieces[i]);
		reportVerseNumber(i+1, m_currentStaff-1);
	}
}



//////////////////////////////
//
// Tool_mei2hum::getEditorialAccidental --
//

string Tool_mei2hum::getEditorialAccidental(vector<xml_node>& children) {
	string output;
	if (children.empty()) {
		return output;
	}

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string function = children[i].attribute("func").value();
		if (function != "edit") {
			continue;
		}
		string accid = children[i].attribute("accid").value();
		if (accid.empty()) {
			continue;
		}
		output = accidToKern(accid);
		if (!output.empty()) {
			output += "i";
		}
		m_editorialAccidentalQ = true;
		break;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getCautionaryAccidental --
// Such as:
//     <accid accid="n" func="caution" />
//

string Tool_mei2hum::getCautionaryAccidental(vector<xml_node>& children) {
	string output;
	if (children.empty()) {
		return output;
	}

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string function = children[i].attribute("func").value();
		if (function != "caution") {
			continue;
		}
		string accid = children[i].attribute("accid").value();
		if (accid.empty()) {
			continue;
		}
		output = accidToKern(accid);
		if ((!output.empty()) && (output != "n")) {
			output += "X";
		}
		break;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::accidToKern -- Convert accid string into **kern accidental.
//

string Tool_mei2hum::accidToKern(const string& accid) {
	string output;
	if (accid == "n") {
		output = "n";
	} else if (accid == "s") {
		output = "#";
	} else if (accid == "f") {
		output = "-";
	} else if (accid == "ff") {
		output = "--";
	} else if (accid == "ss") {
		output = "##";
	} else if (accid == "x") {
		output = "##";
	} else if (accid == "nf") {
		output = "-";
	} else if (accid == "ns") {
		output = "#";
	} else if (accid == "xs") {
		output = "###";
	} else if (accid == "sx") {
		output = "###";
	} else if (accid == "tf") {
		output = "---";
	} else if (accid == "ts") {
		output = "###";
	} else {
		cerr << "Don't know how to interpret " << accid << " accidental" << endl;
	}
	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseMRest -- Full-measure rest.
//

HumNum Tool_mei2hum::parseMRest(xml_node mrest, HumNum starttime) {
	HumNum duration = m_measureDuration.at(m_currentStaff-1);
	duration /= 4;
	int dotcount = 0;
	string recip = getHumdrumRecip(duration, dotcount);
	if (recip.find('%') != string::npos) {
		string recip2 = getHumdrumRecip(duration, 1);
		if (recip2.find('%') == string::npos) {
			recip = recip2;
			dotcount = 1;
		} else {
			string recip3 = getHumdrumRecip(duration, 2);
			if (recip2.find('%') == string::npos) {
				recip = recip3;
				dotcount = 2;
			}
		}
	}
	string tok = recip + "r";
	// Add fermata on whole-measure rest if needed.

	// Deal here with calculating number of dots needed for
	// measure duration.

	m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);

	return starttime + duration; // convert to whole-note units
}



//////////////////////////////
//
// Tool_mei2hum::parseRest --
//

HumNum Tool_mei2hum::parseRest(xml_node rest, HumNum starttime) {
	if (!rest) {
		return starttime;
	}
	string nodename = rest.name();
	if (!((nodename == "rest") || (nodename == "space"))) {
		return starttime;
	}
	if (nodename == "rest") {
		ELEMENT_DEBUG_STATEMENT(rest)
	} else if (nodename == "space") {
		ELEMENT_DEBUG_STATEMENT(space)
	}

	processPreliminaryLinkedNodes(rest);

	HumNum duration = getDuration(rest);
	int dotcount = getDotCount(rest);
	string recip = getHumdrumRecip(duration, dotcount);
	string invisible;
	if (nodename == "space") {
		invisible = "yy";
	}

	string output = recip + "r" + invisible + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	processLinkedNodes(output, rest);
	processFermataAttribute(output, rest);

	m_outdata.back()->addDataToken(output, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::parseRest_mensural --
//

HumNum Tool_mei2hum::parseRest_mensural(xml_node rest, HumNum starttime) {
	if (!rest) {
		return starttime;
	}
	string nodename = rest.name();
	if (!((nodename == "rest") || (nodename == "space"))) {
		return starttime;
	}
	if (nodename == "rest") {
		ELEMENT_DEBUG_STATEMENT(rest)
	} else if (nodename == "space") {
		ELEMENT_DEBUG_STATEMENT(space)
	}

	MAKE_CHILD_LIST(children, rest);

	processPreliminaryLinkedNodes(rest);

	string meidur = rest.attribute("dur").value();
	string mensrhy;
	if      (meidur == "maxima")      { mensrhy = "X"; }
	else if (meidur == "longa")       { mensrhy = "L"; }
	else if (meidur == "brevis")      { mensrhy = "S"; }
	else if (meidur == "semibrevis")  { mensrhy = "s"; }
	else if (meidur == "minima")      { mensrhy = "M"; }
	else if (meidur == "semiminima")  { mensrhy = "m"; }
	else if (meidur == "fusa")        { mensrhy = "U"; }
	else if (meidur == "semifusa")    { mensrhy = "u"; }
	else { mensrhy = "?"; }

	int dotcount = 0;
	HumNum duration = getDuration_mensural(rest, dotcount);

	string invisible;
	if (nodename == "space") {
		invisible = "yy";
	}

	string recip = getHumdrumRecip(duration/4, dotcount);
	string humpitch   = ""; // getHumdrumPitch(rest, children);
	string editorial  = getEditorialAccidental(children);
	string cautionary = getCautionaryAccidental(children);

	xml_attribute dur_qual = rest.attribute("dur.quality");
	string durquality = dur_qual.value();
	string quality;
	if (durquality == "perfecta") {
		quality = "p";
	} else if (durquality == "imperfecta") {
		quality = "i";
	} else if (durquality == "altera") {
		quality = "+";
	}

	humpitch = mensrhy + quality + humpitch;

	string output = mensrhy + "r" + invisible + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	processLinkedNodes(output, rest);
	processFermataAttribute(output, rest);

	m_outdata.back()->addDataToken(output, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::getNoteArticulations --
//

string Tool_mei2hum::getNoteArticulations(xml_node note, xml_node chord) {

	string attribute_artic = note.attribute("artic").value();

	vector<xml_node> element_artic;
	for (pugi::xml_node artic : note.children("artic")) {
		element_artic.push_back(artic);
	}

	string chord_attribute_artic;
	vector<xml_node> chord_element_artic;

	if (chord) {
		chord_attribute_artic = chord.attribute("artic").value();
	}
	for (pugi::xml_node artic : chord.children("artic")) {
		chord_element_artic.push_back(artic);
	}

	string output;

	output += getHumdrumArticulation("\\bstacc\\b", "'", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bacc\\b", "^", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bmarc\\b", "^^", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bstacciss\\b", "`", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bten\\b", "~", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);

	return output;
}



///////////////////////////////
//
// Tool_mei2hum::getHumdrumArticulation --
//

string Tool_mei2hum::getHumdrumArticulation(const string& tag, const string& humdrum,
		const string& attribute_artic, vector<xml_node>& element_artic,
		const string& chord_attribute_artic, vector<xml_node>& chord_element_artic) {
	HumRegex hre;
	string output;

	// First check artic attributes:
	if (hre.search(attribute_artic, tag)) {
		output = humdrum;
		return output;
	}

	// If the note is attached to a chord, search the chord:
	if (hre.search(chord_attribute_artic, tag)) {
		output = humdrum;
		return output;
	}

	// Now search for artic elements in the note:
	for (int i=0; i<(int)element_artic.size(); i++) {
		string nodename = element_artic[i].name();
		if (nodename != "artic") {
			continue;
		}
		string artic = element_artic[i].attribute("artic").value();
		if (hre.search(artic, tag)) {
			output = humdrum;
			output += setPlacement(element_artic[i].attribute("place").value());
			return output;
		}
	}

	// And then in the chord:
	for (int i=0; i<(int)chord_element_artic.size(); i++) {
		string nodename = chord_element_artic[i].name();
		if (nodename != "artic") {
			continue;
		}
		string artic = chord_element_artic[i].attribute("artic").value();
		if (hre.search(artic, tag)) {
			output = humdrum;
			output += setPlacement(chord_element_artic[i].attribute("place").value());
			return output;
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::setPlacement --
//

string Tool_mei2hum::setPlacement(const string& placement) {
	if (!m_placeQ) {
		return "";
	}
	if (placement == "above") {
		m_aboveQ = true;
		return ">";
	} else if (placement == "below") {
		m_belowQ = true;
		return "<";
	}
	return "";
}



//////////////////////////////
//
// Tool_mei2hum::processFermataAttribute -- @fermata="(above|below)"
//

void Tool_mei2hum::processFermataAttribute(string& output, xml_node node) {
	string fermata = node.attribute("fermata").value();
	if (fermata.empty()) {
		return;
	}
	if (fermata == "above") {
		output += ';';
	} else if (fermata == "below") {
		output += ";<";
		m_belowQ = true;
	}
}



//////////////////////////////
//
// Tool_mei2hum::processPreliminaryLinkedNodes -- Process tupletSpan
//      before rhythm of linked notes are processed.
//

void Tool_mei2hum::processPreliminaryLinkedNodes(xml_node node) {
	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_startlinks.find(id);
		if (found != m_startlinks.end()) {
			processNodeStartLinks2(node, (*found).second);
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::processLinkedNodes --
//

void Tool_mei2hum::processLinkedNodes(string& output, xml_node node) {
	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_startlinks.find(id);
		if (found != m_startlinks.end()) {
			processNodeStartLinks(output, node, (*found).second);
		}
		found = m_stoplinks.find(id);
		if (found != m_stoplinks.end()) {
			processNodeStopLinks(output, node, (*found).second);
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::getDotCount --  Get dot count from first note in chord if no @dur on chord.
//

int Tool_mei2hum::getDotCount(xml_node node) {
	string name = node.name();
	if (name == "chord") {
		if (!node.attribute("dur")) {
			node = node.select_node(".//note").node();
		}
	}

	string dots = node.attribute("dots").value();
	int dotcount = 0;
	if (dots != "") {
		dotcount = stoi(dots);
	}
	return dotcount;
}



//////////////////////////////
//
// Tool_mei2hum::processNodeStartLinks --
//

void Tool_mei2hum::processNodeStartLinks(string& output, xml_node node,
		vector<xml_node>& nodelist) {
	for (int i=0; i<(int)nodelist.size(); i++) {
		string nodename = nodelist[i].name();
		if (nodename == "fermata") {
			m_fermata = true; // used to disable note@fermata duplications
			parseFermata(output, node, nodelist[i]);
		} else if (nodename == "slur") {
			parseSlurStart(output, node, nodelist[i]);
		} else if (nodename == "tie") {
			parseTieStart(output, node, nodelist[i]);
		} else if (nodename == "trill") {
			parseTrill(output, node, nodelist[i]);
		} else if (nodename == "arpeg") {
			parseArpeg(output, node, nodelist[i]);
		} else if (nodename == "tupletSpan") {
			// handled in processNodeStartLinks2
		} else {
			cerr << DKHTP << nodename
			     << " element in processNodeStartLinks()" << endl;
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::processNodeStartLinks2 -- process tupletSpan before the
//     duration of the note/rest/chord is calculated.
//

void Tool_mei2hum::processNodeStartLinks2(xml_node node,
		vector<xml_node>& nodelist) {
	for (int i=0; i<(int)nodelist.size(); i++) {
		string nodename = nodelist[i].name();
		if (nodename == "tupletSpan") {
			parseTupletSpanStart(node, nodelist[i]);
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseTupletSpanStart --
//     Such as:
//          <tupletSpan staff="10" num="3" numbase="2" num.visible="true"
//                num.place="below" num.format="count" startid="#4235235"
//                endid="#532532"/>
//

void Tool_mei2hum::parseTupletSpanStart(xml_node node,
		xml_node tupletSpan) {
	NODE_VERIFY(tupletSpan, )

	if (strcmp(tupletSpan.attribute("endid").value(), "") == 0) {
		cerr << "Warning: <tupletSpan> requires endid attribute (at least ";
		cerr << "for this parser)" << endl;
		return;
	}

	if (strcmp(tupletSpan.attribute("startid").value(), "") == 0) {
		cerr << "Warning: <tupletSpan> requires startid attribute (at least ";
		cerr << "for this parser)" << endl;
		return;
	}

	string num = tupletSpan.attribute("num").value();
	string numbase = tupletSpan.attribute("numbase").value();

	HumNum newfactor = 1;

	if (numbase == "") {
		cerr << "Warning: tuplet@numbase is empty" << endl;
	} else {
		newfactor = stoi(numbase);
	}

	if (num == "") {
		cerr << "Warning: tuplet@num is empty" << endl;
	} else {
		newfactor /= stoi(num);
	}

	m_tupletfactor *= newfactor;

}



//////////////////////////////
//
// Tool_mei2hum::parseTupletSpanStop --
//     Such as:
//          <tupletSpan staff="10" num="3" numbase="2" num.visible="true"
//                num.place="below" num.format="count" startid="#4235235"
//                endid="#532532"/>
//

void Tool_mei2hum::parseTupletSpanStop(string& output, xml_node node,
		xml_node tupletSpan) {
	NODE_VERIFY(tupletSpan, )

	if (strcmp(tupletSpan.attribute("endid").value(), "") == 0) {
		return;
	}
	if (strcmp(tupletSpan.attribute("startid").value(), "") == 0) {
		return;
	}

	string num = tupletSpan.attribute("num").value();
	string numbase = tupletSpan.attribute("numbase").value();

	HumNum newfactor = 1;

	if (numbase == "") {
		cerr << "Warning: tuplet@numbase is empty" << endl;
	} else {
		newfactor = stoi(numbase);
	}

	if (num == "") {
		cerr << "Warning: tuplet@num is empty" << endl;
	} else {
		newfactor /= stoi(num);
	}

	// undo the tuplet factor:
	m_tupletfactor /= newfactor;

}



//////////////////////////////
//
// Tool_mei2hum::parseArpeg -- Only handles single chord arpeggiation for now
//    (ignores @endid).
//

void Tool_mei2hum::parseArpeg(string& output, xml_node node, xml_node arpeg) {
	NODE_VERIFY(arpeg, )

	if (strcmp(arpeg.attribute("endid").value(), "") != 0) {
		cerr << "Warning: multi-note arpeggios are not yet handled in the converter." << endl;
	}

	string nodename = node.name();
	if (nodename == "note") {
		output += ':';
	} else if (nodename == "chord") {
		string temp = output;
		output.clear();
		for (int i=0; i<(int)temp.size(); i++) {
			if (temp[i] == ' ') {
				output += ": ";
			} else {
				output += temp[i];
			}
		}
		output += ':';
	} else {
		cerr << DKHTP << "an arpeggio attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::processNodeStopLinks --
//

void Tool_mei2hum::processNodeStopLinks(string& output, xml_node node,
		vector<xml_node>& nodelist) {
	for (int i=0; i<(int)nodelist.size(); i++) {
		string nodename = nodelist[i].name();
		if (nodename == "slur") {
			parseSlurStop(output, node, nodelist[i]);
		} else if (nodename == "tie") {
			parseTieStop(output, node, nodelist[i]);
		} else if (nodename == "tupletSpan") {
			parseTupletSpanStop(output, node, nodelist[i]);
		} else {
			cerr << DKHTP << nodename
			     << " element in processNodeStopLinks()" << endl;
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseSlurStart --
//

void Tool_mei2hum::parseSlurStart(string& output, xml_node node, xml_node slur) {
	NODE_VERIFY(slur, )
	string nodename = node.name();
	if (nodename == "note") {
		output = "(" + setPlacement(slur.attribute("curvedir").value()) + output;
	} else if (nodename == "chord") {
		output = "(" + setPlacement(slur.attribute("curvedir").value()) + output;
	} else {
		cerr << DKHTP << "a slur start attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseSlurStop --
//

void Tool_mei2hum::parseSlurStop(string& output, xml_node node, xml_node slur) {
	NODE_VERIFY(slur, )
	string nodename = node.name();
	if (nodename == "note") {
		output += ")";
	} else if (nodename == "chord") {
		output += ")";
	} else {
		cerr << DKHTP << "a tie end attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseTieStart -- Need to deal with chords later.
//

void Tool_mei2hum::parseTieStart(string& output, xml_node node, xml_node tie) {
	NODE_VERIFY(tie, )

	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_stoplinks.find(id);
		if (found != m_stoplinks.end()) {
			for (auto item : (*found).second) {
				if (strcmp(tie.attribute("startid").value(), item.attribute("endid").value()) == 0) {
					// deal with tie middles in parseTieStop().
					return;
				}
			}
		}
	}

	string nodename = node.name();
	if (nodename == "note") {
		output = "[" + output;
	} else {
		cerr << DKHTP << "a tie start attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseTrill --
//

void Tool_mei2hum::parseTrill(string& output, xml_node node, xml_node trill) {
	NODE_VERIFY(trill, )

	auto loc = output.find(";");
	if (loc != string::npos) {
		output.insert(loc, "T");
		return;
	}

	loc = output.find(")");
	if (loc != string::npos) {
		output.insert(loc, "T");
		return;
	}

	output += "T";

	// Deal with endid attribute on trills later.
}



//////////////////////////////
//
// Tool_mei2hum::parseTieStop -- Need to deal with chords later.
//

void Tool_mei2hum::parseTieStop(string& output, xml_node node, xml_node tie) {
	NODE_VERIFY(tie, )

	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_startlinks.find(id);
		if (found != m_startlinks.end()) {
			for (auto item : (*found).second) {
				if (strcmp(tie.attribute("endid").value(), item.attribute("startid").value()) == 0) {
					output += "_";
					return;
				}
			}
		}
	}

	string nodename = node.name();
	if (nodename == "note") {
		output += "]";
	} else {
		cerr << DKHTP << "a tie end attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseFermata -- deal with a fermata attached to something.
///     output is a Humdrum token string (maybe have it as a HumdrumToken object).
//

void Tool_mei2hum::parseFermata(string& output, xml_node node, xml_node fermata) {
	NODE_VERIFY(fermata, )

	string nodename = node.name();
	if (nodename == "note") {
		output += ';';
	} else if (nodename == "chord") {
		output += ';';
	} else if (nodename == "rest") {
		output += ';';
	} else {
		cerr << DKHTP << "a fermata attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::getHumdrumRecip --
//

string Tool_mei2hum::getHumdrumRecip(HumNum duration, int dotcount) {
	string output;

	if (dotcount > 0) {
		// remove dots from duration
		int top = (1 << (dotcount+1)) - 1;
		int bot = 1 << dotcount;
		HumNum dotfactor(bot, top);
		duration *= dotfactor;
	}

	if (duration.getNumerator() == 1) {
		output = to_string(duration.getDenominator());
	} else if ((duration.getNumerator() == 2) && (duration.getDenominator() == 1)) {
		// breve symbol:
		output = "0";
	} else if ((duration.getNumerator() == 4) && (duration.getDenominator() == 1)) {
		// long symbol:
		output = "00";
	} else if ((duration.getNumerator() == 8) && (duration.getDenominator() == 1)) {
		// maxima symbol:
		output = "000";
	} else {
		output = to_string(duration.getDenominator());
		output += "%";
		output += to_string(duration.getNumerator());
	}

	for (int i=0; i<dotcount; i++) {
		output += '.';
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getChildAccidVis -- Return accid@accid from any element
//   in the list, if it is not editorial or cautionary.
//

string Tool_mei2hum::getChildAccidVis(vector<xml_node>& children) {
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string func = children[i].attribute("func").value();
		if (func == "caution") {
			// cautionary accidental handled elsewhere
			return "";
		} else if (func == "edit") {
			// editorial accidental handled elsewhere
			return "";
		}
		string accid = children[i].attribute("accid").value();
		return accid;
	}
	return "";
}



//////////////////////////////
//
// Tool_mei2hum::getChildAccidGes -- Return the accid@accid.ges value
//    of any element in the input list, but not if the accidental is
//    part of an cautionary or editorial accidental.
//

string Tool_mei2hum::getChildAccidGes(vector<xml_node>& children) {
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string func = children[i].attribute("func").value();
		if (func == "caution") {
			// cautionary accidental handled elsewhere
			return "";
		} else if (func == "edit") {
			// editorial accidental handled elsewhere
			return "";
		}
		string accidges = children[i].attribute("accid.ges").value();
		return accidges;
	}
	return "";
}



//////////////////////////////
//
// Tool_mei2hum::getHumdrumPitch --
//

string Tool_mei2hum::getHumdrumPitch(xml_node note, vector<xml_node>& children) {
	string pname = note.attribute("pname").value();
	string accidvis = note.attribute("accid").value();
	string accidges = note.attribute("accid.ges").value();

	string accidvischild = getChildAccidVis(children);
	string accidgeschild = getChildAccidGes(children);

	int octnum = 4;
	string oct = note.attribute("oct").value();
	if (oct == "") {
		cerr << "Empty octave" << endl;
	} else if (isdigit(oct[0])) {
		octnum = stoi(oct);
	} else {
		cerr << "Unknown octave value: " << oct << endl;
	}

	if (pname == "") {
		cerr << "Empty pname" << endl;
		return "x";
	}

	string output;
	if (octnum < 4) {
		char val = toupper(pname[0]);
		int count = 4 - octnum;
		for (int i=0; i<count; i++) {
			output += val;
		}
	} else {
		char val = pname[0];
		int count = octnum - 3;
		for (int i=0; i<count; i++) {
			output += val;
		}
	}

	if (accidges != "") {
		string acc = accidToKern(accidges);
		if (acc != "n") {
			output += acc;
			// accidental is not visible
			output += "y";
		}
	} else if (accidvis != "") {
		string acc = accidToKern(accidvis);
		output += acc;
	} else if (accidvischild != "") {
		string acc = accidToKern(accidvischild);
		output += acc;
	} else if (accidgeschild != "") {
		string acc = accidToKern(accidgeschild);
		if (acc != "n") {
			output += acc;
			// accidental is not visible
			output += "y";
		}
	}

	// Transpose to C score if part is transposing:
	if (m_currentStaff) {
		if (m_scoreDef.staves[m_currentStaff-1].base40 != 0) {
			int base40 = Convert::kernToBase40(output);
			base40 += m_scoreDef.staves[m_currentStaff-1].base40;
			output = Convert::base40ToKern(base40);
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getDuration -- Get duration from note or chord.  If chord does not
//    have @dur then use @dur of first note in children elements.
//

HumNum Tool_mei2hum::getDuration(xml_node element) {
	xml_attribute dur_attr = element.attribute("dur");
	string name = element.name();
	if ((!dur_attr) && (name == "note")) {
		// real notes must have durations, but this one
		// does not, so assign zero duration
		return 0;
	}
	if ((!dur_attr) && (name == "chord")) {
		// if there is no dur attribute on a chord, then look for it
		// on the first note subelement of the chord.
		auto newelement = element.select_node(".//note").node();
		if (newelement) {
			element = newelement;
			dur_attr = element.attribute("dur");
			name = element.name();
		} else {
			return 0;
		}
	}

	string dur = dur_attr.value();
	if (dur == "") {
		return 0;
	}

	HumNum output;
	if (dur == "breve") {
		output = 2;
	} else if (dur == "long") {
		output = 4;
	} else if (dur == "maxima") {
		output = 8;
	} else if (isdigit(dur[0])) {
		output = 1;
		output /= stoi(dur);
	} else {
		cerr << "Unknown " << element.name() << "@dur: " << dur << endl;
		return 0;
	}

	if (output == 0) {
		cerr << "Error: zero duration for note" << endl;
	}

	int dotcount;
	string dots = element.attribute("dots").value();
	if (dots == "") {
		dotcount = 0;
	} else if (isdigit(dots[0])) {
		dotcount = stoi(dots);
	} else {
		cerr << "Unknown " << element.name() << "@dotcount: " << dur << endl;
		return 0;
	}

	if (dotcount > 0) {
		int top = (1 << (dotcount+1)) - 1;
		int bot = 1 << dotcount;
		HumNum dotfactor(top, bot);
		output *= dotfactor;
	}

	// add a correction for the tuplet factor which is currently active.
	if (m_tupletfactor != 1) {
		output *= m_tupletfactor;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getDuration_mensural -- Get duration from note or chord.  If chord does not
//    have @dur then use @dur of first note in children elements.
//
// @dur: https://music-encoding.org/guidelines/v4/data-types/data.duration.mensural.html
//          X = maxima
//          L = longa
//          S = brevis
//          s = semibrevis
//          M = minima
//          m = semiminima
//          U = fusa
//          u = semifusa
// @dur.quality:
//          i = imperfecta  :: remove augmentation dot
//          p = perfecta    :: add augmentation dot
//          altera = altera :: duration is double the rhythmic value of notes
//

HumNum Tool_mei2hum::getDuration_mensural(xml_node element, int& dotcount) {
	dotcount = 0;

	xml_attribute dur_qual = element.attribute("dur.quality");
	xml_attribute dur_attr = element.attribute("dur");
	string name = element.name();

	if ((!dur_attr) && (name == "note")) {
		// real notes must have durations, but this one
		// does not, so assign zero duration
		return 0;
	}
	if ((!dur_attr) && (name == "chord")) {
		// if there is no dur attribute on a chord, then look for it
		// on the first note subelement of the chord.
		auto newelement = element.select_node(".//note").node();
		if (newelement) {
			element = newelement;
			dur_attr = element.attribute("dur");
			name = element.name();
			dur_qual = element.attribute("dur.quality");
		} else {
			return 0;
		}
	}

	string dur = dur_attr.value();
	if (dur == "") {
		return 0;
	}
	string durquality = dur_qual.value();

	char rhythm = '\0';
	if (dur == "maxima") {
		rhythm = 'X';
	} else if (dur == "longa") {
		rhythm = 'L';
	} else if (dur == "brevis") {
		rhythm = 'S';
	} else if (dur == "semibrevis") {
		rhythm = 's';
	} else if (dur == "minima") {
		rhythm = 'M';
	} else if (dur == "semiminima") {
		rhythm = 'm';
	} else if (dur == "fusa") {
		rhythm = 'U';
	} else if (dur == "semifusa") {
		rhythm = 'u';
	} else {
		cerr << "Error: unknown rhythm" << element.name() << "@dur: " << dur << endl;
		return 0;
	}

	mei_staffDef& ss = m_scoreDef.staves.at(m_currentStaff - 1);
	int maximodus = ss.maximodus == 3 ? 3 : 2;
	int modus     = ss.modus     == 3 ? 3 : 2;
	int tempus    = ss.tempus    == 3 ? 3 : 2;
	int prolatio  = ss.prolatio  == 3 ? 3 : 2;

	bool altera     = false;
	bool perfecta   = false;
	bool imperfecta = false;

	if (durquality == "imperfecta") {
		imperfecta = true;
	} else if (durquality == "perfecta") {
		perfecta = true;
	} else if (durquality == "altera") {
		altera = true;
	}

	HumNum output = Convert::mensToDuration(rhythm, altera, perfecta, imperfecta, maximodus, modus, tempus, prolatio);
	return output;
}




//////////////////////////////
//
// Tool_mei2hum::parseVerse --
//

void Tool_mei2hum::parseVerse(xml_node verse, GridStaff* staff) {
	NODE_VERIFY(verse, )
	MAKE_CHILD_LIST(children, verse);

	string n = verse.attribute("n").value();
	int nnum = 1;
	if (n.empty()) {
		cerr << "Warning: no layer number on layer element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Warning: invalid layer number: " << nnum << endl;
		cerr << "Setting it to 1." << endl;
		nnum = 1;
	}

	string versetext;
	int sylcount = 0;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "syl") {
			if (sylcount > 0) {
				versetext += " ";
			}
			sylcount++;
			versetext += parseSyl(children[i]);
		} else {
			cerr << DKHTP << verse.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (versetext == "") {
		// nothing to store
		return;
	}

	staff->setVerse(nnum-1, versetext);
	reportVerseNumber(nnum, m_currentStaff-1);

	return;
}



//////////////////////////////
//
// Tool_mei2hum::parseBareSyl -- Only one syl allows as a bar child of note element.
//     This function is used to process syl elements that are not wrapped in a verse element.
//

void Tool_mei2hum::parseBareSyl(xml_node syl, GridStaff* staff) {
	NODE_VERIFY(syl, )

	int nnum = 1;
	xml_attribute n_attr = syl.attribute("n");
	if (n_attr) {
		nnum = n_attr.as_int();
	}

	if (nnum < 1) {
		cerr << "Warning: invalid layer number: " << nnum << endl;
		cerr << "Setting it to 1." << endl;
		nnum = 1;
	}

	string versetext = parseSyl(syl);

	if (versetext == "") {
		// nothing to store
		return;
	}

	staff->setVerse(nnum-1, versetext);
	reportVerseNumber(nnum, m_currentStaff-1);

	return;
}



//////////////////////////////
//
// Tool_mei2hum::reportVerseNumber --
//

void Tool_mei2hum::reportVerseNumber(int pmax, int staffindex) {
	if (staffindex < 0) {
		return;
	}
	if (staffindex >= (int)m_maxverse.size()) {
		return;
	}
	if (m_maxverse.at(staffindex) < pmax) {
		m_maxverse[staffindex] = pmax;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseSyl --
//

string Tool_mei2hum::parseSyl(xml_node syl) {
	NODE_VERIFY(syl, "")
	MAKE_CHILD_LIST(children, syl);

	string text = syl.child_value();
	for (int i=0; i<(int)text.size(); i++) {
		if (text[i] == '_') {
			text[i] = ' ';
		}
	}

	string wordpos = syl.attribute("wordpos").value();
	if (wordpos == "i") {
		text = text + "-";
	} else if (wordpos == "m") {
		text = "-" + text + "-";
	} else if (wordpos == "t") {
		text = "-" + text;
	}

	return text;
}



//////////////////////////////
//
// Tool_mei2hum::parseClef --
//
//

void Tool_mei2hum::parseClef(xml_node clef, HumNum starttime) {
	NODE_VERIFY(clef, )

	string shape = clef.attribute("shape").value();
	string line = clef.attribute("line").value();
	string clefdis = clef.attribute("clef.dis").value();
	string clefdisplace = clef.attribute("clef.dis.place").value();

	string tok = makeHumdrumClef(shape, line, clefdis, clefdisplace);

	m_outdata.back()->addClefToken(tok, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, 0, m_staffcount);

}



//////////////////////////////
//
// Tool_mei2hum::makeHumdrumClef --
//
// Example:
//     <clef shape="G" line="2" clef.dis="8" clef.dis.place="below" />
//

string Tool_mei2hum::makeHumdrumClef(const string& shape,
		const string& line, const string& clefdis, const string& clefdisplace) {
	string output = "*clef" + shape;
	if (!clefdis.empty()) {
		int number = stoi(clefdis);
		int count = 0;
		if (number == 8) {
			count = 1;
		} else if (number == 15) {
			count = 2;
		}
		if (clefdisplace != "above") {
			count = -count;
		}
		switch (count) {
			case 1: output += "^"; break;
			case 2: output += "^^"; break;
			case -1: output += "v"; break;
			case -2: output += "vv"; break;
		}
	}
	output += line;
	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseChord --
//

HumNum Tool_mei2hum::parseChord(xml_node chord, HumNum starttime, int gracenumber) {
	NODE_VERIFY(chord, starttime)
	MAKE_CHILD_LIST(children, chord);

	processPreliminaryLinkedNodes(chord);

	HumNum duration = getDuration(chord);

	string tok;
	int counter = 0;
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			counter++;
			if (counter > 1) {
				tok += " ";
			}
			parseNote(children[i], chord, tok, starttime, gracenumber);
		} else if (nodename == "artic") {
			// This is handled within parseNote();
		} else {
			cerr << DKHTP << chord.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	m_fermata = false;
	processLinkedNodes(tok, chord);
	if (!m_fermata) {
		processFermataAttribute(tok, chord);
	}

	m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT, m_currentStaff-1,
		0, m_currentLayer-1, m_staffcount);

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::getChildrenVector -- Return a list of all children elements
//   of a given element.  Pugixml does not allow random access, but storing
//   them in a vector allows that possibility.
//

void Tool_mei2hum::getChildrenVector(vector<xml_node>& children,
		xml_node parent) {
	children.clear();
	for (xml_node child : parent.children()) {
		children.push_back(child);
	}
}



//////////////////////////////
//
// Tool_mei2hum::initialize -- Setup for the tool, mostly parsing command-line
//   (input) options.
//

void Tool_mei2hum::initialize(void) {
	m_recipQ   =  getBoolean("recip");
	m_stemsQ   =  getBoolean("stems");
	m_xmlidQ   =  getBoolean("xmlids");
	m_xmlidQ   = 1;  // for testing
	m_appLabel =  getString("app-label");
	m_placeQ   = !getBoolean("no-place");
}



//////////////////////////////
//
// Tool_mei2hum::buildIdLinkMap -- Build table of startid and endid links between elements.
//
// Reference: https://pugixml.org/docs/samples/traverse_walker.cpp
//

void Tool_mei2hum::buildIdLinkMap(xml_document& doc) {
	class linkmap_walker : public pugi::xml_tree_walker {
		public:
			virtual bool for_each(pugi::xml_node& node) {
				xml_attribute startid = node.attribute("startid");
				xml_attribute endid = node.attribute("endid");
				if (startid) {

					string value = startid.value();
					if (!value.empty()) {
						if (value[0] == '#') {
							value = value.substr(1, string::npos);
						}
					}
					if (!value.empty()) {
						(*startlinks)[value].push_back(node);
					}

				}
				if (endid) {

					string value = endid.value();
					if (!value.empty()) {
						if (value[0] == '#') {
							value = value.substr(1, string::npos);
						}
					}
					if (!value.empty()) {
						(*stoplinks)[value].push_back(node);
					}

				}
				return true; // continue traversal
			}

			map<string, vector<xml_node>>* startlinks = NULL;
			map<string, vector<xml_node>>* stoplinks = NULL;
	};

	m_startlinks.clear();
	m_stoplinks.clear();
	linkmap_walker walker;
	walker.startlinks = &m_startlinks;
	walker.stoplinks = &m_stoplinks;
	doc.traverse(walker);
}



//////////////////////////////
//
// Tool_mei2hum::parseDir -- Meter cannot change in middle of measure.
//     Need to implement @startid version.
//
// Example:
//    <dir xml:id="dir-L408F3" place="below" staff="1" tstamp="2.0">con espressione</dir>
//
// or with normal font specified:
//    <dir xml:id="dir-L7F1" staff="1" tstamp="2.000000">
//       <rend xml:id="rend-0000001696821523" fontstyle="normal">test</rend>
//    </dir>
//
// bold font:
//   <dir xml:id="dir-L25F3" place="above" staff="1" tstamp="3.000000">
//      <rend xml:id="rend-0000001714819172" fontstyle="normal" fontweight="bold">comment</rend>
//   </dir>
//

void Tool_mei2hum::parseDir(xml_node dir, HumNum starttime) {
	NODE_VERIFY(dir, )
	MAKE_CHILD_LIST(children, dir);

	string font = "i";  // italic by default in verovio

	string placement = ""; // a = above, b = below

	string place = dir.attribute("place").value();
	if (place == "above") {
		placement = "a:";
	}
	// Below is the default in Humdrum layout commands.

	string text;

	if (!children.empty()) { // also includes the above text node, but only looking at <rend>.
		int count = 0;
		for (int i=0; i<(int)children.size(); i++) {
			string nodename = children[i].name();
			if (nodename == "rend") {
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].child_value();
				if (strcmp(children[i].attribute("fontstyle").value(), "normal") == 0) {
					font = "";  // normal is default in Humdrum layout
				}
				if (strcmp(children[i].attribute("fontweight").value(), "bold") == 0) {
					font += "B";  // normal is default in Humdrum layout
				}
			} else if (nodename == "") {
				// text node
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].value();
			} else {
				cerr << DKHTP << dir.name() << "/" << nodename << CURRLOC << endl;
			}
		}
	}

	if (text.empty()) {
		return;
	}

	string message = "!LO:TX:";
	message += placement;
	if (!font.empty()) {
		message += font + ":";
	}
	message += "t=" + cleanDirText(text);

	string ts = dir.attribute("tstamp").value();
	if (ts.empty()) {
		cerr << "Error: no timestamp on dir element and can't currently processes with @startid." << endl;
		return;
	}

	xml_attribute atstaffnum = dir.attribute("staff");
	if (!atstaffnum) {
		cerr << "Error: staff number required on dir element in measure "
		     << m_currentMeasure  << " (ignoring text: " << cleanWhiteSpace(text) << ")" << endl;
		return;
	}
	int staffnum = dir.attribute("staff").as_int();
	if (staffnum <= 0) {
		cerr << "Error: staff number on dir element in measure should be positive.\n";
		cerr << "Instead the staff number is: " << m_currentMeasure  << " (ignoring text: " <<  cleanWhiteSpace(text) << ")" << endl;
		return;
	}

	double meterunit = m_currentMeterUnit[staffnum - 1];
	double tsd = (stof(ts)-1) * 4.0 / meterunit;

	GridMeasure* gm = m_outdata.back();
	double tsm = gm->getTimestamp().getFloat();
	bool foundslice = false;
	GridSlice* gs;
	for (auto gsit = gm->begin(); gsit != gm->end(); gsit++) {
		gs = *gsit;
		if (!gs->isDataSlice()) {
			continue;
		}
		double gsts = gs->getTimestamp().getFloat();
		double difference = (gsts-tsm) - tsd;
		if (!(fabs(difference) < 0.0001)) {
			continue;
		}
		// GridVoice* voice = gs->at(staffnum-1)->at(0)->at(0);
		// HTp token = voice->getToken();
		// if (token != NULL) {
		// 	token->setValue("LO", "TX", "t", text);
		// } else {
		// 	cerr << "Strange null-token error while inserting dir element." << endl;
		// }
		foundslice = true;

		// Found data line which should prefixed with a layout line
		// should be done with HumHash post-processing, but do it manually for now.

		auto previousit = gsit;
		previousit--;
		if (previousit == gm->end()) {
			previousit = gsit;
		}
		auto previous = *previousit;
		if (previous->isLayoutSlice()) {
			GridVoice* voice = previous->at(staffnum-1)->at(0)->at(0);
			HTp tok = voice->getToken();
			if (tok == NULL) {
				HTp newtok = new HumdrumToken(message);
				voice->setToken(newtok);
				tok = voice->getToken();
				break;
			} else if (tok->isNull()) {
				tok->setText(message);
				break;
			}
		}

		// Insert a layout slice in front of current data slice.
		GridSlice* ngs = new GridSlice(gm, gs->getTimestamp(), SliceType::Layouts, m_maxStaffInFile);
		int parti = staffnum - 1;
		int staffi = 0;
		int voicei = 0;
		ngs->addToken(message, parti, staffi, voicei);
		gm->insert(gsit, ngs);

		break;
	}
	if (!foundslice) {
		cerr << "Warning: dir elements not occuring at note/rest times are not yet supported" << endl;
	}
}



//////////////////////////////
//
// Tool_mei2hum::cleanWhiteSpace -- Convert newlines to "\n", and trim spaces.
//    Also remove more than one space in a row.
//

string Tool_mei2hum::cleanWhiteSpace(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == '\t') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else if (input[i] == '\n') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else if (input[i] == ' ') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else {
			output += input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::cleanDirText -- convert ":" to "&colon;".
//     Remove tabs and newlines, and trim spaces.  Maybe allow
//     newlines using "\n" and allow font changes in the future.
//     Remove redundant whitespace. Do accents later perhaps or
//     monitor for UTF-8.
//

string Tool_mei2hum::cleanDirText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == ':') {
			output += "&colon;";
		} else if (input[i] == '\t') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else if (input[i] == '\n') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else if (input[i] == ' ') {
			if ((!output.empty()) && (output.back() != ' ')) {
				output += ' ';
			}
		} else {
			output += input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::cleanVerseText --
//     Remove tabs and newlines, and trim spaces.
//     Do accents later perhaps or monitor for UTF-8.
//

string Tool_mei2hum::cleanVerseText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == '\t') {
			output += ' ';
		} else if (input[i] == '\n') {
			output += ' ';
		} else {
			output += input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::cleanReferenceRecordText -- convert ":" to "&colon;".
//     Remove tabs and newlines, and trim spaces.  Maybe allow
//     newlines using "\n" and allow font changes in the future.
//     Do accents later perhaps or monitor for UTF-8.
//

string Tool_mei2hum::cleanReferenceRecordText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	char lastchar = '\0';
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == '\n') {
			if (lastchar != ' ') {
				output += ' ';
			}
			lastchar = ' ';
		} else if (input[i] == '\t') {
			if (lastchar != ' ') {
				output += ' ';
			}
			lastchar = ' ';
		} else {
			output += input[i];
			lastchar = input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseTempo --
//
// Example:
//   <tempo tstamp="1" place="above" staff="1">
//      1 - Allegro con spirito <rend fontname="VerovioText">&#xE1D5;</rend> = 132
//   </tempo>
//
//
// Ways of indicating tempo:
//
// tempo@midi.bpm == tempo per quarter note (Same as Humdrum *MM value)
//
// tempo@midi.mspb == microseconds per quarter note ( bpm = mspb * 60 / 1000000)
//
// tempo@mm == tempo per beat (bpm = mm / unit(dots))
// tempo@mm.unit == beat unit for tempo@mm
// tempo@mm.dots == dots for tempo@unit
//
// Free-form text:
//
// &#xE1D5; == quarter note
//
// #define SMUFL_QUARTER_NOTE "\ue1d5"

void Tool_mei2hum::parseTempo(xml_node tempo, HumNum starttime) {
	NODE_VERIFY(tempo, )

	bool found = false;
	double value = 0.0;

	xml_attribute bpm = tempo.attribute("bpm");
	if (bpm) {
		value = bpm.as_double();
		if (value > 0.0) {
			found = true;
		}
	}

	if (!found) {
		xml_attribute mspb   = tempo.attribute("mspb");
		value = mspb.as_double() * 60.0 / 1000000.0;
		if (value > 0.0) {
			found = true;
		}
	}

	if (!found) {
		xml_attribute mm     = tempo.attribute("mm");
		xml_attribute mmunit = tempo.attribute("mm.unit");
		xml_attribute mmdots = tempo.attribute("mm.dots");
		value = mm.as_double();
		string recip = mmunit.value();
		int dcount = mmdots.as_int();
		for (int i=0; i<dcount; i++) {
			recip += '.';
		}
		HumNum duration = Convert::recipToDuration(recip);
		value *= duration.getFloat();
		if (value > 0.0) {
			found = true;
		}
	}

	if (!found) {
		// search for free-form tempo marking.  Something like:
		//   <tempo tstamp="1" place="above" staff="1">
		//      1 - Allegro con spirito <rend fontname="VerovioText">&#xE1D5;</rend> = 132
		//   </tempo>
		//
		// UTF-8 version in string "\ue1d5";
		string text;

		MAKE_CHILD_LIST(children, tempo);
		for (int i=0; i<(int)children.size(); i++) {
			if (children[i].type() == pugi::node_pcdata) {
				text += children[i].value();
			} else {
				text += children[i].child_value();
			}
			text += " ";

		}
		HumRegex hre;
		// #define SMUFL_QUARTER_NOTE "\ue1d5"
		// if (hre.search(text, SMUFL_QUARTER_NOTE "\\s*=\\s*(\\d+\\.?\\d*)")) {
		if (hre.search(text, "\\s*=\\s*(\\d+\\.?\\d*)")) {
			// assuming quarter note for now.
			value = hre.getMatchDouble(1);
			found = true;
		}
		// further rhythmic values for tempo should go here.
	}

	// also deal with tempo designiations such as "Allegro"...

	if (!found) {
		// no tempo to set
		return;
	}

	// insert tempo
	GridMeasure* gm = m_outdata.back();
	GridSlice* gs = new GridSlice(gm, starttime, SliceType::Tempos, m_maxStaffInFile);
	stringstream stok;
	stok << "*MM" << value;
	string token = stok.str();

	for (int i=0; i<m_maxStaffInFile; i++) {
		gs->at(i)->at(0)->at(0)->setToken(token);
	}

	// insert after time signature at same timestamp if possible
	bool inserted = false;
	for (auto it = gm->begin(); it != gm->end(); it++) {
		if ((*it)->getTimestamp() > starttime) {
			gm->insert(it, gs);
			inserted = true;
			break;
		} else if ((*it)->isTimeSigSlice()) {
			it++;
			gm->insert(it, gs);
			inserted = true;
			break;
		} else if (((*it)->getTimestamp() == starttime) && ((*it)->isNoteSlice()
				|| (*it)->isGraceSlice())) {
			gm->insert(it, gs);
			inserted = true;
			break;
		}
	}

	if (!inserted) {
		gm->push_back(gs);
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseHarm -- Not yet ready to convert <harm> data.
//    There will be different types of harm (such as figured bass), which
//    will need to be subcategorized into different datatypes, such as
//    *fb for figured bass.  Also free-text can be present in <harm>
//    data, so the current datatype for that is **cdata  (meaning chord-like
//    data that will be mapped back into <harm> which converting back to
//    MEI data.
//
// Example:
//     <harm staff="1" tstamp="1.000000">C major</harm>
//

void Tool_mei2hum::parseHarm(xml_node harm, HumNum starttime) {
	NODE_VERIFY(harm, )
	MAKE_CHILD_LIST(children, harm);

	string text = harm.child_value();

	if (text.empty()) { // looking at <rend> sub-elements
		int count = 0;
		for (int i=0; i<(int)children.size(); i++) {
			string nodename = children[i].name();
			if (nodename == "rend") {
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].child_value();
				//if (strcmp(children[i].attribute("fontstyle").value(), "normal") == 0) {
				//	font = "";  // normal is default in Humdrum layout
				//}
				//if (strcmp(children[i].attribute("fontweight").value(), "bold") == 0) {
				//	font += "B";  // normal is default in Humdrum layout
				//}
			} else if (nodename == "") {
				// text node
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].value();
			} else {
				cerr << DKHTP << harm.name() << "/" << nodename << CURRLOC << endl;
			}
		}
	}

	if (text.empty()) {
		return;
	}

   // cerr << "FOUND HARM DATA " << text << endl;

/*

	string startid = harm.attribute("startid").value();

	int staffnum = harm.attribute("staff").as_int();
	if (staffnum == 0) {
		cerr << "Error: staff number required on harm element" << endl;
		return;
	}
	double meterunit = m_currentMeterUnit[staffnum - 1];

	if (!startid.empty()) {
		// Harmony is (or at least should) be attached directly
		// do a note, so it is handled elsewhere.
		cerr << "Warning DYNAMIC " << text << " is not yet processed." << endl;
		return;
	}

	string ts = harm.attribute("tstamp").value();
	if (ts.empty()) {
		cerr << "Error: no timestamp on harm element" << endl;
		return;
	}
	double tsd = (stof(ts)-1) * 4.0 / meterunit;
	double tolerance = 0.001;
	GridMeasure* gm = m_outdata.back();
	double tsm = gm->getTimestamp().getFloat();
	bool foundslice = false;
	GridSlice *nextgs = NULL;
	for (auto gs : *gm) {
		if (!gs->isDataSlice()) {
			continue;
		}
		double gsts = gs->getTimestamp().getFloat();
		double difference = (gsts-tsm) - tsd;
		if (difference < tolerance) {
			// did not find data line at exact timestamp, so move
			// the harm to the next event. Need to think about adding
			// a new timeslice for the harm when it is not attached to
			// a note.
			nextgs = gs;
			break;
		}
		if (!(fabs(difference) < tolerance)) {
			continue;
		}
		GridPart* part = gs->at(staffnum-1);
		part->setHarmony(text);
		m_outdata.setHarmonyPresent(staffnum-1);
		foundslice = true;
		break;
	}
	if (!foundslice) {
		if (nextgs == NULL) {
			cerr << "Warning: harmony not attched to system events "
					<< "are not yet supported in measure " << m_currentMeasure << endl;
		} else {
			GridPart* part = nextgs->at(staffnum-1);
			part->setHarmony(text);
			m_outdata.setHarmonyPresent(staffnum-1);
			// Give a time offset for displaying the harmmony here.
		}
	}
*/

}



//////////////////////////////
//
// Tool_mei2hum::parseDynam --
//
// Example:
//     <dynam staff="1" tstamp="1.000000">p</dynam>
//

void Tool_mei2hum::parseDynam(xml_node dynam, HumNum starttime) {
	NODE_VERIFY(dynam, )
	MAKE_CHILD_LIST(children, dynam);

	string text = dynam.child_value();

	if (text.empty()) { // looking at <rend> sub-elements
		int count = 0;
		for (int i=0; i<(int)children.size(); i++) {
			string nodename = children[i].name();
			if (nodename == "rend") {
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].child_value();
				//if (strcmp(children[i].attribute("fontstyle").value(), "normal") == 0) {
				//	font = "";  // normal is default in Humdrum layout
				//}
				//if (strcmp(children[i].attribute("fontweight").value(), "bold") == 0) {
				//	font += "B";  // normal is default in Humdrum layout
				//}
			} else if (nodename == "") {
				// text node
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].value();
			} else {
				cerr << DKHTP << dynam.name() << "/" << nodename << CURRLOC << endl;
			}
		}
	}

	if (text.empty()) {
		return;
	}

	string startid = dynam.attribute("startid").value();

	int staffnum = dynam.attribute("staff").as_int();
	if (staffnum == 0) {
		cerr << "Error: staff number required on dynam element" << endl;
		return;
	}
	double meterunit = m_currentMeterUnit[staffnum - 1];

	if (!startid.empty()) {
		// Dynamic is (or at least should) be attached directly
		// do a note, so it is handled elsewhere.
		cerr << "Warning DYNAMIC " << text << " is not yet processed." << endl;
		return;
	}

	string ts = dynam.attribute("tstamp").value();
	if (ts.empty()) {
		cerr << "Error: no timestamp on dynam element" << endl;
		return;
	}
	double tsd = (stof(ts)-1) * 4.0 / meterunit;
	double tolerance = 0.001;
	GridMeasure* gm = m_outdata.back();
	double tsm = gm->getTimestamp().getFloat();
	bool foundslice = false;
	GridSlice *nextgs = NULL;
	for (auto gs : *gm) {
		if (!gs->isDataSlice()) {
			continue;
		}
		double gsts = gs->getTimestamp().getFloat();
		double difference = (gsts-tsm) - tsd;
		if (difference < tolerance) {
			// did not find data line at exact timestamp, so move
			// the dynamic to the next event. Maybe think about adding
			// a new timeslice for the dynamic.
			nextgs = gs;
			break;
		}
		if (!(fabs(difference) < tolerance)) {
			continue;
		}
		GridPart* part = gs->at(staffnum-1);
		part->setDynamics(text);
		m_outdata.setDynamicsPresent(staffnum-1);
		foundslice = true;
		break;
	}
	if (!foundslice) {
		if (nextgs == NULL) {
			cerr << "Warning: dynamics not attched to system events "
					<< "are not yet supported in measure " << m_currentMeasure << endl;
		} else {
			GridPart* part = nextgs->at(staffnum-1);
			part->setDynamics(text);
			m_outdata.setDynamicsPresent(staffnum-1);
			// Give a time offset for displaying the dynamic here.
		}
	}
}



// END_MERGE

} // end namespace hum


