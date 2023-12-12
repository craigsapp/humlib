// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov  5 13:25:49 CET 2018
// Last Modified: Sat Apr 30 12:15:31 PDT 2022
// Filename:      HumdrumFileContent-note.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-note.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Content analysis functions related to notes.
//

#include "Convert.h"
#include "HumRegex.h"
#include "HumdrumFileContent.h"

#include <string>


using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeCrossStaffStemDirections -- Calculate stem directions
//    for notes that are cross-staff, and the notes in the presence of cross-staff
//    notes.
//

void HumdrumFileContent::analyzeCrossStaffStemDirections(void) {
	string above = this->getKernAboveSignifier();
	string below = this->getKernBelowSignifier();

	if (above.empty() && below.empty()) {
		// no cross staff notes present in data
		return;
	}

	vector<HTp> kernstarts = getKernSpineStartList();
	for (int i=0; i<(int)kernstarts.size(); i++) {
		analyzeCrossStaffStemDirections(kernstarts[i]);
	}
}



//////////////////////////////
//
// HumdrumFileContent::analyzeCrossStaffStemDirections -- Check for cross-staff
//     notes, and assign stems if they do not have any.  Also assign stems to
//     notes in the target directory if there is only one layer active on that staff.
//

void HumdrumFileContent::analyzeCrossStaffStemDirections(HTp kernstart) {
	if (!kernstart) {
		return;
	}
	if (!kernstart->isKern()) {
		return;
	}
	string above = this->getKernAboveSignifier();
	string below = this->getKernBelowSignifier();
	if (above.empty() && below.empty()) {
		// no cross staff notes present in data
		return;
	}

	HTp current = kernstart;
	while (current) {
		if (current->isData()) {
			checkCrossStaffStems(current, above, below);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// HumdrumFileContent::checkCrossStaffStems -- Check all notes in all
//     sub-spines of the current token (which should be in the top layer)
//     for cross-staff assignment.
//

void HumdrumFileContent::checkCrossStaffStems(HTp token, string& above, string& below) {
	int track = token->getTrack();

	HTp current = token;
	while (current) {
		int ttrack = current->getTrack();
		if (ttrack != track) {
			break;
		}
		checkDataForCrossStaffStems(current, above, below);
		current = current->getNextFieldToken();
	}
}



//////////////////////////////
//
// HumdrumFileContent::checkDataForCrossStaffStems -- Check a note or chord for
//    cross staff
//

void HumdrumFileContent::checkDataForCrossStaffStems(HTp token, string& above, string& below) {
	if (token->isNull()) {
		return;
	}
	if (token->isRest()) {
		// deal with cross-staff rests later
		return;
	}

	if (token->find('/') != std::string::npos) {
		// has a stem-up marker, so do not try to adjust stems;
		return;
	}

	if (token->find('\\') != std::string::npos) {
		// has a stem-down marker, so do not try to adjust stems;
		return;
	}


	HumRegex hre;
	bool hasaboveQ = false;
	bool hasbelowQ = false;

	if (!above.empty()) {
		string searchstring = "[A-Ga-g]+[#n-]*" + above;
		if (hre.search(*token, searchstring)) {
			// note/chord has staff-above signifier
			hasaboveQ = true;
		}
	}

	if (!below.empty()) {
		string searchstring = "[A-Ga-g]+[#n-]*" + below;
		if (hre.search(*token, searchstring)) {
			// note/chord has staff-below signifier
			hasbelowQ = true;
		}
	}

	if (!(hasaboveQ || hasbelowQ)) {
		// no above/below signifier, so nothing to do
		return;
	}
	if (hasaboveQ && hasbelowQ) {
		// strange complication of above and below, so ignore
		return;
	}

	if (hasaboveQ) {
		prepareStaffAboveNoteStems(token);
	} else if (hasbelowQ) {
		prepareStaffBelowNoteStems(token);
	}
}



//////////////////////////////
//
// HumdrumFileContent::prepareStaffAboveNoteStems --
//

void HumdrumFileContent::prepareStaffAboveNoteStems(HTp token) {
	token->setValue("auto", "stem.dir", "-1");
	int track = token->getTrack();
	HTp curr = token->getNextFieldToken();
	int ttrack;

	// Find the next higher **kern spine (if any):
	while (curr) {
		ttrack = curr->getTrack();
		if (!curr->isKern()) {
			curr = curr->getNextFieldToken();
			continue;
		}
		if (ttrack == track) {
			curr = curr->getNextFieldToken();
			continue;
		}
		// is kern data and in a different spine
		break;
	}
	if (!curr) {
		// no higher staff of **kern data.
		return;
	}
	if (!curr->isKern()) {
		// something strange happened
		return;
	}
	HumNum endtime = token->getDurationFromStart() + token->getDuration();
	HTp curr2 = curr;
	while (curr2) {
		if (curr2->getDurationFromStart() >= endtime) {
			// exceeded the duration of the cross-staff note, so stop looking
			break;
		}
		if (!curr2->isData()) {
			// ignore non-data tokens
			curr2 = curr2->getNextToken();
			continue;
		}
		if (curr2->isNull()) {
			curr2 = curr2->getNextToken();
			continue;
		}
		if (curr2->isRest()) {
			// ignore rests
			curr2 = curr2->getNextToken();
			continue;
		}
		if (!curr2->isNote()) {
			curr2 = curr2->getNextToken();
			continue;
		}
		if ((curr2->find('/') != std::string::npos) || (curr2->find('\\') != std::string::npos)) {
			// the note/chord has a stem direction, so ignore it
			curr2 = curr2->getNextToken();
			continue;
		}
		int layer = curr2->getSubtrack();
		// layer != 0 means there is more than one active layer at this point in the
		// above staff.  If so, then do not assign any stem directions.
		if (layer != 0) {
			curr2 = curr2->getNextToken();
			continue;
		}
		// set the stem to up for the current note/chord
		curr2->setValue("auto", "stem.dir", "1");
		curr2 = curr2->getNextToken();
	}
}



//////////////////////////////
//
// HumdrumFileContent::prepareStaffBelowNoteStems --
//

void HumdrumFileContent::prepareStaffBelowNoteStems(HTp token) {
	token->setValue("auto", "stem.dir", "1");
	int track = token->getTrack();
	HTp curr = token->getPreviousFieldToken();
	int ttrack;

	// Find the next lower **kern spine (if any):
	while (curr) {
		ttrack = curr->getTrack();
		if (!curr->isKern()) {
			curr = curr->getPreviousFieldToken();
			continue;
		}
		if (ttrack == track) {
			curr = curr->getPreviousFieldToken();
			continue;
		}
		// is kern data and in a different spine
		break;
	}
	if (!curr) {
		// no lower staff of **kern data.
		return;
	}
	if (!curr->isKern()) {
		// something strange happened
		return;
	}

	// Find the first subtrack of the identified spine
	int targettrack = curr->getTrack();
	while (curr) {
		HTp ptok = curr->getPreviousFieldToken();
		if (!ptok) {
			break;
		}
		ttrack = ptok->getTrack();
		if (targettrack != ttrack) {
			break;
		}
		curr = ptok;
		ptok = curr->getPreviousToken();
	}
	// Should now be at the first subtrack of the target staff.

	HumNum endtime = token->getDurationFromStart() + token->getDuration();
	HTp curr2 = curr;
	while (curr2) {
		if (curr2->getDurationFromStart() >= endtime) {
			// exceeded the duration of the cross-staff note, so stop looking
			break;
		}
		if (!curr2->isData()) {
			// ignore non-data tokens
			curr2 = curr2->getNextToken();
			continue;
		}
		if (curr2->isNull()) {
			curr2 = curr2->getNextToken();
			continue;
		}
		if (curr2->isRest()) {
			// ignore rests
			curr2 = curr2->getNextToken();
			continue;
		}
		if (!curr2->isNote()) {
			curr2 = curr2->getNextToken();
			continue;
		}
		if ((curr2->find('/') != std::string::npos) || (curr2->find('\\') != std::string::npos)) {
			// the note/chord has a stem direction, so ignore it
			curr2 = curr2->getNextToken();
			continue;
		}
		int layer = curr2->getSubtrack();
		// layer != 0 means there is more than one active layer at this point in the
		// above staff.  If so, then do not assign any stem directions.
		if (layer != 0) {
			curr2 = curr2->getNextToken();
			continue;
		}
		// set the stem to up for the current note/chord
		curr2->setValue("auto", "stem.dir", "-1");
		curr2 = curr2->getNextToken();
	}
}


//////////////////////////////
//
// HumdrumFileContent::getNoteCount -- Returns the number of notes in **kern spines.
//    could be expanded to **mens, and kern-like sorts of spines.  Also could be
//    expanded to all staff-like spines, or specific spines.
//

int HumdrumFileContent::getNoteCount(void) {
	HumdrumFileContent& infile = *this;
	int counter = 0;

	int scount = infile.getStrandCount();
	for (int i=0; i<scount; i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}
		HTp send = infile.getStrandEnd(i);
		HTp current = sstart;
		while (current && (current != send)) {
			if (!current->isData()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isNull()) {
				current = current->getNextToken();
				continue;
			}
			if (current->isRest()) {
				current = current->getNextToken();
				continue;
			}
			int subcount = current->getSubtokenCount();
			if (subcount == 1) {
				if (!current->isSecondaryTiedNote()) {
					counter++;
				}
			} else {
				vector<string> subtokens = current->getSubtokens();
				for (int i=0; i<(int)subtokens.size(); i++) {
					if (subtokens[i].find("_") != string::npos) {
						continue;
					}
					if (subtokens[i].find("]") != string::npos) {
						continue;
					}
					if (subtokens[i].find("r") != string::npos) {
						continue;
					}
					counter++;
				}
			}
			current = current->getNextToken();
		}
	}
	return counter;
}



// END_MERGE

} // end namespace hum



