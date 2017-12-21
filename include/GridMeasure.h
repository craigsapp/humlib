//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Mon Sep 18 15:46:44 PDT 2017
// Filename:      GridMeasure.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/GridMeasure.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax. HumGrid is composed
//                of a vector of GridMeasures which contain the data for
//                all parts in particular MusicXML <measure>.
//
//

#ifndef _GRIDMEASURE_H
#define _GRIDMEASURE_H

#include "GridCommon.h"
#include "HumdrumFile.h"

#include <list>

using namespace std;

namespace hum {

// START_MERGE

class GridSlice;
class HumGrid;

class GridMeasure : public list<GridSlice*> {
	public:
		GridMeasure(HumGrid* owner);
		~GridMeasure();

		GridSlice*   addTempoToken  (const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addTimeSigToken(const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addKeySigToken (const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addClefToken   (const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addTransposeToken(const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addLabelToken(const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addLabelAbbrToken(const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addDataToken   (const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addGraceToken  (const string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff,
		                             int gracenumber);
		GridSlice*   addGlobalLayout(const string& tok, HumNum timestamp);
		GridSlice*   addGlobalComment(const string& tok, HumNum timestamp);
		GridSlice*   appendGlobalLayout(const string& tok, HumNum timestamp);
		bool         transferTokens (HumdrumFile& outfile, bool recip,
		                             bool addbar, int startbarnum = 0);
		HumGrid*     getOwner       (void);
		void         setOwner       (HumGrid* owner);
		HumNum       getDuration    (void);
		void         setDuration    (HumNum duration);
		HumNum       getTimestamp   (void);
		void         setTimestamp   (HumNum timestamp);
		HumNum       getTimeSigDur  (void);
		void         setTimeSigDur  (HumNum duration);
		MeasureStyle getStyle       (void) { return m_style; }
		MeasureStyle getBarStyle    (void) { return getStyle(); }
		void         setStyle       (MeasureStyle style) { m_style = style; }
		void         setBarStyle    (MeasureStyle style) { setStyle(style); }
		void         setFinalBarlineStyle(void) { setStyle(MeasureStyle::Final); }
		void         setRepeatEndStyle(void) { setStyle(MeasureStyle::RepeatBackward); }
		void         setRepeatBackwardStyle(void) { setStyle(MeasureStyle::RepeatBackward); }

		bool         isDouble(void) 
		                  {return m_style == MeasureStyle::Double;}
		bool         isFinal(void) 
		                  {return m_style == MeasureStyle::Final;}
		bool         isRepeatBackward(void) 
		                  { return m_style == MeasureStyle::RepeatBackward; }
		bool         isRepeatForward(void) 
		                  { return m_style == MeasureStyle::RepeatForward; }
		bool         isRepeatBoth(void) 
		                  { return m_style == MeasureStyle::RepeatBoth; }
		void         addLayoutParameter(GridSlice* slice, int partindex, const string& locomment);
		void         addDynamicsLayoutParameters(GridSlice* slice, int partindex, const string& locomment);
		bool         isInvisible(void);
		bool         isSingleChordMeasure(void);
		bool         isMonophonicMeasure(void);
		GridSlice*   getLastSpinedSlice(void);
		GridSlice*   getFirstSpinedSlice(void);
	
	protected:
		void         appendInitialBarline(HumdrumFile& infile, int startbarnum = 0);

	private:
		HumGrid*     m_owner;
		HumNum       m_duration;
		HumNum       m_timestamp;
		HumNum       m_timesigdur;
		MeasureStyle m_style;
};

ostream& operator<<(ostream& output, GridMeasure& measure);
ostream& operator<<(ostream& output, GridMeasure* measure);

// END_MERGE

} // end namespace hum

#endif /* _GRIDMEASURE_H */



