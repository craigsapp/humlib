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

	bool pitchesQ   = options["pitches"];
	bool classQ     = options["class"];
	bool restQ      = options["rest"];
	bool lowQ       = options["low"];
	bool asciiQ     = options["ascii"];
	bool unisonQ    = options["unison"];
	bool partialQ   = options["partial"];

	quality.clear();
	root.clear();
	inversion.clear();

	// Non-data lines.
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
	} else if (!infile[index].isData()) {
		return "!!";
	}

	vector<int> notes;

	// Collect notes from all sounding tokens, including resolved nulls.
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp tok = this->token(i);
		if (!tok->isKern()) {
			continue;
		}
		if (!tok->isData()) {
			continue;
		}
		if (tok->isNull()) {
			tok = tok->resolveNull();
			if (!tok || tok->isNull()) {
				continue;
			}
		}
		vector<int> subtoks;
		tok->getMidiPitches(subtoks);

		for (int j=0; j<(int)subtoks.size(); j++) {
			notes.push_back(subtoks[j]);
		}
	}

	if (notes.empty()) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	// Sort pitches if requested.
	if (lowQ) {
		sort(notes.begin(), notes.end(),
			[](const int& a, const int& b) {
				return abs(a) > abs(b);
			}
		);
	} else {
		sort(notes.begin(), notes.end(),
			[](const int& a, const int& b) {
				return abs(a) < abs(b);
			}
		);
	}

	if (pitchesQ) {
		string output = "[";

		for (int i=0; i<(int)notes.size(); i++) {

			int pitch = notes[i];

			if (pitch == 0 && restQ) {
				continue;
			}

			output += to_string(pitch);
			output += " ";
		}

		if (output.size() == 1) {
			output += "]";
		} else {
			output.back() = ']';
		}

		quality = output;
		return output;
	}

	// Bass pitch class = leftmost sounding note.
	int basspc = -1;

	for (int i=0; i<infile[index].getFieldCount(); i++) {

		HTp tok = infile[index].token(i);

		if (tok->isNull()) {
			tok = tok->resolveNull();
			if (!tok || tok->isNull()) {
				continue;
			}
		}

		if (!tok->isData()) {
			continue;
		}

		vector<int> subtoks;
		tok->getMidiPitches(subtoks);

		for (int j=0; j<(int)subtoks.size(); j++) {

			int pitch = subtoks[j];

			if (pitch == 0) {
				continue;
			}

			if (pitch < 0) {
				pitch = -pitch;
			}

			basspc = pitch % 12;
			break;
		}

		if (basspc >= 0) {
			break;
		}
	}

	vector<bool> pcs(12, false);

	for (int i=0; i<(int)notes.size(); i++) {

		int pitch = notes[i];

		if (pitch == 0) {
			continue;
		}

		if (pitch < 0) {
			pitch = -pitch;
		}

		pcs[pitch % 12] = true;
	}

	vector<int> pcs_new;

	for (int i=0; i<12; i++) {
		if (pcs[i]) {
			pcs_new.push_back(i);
		}
	}

	if (classQ) {

		string output = "{";

		for (int i=0; i<(int)pcs_new.size(); i++) {
			output += to_string(pcs_new[i]);
			output += " ";
		}

		if (output.size() == 1) {
			output += "}";
		} else {
			output.back() = '}';
		}

		return output;
	}

	static vector<string> pcnames = {
		"C", "C#", "D", "E♭", "E", "F",
		"F#", "G", "A♭", "A", "B♭", "B"
	};

	if (pcs_new.empty()) {
		if (restQ) {
			quality = "R";
			return "";
		} else {
			return ".";
		}
	}

	// Unison.
	if (pcs_new.size() == 1) {
		if (unisonQ && partialQ) {
			quality = "U";
			root = pcnames[pcs_new[0]];
			if (asciiQ) {
				inversion = "1";
			} else {
				inversion = "₁";
			}
			return "";
		}
	}

	// Dyads.
	if (pcs_new.size() == 2) {

		int pc1 = pcs_new[0];
		int pc2 = pcs_new[1];

		int interval = (pc2 - pc1 + 12) % 12;

		if (interval == 7) {
			if (partialQ) {
				quality = "-5";
				root = pcnames[pc1];
				if (asciiQ) {
					inversion = "5";
				} else {
					inversion = "₅";
				}
			}
		} else if (interval == 5) {
			if (partialQ) {
				quality = "-5";
					root = pcnames[pc2];
				if (asciiQ) {
					inversion = "5";
				} else {
					inversion = "₅";
				}
			}

		} else if (interval == 3) {
			if (partialQ) {
				quality = "-m";
				root = pcnames[pc1];
				root[0] = tolower(root[0]);
				if (asciiQ) {
					inversion = "3";
				} else {
					inversion = "₃";
				}
			}

		} else if (interval == 9) {
			if (partialQ) {
				quality = "-m";
				root = pcnames[pc2];
				root[0] = tolower(root[0]);
				if (asciiQ) {
					inversion = "3";
				} else {
					inversion = "₃";
				}
			}

		} else if (interval == 4) {
			if (partialQ) {
				quality = "-M";
				root = pcnames[pc1];
				if (asciiQ) {
					inversion = "3";
				} else {
					inversion = "₃";
				}
			}

		} else if (interval == 8) {
			if (partialQ) {
				quality = "-M";
				root = pcnames[pc2];
				if (asciiQ) {
					inversion = "3";
				} else {
					inversion = "₃";
				}
			}

		} else {
			quality = "?";
			root.clear();
			inversion.clear();
		}

		return "";
	}

	// More than triad.
	if (pcs_new.size() > 3) {
		// Still allow a bass-root reading when P4 and P5 sound above the bass
		// (e.g. E–A–B plus extra tones).
		if ((basspc >= 0) && pcs[(basspc + 5) % 12] && pcs[(basspc + 7) % 12]) {
			quality = "S";
			root = pcnames[basspc];
			return "";
		}
		quality = "+";
		return "";
	}

	// Try all pitch classes as root.
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

		// Major triad.
		if (has4 && has7) {
			quality = "M";
			root = pcnames[r];
			if (bassint == 4) {
				if (asciiQ) {
					inversion = "6";
				} else {
					inversion += "₆";
				}
			} else if (bassint == 7) {
				if (asciiQ) {
					inversion = "4";
				} else {
					inversion = "₄";
				}
			}
			return "";
		}

		// Minor triad.
		if (has3 && has7) {
			quality = "m";
			root = pcnames[r];
			if (!root.empty()) {
				root[0] = tolower(root[0]);
			}
			if (bassint == 3) {
				if (asciiQ) {
					inversion = "6";
				} else {
					inversion = "₆";
				}
			} else if (bassint == 7) {
				if (asciiQ) {
					inversion = "4";
				} else {
					inversion = "₄";
				}
			}

			return "";
		}

		// Diminished triad.
		if (has3 && has6) {
			quality = "d";
			root = pcnames[r];
			if (asciiQ) {
				root += "o";
			} else {
				root += "°";
			}
			if (!root.empty()) {
				root[0] = tolower(root[0]);
			}
			if (bassint == 3) {
				if (asciiQ) {
					inversion = "6";
				} else {
					inversion = "₆";
				}
			} else if (bassint == 6) {
				if (asciiQ) {
					inversion = "4";
				} else {
					inversion = "₄";
				}
			}
			return "";
		}

		// Augmented triad.
		if (has4 && has8) {
			quality = "A";
			root = pcnames[r];
			if (asciiQ) {
				root += "+";
			} else {
				root += "⁺";
			}
			if (bassint == 4) {
				if (asciiQ) {
					inversion = "6";
				} else {
					inversion = "₆";
				}
			} else if (bassint == 8) {
				if (asciiQ) {
					inversion = "4";
				} else {
					inversion = "₄";
				}
			}
			return "";
		}
	}

	// Suspended sonority: perfect fourth and perfect fifth above the bass
	// (e.g. E–A–B).  Treat the bass pitch as the root.
	if ((basspc >= 0) && pcs[(basspc + 5) % 12] && pcs[(basspc + 7) % 12]) {
		quality = "S";
		root = pcnames[basspc];
		return "";
	}

	// Unknown trichord.
	quality = "?";
	root.clear();
	inversion.clear();

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



