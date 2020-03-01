//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Tue Nov  8 19:59:10 PST 2016
// Filename:      HumGrid.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/HumGrid.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#include "HumGrid.h"
#include "Convert.h"

#include <string.h>

#include <stdio.h>
#include <iomanip>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumGrid::HumGrid -- Constructor.
//

HumGrid::HumGrid(void) {
	// Limited to 100 parts:
	m_verseCount.resize(100);
	m_harmonyCount.resize(100);
	m_dynamics.resize(100);
	m_figured_bass.resize(100);
	fill(m_dynamics.begin(), m_dynamics.end(), false);
	fill(m_figured_bass.begin(), m_figured_bass.end(), false);
	fill(m_harmonyCount.begin(), m_harmonyCount.end(), 0);

	// default options
	m_musicxmlbarlines = false;
	m_recip = false;
	m_pickup = false;
}



//////////////////////////////
//
// HumGrid::~HumGrid -- Deconstructor.
//

HumGrid::~HumGrid(void) {
	for (int i=0; i<(int)this->size(); i++) {
		if (this->at(i)) {
			delete this->at(i);
		}
	}
}



//////////////////////////////
//
// HumGrid::addMeasureToBack -- Allocate a GridMeasure at the end of the
//     measure list.
//

GridMeasure* HumGrid::addMeasureToBack(void) {
	GridMeasure* gm = new GridMeasure(this);
	this->push_back(gm);
	return this->back();
}



//////////////////////////////
//
// HumGrid::enableRecipSpine --
//

void HumGrid::enableRecipSpine(void) {
	m_recip = true;
}



//////////////////////////////
//
// HumGrid::getPartCount -- Return the number of parts in the Grid
//   by looking at the number of parts in the first spined GridSlice.
//

int  HumGrid::getPartCount(void) {
	if (!m_allslices.empty()) {
		return (int)m_allslices[0]->size();
	}

	if (this->empty()) {
		return 0;
	}

	if (this->at(0)->empty()) {
		return 0;
	}

	return (int)this->at(0)->back()->size();
}



//////////////////////////////
//
// HumGrid::getStaffCount --
//

int HumGrid::getStaffCount(int partindex) {
	if (this->empty()) {
		return 0;
	}

	if (this->at(0)->empty()) {
		return 0;
	}

	return (int)this->at(0)->back()->at(partindex)->size();
}



//////////////////////////////
//
// HumGrid::getHarmonyCount --
//

int HumGrid::getHarmonyCount(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_harmonyCount.size())) {
		return 0;
	}
	return m_harmonyCount.at(partindex);
}



//////////////////////////////
//
// HumGrid::getDynamicsCount --
//

int HumGrid::getDynamicsCount(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_dynamics.size())) {
		return 0;
	}
	return m_dynamics[partindex];
}



//////////////////////////////
//
// HumGrid::getFiguredBassCount --
//

int HumGrid::getFiguredBassCount(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_figured_bass.size())) {
		return 0;
	}
	return m_figured_bass[partindex];
}



//////////////////////////////
//
// HumGrid::getVerseCount --
//

int HumGrid::getVerseCount(int partindex, int staffindex) {
	if ((partindex < 0) || (partindex >= (int)m_verseCount.size())) {
		return 0;
	}
	int staffnumber = staffindex + 1;
	if ((staffnumber < 1) ||
			(staffnumber >= (int)m_verseCount.at(partindex).size())) {
		return 0;
	}
	int value = m_verseCount.at(partindex).at(staffnumber);
	return value;
}



//////////////////////////////
//
// HumGrid::hasDynamics -- Return true if there are any dyanmics for the part.
//

bool HumGrid::hasDynamics(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_dynamics.size())) {
		return false;
	}
	return m_dynamics[partindex];
}



//////////////////////////////
//
// HumGrid::hasFiguredBass -- Return true if there is any figured bass for the part.
//

bool HumGrid::hasFiguredBass(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_figured_bass.size())) {
		return false;
	}
	return m_figured_bass[partindex];
}



//////////////////////////////
//
// HumGrid::setDynamicsPresent -- Indicate that part needs a **dynam spine.
//

void HumGrid::setDynamicsPresent(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_dynamics.size())) {
		return;
	}
	m_dynamics[partindex] = true;
}



//////////////////////////////
//
// HumGrid::setFiguredBassPresent -- Indicate that part needs a **fb spine.
//

void HumGrid::setFiguredBassPresent(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_figured_bass.size())) {
		return;
	}
	m_figured_bass[partindex] = true;
}



//////////////////////////////
//
// HumGrid::setHarmonyPresent -- Indicate that part needs a **harm spine.
//

void HumGrid::setHarmonyPresent(int partindex) {
	if ((partindex < 0) || (partindex >= (int)m_harmony.size())) {
		return;
	}
	m_harmony[partindex] = true;
}



//////////////////////////////
//
// HumGrid::setHarmonyCount -- part size hardwired to 100 for now.
//

void HumGrid::setHarmonyCount(int partindex, int count) {
	if ((partindex < 0) || (partindex > (int)m_harmonyCount.size())) {
		return;
	}
	m_harmonyCount[partindex] = count;
}



//////////////////////////////
//
// HumGrid::reportVerseCount --
//

void HumGrid::reportVerseCount(int partindex, int staffindex, int count) {
	if (count <= 0) {
		return;
	}
	int staffnumber = staffindex + 1;
	int partsize = (int)m_verseCount.size();
	if (partindex >= partsize) {
		m_verseCount.resize(partindex+1);
	}
	int staffcount = (int)m_verseCount.at(partindex).size();
	if (staffnumber >= staffcount) {
		m_verseCount.at(partindex).resize(staffnumber+1);
		for (int i=staffcount; i<=staffnumber; i++) {
			m_verseCount.at(partindex).at(i) = 0;
		}
	}
	if (count > m_verseCount.at(partindex).at(staffnumber)) {
		m_verseCount.at(partindex).at(staffnumber) = count;
	}
}



//////////////////////////////
//
// HumGrid::setVerseCount --
//

void HumGrid::setVerseCount(int partindex, int staffindex, int count) {
	if ((partindex < 0) || (partindex > (int)m_verseCount.size())) {
		return;
	}
	int staffnumber = staffindex + 1;
	if (staffnumber < 0) {
		return;
	}
	if (staffnumber < (int)m_verseCount.at(partindex).size()) {
		m_verseCount.at(partindex).at(staffnumber) = count;
	} else {
		int oldsize = (int)m_verseCount.at(partindex).size();
		int newsize = staffnumber + 1;
		m_verseCount.at(partindex).resize(newsize);
		for (int i=oldsize; i<newsize; i++) {
			m_verseCount.at(partindex).at(i) = 0;
		}
		m_verseCount.at(partindex).at(staffnumber) = count;
	}
}



//////////////////////////////
//
// HumGrid::transferTokens --
//   default value: startbarnum = 0.
//

bool HumGrid::transferTokens(HumdrumFile& outfile, int startbarnum) {
	bool status = buildSingleList();
	if (!status) {
		return false;
	}
	calculateGridDurations();
	addNullTokens();
	addInvisibleRestsInFirstTrack();
	addMeasureLines();
	buildSingleList();  // is this needed a second time?
	cleanTempos();
	addLastMeasure();
	if (manipulatorCheck()) {
		cleanupManipulators();
	}

	insertPartNames(outfile);
	insertStaffIndications(outfile);
	insertPartIndications(outfile);
	insertExclusiveInterpretationLine(outfile);
	bool addstartbar = (!hasPickup()) && (!m_musicxmlbarlines);
	for (int m=0; m<(int)this->size(); m++) {
		if (addstartbar && m == 0) {
			status &= at(m)->transferTokens(outfile, m_recip, addstartbar, startbarnum);
		} else {
			status &= at(m)->transferTokens(outfile, m_recip, false);
		}
		if (!status) {
			break;
		}
	}
	insertDataTerminationLine(outfile);
	return true;
}



//////////////////////////////
//
// HumGrid::cleanupManipulators --
//

