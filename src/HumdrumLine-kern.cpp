//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Apr 12 11:58:02 PDT 2022
// Last Modified: Tue Apr 12 11:58:06 PDT 2022
// Filename:      HumdrumLine-kern.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumLine-kern.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   HumdrumLine processing of **kern data.
//

#include "HumdrumLine.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumLine::getMidiPitches: Get MIDI note numbers for **kern pitches on line.
// 0 = rest, negative values are tied notes from previously in the score.
//

void HumdrumLine::getMidiPitches(std::vector<int>& output) {

	HumdrumLine& line = *this;
	output.clear();
	if (!line.isData()) {
		return;
	}
	vector<int> tnotes;
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			return;
		}
		token->getMidiPitches(tnotes);
		output.insert(output.end(), tnotes.begin(), tnotes.end());
	}

}


std::vector<int> HumdrumLine::getMidiPitches(void) {
	vector<int> output;
	this->getMidiPitches(output);
	return output;
}


void HumdrumLine::getMidiPitchesSortHL(std::vector<int>& output) {
	output.clear();
	this->getMidiPitches(output);
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


std::vector<int> HumdrumLine::getMidiPitchesSortHL(void) {
	vector<int> output;
	this->getMidiPitchesSortHL(output);
	return output;
}


void HumdrumLine::getMidiPitchesSortLH(std::vector<int>& output) {
	output.clear();
	this->getMidiPitches(output);
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


std::vector<int> HumdrumLine::getMidiPitchesSortLH(void) {
	vector<int> output;
	this->getMidiPitchesSortLH(output);
	return output;
}



//////////////////////////////
//
// HumdrumLine::getMidiPitches: Get MIDI note numbers for **kern pitches on line.
// Null tokens are resulved to the token which is being sustained.
// 0 = rest, negative values are tied notes from previously in the score.
//

void HumdrumLine::getMidiPitchesResolveNull(std::vector<int>& output) {
	HumdrumLine& line = *this;
	output.clear();
	if (!line.isData()) {
		return;
	}
	vector<int> tnotes;
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (!token->isKern()) {
			continue;
		}
		token->getMidiPitchesResolveNull(tnotes);
		output.insert(output.end(), tnotes.begin(), tnotes.end());
	}
}


std::vector<int> HumdrumLine::getMidiPitchesResolveNull(void) {
	vector<int> output;
	this->getMidiPitchesResolveNull(output);
	return output;
}


void HumdrumLine::getMidiPitchesResolveNullSortHL(std::vector<int>& output) {
	output.clear();
	this->getMidiPitchesResolveNull(output);
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


std::vector<int> HumdrumLine::getMidiPitchesResolveNullSortHL (void) {
	vector<int> output;
	this->getMidiPitchesResolveNullSortHL(output);
	return output;
}


void HumdrumLine::getMidiPitchesResolveNullSortLH(std::vector<int>& output) {
	output.clear();
	this->getMidiPitchesResolveNull(output);
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


std::vector<int> HumdrumLine::getMidiPitchesResolveNullSortLH (void) {
	vector<int> output;
	this->getMidiPitchesResolveNullSortLH(output);
	return output;
}


// END_MERGE

} // end namespace hum



