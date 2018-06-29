//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Mon Oct 17 19:18:43 PDT 2016
// Filename:      GridMeasure.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridMeasure.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   GridMeasure stores the data for each measure in a HumGrid
//                object.
//

#include "HumGrid.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// GridMeasure::GridMeasure -- Constructor.
//

GridMeasure::GridMeasure(HumGrid* owner) {
	m_owner = owner;
	m_style = MeasureStyle::Plain;
}



//////////////////////////////
//
// GridMeasure::~GridMeasure -- Deconstructor.
//

GridMeasure::~GridMeasure(void) {
	for (auto it = this->begin(); it != this->end(); it++) {
		if (*it) {
			delete *it;
			*it = NULL;
		}
	}
}



//////////////////////////////
//
// GridMeasure::appendGlobalLayout --
//

GridSlice* GridMeasure::appendGlobalLayout(const string& tok, HumNum timestamp) {
	GridSlice* gs = new GridSlice(this, timestamp, SliceType::GlobalLayouts, 1);
	gs->addToken(tok, 0, 0, 0);
	gs->setDuration(0);
	this->push_back(gs);
	return gs;
}



//////////////////////////////
//
// GridSlice::addGraceToken -- Add a grace note token at the given
//   gracenumber grace note line before the data line at the given
//   timestamp.
//

GridSlice* GridMeasure::addGraceToken(const string& tok, HumNum timestamp,
	int part, int staff, int voice, int maxstaff, int gracenumber) {
	if (gracenumber < 1) {
		cerr << "ERROR: gracenumber " << gracenumber << " has to be larger than 0" << endl;
		return NULL;
	}

	GridSlice* gs = NULL;
	// GridSlice* datatarget = NULL;
	auto iterator = this->begin();
	if (this->empty()) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::GraceNotes, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else if (timestamp > this->back()->getTimestamp()) {

		// Grace note needs to be added at the end of a measure:
		auto it2 = this->end();
		it2--;
		int counter = 0;
		while (it2 != this->end()) {
			if ((*it2)->isGraceSlice()) {
				counter++;
				if (counter == gracenumber) {
					// insert grace note into this slice
					(*it2)->addToken(tok, part, staff, voice);
					return *it2;
				}
			} else if ((*it2)->isLayoutSlice()) {
				// skip over any layout paramter lines.
				it2--;
				continue;
			} else if ((*it2)->isDataSlice()) {
				// insert grace note after this note
				gs = new GridSlice(this, timestamp, SliceType::GraceNotes, maxstaff);
				gs->addToken(tok, part, staff, voice);
				it2++;
				this->insert(it2, gs);
				return gs;
			}
			it2--;
		}
		return NULL;

	} else {
		// search for existing line with same timestamp on a data slice:

		while (iterator != this->end()) {
			if (timestamp < (*iterator)->getTimestamp()) {
				cerr << "STRANGE CASE 2 IN GRIDMEASURE::ADDGRACETOKEN" << endl;
				cerr << "\tGRACE TIMESTAMP: " << timestamp << endl;
				cerr << "\tTEST  TIMESTAMP: " << (*iterator)->getTimestamp() << endl;
				return NULL;
			}
			if ((*iterator)->isDataSlice()) {
				if ((*iterator)->getTimestamp() == timestamp) {
					// found dataslice just before graceslice(s)
					// datatarget = *iterator;
					break;
				}
			}
			iterator++;
		}

		auto it2 = iterator;
		it2--;
		int counter = 0;
		while (it2 != this->end()) {
			if ((*it2)->isGraceSlice()) {
				counter++;
				if (counter == gracenumber) {
					// insert grace note into this slice
					(*it2)->addToken(tok, part, staff, voice);
					return *it2;
				}
			} else if ((*it2)->isLayoutSlice()) {
				// skip over any layout paramter lines.
				it2--;
				continue;
			} else if ((*it2)->isDataSlice()) {
				// insert grace note after this note
				gs = new GridSlice(this, timestamp, SliceType::GraceNotes, maxstaff);
				gs->addToken(tok, part, staff, voice);
				it2++;
				this->insert(it2, gs);
				return gs;
			}
			it2--;
		}

		// grace note should be added at start of measure
		gs = new GridSlice(this, timestamp, SliceType::GraceNotes, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->insert(this->begin(), gs);

	}

	return NULL;
}



//////////////////////////////
//
// GridMeasure::addDataToken -- Add a data token in the data slice at the given
//    timestamp (or create a new data slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addDataToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::Notes, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if ((timestamp == (*iterator)->getTimestamp()) && ((*iterator)->isGraceSlice())) {
				iterator++;
				continue;
			}
			if (!(*iterator)->isDataSlice()) {
				iterator++;
				continue;
			} else if ((*iterator)->getTimestamp() == timestamp) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				gs = target;
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::Notes, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the lef, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::Notes, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}
	}

	return gs;
}



