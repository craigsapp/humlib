//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  2 11:58:11 CEST 2023
// Last Modified: Sat Sep  2 11:58:15 CEST 2023
// Filename:      tool-textdur.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-textdur.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of text (lyrics).  Either as **text or **sylba
//

#ifndef _TOOL_TEXTDUR_H
#define _TOOL_TEXTDUR_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_textdur : public HumTool {
	public:
		         Tool_textdur  (void);
		        ~Tool_textdur  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);


	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     printMelismas     (HumdrumFile& infile);
		void     printDurations     (HumdrumFile& infile);
		void     getTextSpineStarts(HumdrumFile& infile, vector<HTp>& starts);
		void     processTextSpine  (vector<HTp>& starts, int index);
		int      getMelisma        (HTp tok1, HTp tok2);
		HumNum   getDuration       (HTp tok1, HTp tok2);
		HTp      getTandemKernToken(HTp token);
		void     printInterleaved  (HumdrumFile& infile);
		void     printInterleavedLine(HumdrumLine& line, vector<bool>& textTrack);
		void     printTokenAnalysis(HTp token);
		void     printAnalysis      (void);
		void     printDurationAverage(void);
		void     printMelismaAverage(void);
		void     printHtmlContent   (void);
		void     printMelismaHtmlHistogram(void);
		void     printMelismaHtmlHistogram(int index, int maxVal);
		void     printDurationHtmlHistogram(void);
		void     fillInstrumentNameInfo(void);
		std::string getColumnName   (HTp token);

	private:
		std::vector<HTp>                 m_textStarts;
		std::vector<int>                 m_track2column;
		std::vector<string>              m_columnName;

		std::vector<std::vector<HTp>>    m_syllables;  // List of syllables in **text/**sylba
		std::vector<std::vector<HumNum>> m_durations;  // List of durations excluding trailing rests
		std::vector<std::vector<int>>    m_melismas;   // List of note counts for syllable

		bool m_analysisQ     = false;  // used with -a option
		bool m_melismaQ      = false;  // used with -m option
		bool m_durationQ     = true;   // used with -m, -d option
		bool m_interleaveQ   = false;  // used with -i option
		HumNum m_RhythmFactor = 1;     // uwed with -1, -2, -8, and later -f #

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TEXTDUR_H */



