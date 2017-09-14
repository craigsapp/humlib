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

	m_scoredef.clear();
	m_scoredef.global.timestamp = starttime;

	vector<xml_node> children;
	getChildrenVector(children, scoredef);

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
	cerr << "\tProcessing staffgrp " << endl;

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
	cerr << "\t\tProcessing staffdef " << endl;

	if (!staffdef) {
		return;
	}
	if (strcmp(staffdef.name(), "staffDef") != 0) {
		return;
	}

	string clefshape;
	string clefline;
	string metercount;
	string meterunit;
	string staffnum;

	for (auto atti = staffdef.attributes_begin(); atti != staffdef.attributes_end();
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
		} else if (attname == "n") {
			staffnum = atti->value();
		}
	}

	if ((!clefshape.empty()) && (!clefline.empty())) {
		cerr << "CLEF IS *clef" << clefshape << clefline << endl;
	}
	if ((!metercount.empty()) && (!meterunit.empty())) {
		cerr << "TIMESIG IS *M" << metercount << "/" << meterunit << endl;
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

	cerr << "PARSING SECTION " << endl;

	return starttime;
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