//////////////////////////////
//
// GridMeasure::addTempoToken -- Add a tempo token in the data slice at
//    the given timestamp (or create a new tempo slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addTempoToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::Tempos, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isTempoSlice()) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				break;
			} else if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
				// found the correct timestamp, but no clef slice at the timestamp
				// so add the clef slice before the data slice (eventually keepping
				// track of the order in which the other non-data slices should be placed).
				gs = new GridSlice(this, timestamp, SliceType::Tempos, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::Tempos, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the key signature, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::Tempos, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}

	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::addTimeSigToken -- Add a time signature token in the data slice at
//    the given timestamp (or create a new timesig slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addTimeSigToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::TimeSigs, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isTimeSigSlice()) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				break;
			} else if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
				// found the correct timestamp, but no clef slice at the timestamp
				// so add the clef slice before the data slice (eventually keepping
				// track of the order in which the other non-data slices should be placed).
				gs = new GridSlice(this, timestamp, SliceType::TimeSigs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::TimeSigs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the key signature, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::TimeSigs, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}

	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::addKeySigToken -- Add a key signature  token in a key sig slice at
//    the given timestamp (or create a new keysig slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addKeySigToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::KeySigs, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isKeySigSlice()) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				break;
			} else if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
				// found the correct timestamp, but no clef slice at the timestamp
				// so add the clef slice before the data slice (eventually keepping
				// track of the order in which the other non-data slices should be placed).
				gs = new GridSlice(this, timestamp, SliceType::KeySigs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::KeySigs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the key signature, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::KeySigs, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}

	}
	return gs;
}




//////////////////////////////
//
// GridMeasure::addLabelToken -- Add an instrument label token in a label slice at
//    the given timestamp (or create a new label slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addLabelToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxpart, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::Labels, maxpart);
		gs->addToken(tok, part, maxstaff-1, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isLabelSlice()) {
				target = *iterator;
				target->addToken(tok, part, maxstaff-1, voice);
				break;
			}
			iterator++;
		}
		if (iterator == this->end()) {
			// Couldn't find a place for the label abbreviation line, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::Labels, maxpart);
			gs->addToken(tok, part, maxstaff-1, voice);
			this->insert(this->begin(), gs);
		}
	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::addLabelAbbrToken -- Add an instrument label token in a label slice at
//    the given timestamp (or create a new label slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addLabelAbbrToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxpart, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::LabelAbbrs, maxpart);
		gs->addToken(tok, part, maxstaff-1, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isLabelAbbrSlice()) {
				target = *iterator;
				target->addToken(tok, part, maxstaff-1, voice);
				break;
			}
			iterator++;
		}
		if (iterator == this->end()) {
			// Couldn't find a place for the label abbreviation line, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::LabelAbbrs, maxpart);
			gs->addToken(tok, part, maxstaff-1, voice);
			this->insert(this->begin(), gs);
		}
	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::addTransposeToken -- Add a transposition token in the data slice at
//    the given timestamp (or create a new transposition slice at that timestamp), placing
//    the token at the specified part, staff, and voice index.
//
//    Note: should placed after clef if present and no other transpose slice at
//    same time.
//

GridSlice* GridMeasure::addTransposeToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::Transpositions, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isTransposeSlice()) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				break;
			} else if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
				// found the correct timestamp, but no clef slice at the timestamp
				// so add the clef slice before the data slice (eventually keepping
				// track of the order in which the other non-data slices should be placed).
				gs = new GridSlice(this, timestamp, SliceType::Transpositions, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::Transpositions, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the key signature, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::Transpositions, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}

	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::addClefToken -- Add a clef token in the data slice at the given