void HumGrid::cleanupManipulators(void) {
	GridSlice* current = NULL;
	GridSlice* last = NULL;
	vector<GridSlice*> newslices;
	for (int m=0; m<(int)this->size(); m++) {
		for (auto it = this->at(m)->begin(); it != this->at(m)->end(); it++) {
			last = current;
			current = *it;
			if ((*it)->getType() != SliceType::Manipulators) {
				if (last && (last->getType() != SliceType::Manipulators)) {
					matchVoices(current, last);
				}
				continue;
			}
			if (last && (last->getType() != SliceType::Manipulators)) {
				matchVoices(current, last);
			}
			// check to see if manipulator needs to be split into
			// multiple lines.
			newslices.resize(0);
			cleanManipulator(newslices, *it);
			if (newslices.size()) {
				for (int j=0; j<(int)newslices.size(); j++) {
					this->at(m)->insert(it, newslices.at(j));
				}
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::cleanManipulator --
//

void HumGrid::cleanManipulator(vector<GridSlice*>& newslices, GridSlice* curr) {
	newslices.resize(0);
	GridSlice* output;

	// deal with *^ manipulators:
	while ((output = checkManipulatorExpand(curr))) {
		newslices.push_back(output);
	}

	// deal with *v manipulators:
	while ((output = checkManipulatorContract(curr))) {
		newslices.push_back(output);
	}
}



//////////////////////////////
//
// HumGrid::checkManipulatorExpand -- Check for cases where a spine expands
//    into sub-spines.
//

GridSlice* HumGrid::checkManipulatorExpand(GridSlice* curr) {
	GridStaff* staff     = NULL;
	GridPart*  part      = NULL;
	GridVoice* voice     = NULL;
	HTp        token     = NULL;
	bool       neednew   = false;

	int p, s, v;
	int partcount = (int)curr->size();
	int staffcount;

	for (p=0; p<partcount; p++) {
		part = curr->at(p);
		staffcount = (int)part->size();
		for (s=0; s<staffcount; s++) {
			staff = part->at(s);
			for (v=0; v<(int)staff->size(); v++) {
				voice = staff->at(v);
				token = voice->getToken();
				if (token->compare(0, 2, "*^") == 0) {
					if ((token->size() > 2) && isdigit((*token)[2])) {
						neednew = true;
						break;
					}
				}
			}
			if (neednew) {
				break;
			}
		}
		if (neednew) {
			break;
		}
	}

	if (neednew == false) {
		return NULL;
	}

	// need to split *^#'s into separate *^

	GridSlice* newmanip = new GridSlice(curr->getMeasure(), curr->getTimestamp(),
	curr->getType(), curr);

	for (p=0; p<partcount; p++) {
		part = curr->at(p);
		staffcount = (int)part->size();
		for (s=0; s<staffcount; s++) {
			staff = part->at(s);
			adjustExpansionsInStaff(newmanip, curr, p, s);
		}
	}
	return newmanip;
}



//////////////////////////////
//
// HumGrid::adjustExpansionsInStaff -- duplicate null
//   manipulators, and expand large-expansions, such as *^3 into
//   *^ and *^ on the next line, or *^4 into *^ and *^3 on the
//   next line.  The "newmanip" will be placed before curr, so
//

void HumGrid::adjustExpansionsInStaff(GridSlice* newmanip, GridSlice* curr, int p, int s) {
	HTp token = NULL;
	GridVoice* newvoice  = NULL;
	GridVoice* curvoice  = NULL;
	GridStaff* newstaff  = newmanip->at(p)->at(s);
	GridStaff* curstaff  = curr->at(p)->at(s);

	int originalsize = (int)curstaff->size();
	int cv = 0;

	for (int v=0; v<originalsize; v++) {
		curvoice = curstaff->at(cv);
		token = curvoice->getToken();

		if (token->compare(0, 2, "*^") == 0) {
			if ((token->size() > 2) && isdigit((*token)[2])) {
				// transfer *^ to newmanip and replace with * and *^(n-1) in curr
				// Convert *^3 to *^ and add ^* to next line, for example
				// Convert *^4 to *^ and add ^*3 to next line, for example
				int count = 0;
				if (!sscanf(token->c_str(), "*^%d", &count)) {
					cerr << "Error finding expansion number" << endl;
				}
				newstaff->push_back(curvoice);
				curvoice->getToken()->setText("*^");
				newvoice = createVoice("*", "B", 0, p, s);
				curstaff->at(cv) = newvoice;
				if (count <= 3) {
					newvoice = new GridVoice("*^", 0);
				} else {
					newvoice = new GridVoice("*^" + to_string(count-1), 0);
				}
				curstaff->insert(curstaff->begin()+cv+1, newvoice);
				cv++;
				continue;
			} else {
				// transfer *^ to newmanip and replace with two * in curr
				newstaff->push_back(curvoice);
				newvoice = createVoice("*", "C", 0, p, s);
				curstaff->at(cv) = newvoice;
				newvoice = createVoice("*", "D", 0, p, s);
				curstaff->insert(curstaff->begin()+cv, newvoice);
				cv++;
				continue;
			}
		} else {
			// insert * in newmanip
			newvoice = createVoice("*", "E", 0, p, s);
			newstaff->push_back(newvoice);
			cv++;
			continue;
		}

	}
}



//////////////////////////////
//
// HumGrid::checkManipulatorContract -- Will only check for adjacent
//    *v records across adjacent staves, which should be good enough.
//    Will not check within a staff, but this should not occur within
//    MusicXML input data due to the way it is being processed.
//    The return value is a newly created GridSlice pointer which contains
//    a new manipulator to add to the file (and the current manipultor
//    slice will also be modified if the return value is not NULL).
//

GridSlice* HumGrid::checkManipulatorContract(GridSlice* curr) {
	GridVoice* lastvoice = NULL;
	GridVoice* voice     = NULL;
	GridStaff* staff     = NULL;
	GridPart*  part      = NULL;
	bool       neednew   = false;

	int p, s;
	int partcount = (int)curr->size();
	int staffcount;
	bool init = false;
	for (p=partcount-1; p>=0; p--) {
		part  = curr->at(p);
		staffcount = (int)part->size();
		for (s=staffcount-1; s>=0; s--) {
			staff = part->at(s);
			if (staff->empty()) {
				continue;
			}
			voice = staff->back();
			if (!init) {
				lastvoice = staff->back();
				init = true;
				continue;
			}
			if (lastvoice != NULL) {
           	if ((*voice->getToken() == "*v") &&
						(*lastvoice->getToken() == "*v")) {
					neednew = true;
					break;
				}
			}
			lastvoice = staff->back();
		}
		if (neednew) {
			break;
		}
	}

	if (neednew == false) {
		return NULL;
	}

	// need to split *v's from different adjacent staves onto separate lines.
	GridSlice* newmanip = new GridSlice(curr->getMeasure(), curr->getTimestamp(),
		curr->getType(), curr);
	lastvoice = NULL;
	GridStaff* laststaff    = NULL;
	GridStaff* newstaff     = NULL;
	GridStaff* newlaststaff = NULL;
	bool foundnew = false;
	partcount = (int)curr->size();
	int lastp = 0;
	int lasts = 0;
	int partsplit = -1;
	int voicecount;

	for (p=partcount-1; p>=0; p--) {
		part  = curr->at(p);
		staffcount = (int)part->size();
		for (s=staffcount-1; s>=0; s--) {
			staff = part->at(s);
			voicecount = (int)staff->size();
			voice = staff->back();
			newstaff = newmanip->at(p)->at(s);
			if (lastvoice != NULL) {
           	if ((*voice->getToken() == "*v") &&
						(*lastvoice->getToken() == "*v")) {
               // splitting the slices at this staff boundary

					newlaststaff = newmanip->at(lastp)->at(lasts);
					transferMerges(staff, laststaff, newstaff, newlaststaff, p, s);
					foundnew = true;
					partsplit = p;
					break;
				}
			} else {
				if (voicecount > 1) {
					for (int j=(int)newstaff->size(); j<voicecount; j++) {
						// GridVoice* vdata = createVoice("*", "F", 0, p, s);
						// newstaff->push_back(vdata);
					}
				}
			}
			laststaff = staff;
			lastvoice = laststaff->back();
			lastp = p;
			lasts = s;
		}

		if (foundnew) {
			// transfer all of the subsequent manipulators forward
			// after the staff/newstaff point in the slice
			if (partsplit > 0) {
				transferOtherParts(curr, newmanip, partsplit);
			}
			break;
		}
	}

	// fill in any missing voice null interpretation tokens
	adjustVoices(curr, newmanip, partsplit);

	return newmanip;
}



//////////////////////////////
//
// HumGrid::adjustVoices --
//

void HumGrid::adjustVoices(GridSlice* curr, GridSlice* newmanip, int partsplit) {
	int p1count = (int)curr->size();
	// int p2count = (int)newmanip->size();
	//cerr << "PARTSPLIT " << partsplit << endl;
	for (int p=0; p<p1count; p++) {
		int s1count = (int)curr->at(p)->size();
		// int s2count = (int)curr->at(p)->size();
		// cerr << "\tCURR STAVES " << s1count << "\tNEWM STAVES " << s2count << endl;
		// cerr << "\t\tCURR SCOUNT = " << curr->at(p)->size() << "\tNEWM SCOUNT = " << newmanip->at(p)->size() << endl;
		for (int s=0; s<s1count; s++) {
			GridStaff* s1 = curr->at(p)->at(s);
			GridStaff* s2 = newmanip->at(p)->at(s);
			if ((s1->size() == 0) && (s2->size() > 0)) {
				createMatchedVoiceCount(s1, s2, p, s);
			} else if ((s2->size() == 0) && (s1->size() > 0)) {
				createMatchedVoiceCount(s2, s1, p, s);
			}
			// cerr << "\t\t\tCURR VCOUNT = " << curr->at(p)->at(s)->size() << "\t(" << curr->at(p)->at(s)->getString() << ")" << "\t";
			// cerr << "\tNEWM VCOUNT = " << newmanip->at(p)->at(s)->size() << "\t(" << newmanip->at(p)->at(s)->getString() << ")" << endl;
		}
	}
}



//////////////////////////////
//
// HumGrid::createMatchedVoiceCount --
//

void HumGrid::createMatchedVoiceCount(GridStaff* snew, GridStaff* sold, int p, int s) {
	if (snew->size() != 0) {
		// this function is only for creating a totally new
		return;
	}
	int count = (int)sold->size();
	snew->resize(count);
	for (int i=0; i<count; i++) {
		GridVoice* gv = createVoice("*", "N", p, s, i);
		snew->at(i) = gv;
	}
}



//////////////////////////////
//
// HumGrid::matchVoices --
//

void HumGrid::matchVoices(GridSlice* current, GridSlice* last) {
	if (current == NULL) {
		return;
	}
	if (last == NULL) {
		return;
	}
	int pcount1 = (int)current->size();
	int pcount2 = (int)current->size();
	if (pcount1 != pcount2) {
		return;
	}
	for (int i=0; i<pcount1; i++) {
		GridPart* part1 = current->at(i);
		GridPart* part2 = current->at(i);
		int scount1 = (int)part1->size();
		int scount2 = (int)part2->size();
		if (scount1 != scount2) {
			continue;
		}
		for (int j=0; j<scount1; j++) {
			GridStaff* staff1 = part1->at(j);
			GridStaff* staff2 = part2->at(j);
			int vcount1 = (int)staff1->size();
			int vcount2 = (int)staff2->size();
			if (vcount1 == vcount2) {
				continue;
			}
			if (vcount2 > vcount1) {
				// strange if it happens
				continue;
			}
			int difference = vcount1 - vcount2;
			for (int k=0; k<difference; k++) {
				GridVoice* gv = createVoice("*", "A", 0, i, j);
				staff2->push_back(gv);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::transferOtherParts -- after a line split due to merges
//    occurring at the same time.
//

void HumGrid::transferOtherParts(GridSlice* oldline, GridSlice* newline, int maxpart) {
	GridPart* temp;
	int partcount = (int)oldline->size();
	if (maxpart >= partcount) {
		return;
	}
	for (int i=0; i<maxpart; i++) {
		temp = oldline->at(i);
		oldline->at(i) = newline->at(i);
		newline->at(i) = temp;
		// duplicate the voice counts on the old line (needed if there are more
		// than one voices in a staff when splitting a line due to *v merging.
		for (int j=0; j<(int)oldline->at(i)->size(); j++) {
			int voices = (int)newline->at(i)->at(j)->size();
			int adjustment = 0;
			for (int k=0; k<voices; k++) {
				if (!newline->at(i)->at(j)->at(k)) {
					continue;
				}
				HTp tok = newline->at(i)->at(j)->at(k)->getToken();
				if (*tok == "*v") {
					adjustment++;
				}
			}
			if (adjustment > 0) {
				adjustment--;
			}
			voices -= adjustment;
			oldline->at(i)->at(j)->resize(voices);
			for (int k=0; k<voices; k++) {
				oldline->at(i)->at(j)->at(k) = createVoice("*", "Z", 0, i, j);
			}
		}
	}

	for (int p=0; p<(int)newline->size(); p++) {
			GridPart* newpart = newline->at(p);
			GridPart* oldpart = oldline->at(p);
		for (int s=0; s<(int)newpart->size(); s++) {
			GridStaff* newstaff = newpart->at(s);
			GridStaff* oldstaff = oldpart->at(s);
			if (newstaff->size() >= oldstaff->size()) {
				continue;
			}
			int diff = (int)(oldstaff->size() - newstaff->size());

			for (int v=0; v<diff; v++) {
				GridVoice* voice = createVoice("*", "G", 0, p, s);
				newstaff->push_back(voice);
			}

		}
	}
}



//////////////////////////////
//
// HumGrid::transferMerges -- Move *v spines from one staff to last staff,
//   and re-adjust staff "*v" tokens to a single "*" token.
// Example:
//                 laststaff      staff
// old:            *v   *v        *v   *v
// converts to:
// new:            *v   *v        *    *
// old:            *              *v   *v
//
//
//

void HumGrid::transferMerges(GridStaff* oldstaff, GridStaff* oldlaststaff,
		GridStaff* newstaff, GridStaff* newlaststaff, int pindex, int sindex) {
	if ((oldstaff == NULL) || (oldlaststaff == NULL)) {
		cerr << "Weird error in HumGrid::transferMerges()" << endl;
		return;
	}
	// New staves are presumed to be totally empty.

	GridVoice* gv;

	// First create "*" tokens for newstaff slice where there are
	// "*v" in old staff.  All other tokens should be set to "*".
	int tcount = (int)oldstaff->size();
	int t;
	for (t=0; t<tcount; t++) {
		if (*oldstaff->at(t)->getToken() == "*v") {
			gv = createVoice("*", "H", 0, pindex, sindex);
			newstaff->push_back(gv);
		} else {
			gv = createVoice("*", "I", 0, pindex, sindex);
			newstaff->push_back(gv);
		}
	}

	// Next, all "*v" tokens at end of old previous staff should be
	// transferred to the new previous staff and replaced with
	// a single "*" token.  Non "*v" tokens in the old last staff should
	// be converted to "*" tokens in the new last staff.
	//
	// It may be possible for *v tokens to not be only at the end of
	// the list of oldlaststaff tokens, but does not seem possible.

	tcount = (int)oldlaststaff->size();
	bool addednull = false;
	for (t=0; t<tcount; t++) {
		if (*oldlaststaff->at(t)->getToken() == "*v") {
			newlaststaff->push_back(oldlaststaff->at(t));
			if (addednull == false) {
				gv = createVoice("*", "J", 0, pindex, sindex);
				oldlaststaff->at(t) = gv;
				addednull = true;
			} else {
				oldlaststaff->at(t) = NULL;
			}
		} else {
			gv = createVoice("*", "K", 0, pindex, sindex);
			newlaststaff->push_back(gv);
		}
	}

	// Go back to the oldlaststaff and chop off all ending NULLs
	// * it should never get to zero (there should be at least one "*" left.
	// In theory intermediate NULLs should be checked for, and if they
	// exist, then something bad will happen.  But it does not seem
	// possible to have intermediate NULLs.
	tcount = (int)oldlaststaff->size();
	for (t=tcount-1; t>=0; t--) {
		if (oldlaststaff->at(t) == NULL) {
			int newsize = (int)oldlaststaff->size() - 1;
			oldlaststaff->resize(newsize);
		}
	}
}



//////////////////////////////
//
// HumGrid::createVoice -- create voice with given token contents.
//

GridVoice* HumGrid::createVoice(const string& tok, const string& post, HumNum duration, int pindex, int sindex) {
	//std::string token = tok;
	//token += ":" + post + ":" + to_string(pindex) + "," + to_string(sindex);
	GridVoice* gv = new GridVoice(tok.c_str(), 0);
	return gv;
}



//////////////////////////////
//
// HumGrid::getNextSpinedLine -- Find next spined GridSlice.
//

GridSlice* HumGrid::getNextSpinedLine(const GridMeasure::iterator& it, int measureindex) {
	auto nextone = it;
	nextone++;
	while (nextone != this->at(measureindex)->end()) {
		if ((*nextone)->hasSpines()) {
			break;
		}
		nextone++;
	}

	if (nextone != this->at(measureindex)->end()) {
		return *nextone;
	}

	measureindex++;
	if (measureindex >= (int)this->size()) {
		// end of data, so nothing to adjust with
		// but this should never happen in general.
		return NULL;
	}
	nextone = this->at(measureindex)->begin();
	while (nextone != this->at(measureindex)->end()) {
		if ((*nextone)->hasSpines()) {
			return *nextone;
		}
		nextone++;
	}

	return NULL;
}



//////////////////////////////
//
// HumGrid::manipulatorCheck --
//

bool HumGrid::manipulatorCheck(void) {
	GridSlice* manipulator;
	int m;
	GridSlice* s1;
	GridSlice* s2;
	bool output = false;
	for (m=0; m<(int)this->size(); m++) {
		if (this->at(m)->size() == 0) {
			continue;
		}
		for (auto it = this->at(m)->begin(); it != this->at(m)->end(); it++) {
			if (!(*it)->hasSpines()) {
				// Don't monitor manipulators on no-spined lines.
				continue;
			}
			s1 = *it;
			s2 = getNextSpinedLine(it, m);

			manipulator = manipulatorCheck(s1, s2);
			if (manipulator == NULL) {
				continue;
			}
			output = true;
			auto inserter = it;
			inserter++;
			this->at(m)->insert(inserter, manipulator);
			it++; // skip over the new manipulator line (expand it later)
		}
	}
	return output;
}


//
// HumGrid::manipulatorCheck -- Look for differences in voice/layer count
//   for each part/staff pairing between adjacent lines.  If they do not match,
//   then add spine manipulator line to Grid between the two lines.
//

GridSlice* HumGrid::manipulatorCheck(GridSlice* ice1, GridSlice* ice2) {
	int p1count;
	int p2count;
	int s1count;
	int s2count;
	int v1count;
	int v2count;
	int p;
	int s;
	int v;
	bool needmanip = false;

	if (ice1 == NULL) {
		return NULL;
	}
	if (ice2 == NULL) {
		return NULL;
	}
	if (!ice1->hasSpines()) {
		return NULL;
	}
	if (!ice2->hasSpines()) {
		return NULL;
	}
	p1count = (int)ice1->size();
	p2count = (int)ice2->size();
	if (p1count != p2count) {
		cerr << "Warning: Something weird happend here" << endl;
		cerr << "p1count = " << p1count << endl;
		cerr << "p2count = " << p2count << endl;
		cerr << "ICE1: " << ice1 << endl;
		cerr << "ICE2: " << ice2 << endl;
		cerr << "The above two values should be the same." << endl;
		return NULL;
	}
	for (p=0; p<p1count; p++) {
		s1count = (int)ice1->at(p)->size();
		s2count = (int)ice2->at(p)->size();
		if (s1count != s2count) {
			cerr << "Warning: Something weird happend here with staff" << endl;
			return NULL;
		}
		for (s=0; s<s1count; s++) {
			v1count = (int)ice1->at(p)->at(s)->size();
			// the voice count always must be at least 1.  This case
			// is related to inserting clefs in other parts.
			if (v1count < 1) {
				v1count = 1;
			}
			v2count = (int)ice2->at(p)->at(s)->size();
			if (v2count < 1) {
				v2count = 1;
			}
			if (v1count == v2count) {
				continue;
			}
			needmanip = true;
			break;
		}
		if (needmanip) {
			break;
		}
	}

	if (!needmanip) {
		return NULL;
	}

	// build manipulator line (which will be expanded further if adjacent
	// staves have *v manipulators.

	GridSlice* mslice;
	mslice = new GridSlice(ice1->getMeasure(), ice2->getTimestamp(),
			SliceType::Manipulators);

	int z;
	HTp token;
	GridVoice* gv;
	p1count = (int)ice1->size();
	mslice->resize(p1count);
	for (p=0; p<p1count; p++) {
		mslice->at(p) = new GridPart;
		s1count = (int)ice1->at(p)->size();
		mslice->at(p)->resize(s1count);
		for (s=0; s<s1count; s++) {
			mslice->at(p)->at(s) = new GridStaff;
			v1count = (int)ice1->at(p)->at(s)->size();
			v2count = (int)ice2->at(p)->at(s)->size();
			if (v2count < 1) {
				// empty spines will be filled in with at least one null token.
				v2count = 1;
			}
			if (v1count < 1) {
				// empty spines will be filled in with at least one null token.
				v1count = 1;
			}
			if ((v1count == 0) && (v2count == 1)) {
				// grace note at the start of the measure in another voice
				// no longer can get here due to v1count min being 1.
				token = createHumdrumToken("*", p, s);
				gv = new GridVoice(token, 0);
				mslice->at(p)->at(s)->push_back(gv);
			} else if (v1count == v2count) {
				for (v=0; v<v1count; v++) {
					token = createHumdrumToken("*", p, s);
					gv = new GridVoice(token, 0);
					mslice->at(p)->at(s)->push_back(gv);
				}
			} else if (v1count < v2count) {
				// need to grow
				int grow = v2count - v1count;
				// if (grow == 2 * v1count) {
				if (v2count == 2 * v1count) {
					// all subspines split
					for (z=0; z<v1count; z++) {
						token = new HumdrumToken("*^");
						gv = new GridVoice(token, 0);
						mslice->at(p)->at(s)->push_back(gv);
					}
				} else if ((v1count > 0) && (grow > 2 * v1count)) {
					// too large to split all at the same time, deal with later
					for (z=0; z<v1count-1; z++) {
						token = new HumdrumToken("*^");
						gv = new GridVoice(token, 0);
						mslice->at(p)->at(s)->push_back(gv);
					}
					int extra = v2count - (v1count - 1) * 2;
					if (extra > 2) {
						token = new HumdrumToken("*^" + to_string(extra));
					} else {
						token = new HumdrumToken("*^");
					}
					gv = new GridVoice(token, 0);
					mslice->at(p)->at(s)->push_back(gv);
				} else {
					// only split spines at end of list
					int doubled = v2count - v1count;
					int notdoubled = v1count - doubled;
					for (z=0; z<notdoubled; z++) {
						token = createHumdrumToken("*", p, s);
						gv = new GridVoice(token, 0);
						mslice->at(p)->at(s)->push_back(gv);
					}
					//for (z=0; z<doubled; z++) {
						if (doubled > 1) {
							token = new HumdrumToken("*^" + to_string(doubled+1));
						} else {
							token = new HumdrumToken("*^");
						}
						// token = new HumdrumToken("*^");
						gv = new GridVoice(token, 0);
						mslice->at(p)->at(s)->push_back(gv);
					//}
				}
			} else if (v1count > v2count) {
				// need to shrink
				int shrink = v1count - v2count + 1;
				int notshrink = v1count - shrink;
				for (z=0; z<notshrink; z++) {
					token = createHumdrumToken("*", p, s);
					gv = new GridVoice(token, 0);
					mslice->at(p)->at(s)->push_back(gv);
				}
				for (z=0; z<shrink; z++) {
					token = new HumdrumToken("*v");
					gv = new GridVoice(token, 0);
					mslice->at(p)->at(s)->push_back(gv);
				}
			}
		}
	}
	return mslice;
}



//////////////////////////////
//
// HumGrid::createHumdrumToken --
//

HTp HumGrid::createHumdrumToken(const string& tok, int pindex, int sindex) {
	std::string token = tok;
	// token += ":" + to_string(pindex) + "," + to_string(sindex);
	HTp output = new HumdrumToken(token.c_str());
	return output;
}



//////////////////////////////
//
// HumGrid::addMeasureLines --
//

void HumGrid::addMeasureLines(void) {
	HumNum timestamp;
	GridSlice* mslice;
	GridSlice* endslice;
	GridPart* part;
	GridStaff* staff;
	GridVoice* gv;
	string token;
	int staffcount, partcount, vcount, nextvcount, lcount;
	GridMeasure* measure = NULL;
	GridMeasure* nextmeasure = NULL;

	vector<int> barnums;
	if (!m_musicxmlbarlines) {
		getMetricBarNumbers(barnums);
	}

	for (int m=0; m<(int)this->size()-1; m++) {
		measure = this->at(m);
		nextmeasure = this->at(m+1);
		if (nextmeasure->size() == 0) {
			// next measure is empty for some reason so give up
			continue;
		}
		GridSlice* firstspined = nextmeasure->getFirstSpinedSlice();
		timestamp = firstspined->getTimestamp();
		if (measure->size() == 0) {
			continue;
		}

		if (measure->getDuration() == 0) {
			continue;
		}
		mslice = new GridSlice(measure, timestamp, SliceType::Measures);
		// what to do when endslice is NULL?
		endslice = measure->getLastSpinedSlice(); // this has to come before next line
		measure->push_back(mslice); // this has to come after the previous line
		partcount = (int)firstspined->size();
		mslice->resize(partcount);

		for (int p=0; p<partcount; p++) {
			part = new GridPart();
			mslice->at(p) = part;
			staffcount = (int)firstspined->at(p)->size();
			mslice->at(p)->resize(staffcount);
			for (int s=0; s<(int)staffcount; s++) {
				staff = new GridStaff;
				mslice->at(p)->at(s) = staff;

				// insert the minimum number of barlines based on the
				// voices in the current and next measure.
				vcount = (int)endslice->at(p)->at(s)->size();
				if (firstspined) {
					nextvcount = (int)firstspined->at(p)->at(s)->size();
				} else {
					// perhaps an empty measure?  This will cause problems.
					nextvcount = 0;
				}
				lcount = vcount;
				if (lcount > nextvcount) {
					lcount = nextvcount;
				}
				if (lcount == 0) {
					lcount = 1;
				}
				for (int v=0; v<lcount; v++) {
					int num = measure->getMeasureNumber();
					if (m < (int)barnums.size() - 1) {
						num = barnums[m+1];
					}
					token = createBarToken(m, num, measure);
					gv = new GridVoice(token, 0);
					mslice->at(p)->at(s)->push_back(gv);
				}
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::createBarToken --
//

string HumGrid::createBarToken(int m, int barnum, GridMeasure* measure) {
	string token;
	string barstyle = getBarStyle(measure);
	string number = "";
	if (barnum > 0) {
		number = to_string(barnum);
	}
	if (m_musicxmlbarlines) {
		// m+1 because of the measure number
		// comes from the previous measure.
		if (barstyle == "=") {
			token = "==";
			token += to_string(m+1);
		} else {
			token = "=";
			token += to_string(m+1);
			token += barstyle;
		}
	} else {
		if (barnum > 0) {
			if (barstyle == "=") {
				token = "==";
				token += number;
			} else {
				token = "=";
				token += number;
				token += barstyle;
			}
		} else {
			if (barstyle == "=") {
				token = "==";
			} else {
				token = "=";
				token += barstyle;
			}
		}
	}
	return token;
}



//////////////////////////////
//
// HumGrid::getMetricBarNumbers --
//

void HumGrid::getMetricBarNumbers(vector<int>& barnums) {
	int mcount = (int)this->size();
	barnums.resize(mcount);

	if (mcount == 0) {
		return;
	}

	vector<HumNum> mdur(mcount);
	vector<HumNum> tsdur(mcount); // time signature duration

	for (int m=0; m<(int)this->size(); m++) {
		mdur[m]   = this->at(m)->getDuration();
		tsdur[m] = this->at(m)->getTimeSigDur();
		if (tsdur[m] <= 0) {
			tsdur[m] = mdur[m];
		}
	}

	int start = 0;
	if (!mdur.empty()) {
		if (mdur[0] == 0) {
			start = 1;
		}
	}

	// int counter = 1;  // this was causing a problem https://github.com/humdrum-tools/verovio-humdrum-viewer/issues/254
	int counter = 0;
	if (mdur[start] == tsdur[start]) {
		m_pickup = false;
		counter++;
		// add the initial barline later when creating HumdrumFile.
	} else {
		m_pickup = true;
	}

	for (int m=start; m<(int)this->size(); m++) {
		if ((m == start) && (mdur[m] == 0)) {
			barnums[m] = counter-1;
			continue;
		} else if (mdur[m] == 0) {
			barnums[m] = -1;
			continue;
		}
		if ((m < mcount-1) && (tsdur[m] == tsdur[m+1])) {
			if (mdur[m] + mdur[m+1] == tsdur[m]) {
				barnums[m] = -1;
			} else {
				barnums[m] = counter++;
			}
		} else {
			barnums[m] = counter++;
		}
	}
}



//////////////////////////////
//
// HumGrid::getBarStyle --
//

string HumGrid::getBarStyle(GridMeasure* measure) {
	string output = "";
	if (measure->isDouble()) {
		output = "||";
	} else if (measure->isFinal()) {
		output = "=";
	} else if (measure->isRepeatBoth()) {
		output = ":|!|:";
	} else if (measure->isRepeatBackward()) {
		output = ":|!";
	} else if (measure->isRepeatForward()) {
		output = "!|:";
	}
	return output;
}



//////////////////////////////
//
// HumGrid::addLastMeasure --
//

void HumGrid::addLastMeasure(void) {
   // add the last measure, which will be only one voice
	// for each part/staff.
	GridSlice* model = this->back()->back();
	if (model == NULL) {
		return;
	}

	// probably not the correct timestamp, but probably not important
	// to get correct:
	HumNum timestamp = model->getTimestamp();

	if (this->empty()) {
		return;
	}
	GridMeasure* measure = this->back();

	string barstyle = getBarStyle(measure);

	GridSlice* mslice = new GridSlice(model->getMeasure(), timestamp,
			SliceType::Measures);
	this->back()->push_back(mslice);
	mslice->setTimestamp(timestamp);
	int partcount = (int)model->size();
	mslice->resize(partcount);
	for (int p=0; p<partcount; p++) {
		GridPart* part = new GridPart();
		mslice->at(p) = part;
		int staffcount = (int)model->at(p)->size();
		mslice->at(p)->resize(staffcount);
		for (int s=0; s<staffcount; s++) {
			GridStaff* staff = new GridStaff;
			mslice->at(p)->at(s) = staff;
			HTp token = new HumdrumToken("=" + barstyle);
			GridVoice* gv = new GridVoice(token, 0);
			mslice->at(p)->at(s)->push_back(gv);
		}
	}
}



//////////////////////////////
//
// HumGrid::buildSingleList --
//

bool HumGrid::buildSingleList(void) {
	m_allslices.resize(0);

	int gridcount = 0;
	for (auto it : (vector<GridMeasure*>)*this) {
		gridcount += (int)it->size();
	}
	m_allslices.reserve(gridcount + 100);
	for (int m=0; m<(int)this->size(); m++) {
		for (auto it : (list<GridSlice*>)*this->at(m)) {
			m_allslices.push_back(it);
		}
	}

	HumNum ts1;
	HumNum ts2;
	HumNum dur;
	for (int i=0; i<(int)m_allslices.size() - 1; i++) {
		ts1 = m_allslices[i]->getTimestamp();
		ts2 = m_allslices[i+1]->getTimestamp();
		dur = (ts2 - ts1); // whole-note units
		m_allslices[i]->setDuration(dur);
	}
	return !m_allslices.empty();
}



//////////////////////////////
//
// HumGrid::addNullTokensForGraceNotes -- Avoid grace notes at
//     starts of measures from contracting the subspine count.
//

void HumGrid::addNullTokensForGraceNotes(void) {
	// add null tokens for grace notes in other voices
	GridSlice *lastnote = NULL;
	GridSlice *nextnote = NULL;
	for (int i=0; i<(int)m_allslices.size(); i++) {
		if (!m_allslices[i]->isGraceSlice()) {
			continue;
		}
		// cerr << "PROCESSING " << m_allslices[i] << endl;
		lastnote = NULL;
		nextnote = NULL;

		for (int j=i+1; j<(int)m_allslices.size(); j++) {
			if (m_allslices[j]->isNoteSlice()) {
				nextnote = m_allslices[j];
				break;
			}
		}
		if (nextnote == NULL) {
			continue;
		}

		for (int j=i-1; j>=0; j--) {
			if (m_allslices[j]->isNoteSlice()) {
				lastnote = m_allslices[j];
				break;
			}
		}
		if (lastnote == NULL) {
			continue;
		}

		fillInNullTokensForGraceNotes(m_allslices[i], lastnote, nextnote);
	}
}



//////////////////////////////
//
// HumGrid::addNullTokensForLayoutComments -- Avoid layout in multi-subspine
//     regions from contracting to a single spine.
//

void HumGrid::addNullTokensForLayoutComments(void) {
	// add null tokens for key changes in other voices
	GridSlice *lastnote = NULL;
	GridSlice *nextnote = NULL;
	for (int i=0; i<(int)m_allslices.size(); i++) {
		if (!m_allslices[i]->isLocalLayoutSlice()) {
			continue;
		}
		// cerr << "PROCESSING " << m_allslices[i] << endl;
		lastnote = NULL;
		nextnote = NULL;

		for (int j=i+1; j<(int)m_allslices.size(); j++) {
			if (m_allslices[j]->isNoteSlice()) {
				nextnote = m_allslices[j];
				break;
			}
		}
		if (nextnote == NULL) {
			continue;
		}

		for (int j=i-1; j>=0; j--) {
			if (m_allslices[j]->isNoteSlice()) {
				lastnote = m_allslices[j];
				break;
			}
		}
		if (lastnote == NULL) {
			continue;
		}

		fillInNullTokensForLayoutComments(m_allslices[i], lastnote, nextnote);
	}
}



//////////////////////////////
//
// HumGrid::addNullTokensForClefChanges -- Avoid clef in multi-subspine
//     regions from contracting to a single spine.
//

void HumGrid::addNullTokensForClefChanges(void) {
	// add null tokens for clef changes in other voices
	GridSlice *lastnote = NULL;
	GridSlice *nextnote = NULL;
	for (int i=0; i<(int)m_allslices.size(); i++) {
		if (!m_allslices[i]->isClefSlice()) {
			continue;
		}
		// cerr << "PROCESSING " << m_allslices[i] << endl;
		lastnote = NULL;
		nextnote = NULL;

		for (int j=i+1; j<(int)m_allslices.size(); j++) {
			if (m_allslices[j]->isNoteSlice()) {
				nextnote = m_allslices[j];
				break;
			}
		}
		if (nextnote == NULL) {
			continue;
		}

		for (int j=i-1; j>=0; j--) {
			if (m_allslices[j]->isNoteSlice()) {
				lastnote = m_allslices[j];
				break;
			}
		}
		if (lastnote == NULL) {
			continue;
		}

		fillInNullTokensForClefChanges(m_allslices[i], lastnote, nextnote);
	}
}



//////////////////////////////
//
// HumGrid::fillInNullTokensForClefChanges --
//

void HumGrid::fillInNullTokensForClefChanges(GridSlice* clefslice,
		GridSlice* lastnote, GridSlice* nextnote) {

	if (clefslice == NULL) { return; }
	if (lastnote == NULL)  { return; }
	if (nextnote == NULL)  { return; }

	// cerr << "CHECKING CLEF SLICE: " << endl;
	// cerr << "\tclef\t" << clefslice << endl;
	// cerr << "\tlast\t" << lastnote << endl;
	// cerr << "\tnext\t" << nextnote << endl;

	int partcount = (int)clefslice->size();

	for (int p=0; p<partcount; p++) {
		int staffcount = (int)lastnote->at(p)->size();
		for (int s=0; s<staffcount; s++) {
			int v1count = (int)lastnote->at(p)->at(s)->size();
			int v2count = (int)nextnote->at(p)->at(s)->size();
			int vgcount = (int)clefslice->at(p)->at(s)->size();
			// if (vgcount < 1) {
			// 	vgcount = 1;
			// }
			if (v1count < 1) {
				v1count = 1;
			}
			if (v2count < 1) {
				v2count = 1;
			}
			// cerr << "p=" << p << "\ts=" << s << "\tv1count = " << v1count;
			// cerr << "\tv2count = " << v2count;
			// cerr << "\tvgcount = " << vgcount << endl;
			if (v1count != v2count) {
				// Note slices are expanding or contracting so do
				// not try to adjust clef slice between them.
				continue;
			}
			if (vgcount == v1count) {
				// Grace note slice does not need to be adjusted.
			}
			int diff = v1count - vgcount;
			// fill in a null for each empty slot in voice
			for (int i=0; i<diff; i++) {
				GridVoice* gv = createVoice("*", "P", 0, p, s);
				clefslice->at(p)->at(s)->push_back(gv);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::fillInNullTokensForLayoutComments --
//

void HumGrid::fillInNullTokensForLayoutComments(GridSlice* layoutslice,
		GridSlice* lastnote, GridSlice* nextnote) {

	if (layoutslice == NULL) { return; }
	if (lastnote == NULL)    { return; }
	if (nextnote == NULL)    { return; }

	// cerr << "CHECKING CLEF SLICE: " << endl;
	// cerr << "\tclef\t" << layoutslice << endl;
	// cerr << "\tlast\t" << lastnote << endl;
	// cerr << "\tnext\t" << nextnote << endl;

	int partcount = (int)layoutslice->size();
	int staffcount;
	int vgcount;
	int v1count;
	int v2count;

	for (int p=0; p<partcount; p++) {
		staffcount = (int)lastnote->at(p)->size();
		for (int s=0; s<staffcount; s++) {
			v1count = (int)lastnote->at(p)->at(s)->size();
			v2count = (int)nextnote->at(p)->at(s)->size();
			vgcount = (int)layoutslice->at(p)->at(s)->size();
			// if (vgcount < 1) {
			// 	vgcount = 1;
			// }
			if (v1count < 1) {
				v1count = 1;
			}
			if (v2count < 1) {
				v2count = 1;
			}
			// cerr << "p=" << p << "\ts=" << s << "\tv1count = " << v1count;
			// cerr << "\tv2count = " << v2count;
			// cerr << "\tvgcount = " << vgcount << endl;
			if (v1count != v2count) {
				// Note slices are expanding or contracting so do
				// not try to adjust clef slice between them.
				continue;
			}
			if (vgcount == v1count) {
				// Grace note slice does not need to be adjusted.
			}
			int diff = v1count - vgcount;
			// fill in a null for each empty slot in voice
			for (int i=0; i<diff; i++) {
				GridVoice* gv = new GridVoice("!", 0);
				layoutslice->at(p)->at(s)->push_back(gv);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::fillInNullTokensForGraceNotes --
//

void HumGrid::fillInNullTokensForGraceNotes(GridSlice* graceslice, GridSlice* lastnote,
		GridSlice* nextnote) {

	if (graceslice == NULL) {
		return;
	}
	if (lastnote == NULL) {
		return;
	}
	if (nextnote == NULL) {
		return;
	}

	// cerr << "CHECKING GRACE SLICE: " << endl;
	// cerr << "\tgrace\t" << graceslice << endl;
	// cerr << "\tlast\t" << lastnote << endl;
	// cerr << "\tnext\t" << nextnote << endl;

	int partcount = (int)graceslice->size();
	int staffcount;
	int vgcount;
	int v1count;
	int v2count;

	for (int p=0; p<partcount; p++) {
		staffcount = (int)lastnote->at(p)->size();
		for (int s=0; s<staffcount; s++) {
			v1count = (int)lastnote->at(p)->at(s)->size();
			v2count = (int)nextnote->at(p)->at(s)->size();
			vgcount = (int)graceslice->at(p)->at(s)->size();
			// if (vgcount < 1) {
			// 	vgcount = 1;
			// }
			if (v1count < 1) {
				v1count = 1;
			}
			if (v2count < 1) {
				v2count = 1;
			}
			// cerr << "p=" << p << "\ts=" << s << "\tv1count = " << v1count;
			// cerr << "\tv2count = " << v2count;
			// cerr << "\tvgcount = " << vgcount << endl;
			if (v1count != v2count) {
				// Note slices are expanding or contracting so do
				// not try to adjust grace slice between them.
				continue;
			}
			if (vgcount == v1count) {
				// Grace note slice does not need to be adjusted.
			}
			int diff = v1count - vgcount;
			// fill in a null for each empty slot in voice
			for (int i=0; i<diff; i++) {
				GridVoice* gv = new GridVoice(".", 0);
				graceslice->at(p)->at(s)->push_back(gv);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::addNullTokens --
//

void HumGrid::addNullTokens(void) {
	int i; // slice index
	int p; // part index
	int s; // staff index
	int v; // voice index

	if ((0)) {
		cerr << "SLICE TIMESTAMPS: " << endl;
		for (int x=0; x<(int)m_allslices.size(); x++) {
			cerr << "\tTIMESTAMP " << x << "= "
			     << m_allslices[x]->getTimestamp()
			     << "\tDUR=" << m_allslices[x]->getDuration()
			     << "\t"
			     << m_allslices[x]
			     << endl;
		}
	}

	for (i=0; i<(int)m_allslices.size(); i++) {

		GridSlice& slice = *m_allslices.at(i);
		if (!slice.isNoteSlice()) {
			// probably need to deal with grace note slices here
			continue;
		}
      for (p=0; p<(int)slice.size(); p++) {
			GridPart& part = *slice.at(p);
      	for (s=0; s<(int)part.size(); s++) {
				GridStaff& staff = *part.at(s);
      		for (v=0; v<(int)staff.size(); v++) {
					if (!staff.at(v)) {
						// in theory should not happen
						continue;
					}
					GridVoice& gv = *staff.at(v);
					if (gv.isNull()) {
						continue;
					}
					// found a note/rest which should have a non-zero
					// duration that needs to be extended to the next
					// duration in the
					extendDurationToken(i, p, s, v);
				}
			}
		}

	}

	addNullTokensForGraceNotes();
	adjustClefChanges();
	addNullTokensForClefChanges();
	addNullTokensForLayoutComments();
	checkForNullDataHoles();
}



//////////////////////////////
//
// HumGrid::checkForNullData -- identify any spots in the grid which are NULL
//     pointers and allocate invisible rests for them by finding the next
//     durational item in the particular staff/layer.
//

void HumGrid::checkForNullDataHoles(void) {
	for (int i=0; i<(int)m_allslices.size(); i++) {
		GridSlice& slice = *m_allslices.at(i);
		if (!slice.isNoteSlice()) {
			continue;
		}
      for (int p=0; p<(int)slice.size(); p++) {
			GridPart& part = *slice.at(p);
      	for (int s=0; s<(int)part.size(); s++) {
				GridStaff& staff = *part.at(s);
      		for (int v=0; v<(int)staff.size(); v++) {
					if (!staff.at(v)) {
						staff.at(v) = new GridVoice();
						// Calculate duration of void by searching
						// for the next non-null voice in the current part/staff/voice
						HumNum duration = slice.getDuration();
						GridPart *pp;
						GridStaff *sp;
						GridVoice *vp;
						for (int q=i+1; q<(int)m_allslices.size(); q++) {
							GridSlice *slicep = m_allslices.at(q);
							if (!slicep->isNoteSlice()) {
								// or isDataSlice()?
								continue;
							}
							if (p >= (int)slicep->size() - 1) {
								continue;
							}
							pp = slicep->at(p);
							if (s >= (int)pp->size() - 1) {
								continue;
							}
							sp = pp->at(s);
							if (v >= (int)sp->size() - 1) {
								// Found a data line with no data at given voice, so
								// add slice duration to cumulative duration.
								duration += slicep->getDuration();
								continue;
							}
							vp = sp->at(v);
							if (!vp) {
								// found another null spot which should be dealt with later.
								break;
							} else {
								// there is a token at the same part/staff/voice position.
								// Maybe check if a null token, but if not a null token,
								// then break here also.
								break;
							}
						}
						string recip = Convert::durationToRecip(duration);
						// ggg @ marker is added to keep track of them for more debugging.
						recip += "ryy@";
						staff.at(v)->setToken(recip);
						continue;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::setPartStaffDimensions --
//

void HumGrid::setPartStaffDimensions(vector<vector<GridSlice*>>& nextevent,
		GridSlice* startslice) {
	nextevent.clear();
	for (int i=0; i<(int)m_allslices.size(); i++) {
		if (!m_allslices[i]->isNoteSlice()) {
			continue;
		}
		GridSlice* slice = m_allslices[i];
		nextevent.resize(slice->size());
		for (int p=0; p<(int)slice->size(); p++) {
			nextevent.at(p).resize(slice->at(p)->size());
			for (int j=0; j<(int)nextevent.at(p).size(); j++) {
				nextevent.at(p).at(j) = startslice;
			}
		}
		break;
	}
}



//////////////////////////////
//
// HumGrid::addInvisibleRestsInFirstTrack --  If there are any
//    timing gaps in the first track of a **kern spine, then
//    fill in with invisible rests.
//
// ggg

void HumGrid::addInvisibleRestsInFirstTrack(void) {
	int i; // slice index
	int p; // part index
	int s; // staff index
	int v = 0; // only looking at first voice

	vector<vector<GridSlice*>> nextevent;
	GridSlice* lastslice = m_allslices.back();
	setPartStaffDimensions(nextevent, lastslice);

	for (i=(int)m_allslices.size()-1; i>=0; i--) {
		GridSlice& slice = *m_allslices.at(i);
		if (!slice.isNoteSlice()) {
			continue;
		}
      for (p=0; p<(int)slice.size(); p++) {
			GridPart& part = *slice.at(p);
      	for (s=0; s<(int)part.size(); s++) {
				GridStaff& staff = *part.at(s);
				if (staff.size() == 0) {
					cerr << "EMPTY STAFF VOICE WILL BE FILLED IN LATER!!!!" << endl;
					continue;
				}
				if (!staff.at(v)) {
					// in theory should not happen
					continue;
				}
				GridVoice& gv = *staff.at(v);
				if (gv.isNull()) {
					continue;
				}

				// Found a note/rest.  Check if its duration matches
				// the next non-null data token.  If not, then add
				// an invisible rest somewhere between the two

				// first check to see if the previous item is a
				// NULL.  If so, then store and continue.
				if (nextevent[p][s] == NULL) {
					nextevent[p][s] = &slice;
					continue;
				}
				addInvisibleRest(nextevent, i, p, s);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::addInvisibleRest --
//
// ggg

void HumGrid::addInvisibleRest(vector<vector<GridSlice*>>& nextevent,
		int index, int p, int s) {
	GridSlice *ending = nextevent.at(p).at(s);
	if (ending == NULL) {
		cerr << "Not handling this case yet at end of data." << endl;
		return;
	}
	HumNum endtime = ending->getTimestamp();

	GridSlice* starting = m_allslices.at(index);
	HumNum starttime = starting->getTimestamp();
	HTp token = starting->at(p)->at(s)->at(0)->getToken();
	HumNum duration = Convert::recipToDuration(token);
	HumNum difference = endtime - starttime;
	HumNum gap = difference - duration;
	if (gap == 0) {
		// nothing to do
		nextevent.at(p).at(s) = starting;
		return;
	}
	HumNum target = starttime + duration;

	string kern = Convert::durationToRecip(gap);
	kern += "ryy";

	for (int i=index+1; i<(int)m_allslices.size(); i++) {
		GridSlice* slice = m_allslices[i];
		if (!slice->isNoteSlice()) {
			continue;
		}
		HumNum timestamp = slice->getTimestamp();
		if (timestamp < target) {
			continue;
		}
		if (timestamp > target) {
			cerr << "Cannot deal with this slice addition case yet..." << endl;
			nextevent[p][s] = starting;
			return;
		}
		// At timestamp for adding new token.
		if ((m_allslices.at(i)->at(p)->at(s)->size() > 0) && !m_allslices.at(i)->at(p)->at(s)->at(0)) {
			// Element is null where an invisible rest should be
			// so allocate space for it.
			m_allslices.at(i)->at(p)->at(s)->at(0) = new GridVoice();
		}
		if (m_allslices.at(i)->at(p)->at(s)->size() > 0) {
			m_allslices.at(i)->at(p)->at(s)->at(0)->setToken(kern);
		}
		break;
	}

	// Store the current event in the buffer
	nextevent.at(p).at(s) = starting;
}



//////////////////////////////
//
// HumGrid::adjustClefChanges -- If a clef change starts at the
//     beginning of a meausre, move it to before the measure (unless
//     the measure has zero duration).
//

void HumGrid::adjustClefChanges(void) {
	vector<GridMeasure*>& measures = *this;
	for (int i=1; i<(int)measures.size(); i++) {
		auto it = measures[i]->begin();
		if ((*it) == NULL) {
			cerr << "Warning: GridSlice is null in GridMeasure " << i << endl;
			continue;
		}
		if ((*it)->empty()) {
			cerr << "Warning: GridSlice is empty in GridMeasure "  << i << endl;
			continue;
		}
		if (!(*it)->isClefSlice()) {
			continue;
		}
		// move clef to end of previous measure
		GridSlice* tempslice = *it;
		measures[i]->pop_front();
		measures[i-1]->push_back(tempslice);
	}
}



//////////////////////////////
//
// HumGrid::extendDurationToken --
//

void HumGrid::extendDurationToken(int slicei, int parti, int staffi,
		int voicei) {
	if ((slicei < 0) || (slicei >= ((int)m_allslices.size()) - 1)) {
		// nothing after this line, so can extend further.
		return;
	}

	if (!m_allslices.at(slicei)->hasSpines()) {
		// no extensions needed in non-spined slices.
		return;
	}

	if (m_allslices.at(slicei)->isGraceSlice()) {
		cerr << "THIS IS A GRACE SLICE SO DO NOT FILL" << endl;
		return;
	}

	GridVoice* gv = m_allslices.at(slicei)->at(parti)->at(staffi)->at(voicei);
 	HTp token = gv->getToken();
	if (!token) {
		cerr << "STRANGE: token should not be null" << endl;
		return;
	}
	if (*token == ".") {
		// null data token so ignore;
		// change this later to add a duration for the null token below.
		return;
	}

	HumNum tokendur = Convert::recipToDuration((string)*token);
	HumNum currts   = m_allslices.at(slicei)->getTimestamp();
	HumNum nextts   = m_allslices.at(slicei+1)->getTimestamp();
	HumNum slicedur = nextts - currts;
	HumNum timeleft = tokendur - slicedur;

	if ((0)) {
		cerr << "===================" << endl;
		cerr << "EXTENDING TOKEN    " << token      << endl;
		cerr << "\tTOKEN DUR:       " << tokendur   << endl;
		cerr << "\tTOKEN START:     " << currts     << endl;
		cerr << "\tSLICE DUR:       " << slicedur   << endl;
		cerr << "\tNEXT SLICE START:" << nextts     << endl;
		cerr << "\tTIME LEFT:       " << timeleft   << endl;
		cerr << "\t-----------------" << endl;
	}

	if (timeleft != 0) {
		// fill in null tokens for the required duration.
		if (timeleft < 0) {
			cerr << "ERROR: Negative duration: " << timeleft << endl;
			cerr << "\ttokendur = " << tokendur << endl;
			cerr << "\tslicedur = " << slicedur << endl;
			cerr << "\ttoken    = " << token << endl;
			cerr << "\tCURRENT SLICE = " << m_allslices.at(slicei) << endl;
			cerr << "\tTIMESTAMP " << currts << endl;
			cerr << "\tNEXT SLICE = " << m_allslices.at(slicei) << endl;
			cerr << "\tNEXT TIMESTAMP " << nextts << endl;
			return;
		}

		SliceType type;
		GridStaff* gs;
		int s = slicei+1;

		while ((s < (int)m_allslices.size()) && (timeleft > 0)) {
			if (!m_allslices.at(s)->hasSpines()) {
				s++;
				continue;
			}
			currts = nextts;
			int nexts = 1;
			while (s < (int)m_allslices.size() - nexts) {
				if (!m_allslices.at(s+nexts)->hasSpines()) {
					nexts++;
					continue;
				}
				break;
			}
			if (s < (int)m_allslices.size() - nexts) {
				nextts = m_allslices.at(s+nexts)->getTimestamp();
			} else {
				nextts = currts + m_allslices.at(s)->getDuration();
			}
			slicedur = nextts - currts;
			type = m_allslices[s]->getType();

			if (staffi == (int)m_allslices.at(s)->at(parti)->size()) {
					cerr << "WARNING: staff index " << staffi << " is probably incorrect: increasing staff count for part to " << staffi + 1 << endl;
					m_allslices.at(s)->at(parti)->resize(m_allslices.at(s)->at(parti)->size() + 1);
					m_allslices.at(s)->at(parti)->at(staffi) = new GridStaff();
			}
			gs = m_allslices.at(s)->at(parti)->at(staffi);
			if (gs == NULL) {
				cerr << "Strange error6 in extendDurationToken()" << endl;
				return;
			}

			if (m_allslices.at(s)->isGraceSlice()) {
				m_allslices[s]->setDuration(0);
			} else if (m_allslices.at(s)->isDataSlice()) {
				gs->setNullTokenLayer(voicei, type, slicedur);
				timeleft = timeleft - slicedur;
			} else if (m_allslices.at(s)->isInvalidSlice()) {
				cerr << "THIS IS AN INVALID SLICE" << m_allslices.at(s) << endl;
			} else {
				// store a null token for the non-data slice, but probably skip
				// if there is a token already there (such as a clef-change).
				if ((voicei < (int)gs->size()) && (gs->at(voicei) != NULL)) {
					// there is already a token here, so do not replace it.
					// cerr << "Not replacing token: "  << gs->at(voicei)->getToken() << endl;
				} else {
					gs->setNullTokenLayer(voicei, type, slicedur);
				}
			}
			s++;
			if (s == (int)m_allslices.size() - 1) {
				m_allslices[s]->setDuration(timeleft);
			}
		}
	}
	// walk through zero-dur items and fill them in, but stop at
	// a token (likely a grace note which should not be erased).
}



//////////////////////////////
//
// HumGrid::getGridVoice -- Check to see if GridVoice exists, returns
//    NULL otherwise. Requires HumGrid::buildSingleList() being run first.
//

GridVoice* HumGrid::getGridVoice(int slicei, int parti, int staffi,
		int voicei) {
	if (slicei >= (int)m_allslices.size()) {
		cerr << "Strange error 1a" << endl;
		return NULL;
	}
	GridSlice* gsl = m_allslices.at(slicei);
	if (gsl == NULL) {
		cerr << "Strange error 1b" << endl;
		return NULL;
	}

	if (parti >= (int)gsl->size()) {
		cerr << "Strange error 2a" << endl;
		return NULL;
	}
	GridPart* gp = gsl->at(parti);
	if (gp == NULL) {
		cerr << "Strange error 2" << endl;
		return NULL;
	}

	if (staffi >= (int)gp->size()) {
		cerr << "Strange error 3a" << endl;
		return NULL;
	}
	GridStaff* gst = gp->at(staffi);
	if (gst == NULL) {
		cerr << "Strange error 3b" << endl;
		return NULL;
	}

	if (voicei >= (int)gst->size()) {
		cerr << "Strange error 4a" << endl;
		return NULL;
	}
	GridVoice* gv = gst->at(voicei);
	if (gv == NULL) {
		cerr << "Strange error 4b" << endl;
		return NULL;
	}
	return gv;
}



//////////////////////////////
//
// HumGrid::calculateGridDurations --
//

void HumGrid::calculateGridDurations(void) {

	// the last line has to be calculated from the shortest or
   // longest duration on the line.  Acutally all durations
	// starting on this line must be the same, so just search for
	// the first duration.

	auto last = m_allslices.back();

	// set to zero in case not a duration type of line:
	last->setDuration(0);

	bool finished = false;
	if (last->isNoteSlice()) {
		for (auto part : *last) {
			for (auto staff : *part) {
				for (auto voice : *staff) {
					if (!voice) {
						continue;
					}
					if (voice->getDuration() > 0) {
						last->setDuration(voice->getDuration());
						finished = true;
						break;
					}
				}
				if (finished) {
					break;
				}
			}
			if (finished) {
				break;
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::insertExclusiveInterpretationLine -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.
//

void HumGrid::insertExclusiveInterpretationLine(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}

	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("**recip");
		line->appendToken(token);
	}

	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			token = new HumdrumToken("**kern");
			line->appendToken(token);
			insertExInterpSides(line, p, s); // insert staff sides
		}
		insertExInterpSides(line, p, -1);   // insert part sides
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertExInterpSides --
//

void HumGrid::insertExInterpSides(HumdrumLine* line, int part, int staff) {

	if (staff >= 0) {
		int versecount = getVerseCount(part, staff); // verses related to staff
		for (int i=0; i<versecount; i++) {
			HTp token = new HumdrumToken("**text");
			line->appendToken(token);
		}
	}

	if ((staff < 0) && hasDynamics(part)) {
		HTp token = new HumdrumToken("**dynam");
		line->appendToken(token);
	}

	if ((staff < 0) && hasFiguredBass(part)) {
		HTp token = new HumdrumToken("**fb");
		line->appendToken(token);
	}

	if (staff < 0) {
		int harmonyCount = getHarmonyCount(part);
		for (int i=0; i<harmonyCount; i++) {
			HTp token = new HumdrumToken("**mxhm");
			line->appendToken(token);
		}

	}
}



//////////////////////////////
//
// HumGrid::insertPartNames --
//

void HumGrid::insertPartNames(HumdrumFile& outfile) {
	if (m_partnames.size() == 0) {
		return;
	}
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*");
		line->appendToken(token);
	}

	string text;
	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			text = "*";
			string pname = m_partnames[p];
			if (!pname.empty()) {
				text += "I\"";
				text += pname;
			}
			token = new HumdrumToken(text);
			line->appendToken(token);
			insertSideNullInterpretations(line, p, s);
		}
		insertSideNullInterpretations(line, p, -1);
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertPartIndications -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.  This is the
//    MusicXML Part number. (Some parts will contain more than one
//    staff).
//

void HumGrid::insertPartIndications(HumdrumFile& outfile) {

	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*");
		line->appendToken(token);
	}

	string text;
	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			text = "*part" + to_string(p+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
			insertSidePartInfo(line, p, s);
		}
		insertSidePartInfo(line, p, -1);   // insert part sides
	}
	outfile.insertLine(0, line);

}



//////////////////////////////
//
// HumGrid::insertSideNullInterpretations --
//

void HumGrid::insertSideNullInterpretations(HumdrumLine* line,
		int part, int staff) {
	HTp token;
	string text;

	if (staff < 0) {

		if (hasDynamics(part)) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

		if (hasFiguredBass(part)) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

		int harmcount = getHarmonyCount(part);
		for (int i=0; i<harmcount; i++) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

	} else {
		int versecount = getVerseCount(part, staff);
		for (int i=0; i<versecount; i++) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}
	}
}



//////////////////////////////
//
// HumGrid::insertSidePartInfo --
//

void HumGrid::insertSidePartInfo(HumdrumLine* line, int part, int staff) {
	HTp token;
	string text;

	if (staff < 0) {

		if (hasDynamics(part)) {
			text = "*part" + to_string(part+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}

		if (hasFiguredBass(part)) {
			text = "*part" + to_string(part+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}

		int harmcount = getHarmonyCount(part);
		for (int i=0; i<harmcount; i++) {
			text = "*part" + to_string(part+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}

	} else {
		int versecount = getVerseCount(part, staff);
		for (int i=0; i<versecount; i++) {
			text = "*part" + to_string(part+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}
	}
}



//////////////////////////////
//
// HumGrid::insertStaffIndications -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.  This is the
//    MusicXML Part number. (Some parts will contain more than one
//    staff).
//

void HumGrid::insertStaffIndications(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}

	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*");
		line->appendToken(token);
	}

	string text;
	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index

	int staffcount = 0;
	for (p=0; p<(int)slice.size(); p++) {
		GridPart& part = *slice[p];
		staffcount += (int)part.size();
	}

	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			text = "*staff" + to_string(staffcount--);
			token = new HumdrumToken(text);
			line->appendToken(token);
			insertSideStaffInfo(line, p, s, staffcount+1);
		}
		insertSideStaffInfo(line, p, -1, -1);  // insert part sides
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertSideStaffInfo --
//

void HumGrid::insertSideStaffInfo(HumdrumLine* line, int part, int staff,
		int staffnum) {
	HTp token;
	string text;

	// part-specific sides (no staff markers)
	if (staffnum < 0) {

		if (hasDynamics(part)) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

		if (hasFiguredBass(part)) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

		int harmcount = getHarmonyCount(part);
		for (int i=0; i<harmcount; i++) {
			token = new HumdrumToken("*");
			line->appendToken(token);
		}

		return;
	}

	int versecount = getVerseCount(part, staff);
	for (int i=0; i<versecount; i++) {
		if (staffnum > 0) {
			text = "*staff" + to_string(staffnum);
			token = new HumdrumToken(text);
		} else {
			token = new HumdrumToken("*");
		}
		line->appendToken(token);
	}


}



//////////////////////////////
//
// HumGrid::insertDataTerminationLine -- Currently presumes
//    that the last entry contains spines.  And the first
//    measure in the HumGrid object must contain a slice.
//    Also need to compensate for *v on previous line.
//

void HumGrid::insertDataTerminationLine(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*-");
		line->appendToken(token);
	}

	GridSlice& slice = *this->at(0)->back();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
			insertSideTerminals(line, p, s);
		}
		insertSideTerminals(line, p, -1);   // insert part sides
	}
	outfile.appendLine(line);
}



//////////////////////////////
//
// HumGrid::insertSideTerminals --
//

void HumGrid::insertSideTerminals(HumdrumLine* line, int part, int staff) {
	HTp token;

	if (staff < 0) {

		if (hasDynamics(part)) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
		}

		if (hasFiguredBass(part)) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
		}

		int harmcount = getHarmonyCount(part);
		for (int i=0; i<harmcount; i++) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
		}

	} else {
		int versecount = getVerseCount(part, staff);
		for (int i=0; i<versecount; i++) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
		}
	}
}



//////////////////////////////
//
// HumGrid::transferNonDataSlices --
//

void HumGrid::transferNonDataSlices(GridMeasure* output, GridMeasure* input) {
	for (auto it = input->begin(); it != input->end(); it++) {
		GridSlice* slice = *it;
		if (slice->isDataSlice()) {
			continue;
		}
		output->push_front(slice);
		input->erase(it);
		it--;
	}
}



//////////////////////////////
//
// HumGrid::removeSibeliusIncipit --
//

void HumGrid::removeSibeliusIncipit(void) {

	if (this->size() == 0) {
		return;
	}
	GridMeasure* measure = this->at(0);
	bool invisible = measure->isInvisible();
	if (!invisible) {
		return;
	}

	this->erase(this->begin());
	if (this->size() > 0) {
		// [20171012] remove this for now since it is crashing
		// emscripten version of code.
		// transferNonDataSlices(this->at(0), measure);
	}
	delete measure;
	measure = NULL;

	// remove vocal ranges, if present
	if (this->size() == 0) {
		return;
	}

	measure = this->at(0);
	bool singlechord = measure->isSingleChordMeasure();
	if (!singlechord) {
		return;
	}

	this->erase(this->begin());
	if (this->size() > 0) {
		transferNonDataSlices(this->at(0), measure);
	}
	delete measure;
	measure = NULL;

	measure = this->at(0);
	bool monophonic = measure->isMonophonicMeasure();
	if (!monophonic) {
		return;
	}

	string melody = extractMelody(measure);

	this->erase(this->begin());
	if (this->size() > 0) {
		transferNonDataSlices(this->at(0), measure);
	}
	delete measure;
	measure = NULL;

	if (this->size() > 0) {
		insertMelodyString(this->at(0), melody);
	}

}



//////////////////////////////
//
// HumGrid::insertMelodyString -- Insert a global comment before first data line.
//

void HumGrid::insertMelodyString(GridMeasure* measure, const string& melody) {
	for (auto it = measure->begin(); it != measure->end(); it++) {
		GridSlice* slice = *it;
		if (!slice->isDataSlice()) {
			continue;
		}

		// insert a new GridSlice
		// first need to implement global commands in GridSlice object...
		break;
	}
}



//////////////////////////////
//
// HumGrid::extractMelody --
//

string HumGrid::extractMelody(GridMeasure* measure) {
	string output = "!!";

	int parti  = -1;
	int staffi = -1;
	int voicei = -1;

	// First find the part which has the melody:
	for (auto slice : *measure) {
		if (!slice->isDataSlice()) {
			continue;
		}
		for (int p=0; p<(int)slice->size(); p++) {
			GridPart* part = slice->at(p);
			for (int s=0; s<(int)part->size(); s++) {
				GridStaff* staff = part->at(s);
				for (int v=0; v<(int)staff->size(); v++) {
					GridVoice* voice = staff->at(v);
					HTp token = voice->getToken();
					if (!token) {
						continue;
					}
					if (token->find("yy") == string::npos) {
						parti  = p;
						staffi = s;
						voicei = v;
						goto loop_end;
					}
				}
			}
		}
	}

	loop_end:

	if (parti < 0) {
		return output;
	}

	// First find the part which has the melody:
	for (auto slice : *measure) {
		if (!slice->isDataSlice()) {
			continue;
		}
		HTp token = slice->at(parti)->at(staffi)->at(voicei)->getToken();
		if (!token) {
			continue;
		}
		if (*token == ".") {
			continue;
		}
		output += " ";
		output += *token;
	}

	return output;
}




//////////////////////////////
//
// HumGrid::removeRedundantClefChanges -- Will also have to consider
//		the meter signature.
//

void HumGrid::removeRedundantClefChanges(void) {
	// curclef is a list of the current staff on the part:staff.
	vector<vector<string> > curclef;

	bool hasduplicate = false;
	for (int m=0; m<(int)this->size(); m++) {
		GridMeasure* measure = this->at(m);
		for (auto slice : *measure) {
			if (!slice->isClefSlice()) {
				continue;
			}
			bool allempty = true;
			for (int p=0; p<(int)slice->size(); p++) {
				for (int s=0; s<(int)slice->at(p)->size(); s++) {
					if (slice->at(p)->at(s)->size() < 1) {
						continue;
					}
					GridVoice* voice = slice->at(p)->at(s)->at(0);
					HTp token = voice->getToken();
					if (!token) {
						continue;
					}
					if (string(*token) == "*") {
						continue;
					}
					if (token->find("clef") == string::npos) {
						// something (probably invalid) which is not a clef change
						allempty = false;
						continue;
					}
					if (p >= (int)curclef.size()) {
						curclef.resize(p+1);
					}
					if (s >= (int)curclef[p].size()) {
						// first clef on the staff, so can't be a duplicate
						curclef[p].resize(s+1);
						curclef[p][s] = *token;
						allempty = false;
						continue;
					} else {
						if (curclef[p][s] == (string)*token) {
							// clef is already active, so remove this one
							hasduplicate = true;
							voice->setToken("*");
						} else {
							// new clef change
							curclef[p][s] = *token;
							allempty = false;
						}
					}
				}
			}
			if (!hasduplicate) {
				continue;
			}
			// Check the slice to see if it empty, and delete if so.
			// This algorithm does not consider GridSide content.
			if (allempty) {
				slice->invalidate();
			}

		}
	}
}



//////////////////////////////
//
// HumGrid::cleanTempos --
//

void HumGrid::cleanTempos(void) {
//		std::vector<GridSlice*>       m_allslices;
// ggg
	for (int i=0; i<(int)m_allslices.size(); i++) {
		if (!m_allslices[i]->isTempoSlice()) {
			continue;
		}
		cleanTempos(m_allslices[i]);
	}
}


void HumGrid::cleanTempos(GridSlice* slice) {
	if (!slice->isTempoSlice()) {
		return;
	}
	HTp token = NULL;

	for (int part=0; part<(int)slice->size(); part++) {
		GridPart* gp = slice->at(part);
		for (int staff=0; staff<(int)gp->size(); staff++) {
			GridStaff* gs = gp->at(staff);
			for (int voice=0; voice<(int)gs->size(); voice++) {
				GridVoice* gv = gs->at(voice);
				token = gv->getToken();
				if (token) {
					break;
				}
			}
			if (token) {
				break;
			}
		}
		if (token) {
			break;
		}
	}

	if (!token) {
		return;
	}

	for (int part=0; part<(int)slice->size(); part++) {
		GridPart* gp = slice->at(part);
		for (int staff=0; staff<(int)gp->size(); staff++) {
			GridStaff* gs = gp->at(staff);
			for (int voice=0; voice<(int)gs->size(); voice++) {
				GridVoice* gv = gs->at(voice);
				if (gv->getToken()) {
					continue;
				}
				gv->setToken(*token);
			}
		}
	}
}



//////////////////////////////
//
// HumGrid::hasPickup --
//

bool HumGrid::hasPickup(void) {
	return m_pickup;
}



//////////////////////////////
//
// HumGrid::deleteMeasure --
//

void HumGrid::deleteMeasure(int index) {
	delete this->at(index);
	this->at(index) = NULL;
	this->erase(this->begin() + index);
}



//////////////////////////////
//
// HumGrid::expandLocalCommentLayers -- Walk backwards in the
//   data list, and match the layer count for local comments
//   to have them match to the next data line.  This is needed
//   to attach layout parameters properly to data tokens.  Layout
//   parameters cannot pass through spine manipulator lines, so
//   this function is necessary to prevent spine manipulators
//   from orphaning local layout parameter lines.
//
//   For now just adjust local layout parameter slices, but maybe
//   later do all types of local comments.
//

void HumGrid::expandLocalCommentLayers(void) {
	GridSlice *dataslice = NULL;
	GridSlice *localslice = NULL;
	for (int i=(int)m_allslices.size() - 1; i>=0; i--) {
		if (m_allslices[i]->isDataSlice()) {
			dataslice = m_allslices[i];
		} else if (m_allslices[i]->isMeasureSlice()) {
			dataslice = m_allslices[i];
		}
		// Other slice types should be considered as well,
		// but definitely not manipulator slices:
		if (m_allslices[i]->isManipulatorSlice()) {
			dataslice = m_allslices[i];
		}

		if (!m_allslices[i]->isLocalLayoutSlice()) {
			continue;
		}
		localslice = m_allslices[i];
		if (!dataslice) {
			continue;
		}
		matchLayers(localslice, dataslice);
	}
}


//////////////////////////////
//
// HumGrid::matchLayers -- Make sure every staff in both inputs
//   have the same number of voices.
//

void HumGrid::matchLayers(GridSlice* output, GridSlice* input) {
	if (output->size() != input->size()) {
		// something wrong or one of the slices
		// could be a non-spined line.
		return;
	}
	int partcount = (int)input->size();
	for (int part=0; part<partcount; part++) {
		GridPart* ipart = input->at(part);
		GridPart* opart = output->at(part);
		if (ipart->size() != opart->size()) {
			// something string that should never happen
			continue;
		}
		int scount = (int)ipart->size();
		for (int staff=0; staff<scount; staff++) {
			GridStaff* istaff = ipart->at(staff);
			GridStaff* ostaff = opart->at(staff);
			matchLayers(ostaff, istaff);
		}
	}
}


void HumGrid::matchLayers(GridStaff* output, GridStaff* input) {
	if (input->size() == output->size()) {
		// The voice counts match so nothing to do.
		return;
	}
	if (input->size() < output->size()) {
		// Ignore potentially strange case.
	}

	int diff = (int)input->size() - (int)output->size();
	for (int i=0; i<diff; i++) {
		GridVoice* voice = new GridVoice("!", 0);
		output->push_back(voice);
	}
}



//////////////////////////////
//
// HumGrid::setPartName --
//

void HumGrid::setPartName(int index, const string& name) {
	if (index < 0) {
		return;
	} else if (index < (int)m_partnames.size()) {
		m_partnames[index] = name;
	} else if (index < 100) {
		// grow the array and then store name
		m_partnames.resize(index+1);
		m_partnames.back() = name;
	}
}



//////////////////////////////
//
// HumGrid::getPartName --
//

std::string HumGrid::getPartName(int index) {
	if (index < 0) {
		return "";
	} else if (index < (int)m_partnames.size()) {
		return m_partnames[index];
	} else {
		return "";
	}
}



//////////////////////////////
//
// operator<< -- Debugging printing of Humgrid Contents.
//

ostream& operator<<(ostream& out, HumGrid& grid) {
	for (int i=0; i<(int)grid.size(); i++) {
		out << "\nMEASURE " << i << " =========================" << endl;
		out << grid[i];
	}
	return out;
}


// END_MERGE

} // end namespace hum



