//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Aug 27 07:18:04 PDT 2017
// Last Modified: Sun Aug 27 07:18:07 PDT 2017
// Filename:      tool-msearch.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-msearch.h
// Syntax:        C++11
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
			pc        = token.pc;
			base      = token.base;
			direction = token.direction;
			duration  = token.duration;
			rhythm    = token.rhythm;
			anything  = token.anything;
		}
		MSearchQueryToken& operator=(const MSearchQueryToken& token) {
			if (this == &token) {
				return *this;
			}
			pc        = token.pc;
			base      = token.base;
			direction = token.direction;
			duration  = token.duration;
			rhythm    = token.rhythm;
			anything  = token.anything;
			return *this;
		}
		void clear(void) {
			pc        = NAN;
			base      = 0;
			direction = 0;
			duration  = -1;
			rhythm    = "";
			anything  = false;
		}
		double pc;           // NAN = rest
		int    base;
		int    direction; 
		HumNum duration;
		string rhythm;
		bool   anything;
};



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

		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void    initialize         (void);
		void    doMusicSearch      (HumdrumFile& infile, NoteGrid& grid,
		                            vector<MSearchQueryToken>& query);
		void    doTextSearch       (HumdrumFile& infile, NoteGrid& grid,
		                            vector<MSearchTextQuery>& query);
		void    fillMusicQuery     (vector<MSearchQueryToken>& query,
		                            const string& input);
		void    fillTextQuery      (vector<MSearchTextQuery>& query,
		                            const string& input);
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

	private:
	 	vector<HTp> m_kernspines;
		string      m_text;
		string      m_marker;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MSEARCH_H */



