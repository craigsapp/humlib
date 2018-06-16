// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 12 21:02:48 PDT 2018
// Last Modified: Tue Jun 12 21:02:56 PDT 2018
// Filename:      HumdrumFileContent-rest.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-rest.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Identify the vertical positions of rests in two-layer staves.
//

#include "HumdrumFileContent.h"
#include "HumRegex.h"
#include "Convert.h"


using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeRestPositions -- Calculate the vertical position
//    of rests on staves with two layers.
//

void HumdrumFileContent::analyzeRestPositions(void) {
	vector<HTp> kernstarts = getKernSpineStartList();
	for (int i=0; i<(int)kernstarts.size(); i++) {
		analyzeRestPositions(kernstarts[i]);
	}
}

void HumdrumFileContent::analyzeRestPositions(HTp kernstart) {
	if (!kernstart) {
		return;
	}

	int baseline = Convert::kernClefToBaseline("*clefG2");
	HTp current = kernstart;
	int track = kernstart->getTrack();

	while (current) {
		if (current->isClef()) {
			baseline = Convert::kernClefToBaseline(current);
			current = current->getNextToken();
			continue;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		int strack = -1;
		HTp second = current->getNextFieldToken();
		if (second) {
			strack = second->getTrack();
		}
		if (track != strack) {
			// only one layer in current spine.
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			HTp resolve = current->resolveNull();
			if (resolve && resolve->isRest()) {
				if (second && second->isRest()) {
					if (processRestPitch(second, baseline)) {
						current = current->getNextToken();
						continue;
					}
				}
			}
			current = current->getNextToken();
			continue;
		}
		if (current->isRest()) {
			// assign a default position for the rest, since
			// verovio will try to tweak it when there is
			// more than one layer on the staff.
			setRestOnCenterStaffLine(current, baseline);
		}
		if (current->isRest()) {
			if (processRestPitch(current, baseline)) {
				if (second && second->isRest()) {
					if (processRestPitch(second, baseline)) {
						current = current->getNextToken();
						continue;
					}
				}
				current = current->getNextToken();
				continue;
			}
		}
		if (second && second->isRest()) {
			if (processRestPitch(second, baseline)) {
				current = current->getNextToken();
				continue;
			}
		}
		if (!second) {
			current = current->getNextToken();
			continue;
		}
		if (second->isRest()) {
			// assign a default position for the rest, since
			// verovio will try to tweak it when there is
			// more than one layer on the staff.
			setRestOnCenterStaffLine(current, baseline);
			setRestOnCenterStaffLine(second, baseline);
		}
		if (second->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isRest() && second->isRest()) {
			// not dealing with rest against rest for now
			current = current->getNextToken();
			// what to do with vertical positions?  The are
			// current collapsed into a single rest
			// with the code above.
			continue;
		}
		if (current->isRest() || second->isRest()) {
			assignVerticalRestPosition(current, second, baseline);
		}
		current = current->getNextToken();
		continue;
	}
}



//////////////////////////////
//
// HumdrumFileContent::processRestPitch -- Read any pitch information attached to
//     a rest and convert to ploc/oloc values.
//

bool HumdrumFileContent::processRestPitch(HTp rest, int baseline) {
	HumRegex hre;
	if (!hre.search(rest, "([A-Ga-g]+)")) {
		return false;
	}
	string pitch = hre.getMatch(1);
	int b7 = Convert::kernToBase7(pitch);

	int diff = (b7 - baseline) + 100;
	if (diff % 2) {
		// force to every other diatonic step (stafflines)
		HumNum dur = rest->getDuration();
		if (dur > 1) {
			b7--;
		} else {
			b7++;
		}
	}

	int pc = b7 % 7;
	int oct = b7 / 7;

	string dname;
	switch (pc) {
		case 0: dname = "C"; break;
		case 1: dname = "D"; break;
		case 2: dname = "E"; break;
		case 3: dname = "F"; break;
		case 4: dname = "G"; break;
		case 5: dname = "A"; break;
		case 6: dname = "B"; break;
	}

	if (dname.empty()) {
		return false;
	}

	string oloc = to_string(oct);

	rest->setValue("auto", "ploc", dname);
	rest->setValue("auto", "oloc", oloc);

	return true;
}





//////////////////////////////
//
// HumdrumFileContent::setRestOnCenterStaffLine --
//

void HumdrumFileContent::setRestOnCenterStaffLine(HTp rest, int baseline) {
	int rpos = 4;
	int restdia = rpos + baseline;
	int pc = restdia % 7;
	int oct = restdia / 7;

	string dname;
	switch (pc) {
		case 0: dname = "C"; break;
		case 1: dname = "D"; break;
		case 2: dname = "E"; break;
		case 3: dname = "F"; break;
		case 4: dname = "G"; break;
		case 5: dname = "A"; break;
		case 6: dname = "B"; break;
	}

	if (dname.empty()) {
		return;
	}

	string oloc = to_string(oct);

	rest->setValue("auto", "ploc", dname);
	rest->setValue("auto", "oloc", oloc);
}



//////////////////////////////
//
//	HumdrumFileContents::assignVerticalRestPosition --
//

void HumdrumFileContent::assignVerticalRestPosition(HTp first, HTp second, int baseline) {
	vector<string> tokens;
	vector<int> vpos;

	int notepos = 0;
	HTp rest = NULL;
	HTp notes = NULL;
	if (first->isRest()) {
		rest = first;
		notes = second;
		notepos = -1;
	} else if (second->isRest()) {
		rest = second;
		notes = first;
		notepos = +1;
	} else {
		return;
	}

	int count = notes->getSubtokenCount();
	for (int i=0; i<count; i++) {
		vpos.push_back(Convert::kernToBase7(notes->getSubtoken(i)) - baseline);
	}

	int rpos = 0;
	if (notepos > 0) {
		rpos = getRestPositionBelowNotes(rest, vpos);
	} else if (notepos < 0) {
		rpos = getRestPositionAboveNotes(rest, vpos);
	} else {
		return;
	}

	int restdia = rpos + baseline;
	int pc = restdia % 7;
	int oct = restdia / 7;

	string dname;
	switch (pc) {
		case 0: dname = "C"; break;
		case 1: dname = "D"; break;
		case 2: dname = "E"; break;
		case 3: dname = "F"; break;
		case 4: dname = "G"; break;
		case 5: dname = "A"; break;
		case 6: dname = "B"; break;
	}

	if (dname.empty()) {
		return;
	}

	string oloc = to_string(oct);

	rest->setValue("auto", "ploc", dname);
	rest->setValue("auto", "oloc", oloc);
}



//////////////////////////////
//
// HumdrumFileContent::getRestPositionBelowNotes --
//

int HumdrumFileContent::getRestPositionBelowNotes(HTp rest, vector<int>& vpos) {
	if (vpos.empty()) {
		return 4;
	}
	int lowest = vpos[0];
	for (int i=1; i<(int)vpos.size(); i++) {
		if (lowest > vpos[i]) {
			lowest = vpos[i];
		}
	}
	int restint = 0;
	double resttype = log(rest->getDuration().getFloat()) / log(2.0);
	restint = int(resttype + 1000.0) - 1000;
	int output = 0;

	switch (restint) {

		case 0: // quarter-note rests
			output = 0;
			switch (lowest) {
				case -2: output = -8; break;
				case -1: output = -6; break;
				case  0: output = -6; break;
				case  1: output = -4; break;
				case  2: output = -4; break;
				case  3: output = -2; break;
				case  4: output = -2; break;
				case  5: output =  0; break;
				case  6: output =  0; break;
				case  7: output =  2; break;
				case  8: output =  2; break;
				case  9: output =  4; break;
				case 10: output =  4; break;
			}
			if (lowest > 10) {
				output = 4;
			}
			if (lowest < -2) {
				output = lowest - 6;
				if (lowest % 2) {
					output++;
				}
			}
			return output;
			break;

		case -1: // eighth-note rests
		case -2: // sixteenth-note rests
			output = 0;
			switch (lowest) {
				case -2: output = -6; break;
				case -1: output = -4; break;
				case  0: output = -4; break;
				case  1: output = -2; break;
				case  2: output = -2; break;
				case  3: output =  0; break;
				case  4: output =  0; break;
				case  5: output =  2; break;
				case  6: output =  2; break;
				case  7: output =  4; break;
				case  8: output =  4; break;
				case  9: output =  4; break;
				case 10: output =  4; break;
			}
			if (lowest > 10) {
				output = 4;
			}
			if (lowest < -2) {
				output = lowest - 4;
				if (lowest % 2) {
					output++;
				}
			}
			return output;
			break;

		case -3: // 32nd-note rests
		case -4: // 64h-note rests
			output = 0;
			switch (lowest) {
				case -2: output = -8; break;
				case -1: output = -6; break;
				case  0: output = -6; break;
				case  1: output = -4; break;
				case  2: output = -4; break;
				case  3: output = -2; break;
				case  4: output = -2; break;
				case  5: output =  0; break;
				case  6: output =  0; break;
				case  7: output =  2; break;
				case  8: output =  2; break;
				case  9: output =  4; break;
				case 10: output =  4; break;
			}
			if (lowest > 10) {
				output = 4;
			}
			if (lowest < -2) {
				output = lowest - 6;
				if (lowest % 2) {
					output++;
				}
			}
			return output;
			break;

		case -5: // 128th-note rests
		case -6: // 256th-note rests
			output = 0;
			switch (lowest) {
				case -2: output = -10; break;
				case -1: output = -8;  break;
				case  0: output = -8;  break;
				case  1: output = -6;  break;
				case  2: output = -6;  break;
				case  3: output = -4;  break;
				case  4: output = -4;  break;
				case  5: output = -2;  break;
				case  6: output = -2;  break;
				case  7: output =  0;  break;
				case  8: output =  0;  break;
				case  9: output =  2;  break;
				case 10: output =  2;  break;
			}
			if (lowest > 10) {
				output = 4;
			}
			if (lowest < -2) {
				output = lowest - 8;
				if (lowest % 2) {
					output++;
				}
			}
			return output;
			break;

		case 1: // half-note rests
		case 2: // whole-note rests
		case 3: // breve-note rests
			output = 0;
			switch (lowest) {
				case -2: output = -6; break;
				case -1: output = -6; break;
				case  0: output = -4; break;
				case  1: output = -4; break;
				case  2: output = -2; break;
				case  3: output = -2; break;
				case  4: output =  0; break;
				case  5: output =  0; break;
				case  6: output =  2; break;
				case  7: output =  2; break;
				case  8: output =  4; break;
				case  9: output =  4; break;
				case 10: output =  4; break;
			}
			if (lowest > 10) {
				output = 4;
			}
			if (lowest < -2) {
				output = lowest - 4;
				if (lowest % 2) {
					output--;
				}
			}
			return output;
			break;

	}
	return 0;
}



//////////////////////////////
//
// HumdrumFileContent::getRestPositionAboveNotes --
//

int HumdrumFileContent::getRestPositionAboveNotes(HTp rest, vector<int>& vpos) {
	if (vpos.empty()) {
		return 4;
	}
	int highest = vpos[0];
	for (int i=1; i<(int)vpos.size(); i++) {
		if (highest < vpos[i]) {
			highest = vpos[i];
		}
	}
	int restint = 0;
	double resttype = log(rest->getDuration().getFloat()) / log(2.0);
	restint = int(resttype + 1000.0) - 1000;
	int output = 8;

	switch (restint) {

		case 0: // quarter-note rests
			output = 0;
			switch (highest) {
				case -2: output =  4; break;
				case -1: output =  4; break;
				case  0: output =  4; break;
				case  1: output =  6; break;
				case  2: output =  6; break;
				case  3: output =  8; break;
				case  4: output =  8; break;
				case  5: output = 10; break;
				case  6: output = 10; break;
				case  7: output = 12; break;
				case  8: output = 12; break;
				case  9: output = 14; break;
				case 10: output = 14; break;
			}
			if (highest < -2) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 4;
				if (highest % 2) {
					output++;
				}
			}
			return output;
			break;

		case -1: // eighth-note rests
			output = 0;
			switch (highest) {
				case -2: output =  4; break;
				case -1: output =  4; break;
				case  0: output =  4; break;
				case  1: output =  4; break;
				case  2: output =  6; break;
				case  3: output =  6; break;
				case  4: output =  8; break;
				case  5: output =  8; break;
				case  6: output = 10; break;
				case  7: output = 10; break;
				case  8: output = 12; break;
				case  9: output = 12; break;
				case 10: output = 14; break;
			}
			if (highest < -2) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 4;
				if (highest % 2) {
					output--;
				}
			}
			return output;
			break;

		case -2: // sixteenth-note rests
		case -3: // 32nd-note rests
			output = 0;
			switch (highest) {
				case -2: output =  4; break;
				case -1: output =  4; break;
				case  0: output =  6; break;
				case  1: output =  6; break;
				case  2: output =  8; break;
				case  3: output =  8; break;
				case  4: output = 10; break;
				case  5: output = 10; break;
				case  6: output = 12; break;
				case  7: output = 12; break;
				case  8: output = 14; break;
				case  9: output = 14; break;
				case 10: output = 16; break;
			}
			if (highest < -2) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 6;
				if (highest % 2) {
					output--;
				}
			}
			return output;
			break;

		case -4: // 64th-note rests
		case -5: // 128th-note rests
			output = 0;
			switch (highest) {
				case -2: output =  6; break;
				case -1: output =  6; break;
				case  0: output =  8; break;
				case  1: output =  8; break;
				case  2: output = 10; break;
				case  3: output = 10; break;
				case  4: output = 12; break;
				case  5: output = 12; break;
				case  6: output = 14; break;
				case  7: output = 14; break;
				case  8: output = 16; break;
				case  9: output = 16; break;
				case 10: output = 18; break;
			}
			if (highest < -2) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 8;
				if (highest % 2) {
					output--;
				}
			}
			return output;
			break;

		case -6: // 256th-note rests
			output = 0;
			switch (highest) {
				case -4: output =  6; break;
				case -3: output =  6; break;
				case -2: output =  8; break;
				case -1: output =  8; break;
				case  0: output = 10; break;
				case  1: output = 10; break;
				case  2: output = 12; break;
				case  3: output = 12; break;
				case  4: output = 14; break;
				case  5: output = 14; break;
				case  6: output = 16; break;
				case  7: output = 16; break;
				case  8: output = 18; break;
				case  9: output = 18; break;
				case 10: output = 20; break;
			}
			if (highest < -4) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 10;
				if (highest % 2) {
					output--;
				}
			}
			return output;
			break;

		case 1: // half-note rests
		case 2: // whole-note rests
		case 3: // breve-note rests
			output = 0;
			switch (highest) {
				case -2: output =  4; break;
				case -1: output =  4; break;
				case  0: output =  4; break;
				case  1: output =  4; break;
				case  2: output =  6; break;
				case  3: output =  6; break;
				case  4: output =  8; break;
				case  5: output =  8; break;
				case  6: output = 10; break;
				case  7: output = 10; break;
				case  8: output = 12; break;
				case  9: output = 12; break;
				case 10: output = 14; break;
			}
			if (highest < -2) {
				output = 4;
			}
			if (highest > 10) {
				output = highest + 4;
				if (highest % 2) {
					output--;
				}
			}
			return output;
			break;

	}

	return output;
}


// END_MERGE

} // end namespace hum



