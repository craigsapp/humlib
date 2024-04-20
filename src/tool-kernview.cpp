//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Mar  1 21:40:18 PST 2020
// Last Modified: Sun Mar  1 21:40:21 PST 2020
// Filename:      tool-view.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-view.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   View or hide staves when rendering to graphical notation.
//

#include "tool-kernview.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_kernview::Tool_kernview -- Set the recognized options for the tool.
//

Tool_kernview::Tool_kernview(void) {
	define("v|view|s|show=s",   "view the list of spines");
	define("g=s",               "regular expression of kern spines to view");
	define("G=s",               "regular expression of kern spines to hide");
	define("h|hide|r|remove=s", "hide the list of spines");
}



/////////////////////////////////
//
// Tool_kernview::run -- Do the main work of the tool.
//

bool Tool_kernview::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_kernview::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kernview::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_kernview::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_kernview::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_kernview::initialize(HumdrumFile& infile) {
	m_view_string = getString("view");
	m_hide_string = getString("hide");
	if (getBoolean("g")) {
		m_view_string = getKernString(infile, getString("g"));
	}
	if (getBoolean("G")) {
		m_hide_string = getKernString(infile, getString("G"));
	}
}



//////////////////////////////
//
// Tool_kernview::getKernString -- Return a list of the **kern spines that match to the given
//    comman-separated list of patterns.
//

string Tool_kernview::getKernString(HumdrumFile& infile, const string& list) {
	HumRegex hre;
	vector<string> pieces;
	hre.split(pieces, list, "\\s*,\\s*");
	string output;
	vector<HTp> starts = infile.getKernSpineStartList();
	vector<bool> targets(starts.size(), false);
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces.empty()) {
			continue;
		}
		for (int j=0; j<(int)starts.size(); j++) {
			if (targets[j]) {
				continue;
			}
			HTp current = starts[j];
			while (current) {
				if (current->isData()) {
					break;
				}
				if (hre.search(current, pieces[i])) {
					targets[j] = true;
					break;
				}
				current = current->getNextToken();
			}
		}
	}

	for (int i=0; i<(int)targets.size(); i++) {
		if (targets[i]) {
			if (output.empty()) {
				output += to_string(i+1);
			} else {
				output += "," + to_string(i+1);
			}
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_kernview::processFile --
//

void Tool_kernview::processFile(HumdrumFile& infile) {
	if (m_view_string.empty() && m_hide_string.empty()) {
		return;
	}

	int count = 0;
	vector<HTp> spines;
	infile.getSpineStartList(spines);
	vector<HTp> kernish;
	for (int i=0; i<(int)spines.size(); i++) {
		string exinterp = spines[i]->getDataType();
		if (exinterp.find("kern") != string::npos) {
			count++;
			kernish.push_back(spines[i]);
		}
	}

	if (kernish.empty()) {
		return;
	}

	vector<int> viewlist;
	vector<int> hidelist;
	if (!m_view_string.empty()) {
		viewlist = Convert::extractIntegerList(m_view_string, count);

		// First hide every kernish spine:
		for (int i=0; i<(int)kernish.size(); i++) {
			kernish[i]->setText("**kernyy");
		}
		// Then show given kernish spines:
		for (int i=0; i<(int)viewlist.size(); i++) {
			int value = viewlist[i];
			value--;
			if (value < 0) {
				continue;
			}
			if (value >= (int)kernish.size()) {
				// invalid: too large of a spine number
				continue;
			}
			kernish[value]->setText("**kern");
		}

	} else if (!m_hide_string.empty()) {
		hidelist = Convert::extractIntegerList(m_hide_string, count);

		// First show every kernish spine:
		for (int i=0; i<(int)kernish.size(); i++) {
			kernish[i]->setText("**kern");
		}
		// Then hide given kernish spines:
		for (int i=0; i<(int)hidelist.size(); i++) {
			int value = hidelist[i];
			value--;
			if (value < 0) {
				continue;
			}
			if (value >= (int)kernish.size()) {
				// invalid: too large of a spine number
				continue;
			}
			kernish[value]->setText("**kernyy");
		}
	}

	int line = kernish[0]->getLineIndex();
	infile[line].createLineFromTokens();

}



// END_MERGE

} // end namespace hum