//    timestamp (or create a new clef slice at that timestamp), placing the
//    token at the specified part, staff, and voice index.
//

GridSlice* GridMeasure::addClefToken(const string& tok, HumNum timestamp,
		int part, int staff, int voice, int maxstaff) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::Clefs, maxstaff);
		gs->addToken(tok, part, staff, voice);
		this->push_back(gs);
	} else {
		// search for existing line with same timestamp and the same slice type
		GridSlice* target = NULL;
		auto iterator = this->begin();
		while (iterator != this->end()) {
			if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isClefSlice()) {
				target = *iterator;
				target->addToken(tok, part, staff, voice);
				break;
			} else if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
				// found the correct timestamp, but no clef slice at the timestamp
				// so add the clef slice before the data slice (eventually keepping
				// track of the order in which the other non-data slices should be placed).
				gs = new GridSlice(this, timestamp, SliceType::Clefs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::Clefs, maxstaff);
				gs->addToken(tok, part, staff, voice);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}

		if (iterator == this->end()) {
			// Couldn't find a place for the key signature, so place at end of measure.
			gs = new GridSlice(this, timestamp, SliceType::Clefs, maxstaff);
			gs->addToken(tok, part, staff, voice);
			this->insert(iterator, gs);
		}
	}

	return gs;
}



//////////////////////////////
//
// GridMeasure::addGlobalComment -- Add a global comment at the given
//    timestamp (before any data line at the same timestamp).
//

GridSlice* GridMeasure::addGlobalComment(const string& tok, HumNum timestamp) {
	GridSlice* gs = NULL;
	if (this->empty() || (this->back()->getTimestamp() < timestamp)) {
		// add a new GridSlice to an empty list or at end of list if timestamp
		// is after last entry in list.
		gs = new GridSlice(this, timestamp, SliceType::GlobalComments, 1);
		gs->addToken(tok, 0, 0, 0);
		this->push_back(gs);
	} else {
		// search for existing data line (or any other type)  with same timestamp
		auto iterator = this->begin();
		while (iterator != this->end()) {
			// does it need to be before data slice or any slice?
			// if (((*iterator)->getTimestamp() == timestamp) && (*iterator)->isDataSlice()) {
			if ((*iterator)->getTimestamp() == timestamp) {
				// found the correct timestamp on a data slice, so add the global comment
				// before the data slice.  But don't add if the previous
				// grid slice is a global comment with the same text.
				if ((iterator != this->end()) && (*iterator)->isGlobalComment()) {
					if (tok == *(*iterator)->at(0)->at(0)->at(0)->getToken()) {
						// do not insert duplicate global comment
						break;
					}
				}
				gs = new GridSlice(this, timestamp, SliceType::GlobalComments, 1);
				gs->addToken(tok, 0, 0, 0);
				this->insert(iterator, gs);
				break;
			} else if ((*iterator)->getTimestamp() > timestamp) {
				gs = new GridSlice(this, timestamp, SliceType::GlobalComments, 1);
				gs->addToken(tok, 0, 0, 0);
				this->insert(iterator, gs);
				break;
			}
			iterator++;
		}
	}
	return gs;
}



//////////////////////////////
//
// GridMeasure::transferTokens --
//    default value: startbarnum = 0
//

