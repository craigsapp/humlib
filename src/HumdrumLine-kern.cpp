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
#include "HumdrumFile.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumLine::getTriadicQuality --
//

//
// HumdrumLine::getTriadicQuality --
//

//
// HumdrumLine::getTriadicQuality --
//

string HumdrumLine::getTriadicQuality(HumdrumFile& infile, int index,
		string& quality, string& root, string& inversion,
		bool pitchesQ, bool classQ, bool restQ) {

	quality.clear();
	root.clear();
	inversion.clear();

	if (infile[index].isExclusiveInterpretation()) {
		return "**cdata";
	}

	if (infile[index].isManipulator()) {
		return "*";
	}

	if (infile[index].isInterpretation()) {
		if (infile[index].getFieldCount() > 0) {
			if (*infile[index].token(0) == "*-") {
				return "*-";
			}
		}
		return "*";
	}

	if (infile[index].isBarline()) {
		return *(infile[index].token(0));
	}

	if (infile[index].isCommentLocal()) {
		return "!";
	}

	if (!infile[index].isData()) {
		return "!!";
	}

	vector<int> notes;
	vector<int> pcs;

	infile[index].getMidiPitchesSortHL(notes);

	if (notes.empty()) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	// Get leftmost sounding pitch class (NOT lowest pitch).

	int basspc = -1;

	for (int i=0; i<infile[index].getFieldCount(); i++) {

		HTp tok = infile[index].token(i);

		if (!tok->isData()) {
			continue;
		}

		if (tok->isNull()) {
			continue;
		}

		vector<int> subtoks;
		tok->getMidiPitches(subtoks);

		if (subtoks.empty()) {
			continue;
		}

		basspc = subtoks[0] % 12;

		if (basspc < 0) {
			basspc += 12;
		}

		break;
	}

	// Extract unique pitch classes.

	for (int i=0; i<(int)notes.size(); i++) {
		int pc = notes[i] % 12;

		if (pc < 0) {
			pc += 12;
		}

		bool found = false;

		for (int j=0; j<(int)pcs.size(); j++) {
			if (pcs[j] == pc) {
				found = true;
				break;
			}
		}

		if (!found) {
			pcs.push_back(pc);
		}
	}

	sort(pcs.begin(), pcs.end());

	if (pitchesQ) {
		for (int i=0; i<(int)notes.size(); i++) {
			if (i > 0) {
				quality += " ";
			}
			quality += to_string(notes[i]);
		}
		return "";
	}

	if (classQ) {
		for (int i=0; i<(int)pcs.size(); i++) {
			if (i > 0) {
				quality += " ";
			}
			quality += to_string(pcs[i]);
		}
		return "";
	}

	static vector<string> pcnames = {
		"C", "C#", "D", "E-", "E", "F",
		"F#", "G", "A-", "A", "B-", "B"
	};

	if (pcs.size() == 0) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	if (pcs.size() == 1) {
		quality = "U";
		root = pcnames[pcs[0]];
		return "";
	}

	if (pcs.size() == 2) {
		int interval = (pcs[1] - pcs[0] + 12) % 12;

		if (interval == 7) {
			quality = "-5";   // missing third
		} else if (interval == 3) {
			quality = "-m";   // missing 5th (major)
		} else if (interval == 4) {
			quality = "-M";   // missing 5th (minor)
		} else {
			quality = "?";    // non-triadic dyad:
		}

		root = pcnames[pcs[0]];
		return "";
	}

	if (pcs.size() > 3) {
		quality = "+";
		return "";
	}

	// Try all roots.

	for (int i=0; i<3; i++) {

		int r = pcs[i];

		bool has3 = false;
		bool has4 = false;
		bool has6 = false;
		bool has7 = false;
		bool has8 = false;

		for (int j=0; j<3; j++) {
			int interval = (pcs[j] - r + 12) % 12;

			if (interval == 3) {
				has3 = true;
			} else if (interval == 4) {
				has4 = true;
			} else if (interval == 6) {
				has6 = true;
			} else if (interval == 7) {
				has7 = true;
			} else if (interval == 8) {
				has8 = true;
			}
		}

		int bassint = (basspc - r + 12) % 12;

		// Major

		if (has4 && has7) {
			quality = "M";
			root = pcnames[r];

			if (bassint == 4) {
				inversion = "b";
			} else if (bassint == 7) {
				inversion = "c";
			}

			return "";
		}

		// Minor

		if (has3 && has7) {
			quality = "m";
			root = pcnames[r];

			if (bassint == 3) {
				inversion = "b";
			} else if (bassint == 7) {
				inversion = "c";
			}

			return "";
		}

		// Diminished

		if (has3 && has6) {
			quality = "d";
			root = pcnames[r];

			if (bassint == 3) {
				inversion = "b";
			} else if (bassint == 6) {
				inversion = "c";
			}

			return "";
		}

		// Augmented

		if (has4 && has8) {
			quality = "A";
			root = pcnames[r];

			if (bassint == 4) {
				inversion = "b";
			} else if (bassint == 8) {
				inversion = "c";
			}

			return "";
		}
	}

	quality = "??";
	return "";
}

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
			continue;
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



