//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Aug 22 18:30:37 PDT 2024
// Last Modified: Sun Aug 25 08:53:35 PDT 2024
// Filename:      tool-esac2hum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-esac2hum.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Convert newer EsAC data into Humdrum data.
//

#ifndef _TOOL_ESAC2HUM_H
#define _TOOL_ESAC2HUM_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace hum {

// START_MERGE

class Tool_esac2hum : public HumTool {
	public:

		class Note {
			public:
				std::vector<std::string> m_errors;
				std::string esac;
				int    m_dots        = 0;
				int    m_underscores = 0;
				int    m_octave      = 0;
				int    m_degree      = 0;  // scale degree (wrt major key)
				int    m_b40degree   = 0;  // scale degree as b40 interval
				int    m_alter       = 0;  // chromatic alteration of degree (flats/sharp from major scale degrees)
				double m_ticks       = 0.0;
				bool   m_tieBegin    = false;
				bool   m_tieEnd      = false;
				bool   m_phraseBegin = false;
				bool   m_phraseEnd   = false;
				std::string m_humdrum; // **kern conversion of EsAC note
				int    m_b40         = 0;  // absolute b40 pitch (-1000 = rest);
				int    m_b12         = 0;  // MIDI note number (-1000 = rest);
				HumNum m_factor      = 1;  // for triplet which is 2/3 duration

				void calculateRhythms(int minrhy);
				void calculatePitches(int tonic);
				bool parseNote(const std::string& note, HumNum factor);
				void generateHumdrum(int minrhy, int b40tonic);
				bool isPitch(void);
				bool isRest(void);
				std::string getScaleDegree(void);
		};

		class Measure : public std::vector<Tool_esac2hum::Note> {
			public:
				std::vector<std::string> m_errors;
				std::string esac;
				int m_barnum = -1000; // -1000 == unassigned bar number for this measure
				// m_barnum = -1 == invisible barline (between two partial measures)
				// m_barnum =  0 == pickup measure (partial measure at start of music)
				double m_ticks = 0.0;
				double m_tsticks = 0.0;
				// m_measureTimeSignature is a **kern time signature
				// (change) to display in the converted score.
				std::string m_measureTimeSignature = "";
				bool m_partialBegin = false;  // start of an incomplete measure
				bool m_partialEnd = false;    // end of an incomplete measure (pickup)
				bool m_complete = false;      // a complste measure
				void calculateRhythms(int minrhy);
				void calculatePitches(int tonic);
				bool parseMeasure(const std::string& measure);
				bool isUnassigned(void);
				void setComplete(void);
				bool isComplete(void);
				void setPartialBegin(void);
				bool isPartialBegin(void);
				void setPartialEnd(void);
				bool isPartialEnd(void);
		};

		class Phrase : public std::vector<Tool_esac2hum::Measure> {
			public:
				std::vector<std::string> m_errors;
				double m_ticks = 0.0;
				std::string esac;
				void calculateRhythms(int minrhy);
				void calculatePitches(int tonic);
				bool parsePhrase(const std::string& phrase);
				std::string getLastScaleDegree();
				void getNoteList(std::vector<Tool_esac2hum::Note*>& notelist);
				std::string getNO_REP(void);
				int getFullMeasureCount(void);
		};

		class Score : public std::vector<Tool_esac2hum::Phrase> {
			public:
				int m_b40tonic = 0;
				int m_minrhy   = 0;
				std::string m_clef;
				std::string m_keysignature;
				std::string m_keydesignation;
				std::string m_timesig;
				std::map<std::string, std::string> m_params;
				std::vector<std::string> m_errors;
				bool m_finalBarline = false;
				bool hasFinalBarline(void) { return m_finalBarline; }
				void calculateRhythms(int minrhy);
				void calculatePitches(int tonic);
				bool parseMel(const std::string& mel);
				void analyzeTies(void);
				void analyzePhrases(void);
				void getNoteList(std::vector<Tool_esac2hum::Note*>& notelist);
				void getMeasureList(std::vector<Tool_esac2hum::Measure*>& measurelist);
				void getPhraseNoteList(std::vector<Tool_esac2hum::Note*>& notelist, int index);
				void generateHumdrumNotes(void);
				void calculateClef(void);
				void calculateKeyInformation(void);
				void calculateTimeSignatures(void);
				void setAllTimesigTicks(double ticks);
				void assignFreeMeasureNumbers(void);
				void assignSingleMeasureNumbers(void);
				void prepareMultipleTimeSignatures(const std::string& ts);

