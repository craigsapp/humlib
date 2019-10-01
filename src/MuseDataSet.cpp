//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 17 13:27:39 PDT 2010
// Last Modified: Wed Sep 25 07:08:59 PDT 2019 Convert to STL.
// Filename:      humlib/src//MuseDataSet.cpp
// Web Address:   https://github.com/craigsapp/humlib/blob/master/src/MuseDataSet.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   A class that stores a collection of MuseDataSet files
//                representing parts in the same score.
//

#include "MuseDataSet.h"

#include <string.h>

#include <sstream>
#include <fstream>

using namespace std;

namespace hum {

// START_MERGE


///////////////////////////////////////////////////////////////////////////
//
// MuseDataSet class functions --
//


//////////////////////////////
//
// MuseDataSet::MuseDataSet --
//

MuseDataSet::MuseDataSet (void) {
	m_part.reserve(100);
}



//////////////////////////////
//
// MuseDataSet::clear -- Remove contents of object.
//

void MuseDataSet::clear(void) {
	int i;
	for (i=0; i<(int)m_part.size(); i++) {
		delete m_part[i];
	}

}



//////////////////////////////
//
// MuseDataSet::operator[] --
//

MuseData& MuseDataSet::operator[](int index) {
	return *m_part[index];
}



//////////////////////////////
//
// MuseDataSet::readPart -- read a single MuseData part, appending it
//      to the current list of parts.
//

int MuseDataSet::readPartFile(const string& filename) {
	MuseData* md = new MuseData;
	md->readFile(filename);
	md->setFilename(filename);
	return appendPart(md);
}

int MuseDataSet::readPartString(const string& data) {
	stringstream ss;
	ss << data;
	return readPart(ss);
}


int MuseDataSet::readPart(istream& input) {
	MuseData* md = new MuseData;
	md->read(input);
	return appendPart(md);
}



//////////////////////////////
//
// MuseDataSet::read -- read potentially Multiple parts from a single file.
//   First clear the contents of any previous part data.
//

int MuseDataSet::readFile(const string& filename) {
	MuseDataSet::clear();
	ifstream infile(filename);
	return MuseDataSet::read(infile);
}

int MuseDataSet::readString(const string& data) {
	stringstream ss;
	ss << data;
	return MuseDataSet::read(ss);
}


int MuseDataSet::read(istream& infile) {
	vector<string> datalines;
	datalines.reserve(100000);
	string thing;

	while (!infile.eof()) {
		getline(infile, thing);
		if (infile.eof() && (thing.length() == 0)) {
			// last line was not terminated by a newline character
			break;
		}
		datalines.push_back(thing);
	}

	vector<int> startindex;
	vector<int> stopindex;
	analyzePartSegments(startindex, stopindex, datalines);

	stringstream *sstream;
	MuseData* md;
	for (int i=0; i<(int)startindex.size(); i++) {
		sstream = new stringstream;
		for (int j=startindex[i]; j<=stopindex[i]; j++) {
			 (*sstream) << datalines[j] << '\n';
		}
		md = new MuseData;
		md->read(*sstream);
		appendPart(md);
		delete sstream;
	}
	return 1;
}



//////////////////////////////
//
// MuseDataSet::appendPart -- append a MuseData pointer to the end of the
//   parts list and return the index number of the new part.
//

int MuseDataSet::appendPart(MuseData* musedata) {
	int index = (int)m_part.size();
	m_part.resize(m_part.size()+1);
	m_part[index] = musedata;
	return index;
}



//////////////////////////////
//
// MuseData::analyzePartSegments -- Calculate the starting line index
//    and the ending line index for each part in the input.
//

void MuseDataSet::analyzePartSegments(vector<int>& startindex,
		vector<int>& stopindex, vector<string>& lines) {

	startindex.clear();
	stopindex.clear();
	startindex.reserve(1000);
	stopindex.reserve(1000);

	vector<int> types;
	// MuseData& thing = *this;

	types.resize(lines.size());
	std::fill(types.begin(), types.end(), E_muserec_unknown);

	// first identify lines which are multi-line comments so they will
	// not cause confusion in the next step
	int commentstate = 0;
	for (int i=0; i<(int)lines.size(); i++) {
		if (lines[i].c_str()[0] == '&') {
			types[i] = E_muserec_comment_toggle;
			commentstate = !commentstate;
			continue;
		}
		if (commentstate) {
			types[i] = E_muserec_comment_line;
		}
	}

	// search the data for "Group memberships:" lines which are required
	// to be in the header of each part.
	vector<int> groupmemberships;
	groupmemberships.reserve(1000);
	int len = strlen("Group memberships:");
	for (int i=0; i<(int)lines.size(); i++) {
		if (strncmp("Group memberships:", lines[i].c_str(), len) == 0) {
			if (types[i] != E_muserec_comment_line) {
				groupmemberships.push_back(i);
			}
		}
	}

	// search backwards from "Group memberships:" until the end of the
	// header, sucking any comments which occurs just before the start
	// of the header. (currently only multi-line comments, but also need
	// to add single-line comments)
	int value;
	int headerline;
	int found = 0;
	for (int ii=0; ii<(int)groupmemberships.size(); ii++) {
		int i = groupmemberships[ii];
		types[i] = E_muserec_group_memberships;
		found = 0;
		headerline = 11;
		for (int j=i-1; j>=0; j--) {
			if (j < 0) {
				break;
			}
			if (lines[j].compare(0, 4, "/eof") == 0) {
				// end of previous file
				found = 1;
				value = j + 1;
				startindex.push_back(value);
				break;
			}
			if ((types[j] == E_muserec_comment_line) ||
				 (types[j] == E_muserec_comment_toggle)) {
//				j--;
				continue;
			}
			if (j < 0) {
				break;
			}
			headerline--;

			if (headerline == 0) {
				while ((j>= 0) && (lines[j][0] == '@')) {
					j--;
				}
				value = j+1;
				//value = j+2;
				found = 1;
				startindex.push_back(value);
				break;
			}

			if ((j >= 0) && (headerline == 0)) {
				value = j+1;
				found = 1;
				startindex.push_back(value);
				break;
			}
			if (j<0) {
				value = 0;
				found = 1;
				startindex.push_back(value);
				continue;
			}
			switch (headerline) {
				case 11: types[j] = E_muserec_header_11; break;
				case 10: types[j] = E_muserec_header_10; break;
				case  9: types[j] = E_muserec_header_9; break;
				case  8: types[j] = E_muserec_header_8; break;
				case  7: types[j] = E_muserec_header_7; break;
				case  6: types[j] = E_muserec_header_6; break;
				case  5: types[j] = E_muserec_header_5; break;
				case  4: types[j] = E_muserec_header_4; break;
				case  3: types[j] = E_muserec_header_3; break;
				case  2: types[j] = E_muserec_header_2; break;
				case  1: types[j] = E_muserec_header_1; break;
			}
		}
		if (!found) {
			value = 0;
			startindex.push_back(value);
		}
	}

	// now calculate the stopindexes:
	stopindex.resize(startindex.size());
	stopindex[(int)stopindex.size()-1] = (int)lines.size()-1;
	for (int i=0; i<(int)startindex.size()-1; i++) {
		stopindex[i] = startindex[i+1]-1;
	}
}



//////////////////////////////
//
// MuseDataSet::getPartCount -- return the number of parts found
//      in the MuseDataSet
//

int MuseDataSet::getPartCount(void) {
	return (int)m_part.size();
}



//////////////////////////////
//
// MuseDataSet::deletePart -- remove a particular part from the data set.
//

void MuseDataSet::deletePart(int index) {
	if (index < 0 || index > (int)m_part.size()-1) {
		cerr << "Trying to delete a non-existent part" << endl;
		return;
	}

	delete m_part[index];
	int i;
	for (i=index+1; i<(int)m_part.size(); i++) {
		m_part[i-1] = m_part[i];
	}
	m_part.resize(m_part.size()-1);
}



//////////////////////////////
//
// MuseDataSet::cleanLineEndings -- remove spaces for ends of lines.
//

void MuseDataSet::cleanLineEndings(void) {
	for (int i=0; i<(int)m_part.size(); i++) {
		m_part[i]->cleanLineEndings();
	}
}



//////////////////////////////
//
// MuseDataSet::clearError --
//

void MuseDataSet::clearError(void) {
	m_error = "";
}



//////////////////////////////
//
// MuseDataSet::hasError --
//

bool MuseDataSet::hasError(void) {
	return !m_error.empty();
}



//////////////////////////////
//
// MuseDataSet::setError --
//

void MuseDataSet::setError(const string& error) {
	m_error = error;
}



//////////////////////////////
//
// MuseDataSet::getError --
//

string MuseDataSet::getError(void) {
	return m_error;
}



///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// operator<< -- print out all parts in sequential order
//

ostream& operator<<(ostream& out, MuseDataSet& musedataset) {
	for (int i=0; i<musedataset.getPartCount(); i++) {
		for (int j=0; j<musedataset[i].getNumLines(); j++) {
			out << musedataset[i][j] << '\n';
		}
	}
	return out;
}



// END_MERGE

} // end namespace hum



