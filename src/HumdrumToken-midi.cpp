//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun May 15 17:51:11 PDT 2022
// Last Modified: Sun May 15 17:51:14 PDT 2022
// Filename:      HumdrumToken-midi.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumToken-midi.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   HumdrumToken functions related to MIDI.
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
// HumdrumToken::getMidiPitches -- Returns 0 if a rest.
//    Does not check for valid MIDI note number range of 0-127.
//    Also not checking to see if input data type is not **kern.
//    Negative value means a sustained pitch.
//

void HumdrumToken::getMidiPitches(vector<int>& output) {
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
			output[i] = Convert::kernToMidiNoteNumber(pieces[i]);
			// sustained notes are negative values:
			if (pieces[i].find("_") != string::npos) {
				output[i] = -output[i];
			} else if (pieces[i].find("]") != string::npos) {
				output[i] = -output[i];
			}
		}
	}
}


vector<int> HumdrumToken::getMidiPitches(void) {
	vector<int> output;
	this->getMidiPitches(output);
	return output;
}


int HumdrumToken::getMidiPitch(void) {
	vector<int> pitches = getMidiPitches();
	if (pitches.size() > 0) {
		return pitches[0];
	} else {
		return 0;
	}
}



//////////////////////////////
//
// HumdrumToken::getMidiPitchesSortHL -- Sort extracted MIDI pitches from
//    high to low (when there is a chord).  Does not check for valid MIDI
//    note number range of 0-127.  Also not checking to see if input data
//    type is not **kern.  Sustained notes are negative values, but pitches
//    are sorted by absolute value.
//

void HumdrumToken::getMidiPitchesSortHL(vector<int>& output) {
	this->getMidiPitches(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


vector<int> HumdrumToken::getMidiPitchesSortHL(void) {
	vector<int> output;
	this->getMidiPitchesSortHL(output);
	return output;
}



//////////////////////////////
//
// HumdrumToken::getMidiPitchesSortLH -- Sort extracted MIDI pitches from
//    low to high (when there is a chord).  Does not check for valid MIDI
//    note number range of 0-127.  Also not checking to see if input data
//    type is not **kern.  Sustained notes are negative values, but pitches
//    are sorted by absolute value.
//

void HumdrumToken::getMidiPitchesSortLH(vector<int>& output) {
	this->getMidiPitches(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


vector<int> HumdrumToken::getMidiPitchesSortLH(void) {
	vector<int> output;
	this->getMidiPitchesSortLH(output);
	return output;
}



//////////////////////////////
//
// getMidiPitchesResolveNull -- same as getMidiPitches*() functions
//     but resolves null tokens (get the last non-null token if null).
//

void HumdrumToken::getMidiPitchesResolveNull(vector<int>& output) {
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
			output[i] = Convert::kernToMidiNoteNumber(pieces[i]);
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


vector<int> HumdrumToken::getMidiPitchesResolveNull(void) {
	vector<int> output;
	this->getMidiPitchesResolveNull(output);
	return output;
}


int HumdrumToken::getMidiPitchResolveNull(void) {
	vector<int> pitches = getMidiPitchesResolveNull();
	if (pitches.size() > 0) {
		return pitches[0];
	} else {
		return 0;
	}
}


void HumdrumToken::getMidiPitchesResolveNullSortHL(vector<int>& output) {
	this->getMidiPitchesResolveNull(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) > abs(b);
		}
	);
}


vector<int> HumdrumToken::getMidiPitchesResolveNullSortHL(void) {
	vector<int> output;
	this->getMidiPitchesResolveNullSortHL(output);
	return output;
}


void HumdrumToken::getMidiPitchesResolveNullSortLH(vector<int>& output) {
	this->getMidiPitchesResolveNull(output);
	if (output.size() <= 1) {
		return;
	}
	sort(output.begin(), output.end(),
		[](const int& a, const int& b) {
			return abs(a) < abs(b);
		}
	);
}


vector<int> HumdrumToken::getMidiPitchesResolveNullSortLH (void) {
	vector<int> output;
	this->getMidiPitchesResolveNullSortLH(output);
	return output;
}



// END_MERGE

} // end namespace hum



