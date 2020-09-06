//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 27 07:18:04 PDT 2017
// Last Modified: Sun Aug 27 07:18:07 PDT 2017
// Filename:      tool-msearch.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-msearch.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Interface for msearch tool.
//

#ifndef _TOOL_MSEARCH_H
#define _TOOL_MSEARCH_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"

namespace hum {

// START_MERGE

class MSearchQueryToken {
	public:
		MSearchQueryToken(void) {
			clear();
		}
		MSearchQueryToken(const MSearchQueryToken& token) {
			anything    = token.anything;
			anypitch    = token.anypitch;
			anyinterval = token.anyinterval;
			anyrhythm   = token.anyrhythm;
			pc          = token.pc;
			base        = token.base;
			direction   = token.direction;
			dinterval   = token.dinterval;
			cinterval   = token.cinterval;
			duration    = token.duration;
			rhythm      = token.rhythm;
		}
		MSearchQueryToken& operator=(const MSearchQueryToken& token) {
			if (this == &token) {
				return *this;
			}
			anything    = token.anything;
			anypitch    = token.anypitch;
			anyinterval = token.anyinterval;
			anyrhythm   = token.anyrhythm;
			pc          = token.pc;
			base        = token.base;
			direction   = token.direction;
			dinterval   = token.dinterval;
			cinterval   = token.cinterval;
			duration    = token.duration;
			rhythm      = token.rhythm;
			return *this;
		}
		void clear(void) {
			anything     = true;
			anypitch     = true;
			anyrhythm    = true;
			anyinterval  = true;
			pc           = NAN;
			base         = 0;
			direction    = -123456789; // interval direction
			dinterval    = -123456789; // diatonic interval
			cinterval    = -123456789; // chromatic interval
			duration     = -1;
			rhythm       = "";
		}

		bool   anything    = true;  // element can match any note/rest
		bool   anypitch    = true;  // element can match any pitch class
		bool   anyrhythm   = true;  // element can match any rhythm
		bool   anyinterval = true;  // element can match any interval

		// pitch features
		double pc;           // NAN = rest
		int    base;

		// interval features:
		int    direction;   // which melodic direction for interval?
		int    dinterval;   // diatonic interval
		int    cinterval;   // chromatic interval (base-40; up to 2 sharps/flats)

		// rhythm features:
		HumNum duration;
		string rhythm;
};


ostream& operator<<(ostream& out, MSearchQueryToken& item);

class MSearchTextQuery {
	public:
		MSearchTextQuery(void) {
			clear();
		}
		MSearchTextQuery(const MSearchTextQuery& token) {
			word = token.word;
			link = token.link;
		}
		MSearchTextQuery& operator=(const MSearchTextQuery& token) {
			if (this == &token) {
				return *this;
			}
			word = token.word;
			link = token.link;
			return *this;
		}
		void clear(void) {
			word.clear();
			link = false;
		}
		string word;
		bool link = false;
};


class TextInfo {
	public:
		TextInfo(void) {
			clear();
		}
		TextInfo(const TextInfo& info) {
			fullword = info.fullword;
			starttoken = info.starttoken;
			nexttoken = info.nexttoken;
		}
		TextInfo& operator=(const TextInfo& info) {
			if (this == &info) {
				return *this;
			}
			fullword = info.fullword;
			starttoken = info.starttoken;
			nexttoken = info.nexttoken;
			return *this;
		}
		void clear(void) {
			fullword.clear();
			starttoken = NULL;
			nexttoken = NULL;
		}
		string fullword;
		HTp starttoken;
		HTp nexttoken;
};


class Tool_msearch : public HumTool {
	public:
		         Tool_msearch      (void);
		        ~Tool_msearch      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    initialize         (void);
		void    doMusicSearch      (HumdrumFile& infile, NoteGrid& grid,
		                            vector<MSearchQueryToken>& query);
		void    doTextSearch       (HumdrumFile& infile, NoteGrid& grid,
		                            vector<MSearchTextQuery>& query);
		void    fillMusicQuery     (vector<MSearchQueryToken>& query);
		void    fillMusicQueryInterleaved(vector<MSearchQueryToken>& query,
		                            const std::string& input, bool rhythmQ = false);
		void    fillMusicQueryPitch(vector<MSearchQueryToken>& query,
		                            const std::string& input);
		void    fillMusicQueryInterval(vector<MSearchQueryToken>& query,
		                            const std::string& input);
		void    fillMusicQueryRhythm(vector<MSearchQueryToken>& query,
		                            const std::string& input);
		void    fillTextQuery      (vector<MSearchTextQuery>& query,
		                            const std::string& input);
		bool    checkForMatchDiatonicPC(vector<NoteCell*>& notes, int index,
		                            vector<MSearchQueryToken>& dpcQuery,
		                            vector<NoteCell*>& match);
		void    markMatch          (HumdrumFile& infile,
		                            vector<NoteCell*>& match);
		void    markTextMatch      (HumdrumFile& infile, TextInfo& word);
		void    fillWords          (HumdrumFile& infile,
		                            vector<TextInfo*>& words);
		void    fillWordsForTrack  (vector<TextInfo*>& words,
		                            HTp starttoken);
		void    printQuery         (vector<MSearchQueryToken>& query);
		void    addMusicSearchSummary (HumdrumFile& infile, int mcount, const string& marker);
		void    addTextSearchSummary (HumdrumFile& infile, int mcount, const string& marker);
		int     makeBase40Interval  (int diatonic, const string& alteration);

	private:
	 	vector<HTp> m_kernspines;
		string      m_text;
		string      m_marker;
		bool        m_markQ      = false;
		bool        m_quietQ     = false;
		bool        m_debugQ     = false;
		bool        m_nooverlapQ = false;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MSEARCH_H */



