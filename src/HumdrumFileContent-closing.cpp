//
// Programmer:    Alexander Morgan
// Creation Date: Sun Aug 16 2026
// Filename:      HumdrumFileContent-closing.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-closing.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Identify where each voice stops sounding.  A "closingRest" is
//                the first rest after one or more notes in a voice, and the
//                "closingAttack" is the last note attack in that voice before
//                that rest, or before the end of the voice, which counts as a
//                rest as well.  Both are stored as "auto" token parameters:
//
//                    closingRest   == "1" on the rest that ends a note group
//                    closingAttack == "1" on the last attack before that rest
//
//                Together they mark the events where a voice is about to drop
//                out, which Tool_closing sums across voices to evaluate
//                potential cadence points.
//

#include "HumdrumFileContent.h"

#include <set>
#include <utility>

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeClosingRests -- Mark the closing rests and closing
//     attacks in every **kern voice.  Returns true if there was a voice to
//     process.
//

bool HumdrumFileContent::analyzeClosingRests(void) {
	bool output = false;
	for (HTp kernstart : getKernSpineStartList()) {
		output |= analyzeClosingRests(kernstart);
	}
	return output;
}



//////////////////////////////
//
// HumdrumFileContent::analyzeClosingRests -- Process a single voice, marking
//     each closingRest (the first rest after one or more notes) and the matching
//     closingAttack (the last note attack before that rest).  The end of the
//     voice counts as a rest, so the last attack of the voice is a closingAttack
//     even when no rest follows it.
//
//     Subspines (layers) are followed as separate paths, and a path stops where
//     it meets a token that another path already processed (i.e., where
//     subspines merge again).
//

bool HumdrumFileContent::analyzeClosingRests(HTp spinestart) {
	if (!spinestart || !spinestart->isStaffLike()) {
		return false;
	}

	// Each entry is a token to process, paired with the most recent note attack
	// before it (NULL if the previous event was a rest or there was no note).
	vector<pair<HTp, HTp>> tovisit;
	tovisit.push_back(make_pair(spinestart, (HTp)NULL));
	set<HTp> visited;

	while (!tovisit.empty()) {
		HTp current    = tovisit.back().first;
		HTp lastattack = tovisit.back().second;
		tovisit.pop_back();
		if (!current || visited.count(current)) {
			continue;
		}
		visited.insert(current);

		if (current->isData() && !current->isNull()) {
			if (current->isRest()) {
				if (lastattack) {
					current->setValue("auto", "closingRest", "1");
					lastattack->setValue("auto", "closingAttack", "1");
					lastattack = NULL;
				}
			} else if (current->isNoteAttack()) {
				lastattack = current;
			}
			// A tied continuation keeps the attack that started it.
		}

		int count = current->getNextTokenCount();
		if ((count == 0) && lastattack) {
			// The end of the voice acts as a rest, so its last attack closes.
			lastattack->setValue("auto", "closingAttack", "1");
		}

		// Pushed in reverse so that the first subspine is processed first and
		// its state is the one that carries past a merge.
		for (int i=count-1; i>=0; i--) {
			tovisit.push_back(make_pair(current->getNextToken(i), lastattack));
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::isClosingRest -- Returns true if the token is the first
//     rest after one or more notes in its voice.  Run analyzeClosingRests()
//     first.
//

bool HumdrumFileContent::isClosingRest(HTp token) {
	return token && token->getValueBool("auto", "closingRest");
}



//////////////////////////////
//
// HumdrumFileContent::isClosingAttack -- Returns true if the token is the last
//     note attack in its voice before a closing rest or before the end of the
//     voice.  Run analyzeClosingRests() first.
//

bool HumdrumFileContent::isClosingAttack(HTp token) {
	return token && token->getValueBool("auto", "closingAttack");
}



//////////////////////////////
//
// HumdrumFileContent::isClosingEvent -- Returns true if the token is a
//     closingAttack or a closingRest, meaning that its voice stops sounding at
//     this event or at its next attack.  Run analyzeClosingRests() first.
//

bool HumdrumFileContent::isClosingEvent(HTp token) {
	return isClosingAttack(token) || isClosingRest(token);
}


// END_MERGE

} // end namespace hum
