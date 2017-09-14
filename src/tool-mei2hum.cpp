//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:58:26 PDT 2017
// Last Modified: Wed Sep 13 14:58:29 PDT 2017
// Filename:      mei2hum.cpp
// URL:           https://github.com/craigsapp/mei2hum/blob/master/src/mei2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "tool-mei2hum.h"
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
// Tool_mei2hum::Tool_mei2hum --
//

Tool_mei2hum::Tool_mei2hum(void) {
	// Options& options = m_options;
	// options.define("k|kern=b","display corresponding **kern data");

	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");

}



//////////////////////////////
//
// Tool_mei2hum::convert -- Convert a MusicXML file into
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
	auto result = doc.load(input);
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

	auto score = doc.select_single_node("/mei/music/body/mdiv/score").node();

	if (!score) {
		cerr << "Cannot find score, so cannot convert MEI file to Humdrum" << endl;
		return false;
	}

	HumGrid outdata;
	if (m_recipQ) {
		outdata.enableRecipSpine();
	}

	HumNum systemstamp = 0;  // timestamp for music.
	systemstamp = parseScore(outdata, score, systemstamp);


	outdata.removeRedundantClefChanges();
	outdata.removeSibeliusIncipit();

	// set the duration of the last slice

	HumdrumFile outfile;

	outdata.transferTokens(outfile);

	// addHeaderRecords(outfile, doc);
	// addFooterRecords(outfile, doc);

	for (int i=0; i<outfile.getLineCount(); i++) {
		outfile[i].createLineFromTokens();
	}
	out << outfile;

	return status;
}



///////////////////////////////////
//
// Tool_mei2hum::parseScore -- Convert an MEI <score> element into Humdrum data.
//

