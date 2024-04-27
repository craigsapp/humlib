//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Mar  5 21:32:27 PST 2002
// Last Modified: Tue Jun  6 10:07:14 CEST 2017
// Filename:      tool-esac2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-esac2hum.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for esac2hum tool.
//

#ifndef _TOOL_ESAC2HUM_H_INCLUDED
#define _TOOL_ESAC2HUM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <iostream>
#include <string>
#include <vector>

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
		std::string text;
};



class Tool_esac2hum : public HumTool {
	public:
		         Tool_esac2hum         (void);
		        ~Tool_esac2hum         () {};

		bool    convertFile          (std::ostream& out, const std::string& filename);
		bool    convert              (std::ostream& out, const std::string& input);
		bool    convert              (std::ostream& out, std::istream& input);

	protected:
		bool      initialize            (void);
		void      checkOptions          (Options& opts, int argc, char** argv);
		void      example               (void);
		void      usage                 (const std::string& command);
		void      convertEsacToHumdrum  (std::ostream& out, std::istream& input);
		bool      getSong               (std::vector<std::string>& song, std::istream& infile,
		                                int init);
		void      convertSong           (std::vector<std::string>& song, std::ostream& out);
		bool      getKeyInfo            (std::vector<std::string>& song, std::string& key,
		                                 double& mindur, int& tonic, std::string& meter,
		                                 std::ostream& out);
		void      printNoteData         (NoteData& data, int textQ, std::ostream& out);
		bool      getNoteList           (std::vector<std::string>& song,
		                                 std::vector<NoteData>& songdata, double mindur,
		                                 int tonic);
		void      getMeterInfo          (std::string& meter, std::vector<int>& numerator,
		                                 std::vector<int>& denominator);
		void      postProcessSongData   (std::vector<NoteData>& songdata,
		                                 std::vector<int>& numerator,std::vector<int>& denominator);
		void      printKeyInfo          (std::vector<NoteData>& songdata, int tonic,
		                                 int textQ, std::ostream& out);
		int       getAccidentalMax      (int a, int b, int c);
		bool      printTitleInfo        (std::vector<std::string>& song, std::ostream& out);
		void      getLineRange          (std::vector<std::string>& song, const std::string& field,
		                                 int& start, int& stop);
		void      printChar             (unsigned char c, std::ostream& out);
		void      printBibInfo          (std::vector<std::string>& song, std::ostream& out);
		void      printString           (const std::string& string, std::ostream& out);
		void      printSpecialChars     (std::ostream& out);
		bool      placeLyrics           (std::vector<std::string>& song,
		                                 std::vector<NoteData>& songdata);
		bool      placeLyricPhrase      (std::vector<NoteData>& songdata,
		                                 std::vector<std::string>& lyrics, int line);
		void      getLyrics             (std::vector<std::string>& lyrics, const std::string& buffer);
		void      cleanupLyrics         (std::vector<std::string>& lyrics);
		bool      getFileContents       (std::vector<std::string>& array, const std::string& filename);
		void      chopExtraInfo         (std::string& buffer);
		void      printHumdrumHeaderInfo(std::ostream& out, std::vector<std::string>& song);
		void      printHumdrumFooterInfo(std::ostream& out, std::vector<std::string>& song);

	private:
		int            debugQ = 0;        // used with --debug option
		int            verboseQ = 0;      // used with -v option
		int            splitQ = 0;        // used with -s option
		int            firstfilenum = 1;  // used with -f option
		std::vector<std::string> header;            // used with -h option
		std::vector<std::string> trailer;           // used with -t option
		std::string         fileextension;     // used with -x option
		std::string         namebase;          // used with -s option

		std::vector<int>    chartable;  // used printChars() & printSpecialChars()
		int inputline = 0;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ESAC2HUM_H_INCLUDED */