bool GridMeasure::transferTokens(HumdrumFile& outfile, bool recip,
		bool addbar, int startbarnum) {

	// If the last data slice duration is zero, then calculate
	// the true duration from the duration of the measure.
	if (this->size() > 0) {
		GridSlice* slice = back();
		if (slice->isMeasureSlice() && (this->size() >= 2)) {
			auto ending = this->end();
			--ending;
			--ending;
			while ((ending != this->begin()) && (!(*ending)->isDataSlice())) {
				--ending;
			}
			slice = *ending;
		} else {
			slice = NULL;
		}
		if ((slice != NULL) && slice->isDataSlice()
				&& (slice->getDuration() == 0)) {
			HumNum mts  = getTimestamp();
			HumNum mdur = getDuration();
			HumNum sts  = slice->getTimestamp();
			HumNum slicedur = (mts + mdur) - sts;
			slice->setDuration(slicedur);
		}
	}

	bool founddata = false;
	bool addedbar = false;

	for (auto it : *this) {
		if (it->isInvalidSlice()) {
			// ignore slices to be removed from output (used for
			// removing redundant clef slices).
			continue;
		}
		if (it->isDataSlice()) {
			founddata = true;
		}
		if (it->isLayoutSlice()) {
			// didn't actually find data, but barline should
			// not cross this line.
			founddata = true;
		}
		if (it->isManipulatorSlice()) {
			// didn't acutally find data, but the barline should
			// be placed before any manipulator (a spine split), since
			// that is more a property of the data than of the header
			// interpretations.
			founddata = true;
		}
		if (founddata && addbar && !addedbar) {
			if (getDuration() == 0) {
				// do nothing
			} else {
				if (startbarnum) {
					appendInitialBarline(outfile, startbarnum);
				} else {
					appendInitialBarline(outfile);
				}
				addedbar = true;
			}
		}
		it->transferTokens(outfile, recip);
	}
	return true;
}



//////////////////////////////
//
// GridMeasure::appendInitialBarline -- The barline will be
//    duplicated to all spines later.
//

void GridMeasure::appendInitialBarline(HumdrumFile& infile, int startbarline) {
	if (infile.getLineCount() == 0) {
		// strange case which should never happen.
		return;
	}
	int fieldcount = infile.back()->getFieldCount();
	HumdrumLine* line = new HumdrumLine;
	string tstring = "=";
	if (startbarline) {
		tstring += to_string(startbarline);
	} else {
		tstring += "1";
	}
	tstring += "-";
	HTp token;
	for (int i=0; i<fieldcount; i++) {
		token = new HumdrumToken(tstring);
		line->appendToken(token);
	}
	infile.push_back(line);
}



//////////////////////////////
//
// GridMeasure::getOwner --
//

HumGrid* GridMeasure::getOwner(void) {
	return m_owner;
}



//////////////////////////////
//
// GridMeasure::setOwner --
//

void GridMeasure::setOwner(HumGrid* owner) {
	m_owner = owner;
}



//////////////////////////////
//
// GridMeasure::setDuration --
//

void GridMeasure::setDuration(HumNum duration) {
	m_duration = duration;
}



//////////////////////////////
//
// GridMeasure::getDuration --
//

HumNum GridMeasure::getDuration(void) {
	return m_duration;
}



//////////////////////////////
//
// GridMeasure::getTimestamp --
//

HumNum GridMeasure::getTimestamp(void) {
	return m_timestamp;
}



//////////////////////////////
//
// GridMeasure::setTimestamp --
//

void GridMeasure::setTimestamp(HumNum timestamp) {
	m_timestamp = timestamp;
}



//////////////////////////////
//
// GridMeasure::getTimeSigDur --
//

HumNum GridMeasure::getTimeSigDur(void) {
	return m_timesigdur;
}



//////////////////////////////
//
// GridMeasure::setTimeSigDur --
//

void GridMeasure::setTimeSigDur(HumNum duration) {
	m_timesigdur = duration;
}



//////////////////////////////
//
// GridMeasure::addLayoutParameter --
//

void GridMeasure::addLayoutParameter(GridSlice* slice, int partindex, const string& locomment) {
	auto iter = this->rbegin();
	if (iter == this->rend()) {
		// something strange happened: expecting at least one item in measure.
		return;
	}
	GridPart* part;
	GridStaff* staff;
	GridVoice* voice;

	auto previous = iter;
	previous++;
	while (previous != this->rend()) {
		if ((*previous)->isLayoutSlice()) {
			part = (*previous)->at(partindex);
			staff = part->at(0);
			voice = staff->at(0);
			if (voice) {
				if (voice->getToken() == NULL) {
					// create a token with text
					HTp newtoken = new HumdrumToken(locomment);
					voice->setToken(newtoken);
					return;
				} else if (*voice->getToken() == "!") {
					// replace token with text
					HTp newtoken = new HumdrumToken(locomment);
					voice->setToken(newtoken);
					return;
				}
			} else {
				previous++;
				continue;
			}
		} else {
			break;
		}
		previous++;
	}

	auto insertpoint = previous.base();
	GridSlice* newslice = new GridSlice(this, (*iter)->getTimestamp(), SliceType::Layouts);
	newslice->initializeBySlice(*iter);
	this->insert(insertpoint, newslice);
	HTp newtoken = new HumdrumToken(locomment);
	newslice->at(partindex)->at(0)->at(0)->setToken(newtoken);
}