HumNum Tool_mei2hum::parseScore(HumGrid& outdata, xml_node score, HumNum starttime) {
	if (!score) {
		return starttime;
	}
	if (strcmp(score.name(), "score") != 0) {
		return starttime;
	}

	vector<xml_node> children;
	getChildrenVector(children, score);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "scoreDef") {
			parseScoreDef(outdata, item, starttime);
		} else if (nodename == "section") {
			starttime = parseSection(outdata, item, starttime);
		} else {
			cerr << "Do not know how to process " << nodename << " elements";
			cerr << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseScoreDef -- Process a <scoreDef> element in an MEI file.
//

void Tool_mei2hum::parseScoreDef(HumGrid& outdata, xml_node scoredef,
		HumNum starttime) {
	if (!scoredef) {
		return;
	}
	if (strcmp(scoredef.name(), "scoreDef") != 0) {
		return;
	}

	if (m_scoredef.global.timestamp == starttime) {
		m_scoredef.clear();
	}
	m_scoredef.global.timestamp = starttime;

	vector<xml_node> children;
	getChildrenVector(children, scoredef);

	fillWithStaffDefAttributes(m_scoredef.global, scoredef);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    processStaffGrp(outdata, item, starttime);
		} else if (nodename == "staffDef") {
		    processStaffDef(outdata, item, starttime);
		} else {
			cerr << "Do not know how to process scoreDef/" << nodename;
			cerr << " elements";
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseStaffGrp -- Process a <staffGrp> element in an MEI file.
//

void Tool_mei2hum::processStaffGrp(HumGrid& outdata, xml_node staffgrp,
		HumNum starttime) {

	if (!staffgrp) {
		return;
	}
	if (strcmp(staffgrp.name(), "staffGrp") != 0) {
		return;
	}

	vector<xml_node> children;
	getChildrenVector(children, staffgrp);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    processStaffGrp(outdata, item, starttime);
		} else if (nodename == "staffDef") {
		    processStaffDef(outdata, item, starttime);
		} else {
			cerr << "Do not know how to process scoreDef/" << nodename;
			cerr << " elements";
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseStaffDef -- Process a <staffDef> element in an MEI file.
//

void Tool_mei2hum::processStaffDef(HumGrid& outdata, xml_node staffdef,
		HumNum starttime) {

	if (!staffdef) {
		return;
	}
	if (strcmp(staffdef.name(), "staffDef") != 0) {
		return;
	}


	string staffnum = staffdef.attribute("n").value();
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

	m_scoredef.minresize(num);

	cerr << "STAFF " << num << endl;

	m_scoredef.staves[num].clear();
	m_scoredef.staves[num] = m_scoredef.global;

	fillWithStaffDefAttributes(m_scoredef.staves[num], staffdef);

}



//////////////////////////////
//
// Tool_mei2hum::fillWithStaffDefAttributes --
//

void Tool_mei2hum::fillWithStaffDefAttributes(mei_staffdef& staffinfo,
		xml_node element) {

	string clefshape;
	string clefline;
	string metercount;
	string meterunit;
	string staffnum;
	string keysig;
	string midibpm;

	for (auto atti = element.attributes_begin(); atti != element.attributes_end();
				atti++) {
		string attname = atti->name();
		if (attname == "clef.shape") {
			clefshape = atti->value();
		} else if (attname == "clef.line") {
			clefline = atti->value();
		} else if (attname == "meter.count") {
			metercount = atti->value();
		} else if (attname == "meter.unit") {
			meterunit = atti->value();
		} else if (attname == "key.sig") {
			keysig = atti->value();
		} else if (attname == "midi.bpm") {
			midibpm = atti->value();
		}
	}

	if ((!clefshape.empty()) && (!clefline.empty())) {
		staffinfo.clef = clefshape + clefline;
		cerr << "\tCLEF IS *clef" << staffinfo.clef << endl;
	}
	if ((!metercount.empty()) && (!meterunit.empty())) {
		staffinfo.timesig = "*M" + metercount + "/" + meterunit;
		cerr << "\tTIMESIG IS " << staffinfo.timesig << endl;
	}
	if (!keysig.empty()) {
		int count = stoi(keysig);
		int accid = 0;
		if (keysig.find("s") != string::npos) {
			accid = +1;
		} else if (keysig.find("f") != string::npos) {
			accid = -1;
		}

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
		cerr << "\tKEYSIG IS " << staffinfo.keysig << endl;
	}
	if (!midibpm.empty()) {
		staffinfo.midibpm = "*MM" + midibpm;
		cerr << "\tTEMPO IS " << staffinfo.midibpm << endl;
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseSection -- Process a <section> element in an MEI file.
//

HumNum Tool_mei2hum::parseSection(HumGrid& outdata, xml_node section,
		HumNum starttime) {
	if (!section) {
		return starttime;
	}
	if (strcmp(section.name(), "section") != 0) {
		return starttime;
	}

	vector<xml_node> children;
	getChildrenVector(children, section);
	cerr << "PARSING SECTION " << endl;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << "\tPARSING section/" << nodename << endl;
		if (nodename == "section") {
			starttime = parseSection(outdata, children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(outdata, children[i], starttime);
		} else {
			cerr << "Don't know how to parse element " << nodename << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseMeasure --
//

HumNum Tool_mei2hum::parseMeasure(HumGrid& outdata, xml_node measure, HumNum starttime) {
	cerr << "\tparsing MEASURE" << endl;

	vector<xml_node> children;
	getChildrenVector(children, measure);
cerr << "CHILDREN SIZE " << children.size() << endl;

	vector<HumNum> durations(children.size(), 0);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "staff") {
			durations[i] = parseStaff(outdata, children[i], starttime);
		} else {
			cerr << "Do not know how to parse measure/" << nodename << endl;
		}
	}

	return 0;
}



//////////////////////////////
//
// Tool_mei2hum::parseStaff --
//

HumNum Tool_mei2hum::parseStaff(HumGrid& outdata, xml_node staff, HumNum starttime) {
	cerr << "PARSING STAFF" << endl;

	vector<xml_node> children;
	getChildrenVector(children, staff);

	vector<HumNum> durations(children.size(), 0);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "layer") {
			durations[i] = parseLayer(outdata, children[i], starttime);
		} else {
			cerr << "Do not know how to parse measure/staff/" << nodename << endl;
		}
	}

	return 0;
}



//////////////////////////////
//
// Tool_mei2hum::parseLayer --
//

HumNum Tool_mei2hum::parseLayer(HumGrid& outdata, xml_node layer, HumNum starttime) {
	cerr << "PARSING LAYER" << endl;

	vector<xml_node> children;
	getChildrenVector(children, layer);

	vector<HumNum> durations(children.size(), 0);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << "processing measure/staff/layer/" << nodename << endl;
	}

	return 0;
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
	m_recipQ = getBoolean("recip");
	m_stemsQ = getBoolean("stems");
}


// END_MERGE

} // end namespace hum