				void doAnalyses(void);
				void analyzeMEL_SEM(void);
				void analyzeMEL_RAW(void);
				void analyzeNO_REP(void);
				void analyzeRTM(void);
				void analyzeSCL_DEG(void);
				void analyzeSCL_SEM(void);
				void analyzePHR_NO(void);
				void analyzePHR_BARS(void);
				void analyzePHR_CAD(void);
				void analyzeACC(void);
		};

		            Tool_esac2hum    (void);
		           ~Tool_esac2hum    () {};

		bool       convertFile          (std::ostream& out, const std::string& filename);
		bool       convert              (std::ostream& out, const std::string& input);
		bool       convert              (std::ostream& out, std::istream& input);


	protected:
		void        initialize          (void);

		void        convertEsacToHumdrum(std::ostream& output, std::istream& infile);
		bool        getSong             (std::vector<std::string>& song, std::istream& infile);
		void        convertSong         (std::ostream& output, std::vector<std::string>& infile);
		static std::string trimSpaces   (const std::string& input);
		void        printHeader         (std::ostream& output);
		void        printFooter         (std::ostream& output, std::vector<std::string>& infile);
		void        printConversionDate (std::ostream& output);
		void        printPdfLinks       (std::ostream& output);
		void        printParameters     (void);
		void        printPageNumbers    (std::ostream& output);
		void        getParameters       (std::vector<std::string>& infile);
		void        cleanText           (std::string& buffer);
		std::string createFilename      (void);
		void        printBemComment     (std::ostream& output);
		void        processSong         (void);
		void        printScoreContents  (std::ostream& output);
		void        embedAnalyses       (std::ostream& output);
		void        printPdfUrl         (std::ostream& output);
		std::string getKolbergUrl       (int volume);
      void        printKolbergPdfUrl  (std::ostream& output);

	private:
		bool        m_debugQ     = false;  // used with --debug option
		bool        m_verboseQ   = false;  // used with --verbose option
		std::string m_verbose;             //    p = print EsAC phrases, m = print measures, n = print notes.
		                                   //    t after p, m, or n means print tick info
		bool        m_embedEsacQ = true;   // used with -E option
		bool        m_dwokQ      = false;  // true if source is Oskar Kolberg: Dzieła Wszystkie
		                                   // (Oskar Kolberg: Complete Works)
		                                   // determined automatically if header line or TRD source contains "DWOK" string.
		bool        m_analysisQ  = false;  // used with -a option

		int         m_inputline = 0;       // used to keep track if the EsAC input line.

		std::string m_filePrefix;
		std::string m_filePostfix = ".krn";
		bool m_fileTitleQ = false;

		std::string m_prevline;
		std::string m_cutline;
		std::vector<std::string> m_globalComments;

		int m_minrhy = 0;

		Tool_esac2hum::Score m_score;

		class KolbergInfo {
			public:
				std::string titlePL;
				std::string titleEN;
				int firstPrintPage;
				int firstScanPage;
				std::vector<int> plates;

				KolbergInfo(void) { firstPrintPage = 0; firstScanPage = 0; }
				KolbergInfo(
					const std::string& pl, const std::string& en, int fpp, int fsp, const std::vector<int>& plts)
        				: titlePL(pl), titleEN(en), firstPrintPage(fpp), firstScanPage(fsp), plates(plts) {}
		};
		std::map<int, KolbergInfo> m_kinfo;
		KolbergInfo getKolbergInfo(int volume);
		std::string getKolbergUrl(int volume, int printPage);
		int calculateScanPage(int inputPrintPage, int printPage, int scanPage, const std::vector<int>& platePages);


};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_ESAC2HUM_H */



