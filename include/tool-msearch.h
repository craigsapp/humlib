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
#include "Convert.h"

namespace hum {

// START_MERGE


class SonorityNoteData {
	public:

		SonorityNoteData(void) {
			clear();
		}

		void clear(void) {
			m_token = NULL;
			m_tok.clear();
			m_accidentalQ = false;
			m_upperQ = false;
			m_attackQ = false;
			m_index = 0;
			m_base7 = -1;
			m_base12 = -1;
			m_base40 = -1;
		}

		ostream& print(ostream& out) {
			out << "NOTE:\t"   << m_token   << endl;
			out << "\tINDEX:\t"  << m_index   << endl;
			out << "\tSTRING:\t" << m_tok     << endl;
			out << "\tATTACK:\t" << m_attackQ << endl;
			out << "\tBASE7:\t"  << m_base7   << endl;
			out << "\tBASE40:\t" << m_base40  << endl;
			return out;
		}

		void setToken(HTp token, bool nullQ, int index) {
			m_attackQ = true;
			if (nullQ) {
				m_attackQ = false;
			}
			m_token = token;
			m_index = index;
			if (token->isChord()) {
				m_tok = token->getSubtoken(index);
			} else {
				m_tok = *token;
				m_index = 0;
			}
			if (m_tok.find('_') != string::npos) {
				m_attackQ = false;
			}
			if (m_tok.find(']') != string::npos) {
				m_attackQ = false;
			}
			m_base7 = Convert::kernToBase7(m_tok);
			m_base12 = Convert::kernToBase12(m_tok);
			m_base40 = Convert::kernToBase40(m_tok);
		}

		void setString(std::string tok) {
			// tok cannot be a chord or a null token
			// This version is for vertical queries not for searching data.
			m_attackQ = true;
			m_token = NULL;
			m_index = 0;
			m_tok = tok;
			if (m_tok.find('_') != string::npos) {
				m_attackQ = false;
			}
			if (m_tok.find(']') != string::npos) {
				m_attackQ = false;
			}
			m_base7 = Convert::kernToBase7(m_tok);
			m_base12 = Convert::kernToBase12(m_tok);
			m_base40 = Convert::kernToBase40(m_tok);

			if (m_tok.find('n') != string::npos) {
				m_accidentalQ = true;
			} if (m_tok.find('-') != string::npos) {
				m_accidentalQ = true;
			} if (m_tok.find('#') != string::npos) {
				m_accidentalQ = true;
			}
			for (int i=0; i<(int)m_tok.size(); i++) {
				if (isupper(m_tok[i])) {
					m_upperQ = true;
				}
				break;
			}
		}
	
		bool hasAccidental(void) {
			// Set only with setText() input.
			return m_accidentalQ;
		}

		bool hasUpperCase(void) {
			// Set only with setText() input.
			return m_upperQ;
		}

		bool isValid(void)     { return m_token != NULL;    }
		HTp  getToken(void)    { return m_token;            }
		std::string getText(void) { return m_tok;           }
		int  getIndex(void)    { return m_index;            }
		bool isAttack(void)    { return m_attackQ;          }
		bool isSustain(void)   { return !m_attackQ;         }
		int  getBase12(void)   { return (int)m_base12;      }
		int  getBase12Pc(void) { return (int)m_base12 % 7;  }
		int  getBase7(void)    { return (int)m_base7;       }
		int  getBase7Pc(void)  { return (int)m_base7 % 7;   }
		int  getBase40(void)   { return (int)m_base40;      }
		int  getBase40Pc(void) { return (int)m_base40 % 40; }

	private:
		HTp m_token;
		string m_tok;       // note string from token
		bool m_accidentalQ; // note contains an accidental
		bool m_upperQ;      // Diatonic note name contains an upper case letter
		bool m_attackQ;     // true if note is an attack
		char m_index;       // chord index of note (zero offset)
		char m_base7;       // pitch in base-7 representation
		char m_base12;      // pitch in base-12 representation
		short int m_base40; // pitch in base-40 representation
};



class SonorityDatabase {
	public:
		SonorityDatabase(void) { clear(); }
		void clear(void)       { m_notes.clear(); m_line = NULL; }
		int getCount(void)     { return (int)m_notes.size(); }
		int getNoteCount(void) { return (int)m_notes.size(); }
		int getSize(void)      { return (int)m_notes.size(); }
		bool isEmpty(void)     { return m_notes.empty(); }
		HLp getLine(void)      { return m_line; }
		SonorityNoteData& getLowest(void) { return m_lowest; };
		void addNote          (const std::string& text);
		void buildDatabase     (HLp line);
		SonorityNoteData& operator[](int index) {
			return m_notes.at(index);
		}
	protected:
		void expandList(void) {
			m_notes.resize(m_notes.size() + 1);
		}

	private:
		SonorityNoteData m_lowest;
		std::vector<SonorityNoteData> m_notes;
		HLp m_line = NULL;
};


//////////////////////////////
//
// MSearchQueryToken -- one element of the music search.  This is a combined
//    search of pitch, interval, rhythm and harmony.
//

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
			harmonic    = token.harmonic;
			hpieces     = token.hpieces;
			hquery      = token.hquery;
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
			harmonic    = token.harmonic;
			hpieces     = token.hpieces;
			hquery      = token.hquery;
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
			harmonic     = "";
			hpieces.clear();
			hquery.clear();
			rhythm       = "";
		}
		void parseHarmonicQuery(void);

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
		std::string harmonic; // harmonic query
		std::vector<std::string> hpieces;
		std::vector<SonorityNoteData> hquery;

		// rhythm features:
		HumNum duration;
		std::string rhythm;
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
		std::string word;
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
		std::string fullword;
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
		bool    doHarmonicPitchSearch(MSearchQueryToken& query, HTp token);
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
		bool    checkForMusicMatch(vector<NoteCell*>& notes, int index,
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
		void    addMusicSearchSummary(HumdrumFile& infile, int mcount, const std::string& marker);
		void    addTextSearchSummary(HumdrumFile& infile, int mcount, const std::string& marker);
		int     makeBase40Interval (int diatonic, const std::string& alteration);
		std::string convertPitchesToIntervals(const std::string& input);
		void    markNote           (HTp token, int index);
		int     checkHarmonicPitchMatch (SonorityNoteData& query,
		                           SonorityDatabase& sonorities, bool suppressQ);
		bool    checkVerticalOnly  (const string& input);

	private:
	 	vector<HTp> m_kernspines;
		string      m_text;
		string      m_marker;
		bool        m_verticalOnlyQ = false;
		bool        m_markQ      = false;
		bool        m_quietQ     = false;
		bool        m_debugQ     = false;
		bool        m_nooverlapQ = false;
		std::vector<SonorityDatabase> m_sonorities;
		std::vector<pair<HTp, int>> m_tomark;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_MSEARCH_H */



