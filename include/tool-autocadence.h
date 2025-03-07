//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 16 13:21:38 PST 2025
// Last Modified: Sun Mar  2 11:57:48 PST 2025
// Filename:      tool-autocadence.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-autocadence.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Automatic detection of cadences in Renaissance music.
//

#ifndef _TOOL_AUTOCADENCE_H
#define _TOOL_AUTOCADENCE_H

#include "HumTool.h"
#include "HumdrumFile.h"

// #include <algorithm>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace hum {

// START_MERGE

class Tool_autocadence : public HumTool {
	class CadenceDefinition {
		public:
			std::string m_funcL;
			std::string m_funcU;
			std::string m_name;
			std::string m_regex;
			void setDefinition(const std::string& funcL, const std::string& funcU,
					const std::string& name, const std::string& regex) {
				m_funcL = funcL;
				m_funcU = funcU;
				m_name = name;
				m_regex = regex;
				// int count = std::count(text.begin(), text.end(), target);
			}
	};

	public:
		            Tool_autocadence           (void);
		           ~Tool_autocadence           () {};

		bool        run                 (HumdrumFileSet& infiles);
		bool        run                 (HumdrumFile& infile);
		bool        run                 (const std::string& indata, std::ostream& out);
		bool        run                 (HumdrumFile& infile, std::ostream& out);
		void        initialize          (void);

	protected:
		void        processFile         (HumdrumFile& infile);
		void        generateCounterpointStrings(std::vector<HTp>& kspines, int indexL, int indexU);
		std::string generateCounterpointString(std::vector<std::vector<HTp>>& pairings, int index);
      void        printModules        (std::vector<std::string>& modules, int lowerPart, int upperPart);
		void        fillNotes           (std::vector<HTp>& voice, HTp exinterp);
		std::string getDiatonicIntervalString  (int lower, int upper);
		int         getDiatonicInterval        (int lower, int upper);
		void        prepareCadenceDefinitions  (void);
		void        addCadenceDefinition       (const std::string& funcL, const std::string& funcU,
		                                        const std::string& name, const std::string& regex);
		void        prepareLowestPitches       (void);
		void        preparePitchInfo           (HumdrumFile& infile);
		void        prepareDiatonicPitches     (HumdrumFile& infile);
		void        printExtractedPitchInfo    (HumdrumFile& infile);
		void        printExtractedIntervalInfo (HumdrumFile& infile);
		void        prepareIntervalInfo        (HumdrumFile& infile);
		int         getPairIndex               (HTp lowerExInterp, HTp upperExInterp);
		void        prepareTrackToVoiceIndex   (HumdrumFile& infile, std::vector<HTp> kspines);
		void        printIntervalManipulatorLine(HumdrumFile& infile, int index, int kcount);
		void        printIntervalDataLine      (HumdrumFile& infile, int index, int kcount);
		void        printIntervalDataLineScore (HumdrumFile& infile, int index, int kcount);
		void        printIntervalLine          (HumdrumFile& infile, int index, int kcount, const std::string& tok);
		void        prepareAbbreviations       (HumdrumFile& infile);
		void        prepareIntervalSequences   (HumdrumFile& infile);
		void        prepareSinglePairSequences (HumdrumFile& infile, int vindex, int pindex);
		std::string generateSequenceString     (HumdrumFile& infile, int lindex, int vindex, int pindex);
		void        printSequenceInfo          (void);
		void        printSequenceMatches       (void);
		void        printSequenceMatches2      (void);
		void        searchIntervalSequences    (void);
		void        printScore                 (HumdrumFile& infile);
		void        printMatchCount            (void);
		void        markupScore                (HumdrumFile& infile);
		void        addMatchToScore            (HumdrumFile& infile, int matchIndex);
		int         getRegexSliceCount         (const std::string& regex);
		void        colorNotes                 (HTp startTok, HTp endTok);
		void        prepareDefinitionList      (std::set<int>& list);
		void        printRegexTable            (void);
		void        printDefinitionRow         (int index);
		void        prepareCvfNames            (void);
		std::string getFunctionNames           (const std::string& input);
		bool        getCadenceEndSliceNotes    (HTp& endL, HTp& endU, int count, HumdrumFile& infile,
		                                        int lindex, int vindex, int pindex);

