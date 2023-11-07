//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Sep 28 12:08:25 PDT 2020
// Last Modified: Mon Sep 28 13:10:14 PDT 2020
// Filename:      tool-colorgroups.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-colorgroups.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color groups.  Groups are indicated by the interpretation *grp: followed by the group label.
//                Known groups to the filter:
//                   *grp:A
//                   *grp:B
//                   *grp:C
//                Maybe allow more groups and allow for *Xgrp: to turn off a group.
//
//                Essentially the commands:
//                   shed -e "s/grp:A/color:crimson/I; s/grp:B/color:dodgerblue/I; s/grp:C/color:purple/I"
//

#include "tool-colorgroups.h"
#include "tool-shed.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_colorgroups::Tool_colorgroups -- Set the recognized options for the tool.
//

Tool_colorgroups::Tool_colorgroups(void) {
	define("A=s:crimson",    "Color for group A");
	define("B=s:dodgerblue", "Color for group B");
	define("C=s:purple",     "Color for group C");
	define("command=b",     "print shed command only");
}



/////////////////////////////////
//
// Tool_colorgroups::run -- Do the main work of the tool.
//

bool Tool_colorgroups::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_colorgroups::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colorgroups::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colorgroups::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_colorgroups::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_colorgroups::initialize(void) {
	// do nothing
}



//////////////////////////////
//
// Tool_colorgroups::processFile --
//

void Tool_colorgroups::processFile(HumdrumFile& infile) {
	Tool_shed shed;
	vector<string> argv;

	string command = "s/grp:A/color:";
	command += getString("A");
	command += "/I; ";
	command += "s/grp:B/color:";
	command += getString("B");
	command += "/I; ";
	command += "s/grp:C/color:";
	command += getString("C");
	command += "/I";

	if (getBoolean("command")) {
		m_free_text << command << endl;
		return;
	}

	argv.clear();
	argv.push_back("shed"); // name of program (placeholder)
	argv.push_back("-e");
	argv.push_back(command);

	shed.process(argv);
	shed.run(infile);
}



// END_MERGE

} // end namespace hum



