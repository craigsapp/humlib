//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 20:36:38 PST 2016
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-myank.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-myank.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for myank tool.
//

#ifndef _TOOL_MYANK_H_INCLUDED
#define _TOOL_MYANK_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class MyCoord {
	public:
		     MyCoord   (void) { clear(); }
		void clear   (void) { x = -1; y = -1; }
		bool isValid (void) { return ((x < 0) || (y < 0)) ? false : true; }
		int  x;
		int  y;
};

class MeasureInfo {
	public:
		MeasureInfo(void) { clear(); }
		void clear(void)  { num = seg = start = stop = -1;
			sclef.resize(0); smclef.resize(0); soclef.resize(0);
			skeysig.resize(0); skey.resize(0);
			stimesig.resize(0); smet.resize(0); stempo.resize(0);
			eclef.resize(0); emclef.resize(0); eoclef.resize(0);
			ekeysig.resize(0); ekey.resize(0);
			etimesig.resize(0); emet.resize(0); etempo.resize(0);
			file = NULL;
		}
		void setTrackCount(int tcount) {
			sclef.resize(tcount+1);
			smclef.resize(tcount+1);
			soclef.resize(tcount+1);
			skeysig.resize(tcount+1);
			skey.resize(tcount+1);
			stimesig.resize(tcount+1);
			smet.resize(tcount+1);
			stempo.resize(tcount+1);
			eclef.resize(tcount+1);
			emclef.resize(tcount+1);
			eoclef.resize(tcount+1);
			ekeysig.resize(tcount+1);
			ekey.resize(tcount+1);
			etimesig.resize(tcount+1);
			emet.resize(tcount+1);
			etempo.resize(tcount+1);
			int i;
			for (i=0; i<tcount+1; i++) {
				sclef[i].clear();
				smclef[i].clear();
				soclef[i].clear();
				skeysig[i].clear();
				skey[i].clear();
				stimesig[i].clear();
				smet[i].clear();
				stempo[i].clear();
				eclef[i].clear();
				emclef[i].clear();
				eoclef[i].clear();
				ekeysig[i].clear();
				ekey[i].clear();
				etimesig[i].clear();
				emet[i].clear();
				etempo[i].clear();
			}
			tracks = tcount;
		}
		int num;          // measure number
		string stopStyle;  // styling for end of last measure
		string startStyle; // styling for start of first measure
		int seg;          // measure segment
		int start;        // starting line of segment
		int stop;         // ending line of segment
		int tracks;       // number of primary tracks in file.
		HumdrumFile* file;

		// musical settings at start of measure
		vector<MyCoord> sclef;     // starting clef of segment
		vector<MyCoord> smclef;    // starting mclef of segment
		vector<MyCoord> soclef;    // starting oclef of segment
		vector<MyCoord> skeysig;   // starting keysig of segment
		vector<MyCoord> skey;      // starting key of segment
		vector<MyCoord> stimesig;  // starting timesig of segment
		vector<MyCoord> smet;      // starting met of segment
		vector<MyCoord> stempo;    // starting tempo of segment

		// musical settings at start of measure
		vector<MyCoord> eclef;     // ending clef    of segment
		vector<MyCoord> emclef;    // ending mclef   of segment
		vector<MyCoord> eoclef;    // ending oclef   of segment
		vector<MyCoord> ekeysig;   // ending keysig  of segment
		vector<MyCoord> ekey;      // ending key     of segment
		vector<MyCoord> etimesig;  // ending timesig of segment
		vector<MyCoord> emet;      // ending met     of segment
		vector<MyCoord> etempo;    // ending tempo   of segment
};



class Tool_myank : public HumTool {
	public:
		         Tool_myank            (void);
		        ~Tool_myank            () {};

		bool     run                   (HumdrumFileSet& infiles);
		bool     run                   (HumdrumFile& infile);
		bool     run                   (const string& indata, ostream& out);
		bool     run                   (HumdrumFile& infile, ostream& out);

