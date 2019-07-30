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
	for (int i=0; i<(int)data.size(); i++) {
		delete data[i];
		data[i] = NULL;
	}
	data.resize(0);
}



//////////////////////////////
//
// HumdrumFileSet::getSize -- Return the number of Humdrum files in the
//     set.
//

int HumdrumFileSet::getSize(void) {
	return (int)data.size();
}



//////////////////////////////
//
// HumdrumFileSet::operator[] -- Return a HumdrumFile.
//

HumdrumFile& HumdrumFileSet::operator[](int index) {
	return *(data.at(index));
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
		data.push_back(pfile);
		pfile = new HumdrumFile;
	}
	delete pfile;
	return (int)data.size();
}


// END_MERGE

} // end namespace hum