	private:

		// m_definitions: A list of the cadence regular expression definitions.
		std::vector<Tool_autocadence::CadenceDefinition> m_definitions;

		// m_pitches: A list of the diatonic pitches for the score, organized
		// in a 2-D array that matches the line/field number of the notes.
		// Middle C is 28, rests are 0, and negative values are sustained
		// pitches.
		std::vector<std::vector<int>> m_pitches;

		// m_lowestPitch: the lowest sounding pitch at every instance in the score.
		// the pitch is stored as an absolute diatonic pitch, middle C is 28, 0 is a rest
		std::vector<int> m_lowestPitch;

		// m_intervals: The counterpoint intervals for each pair of notes.
		// The data is store in a 3-D vector, where the first dimension is the
		// line in the score, the second dimension is the voice (kern spine) index in
		// the score for the lower voice.  For example, if the score has four **kern
		// spines, the second dimension size will be four, and the third dimensions
		// will be 3, 2, 1 and 0.  
		// Dimensions:
		//     0: line index in the Humdrum data
		//     1: voice index of the lower voice
		//     2: pairing interval of two voice generating the interval
		// Parameters for tuple:
		//     0: interval string for the note pair (or empty if no interval.
		//     1: the token for the lower voice note
		//     2: the token for the upper voice note
		std::vector<std::vector<std::vector<std::tuple<std::string, HTp, HTp>>>> m_intervals;

		// m_sequences: The counterpoint interval strings which are searched
		// with the regular expressions in m_definitions.
		// Dimensions:
		//    0: voice index
		//    1: voice pair index
		//    2: an array of tuples, only one which is used.  zero length means no sequence.
		// Parameters for tuple:
		//    0: sequence to search
		//    1: first note of sequence in score (lower voice)
		//    2: first note of sequence in score (upper voice)
		//    3: vector of ints listing matches where int is the m_definitions index(es) that match.
		std::vector<std::vector<std::vector<std::tuple<std::string, HTp, HTp, std::vector<int>>>>> m_sequences;

		// m_matches: List of sequences that match to a cadence formula.  The list contains
		// the vindex, pindex, and nth sequence in m_sequences.
		std::vector<std::vector<int>> m_matches;

		// m_trackToVoiceIndex: Mapping from track to voice index (starting with 0)
		// for the lowest voice).
		std::vector<int> m_trackToVoiceIndex;

		// m_abbr: voice abbreviations to prefix interval data in score for voices.
		// Indexed by voice (lowest = 0).
		std::vector<std::string> m_abbr;

		// m_functionNames: mapping from CVF abbreviation to full name of CFV.
		std::map<std::string, std::string> m_functionNames;

		// options:
		bool m_intervalsQ               = false; // -i: show counterpoint interval infomation
		bool m_intervalsOnlyQ           = false; // -I: show counterpoint interval infomation but
		                                         // do not do cadence analysis
		bool m_printRawDiatonicPitchesQ = false; // -p: display m_pitches after filling
		int  m_sequenceLength           = 7;     // maximum regex interval sequence length
		bool m_matchesQ                 = false; // -m: display sequences that match to cadence formula(s)
		bool m_printSequenceInfoQ       = false; // -s: print list of interval sequences
		bool m_countQ                   = false; // --count: print number of cadences found
		bool m_colorQ                   = false; // -c: color matched cadence formula
		std::string m_color      = "dodgerblue"; // --color "string" to set matched notes to a specific color
		bool m_showFormulaIndexQ        = false; // -f: show formulation index after CVF label
		bool m_evenNoteSpacingQ         = false; // -e: compress notation (verovio option evenNoteSpacing)
		bool m_regexQ                   = false; // -r: show table of matched regular expressions
		bool m_popupQ                   = true;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_AUTOCADENCE_H */