	protected:
		void      initialize            (HumdrumFile& infile);
		void      example              (void);
		void      usage                (const string& command);
		void      myank                (HumdrumFile& infile,
		                                vector<MeasureInfo>& outmeasure);
		void      removeDollarsFromString(string& buffer, int maxx);
		void      processFieldEntry    (vector<MeasureInfo>& field,
		                                const string& str,
		                                HumdrumFile& infile, int maxmeasure,
		                                vector<MeasureInfo>& inmeasures,
		                                vector<int>& inmap);
		void      expandMeasureOutList (vector<MeasureInfo>& measureout,
		                                vector<MeasureInfo>& measurein,
		                                HumdrumFile& infile, const string& optionstring);
		void      getMeasureStartStop  (vector<MeasureInfo>& measurelist,
		                                HumdrumFile& infile);
		void      printEnding          (HumdrumFile& infile, int lastline, int adjlin);
		void      printStarting        (HumdrumFile& infile);
		void      reconcileSpineBoundary(HumdrumFile& infile, int index1, int index2);
		void      reconcileStartingPosition(HumdrumFile& infile, int index2);
		void      printJoinLine        (vector<int>& splits, int index, int count);
		void      printInvisibleMeasure(HumdrumFile& infile, int line);
		void      fillGlobalDefaults   (HumdrumFile& infile,
		                                vector<MeasureInfo>& measurein,
		                                vector<int>& inmap);
		void      adjustGlobalInterpretations(HumdrumFile& infile, int ii,
		                                vector<MeasureInfo>& outmeasures,
		                                int index);
		void      adjustGlobalInterpretationsStart(HumdrumFile& infile, int ii,
		                                vector<MeasureInfo>& outmeasures,
		                                int index);
		void      getMarkString        (ostream& out, HumdrumFile& infile);
		void      printDoubleBarline   (HumdrumFile& infile, int line);
		void      insertZerothMeasure  (vector<MeasureInfo>& measurelist,
		                                HumdrumFile& infile);
		void      getMetStates         (vector<vector<MyCoord> >& metstates,
		                                HumdrumFile& infile);
		MyCoord   getLocalMetInfo      (HumdrumFile& infile, int row, int track);
		int       atEndOfFile          (HumdrumFile& infile, int line);
		void      processFile          (HumdrumFile& infile);
		int       getSectionCount      (HumdrumFile& infile);
		void      getSectionString     (string& sstring, HumdrumFile& infile,
		                                int sec);
		void      collapseSpines       (HumdrumFile& infile, int line);
		void      printMeasureStart    (HumdrumFile& infile, int line, const string& style);
		std::string expandMultipliers  (const string& inputstring);

		vector<int> analyzeBarNumbers  (HumdrumFile& infile);
		int         getBarNumberForLineNumber(int lineNumber);
		int         getStartLineNumber (void);
		int         getEndLineNumber   (void);
		void        printDataLine      (HLp line, bool& startLineHandled, const vector<int>& lastLineResolvedTokenLineIndex, const vector<HumNum>& lastLineDurationsFromNoteStart);

	private:
		int    m_debugQ      = 0;             // used with --debug option
		// int    inputlist     = 0;             // used with --inlist option
		int    m_inlistQ     = 0;             // used with --inlist option
		int    m_outlistQ    = 0;             // used with --outlist option
		int    m_verboseQ    = 0;             // used with -v option
		int    m_invisibleQ  = 1;             // used with --visible option
		int    m_maxQ        = 0;             // used with --max option
		int    m_minQ        = 0;             // used with --min option
		int    m_instrumentQ = 0;             // used with -I option
		int    m_nolastbarQ  = 0;             // used with -B option
		int    m_markQ       = 0;             // used with --mark option
		int    m_doubleQ     = 0;             // used with --mdsep option
		int    m_barnumtextQ = 0;             // used with -T option
		int    m_section     = 0;             // used with --section option
		int    m_sectionCountQ = 0;           // used with --section-count option
		vector<MeasureInfo> m_measureOutList; // used with -m option
		vector<MeasureInfo> m_measureInList;  // used with -m option
		vector<vector<MyCoord> > m_metstates;

		string      m_lineRange;              // used with -l option
		vector<int> m_barNumbersPerLine;      // used with -l option
		bool m_hideStarting;                  // used with --hide-starting option
		bool m_hideEnding;                    // used with --hide-ending option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MYANK_H_INCLUDED */



