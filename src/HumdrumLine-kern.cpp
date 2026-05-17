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

string HumdrumLine::getTriadicQuality(HumdrumFile& infile, int index,
		string& quality, string& root, string& inversion,
		map<string, bool>& options) {

	bool pitchesQ = options["pitches"]; // show list of pitches
	bool classQ   = options["class"]; // show list of unique pitch classes
	bool restQ    = options["rest"];  // include rest
	bool lowQ     = options["low"];   // sort pitches from low to high

	quality.clear();
	root.clear();
	inversion.clear();

	// Return non-data line types:
	if (infile[index].isExclusiveInterpretation()) {
		return "**cdata";
	} else if (infile[index].isManipulator()) {
		return "*";
	} else if (infile[index].isInterpretation()) {
		if (infile[index].getFieldCount() > 0) {
			if (*infile[index].token(0) == "*-") {
				return "*-";
			}
		}
		return "*";
	} else if (infile[index].isBarline()) {
		return *(infile[index].token(0));
	} else if (infile[index].isCommentLocal()) {
		return "!";
	} else  if (!infile[index].isData()) {
		return "!!";
	}

	vector<int> notes;
	//Collect note attacks/sustains/rests on line
	// 0 = rests
	// <0 = sustain
	// Sorting ignores sign
	if (lowQ) {
		infile[index].getMidiPitchesSortHL(notes); // reversed for some reason
	} else {
		infile[index].getMidiPitchesSortLH(notes);
	}

	if (pitchesQ) {
		string output = "[";
		for (int i=0; i<(int)notes.size(); i++) {
			if (restQ && notes[i] == 0) {
				continue;
			}
			output += to_string(notes[i]) +  " ";
		}
		output.back() = ']';
		quality = output;
cerr << "Quality = " << output << endl;
		root = "";
		inversion = "";
		return output;
	}

	if (notes.empty()) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	// Get leftmost sounding pitch class (NOT necessarily lowest pitch).
	int basspc = -1;

	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp tok = infile[index].token(i);
		if (tok->isNull()) {
			// deal with null tokens
			tok = tok->resolveNull();
			if (!tok || tok->isNull()) {
				continue;
			}
			continue;
		}
		if (!tok->isData()) {
			continue;
		} 
		vector<int> subtoks;
		tok->getMidiPitches(subtoks);
		for (int j=0; j<(int)subtoks.size(); j++) {
			notes.push_back(subtoks[i]);
		}
	}

	// Extract unique pitch classes.
	vector<bool> pcs(12);
	for (int i=0; i<(int)notes.size(); i++) {
		int pc = notes[i];
		if (pc == 0) {
			// ignore rests
			continue;
		}
		if (pc < 0) {
			// ignore sustains information
			pc = -pc;
		}
		pc = pc % 12;
		pcs.at(pc) = true;
	}

	vector<int> pcs_new;
	for (int i=0; i<(int)pcs.size(); i++) {
		if (pcs[i]) {
			pcs_new.push_back(i);
		}
	}

	if (classQ) {
		string output = "{";
		for (int i=0; i<(int)pcs_new.size(); i++) {
			output += to_string(pcs_new[i]) + " ";
		}
		if (output.size() == 1) {
			output += "}";
		} else {
			output.back() = '}';
		}
		return output;
	}

	static vector<string> pcnames = {
		"C", "C#", "D", "E-", "E", "F",
		"F#", "G", "A-", "A", "B-", "B"
	};

	if (pcs_new.size() == 0) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	if (pcs_new.size() == 1) {
		quality = "U";
		root = pcnames[pcs_new[0]];
		return "";
	}

	if (pcs_new.size() == 2) {
		int interval = (pcs_new[1] - pcs_new[0] + 12) % 12;

		if (interval == 7) {
			quality = "-5";   // missing third
		} else if (interval == 3) {
			quality = "-m";   // missing 5th (major)
		} else if (interval == 4) {
			quality = "-M";   // missing 5th (minor)
		} else {
			quality = "?";    // non-triadic dyad:
		}

		root = pcnames[pcs_new[0]];
		return "";
	}

	if (pcs_new.size() > 3) {
		quality = "+";
		return "";
	}

	// Try all roots.

	for (int i=0; i<3; i++) {

		int r = pcs_new[i];

		bool has3 = false;
		bool has4 = false;
		bool has6 = false;
		bool has7 = false;
		bool has8 = false;

		for (int j=0; j<3; j++) {
			int interval = (pcs_new[j] - r + 12) % 12;

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
			return std::abs(a) > std::abs(b);
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



