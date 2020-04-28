//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Mar 29 15:14:19 PDT 2013
// Last Modified: Sun Jul 28 20:18:00 CEST 2019 Convert to humlib.
// Filename:      HumdrumFileSet.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileSet.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Manage a list of HumdrumFile objects.
//

#include "HumdrumFileSet.h"
#include "HumdrumFileStream.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumdrumFileSet::HumdrumFileSet --
//

HumdrumFileSet::HumdrumFileSet(void) {
	// do nothing
}

HumdrumFileSet::HumdrumFileSet(Options& options) {
	read(options);
}

HumdrumFileSet::HumdrumFileSet(const string& contents) {
	readString(contents);
}



//////////////////////////////
//
// HumdrumFileSet::~HumdrumFileSet --
//

HumdrumFileSet::~HumdrumFileSet() {
	clear();
}



//////////////////////////////
//
// HumdrumFileSet::clear -- Remove all Humdrum file content from set.
//

void HumdrumFileSet::clear(void) {
	for (int i=0; i<(int)m_data.size(); i++) {
		delete m_data[i];
		m_data[i] = NULL;
	}
	m_data.resize(0);
}



//////////////////////////////
//
// HumdrumFileSet::clearNoFree -- Remove all Humdrum file content from set
//    but do not delete the contents (since it should be handled externally).
//

void HumdrumFileSet::clearNoFree(void) {
	for (int i=0; i<(int)m_data.size(); i++) {
		m_data[i] = NULL;
	}
	m_data.resize(0);
}



//////////////////////////////
//
// HumdrumFileSet::getSize -- Return the number of Humdrum files in the
//     set.
//

int HumdrumFileSet::getSize(void) {
	return (int)m_data.size();
}



//////////////////////////////
//
// HumdrumFileSet::operator[] -- Return a HumdrumFile.
//

HumdrumFile& HumdrumFileSet::operator[](int index) {
	return *(m_data.at(index));
}



//////////////////////////////
//
// HumdrumFileSet::swap -- Switch position of two scores in the set.
//

bool HumdrumFileSet::swap(int index1, int index2) {
	if (index1 < 0) { return false; }
	if (index2 < 0) { return false; }
	if (index1 >= (int)m_data.size()) { return false; }
	if (index2 >= (int)m_data.size()) { return false; }

	HumdrumFile* temp = m_data[index1];
	m_data[index1] = m_data[index2];
	m_data[index2] = temp;

	return true;
}



//////////////////////////////
//
// HumdrumFileSet::read -- Returns the total number of segments
//

int HumdrumFileSet::readFile(const string& filename) {
	clear();
	return readAppendFile(filename);
}

int HumdrumFileSet::readString(const string& contents) {
	clear();
	return readAppendString(contents);
}

int HumdrumFileSet::readStringCsv(const string& contents) {
	clear();
	return readAppendStringCsv(contents);
}

int HumdrumFileSet::read(istream& inStream) {
	clear();
	return readAppend(inStream);
}

int HumdrumFileSet::read(Options& options) {
	clear();
	return readAppend(options);
}

int HumdrumFileSet::read(HumdrumFileStream& instream) {
	clear();
	return readAppend(instream);
}




//////////////////////////////
//
// HumdrumFileSet::readAppend -- Returns the total number of segments
//    Adds each new HumdrumFile segment to the end of the current data.
//

int HumdrumFileSet::readAppendFile(const string& filename) {
	ifstream indata;
	indata.open(filename);
	string contents((istreambuf_iterator<char>(indata)), istreambuf_iterator<char>());
	HumdrumFileStream instream(contents);
	return readAppend(instream);
}


int HumdrumFileSet::readAppendString(const string& contents) {
	HumdrumFileStream instream(contents);
	return readAppend(instream);
}

int HumdrumFileSet::readAppendStringCsv(const string& contents) {
	cerr << "NOT implemented yet" << endl;
	return 0;
}


int HumdrumFileSet::readAppend(istream& inStream) {
	string contents((istreambuf_iterator<char>(inStream)), istreambuf_iterator<char>());
	HumdrumFileStream instream(contents);
	return readAppend(instream);
}


int HumdrumFileSet::readAppend(Options& options) {
	HumdrumFileStream instream(options);
	return readAppend(instream);
}


int HumdrumFileSet::readAppend(HumdrumFileStream& instream) {
	HumdrumFile* pfile = new HumdrumFile;
	while (instream.read(*pfile)) {
		m_data.push_back(pfile);
		pfile = new HumdrumFile;
	}
	delete pfile;
	return (int)m_data.size();
}


int HumdrumFileSet::readAppendHumdrum(HumdrumFile& infile) {
	stringstream ss;
	ss << infile;
	return readAppendString(ss.str());
}


//////////////////////////////
//
// appendHumdrumPointer --  The infile will be deleted by the object
//    (so do not delete outside of the object or allow it to be inserted
//    from a stack pointer). I.e., this function is dangerous if you do
//    no know what you are doing.
//

int HumdrumFileSet::appendHumdrumPointer(HumdrumFile* infile) {
	m_data.push_back(infile);
	return 1;
}



//////////////////////////////
//
// HumdrumFileSet::hasFilters -- Returns true if has any
//    reference records starting with "!!!!filter:"
//    (universal filters).
//

bool HumdrumFileSet::hasFilters(void) {
	HumdrumFileSet& infiles = *this;
	for (int i=0; i<infiles.getCount(); i++) {
		int lcount = infiles[i].getLineCount();
		for (int j=0; j<lcount; j++) {
			if (!infiles[i][j].isComment()) {
				continue;
			}
			HTp token = infiles[i].token(j, 0);
			if (token->compare(0, 11, "!!!!filter:") == 0) {
				return true;
			}
			if (token->compare(0, 10, "!!!filter:") == 0) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileSet::hasGlobalFilters -- Returns true if has any
//    reference records starting with "!!!!filter:"
//    (universal filters).
//

bool HumdrumFileSet::hasGlobalFilters(void) {
	HumdrumFileSet& infiles = *this;
	for (int i=0; i<infiles.getCount(); i++) {
		int lcount = infiles[i].getLineCount();
		for (int j=0; j<lcount; j++) {
			if (!infiles[i][j].isComment()) {
				continue;
			}
			HTp token = infiles[i].token(j, 0);
			if (token->compare(0, 10, "!!!filter:") == 0) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileSet::hasUniversalFilters -- Returns true if has any
//    reference records starting with "!!!!filter:"
//    (universal filters).
//

bool HumdrumFileSet::hasUniversalFilters(void) {
	HumdrumFileSet& infiles = *this;
	for (int i=0; i<infiles.getCount(); i++) {
		int lcount = infiles[i].getLineCount();
		for (int j=0; j<lcount; j++) {
			if (!infiles[i][j].isComment()) {
				continue;
			}
			HTp token = infiles[i].token(j, 0);
			if (token->compare(0, 11, "!!!!filter:") == 0) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileSet::getUniversalReferenceRecords --
//

vector<HLp> HumdrumFileSet::getUniversalReferenceRecords(void) {
	vector<HLp> hlps;
	hlps.reserve(32);
	HLp hlp;
	HumdrumFileSet& infiles = *this;
	for (int i=0; i<infiles.getCount(); i++) {
		HumdrumFileBase& infile = infiles[i];
		for (int j=0; j<infile.getLineCount(); j++) {
			if (infile[j].isUniversalReference()) {
				hlp = &infile[j];
				hlps.push_back(hlp);
			}
		}
	}
	return hlps;
}



// END_MERGE

} // end namespace hum