//////////////////////////////
//
// GridMeasure::addDynamicsLayoutParameters --
//

void GridMeasure::addDynamicsLayoutParameters(GridSlice* slice, int partindex,
		const string& locomment) {
	auto iter = this->rbegin();
	if (iter == this->rend()) {
		// something strange happened: expecting at least one item in measure.
		return;
	}
	GridPart* part;

	while ((iter != this->rend()) && (*iter != slice)) {
		iter++;
	}

	if (*iter != slice) {
		// cannot find owning line.
		return;
	}

	auto previous = iter;
	previous++;
	while (previous != this->rend()) {
		if ((*previous)->isLayoutSlice()) {
			part = (*previous)->at(partindex);
			if ((part->getDynamics() == NULL) || (*part->getDynamics() == "!")) {
				HTp token = new HumdrumToken(locomment);
				part->setDynamics(token);
				return;
			} else {
				previous++;
				continue;
			}
		} else {
			break;
		}
	}

	auto insertpoint = previous.base();
	GridSlice* newslice = new GridSlice(this, (*iter)->getTimestamp(), SliceType::Layouts);
	newslice->initializeBySlice(*iter);
	this->insert(insertpoint, newslice);

	HTp newtoken = new HumdrumToken(locomment);
	newslice->at(partindex)->setDynamics(newtoken);
}



//////////////////////////////
//
// GridMeasure::isMonophonicMeasure --  One part starts with note/rest, the others
//     with invisible rest.
//

bool GridMeasure::isMonophonicMeasure(void) {
	int inviscount = 0;
	int viscount = 0;

	for (auto slice : *this) {
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
						return false;
					}
					if (token->find("yy")) {
						inviscount++;
					} else {
						viscount++;
					}
				}
				if (inviscount + viscount) {
					break;
				}
			}
			if (inviscount + viscount) {
				break;
			}
		}
		if (inviscount + viscount) {
			break;
		}
	}
	if ((viscount = 1) && (inviscount > 0)) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// GridMeasure::isSingleChordMeasure --
//

bool GridMeasure::isSingleChordMeasure(void) {

	for (auto slice : *this) {
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
						return false;
					}
					if (!token->isChord()) {
						return false;
					}
				}
			}
		}
	}
	return true;

}



//////////////////////////////
//
// GridMeasure::isInvisible --
//

bool GridMeasure::isInvisible(void) {

	for (auto slice : *this) {
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
						return false;
					}
					if (token->find("yy") == string::npos) {
						return false;
					}
				}
			}
		}
	}
	return true;

}



//////////////////////////////
//
// GridMeasure::getFirstSpinedSlice --
//

GridSlice* GridMeasure::getFirstSpinedSlice(void) {
	GridSlice* output = NULL;
	for (auto tslice : *this) {
		if (!tslice->hasSpines()) {
			continue;
		}
		output = tslice;
		break;
	}
	return output;
}



//////////////////////////////
//
// GridMeasure::getLastSpinedSlice --
//

GridSlice* GridMeasure::getLastSpinedSlice(void) {
	for (auto rit = this->rbegin(); rit != this->rend(); rit++) {
		GridSlice* slice = *rit;
		if (!slice) {
			continue;
		}
		if (slice->isGlobalLayout()) {
			continue;
		}
		if (slice->isGlobalComment()) {
			continue;
		}
		if (slice->isReferenceRecord()) {
			continue;
		}
		return slice;
	}
	return NULL;
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& output, GridMeasure* measure) {
	output << *measure;
	return output;
}

ostream& operator<<(ostream& output, GridMeasure& measure) {
	for (auto item : measure) {
		output << item << endl;
	}
	return output;
}



// END_MERGE

} // end namespace hum



