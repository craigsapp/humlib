//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Mon Sep 18 15:46:44 PDT 2017
// Filename:      GridMeasure.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/GridMeasure.h
// Syntax:        C++11; humlib
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
#include <string>

namespace hum {

// START_MERGE

class GridSlice;
class HumGrid;

class GridMeasure : public std::list<GridSlice*> {
	public:
		GridMeasure(HumGrid* owner);
		~GridMeasure();

		GridSlice*   addTempoToken  (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addTempoToken  (GridSlice* slice, int partindex,
		                             const std::string& tempo);
		GridSlice*   addTimeSigToken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addMeterSigToken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addKeySigToken (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addClefToken   (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addBarlineToken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addTransposeToken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addLabelToken  (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxpart,
		                             int maxstaff);
		GridSlice*   addLabelAbbrToken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxpart,
		                             int maxstaff);
		GridSlice*   addDataToken   (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff);
		GridSlice*   addDataSubtoken(const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice);
		GridSlice*   addGraceToken  (const std::string& tok, HumNum timestamp,
		                             int part, int staff, int voice, int maxstaff,
		                             int gracenumber);
		GridSlice*   addGlobalLayout(const std::string& tok, HumNum timestamp);
		GridSlice*   addGlobalComment(const std::string& tok, HumNum timestamp);
		GridSlice*   appendGlobalLayout(const std::string& tok, HumNum timestamp);
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
		void         setKernBar     (const std::string& tok);
		void         setInvisibleBarline(void) { setStyle(MeasureStyle::Invisible); }
		void         setFinalBarlineStyle(void) { setStyle(MeasureStyle::Final); }
		void         setRepeatEndStyle(void) { setStyle(MeasureStyle::RepeatBackward); }
		void         setRepeatBackwardStyle(void) { setStyle(MeasureStyle::RepeatBackward); }
		void         setMeasureNumber(int value);
		int          getMeasureNumber(void);

		bool         isDouble(void)
		                  {return m_style == MeasureStyle::Double;}
		bool         isFinal(void)
		                  {return m_style == MeasureStyle::Final;}
		bool         isRepeatBackward(void)
		                  { return m_style == MeasureStyle::RepeatBackward; }
		bool         isInvisibleBarline(void)
		                  { return m_style == MeasureStyle::Invisible; }
		bool         isRepeatForward(void)
		                  { return m_style == MeasureStyle::RepeatForward; }
		bool         isRepeatBoth(void)
		                  { return m_style == MeasureStyle::RepeatBoth; }
		void         addInterpretationBefore(GridSlice* slice, int partindex, int staffindex, int voiceindex, const std::string& interpretation);
		void         addInterpretationAfter(GridSlice* slice, int partindex, int staffindex, int voiceindex, const std::string& interpretation, HumNum timestamp);
		void         addLayoutParameter(GridSlice* slice, int partindex, const std::string& locomment);
		void         addLayoutParameter(HumNum timestamp, int partindex, int staffindex, const std::string& locomment);
		void         addDynamicsLayoutParameters(GridSlice* slice, int partindex, const std::string& locomment);
		void         addFiguredBassLayoutParameters(GridSlice* slice, int partindex, const std::string& locomment);
		GridSlice*   addFiguredBass(HTp token, HumNum timestamp, int part, int maxstaff);
		GridSlice*   addFiguredBass(const std::string& tok, HumNum timestamp, int part, int maxstaff);
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
		std::string  m_kernBar;
		int          m_barnum = -1;
};

std::ostream& operator<<(std::ostream& output, GridMeasure& measure);
std::ostream& operator<<(std::ostream& output, GridMeasure* measure);

// END_MERGE

} // end namespace hum

#endif /* _GRIDMEASURE_H */



