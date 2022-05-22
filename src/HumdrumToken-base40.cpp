//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 15 17:51:11 PDT 2022
// Last Modified: Sun May 15 17:51:14 PDT 2022
// Filename:      HumdrumToken-midi.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumToken-midi.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   HumdrumToken functions related to Base-40.
//

#include "HumdrumToken.h"
#include "Convert.h"

#include <string>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumToken::getBase40Pitches -- Returns 0 if a rest.
//    (This will be a problem for very low notes).
//    Not checking to see if input data type is not **kern.
//    Negative value means a sustained pitch.
//

void HumdrumToken::getBase40Pitches(vector<int>& output) {
	if (*this == ".") {
		// Not resolving null tokens in this function.
		output.clear();
		return;
	}
	vector<string> pieces = this->getSubtokens();
	output.resize(pieces.size());
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].find("r") != string::npos) {
			output[i] = 0;
		} else {
			output[i] = Convert::kernToBase40(pieces[i]);
			// sustained notes are negative values:
			if (pieces[i].find("_") != string::npos) {
				output[i] = -output[i];
			} else if (pieces[i].find("]") != string::npos) {
				output[i] = -output[i];
			}
		}
	}
}


vector<int> HumdrumToken::getBase40Pitches(void) {
	vector<int> output;
	this->getBase40Pitches(output);
	return output;
}


int HumdrumToken::getBase40Pitch(void) {
	vector<int> pitches = getBase40Pitches();
	if (pitches.size() > 0) {
		return pitches[0];
	} else {
		return 0;
	}
}



//////////////////////////////
//
// HumdrumToken::getBase40PitchesSortHL -- Sort extracted Base-40 pitches from
//    high to low (when there is a chord).  Does not check for valid Base-40
//    note number range of 0-127.  Also not checking to see if input data
//    type is not **kern.  Sustained notes are negative values, but pitches
//    are sorted by absolute value.
//

void HumdrumToken::getBase40PitchesSortHL(vector<int>& output) {
	this->getBase40Pitches(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


vector<int> HumdrumToken::getBase40PitchesSortHL(void) {
	vector<int> output;
	this->getBase40PitchesSortHL(output);
	return output;
}



//////////////////////////////
//
// HumdrumToken::getBase40PitchesSortLH -- Sort extracted Base-40 pitches from
//    low to high (when there is a chord).  Does not check for valid Base-40
//    note number range of 0-127.  Also not checking to see if input data
//    type is not **kern.  Sustained notes are negative values, but pitches
//    are sorted by absolute value.
//

void HumdrumToken::getBase40PitchesSortLH(vector<int>& output) {
	this->getBase40Pitches(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


vector<int> HumdrumToken::getBase40PitchesSortLH(void) {
	vector<int> output;
	this->getBase40PitchesSortLH(output);
	return output;
}



//////////////////////////////
//
// getBase40PitchesResolveNull -- same as getBase40Pitches*() functions
//     but resolves null tokens (get the last non-null token if null).
//

void HumdrumToken::getBase40PitchesResolveNull(vector<int>& output) {
	bool nullQ = (*this == ".");
	HTp token = this;
	if (nullQ) {
		token = this->resolveNull();
	}
	output.clear();
	if (token == NULL) {
		return;
	}
	if (*token == ".") {
		return;
	}
	vector<string> pieces = token->getSubtokens();
	output.resize(pieces.size());
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].find("r") != string::npos) {
			output[i] = 0;
		} else {
			output[i] = Convert::kernToBase40(pieces[i]);
			// sustained notes are negative values:
			if (nullQ) {
				output[i] = -output[i];
			} else if (pieces[i].find("_") != string::npos) {
				output[i] = -output[i];
			} else if (pieces[i].find("]") != string::npos) {
				output[i] = -output[i];
			}
		}
	}
}


vector<int> HumdrumToken::getBase40PitchesResolveNull(void) {
	vector<int> output;
	this->getBase40PitchesResolveNull(output);
	return output;
}


int HumdrumToken::getBase40PitchResolveNull(void) {
	vector<int> pitches = getBase40PitchesResolveNull();
	if (pitches.size() > 0) {
		return pitches[0];
	} else {
		return 0;
	}
}


void HumdrumToken::getBase40PitchesResolveNullSortHL(vector<int>& output) {
	this->getBase40PitchesResolveNull(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


vector<int> HumdrumToken::getBase40PitchesResolveNullSortHL(void) {
	vector<int> output;
	this->getBase40PitchesResolveNullSortHL(output);
	return output;
}


void HumdrumToken::getBase40PitchesResolveNullSortLH(vector<int>& output) {
	this->getBase40PitchesResolveNull(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


vector<int> HumdrumToken::getBase40PitchesResolveNullSortLH (void) {
	vector<int> output;
	this->getBase40PitchesResolveNullSortLH(output);
	return output;
}



// END_MERGE

} // end namespace hum



