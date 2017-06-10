//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar  5 21:32:27 PST 2002
// Last Modified: Tue Jun  6 10:07:14 CEST 2017
// Filename:      tool-esac2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-esac2hum.h
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for esac2hum tool.
//

#ifndef _TOOL_ESAC2HUM_H_INCLUDED
#define _TOOL_ESAC2HUM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE


#define ND_NOTE 0  /* notes or rests + text and phrase markings */
#define ND_BAR  1  /* explicit barlines */


class NoteData {
	public:
		NoteData(void) { clear(); }
		void clear(void) { bar = pitch = phstart = phend = 0;
							  phnum = -1;
							  lyricerr = lyricnum = 0;
							  tiestart = tiecont = tieend = 0;
							  slstart = slend = 0;
							  num = denom = barnum = 0;
							  barinterp = 0; bardur = 0.0;
							  duration = 0.0; text = ""; }
		double duration;
		int    bar;       int    num;
		int    denom;     int    barnum;
		double bardur;    int    barinterp;
		int    pitch;     int    lyricerr;
		int    phstart;   int    phend;    int phnum;
		int    slstart;   int    slend;    int lyricnum;
		int    tiestart;  int    tiecont;  int tieend;
		string text;
};

		

class Tool_esac2hum : public HumTool {
	public:
		         Tool_esac2hum         (void);
		        ~Tool_esac2hum         () {};

		bool    convertFile          (ostream& out, const string& filename);
		bool    convert              (ostream& out, const string& input);
		bool    convert              (ostream& out, istream& input);

	protected:
		bool      initialize            (void);
		void      checkOptions          (Options& opts, int argc, char** argv);
		void      example               (void);
		void      usage                 (const string& command);
		void      convertEsacToHumdrum  (ostream& out, istream& input);
		void      getSong               (vector<string>& song, istream& infile, 
		                                int init);
		void      convertSong           (vector<string>& song, ostream& out);
		bool      getKeyInfo            (vector<string>& song, string& key, 
		                                 double& mindur, int& tonic, string& meter,
		                                 ostream& out);
		void      printNoteData         (NoteData& data, int textQ, ostream& out);
		bool      getNoteList           (vector<string>& song, 
		                                 vector<NoteData>& songdata, double mindur,
		                                 int tonic);
		void      getMeterInfo          (string& meter, vector<int>& numerator, 
		                                 vector<int>& denominator);
		void      postProcessSongData   (vector<NoteData>& songdata,
		                                 vector<int>& numerator,vector<int>& denominator);
		void      printKeyInfo          (vector<NoteData>& songdata, int tonic, 
		                                 int textQ, ostream& out);
		int       getAccidentalMax      (int a, int b, int c);
		bool      printTitleInfo        (vector<string>& song, ostream& out);
		void      getLineRange          (vector<string>& song, const string& field, 
		                                 int& start, int& stop);
		void      printChar             (unsigned char c, ostream& out);
		void      printBibInfo          (vector<string>& song, ostream& out);
		void      printString           (const string& string, ostream& out);
		void      printSpecialChars     (ostream& out);
		bool      placeLyrics           (vector<string>& song,
		                                 vector<NoteData>& songdata);
		bool      placeLyricPhrase      (vector<NoteData>& songdata, 
		                                 vector<string>& lyrics, int line);
		void      getLyrics             (vector<string>& lyrics, const string& buffer);
		void      cleanupLyrics         (vector<string>& lyrics);
		bool      getFileContents       (vector<string>& array, const string& filename);
		void      chopExtraInfo         (char* holdbuffer);
		void      printHumdrumHeaderInfo(ostream& out, vector<string>& song);
		void      printHumdrumFooterInfo(ostream& out, vector<string>& song);
		
	private:
		int            debugQ = 0;        // used with --debug option
		int            verboseQ = 0;      // used with -v option
		int            splitQ = 0;        // used with -s option
		int            firstfilenum = 1;  // used with -f option
		vector<string> header;            // used with -h option
		vector<string> trailer;           // used with -t option
		string         fileextension;     // used with -x option
		string         namebase;          // used with -s option

		vector<int>    chartable;  // used printChars() & printSpecialChars()
		int inputline = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ESAC2HUM_H_INCLUDED */



