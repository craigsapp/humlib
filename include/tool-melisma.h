//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug 24 17:41:44 EDT 2019
// Last Modified: Sat Aug 24 17:41:47 EDT 2019
// Filename:      tool-melisma.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-melisma.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyze melismatic activity in vocal music.
//

#ifndef _TOOL_MELISMA_H_INCLUDED
#define _TOOL_MELISMA_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class WordInfo {
	public:
		string word;                 // text of word
		int notes = 0;               // number of notes in word
		HumNum starttime;            // start time of word
		HumNum endtime;              // end time of word
		int bar = 0;                 // starting barline number for word
	  	vector<int> bars;            // starting barline number for each syllable
		vector<string> syllables;    // list of syllables in word with melisma
		vector<int> notecounts;      // list of note counts for each syllable in word
		vector<HumNum> starttimes;   // list of start times for each syllable
		vector<HumNum> endtimes;     // list of end times for each syllable
		HumNum duration(void) { return endtime - starttime; }
		string name;
		string abbreviation;
		int partnum = 0;
		void clear(void) {
			starttime = 0;
			endtime   = 0;
			partnum   = 0;
			notes     = 0;
			bar       = 0;
			abbreviation.clear();
			notecounts.clear();
			starttimes.clear();
			syllables.clear();
			endtimes.clear();
			word.clear();
			name.clear();
			bars.clear();
		}
};


class Tool_melisma : public HumTool {
	public:
		      Tool_melisma             (void);
		     ~Tool_melisma             () {};

		bool  run                      (HumdrumFileSet& infiles);
		bool  run                      (HumdrumFile& infile);
		bool  run                      (const string& indata, ostream& out);
		bool  run                      (HumdrumFile& infile, ostream& out);

	protected:
		void   initialize              (HumdrumFile& infile);
		void   processFile             (HumdrumFile& infile);
		void   getNoteCounts           (HumdrumFile& infile, vector<vector<int>>& counts);
		void   getNoteCountsForLyric   (vector<vector<int>>& counts, HTp lyricStart);
		int    getCountForSyllable     (HTp token);
		void   replaceLyrics           (HumdrumFile& infile, vector<vector<int>>& counts);
		void   markMelismas            (HumdrumFile& infile, vector<vector<int>>& counts);
		void   markMelismaNotes        (HTp text, int count);
		void   extractWordlist         (vector<WordInfo>& wordinfo, map<string, int>& wordlist,
		                                HumdrumFile& infile, vector<vector<int>>& notecount);
		string extractWord             (WordInfo& winfo, HTp token, vector<vector<int>>& counts);
		HumNum getEndtime              (HTp text);
		void   printWordlist           (HumdrumFile& infile, vector<WordInfo>& wordinfo, 
		                                map<string, int>);
		void   initializePartInfo      (HumdrumFile& infile);
		void   getMelismaNoteCounts    (vector<int>& ncounts, vector<int>& mcounts,
		                                HumdrumFile& infile);
		double getScoreDuration        (HumdrumFile& infile);
		void   initBarlines            (HumdrumFile& infile);

	private:
		vector<vector<HumNum>> m_endtimes;      // end time of syllables indexed by line/field
		vector<string>         m_names;         // name of parts indexed by track
		vector<string>         m_abbreviations; // abbreviation of parts indexed by track
		vector<int>            m_partnums;      // part number index by track
		vector<int>            m_measures;      // current measure number

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_MELISMA_H_INCLUDED */



