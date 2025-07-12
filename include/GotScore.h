//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon May  5 06:31:29 PDT 2025
// Last Modified: Wed Jul  9 08:20:16 CEST 2025
// Filename:      GotScore.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/GotScore.h
// Syntax:        C++17; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert from spreadsheet (TSV) GOT data to **gotr/**gotp
//                or **kern data.
//

#ifndef _GOTSCORE_H_INCLUDED
#define _GOTSCORE_H_INCLUDED

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE


class GotScore {

	public:
		struct EventAtTime {
			double timestamp;
			std::vector<std::string> rhythms;
			std::vector<std::string> pitches;
		};

		class Measure {
			public:
				Measure(void) {};
				~Measure() {};
				std::ostream& print(std::ostream& out);

				void printKernBarline(std::ostream& out, bool textQ);

				// Each rhythm+pitch pairing with computed start time and duration
				class TimedEvent {
					public:
						double timestamp;
						double duration;
						std::string rhythm;
						std::string pitch;
						bool isInterpretation = false;  // true if rhythm starts with '*'
				};

			public:
				// m_parent: A pointer to the GotScore object that the measure belongs to.
				GotScore* m_owner = NULL;

				// m_barnum: The measure number for the measure.
				std::string m_barnum;

				// m_text: the text content for the measure.
				std::string m_text;

				// m_error: Any parsing error message when converting to **kern
				std::vector<std::string> m_error;

				// m_rhythms: First dimension is voice (highest to lowest)
				// Second dimension is rhythm "word" first to last in measure
				std::vector<std::vector<std::string>> m_rhythms;

				// m_pitches: First dimension is voice (highest to lowest)
				// Second dimension is pitch "word" first to last in measure
				std::vector<std::vector<std::string>> m_pitches;

				// m_splitRhythms: First dimension is voice (highest to lowest)
				// Second dimension is rhythm "word" first to last in measure
				// Third dimension is rhythm "token" first to last in word
				std::vector<std::vector<std::vector<std::string>>> m_splitRhythms;

				// m_splitPitches: First dimension is voice (highest to lowest)
				// Second dimension is pitch "word" first to last in measure
				// Third dimension is pitch "token" first to last in word
				std::vector<std::vector<std::vector<std::string>>> m_splitPitches;

				// m_kerns: First dimension is the voice (highest to lowest),
				// second dimension is flattening of the 2nd and 3rd dimensions
				// of m_splitPitches.
				std::vector<std::vector<std::string*>> m_kerns;

				// m_diatonic: linearized pitch by voice for marking editorial accidentals.
				std::vector<std::vector<int>> m_diatonic;

				// m_accid: chromatic alterations of m_diatonic pitches.
				std::vector<std::vector<int>> m_accid;

				// m_accidState: The diatonic accidental state at the end of the
				// measure.  This is used to create editorial accidentals and 
				// cautionary natural accidentals.  First dimension is voice number,
				// second dimension is CDEFGAB accidental states, 0 = natural 1=sharp, -1=flat.
				std::vector<std::vector<int>> m_accidState;

				// m_voiceEvents: events per voice
				std::vector<std::vector<TimedEvent>> m_voiceEvents;

		};

	public:
		         GotScore        (void);
		         GotScore        (std::stringstream& ss);
		         GotScore        (const std::string& s);
		        ~GotScore        ();

		void     loadLines       (std::stringstream& ss);
		void     loadLines       (const std::string& s);
		bool     prepareMeasures (std::ostream& out);
		void     clear           (void);

		std::ostream& printInputFile  (std::ostream& out);
		std::ostream& printCells      (std::ostream& out);
		std::ostream& printMeasures   (std::ostream& out);

		std::string getGotHumdrum         (void);
		std::string getGotHumdrumMeasure  (GotScore::Measure& mdata);

		std::string getKernHumdrum        (void);
		std::string getKernHumdrumMeasure (GotScore::Measure& mdata);

		void        processUnderscoreTies(std::vector<std::string*>& pitches);
		void        processRhythmTies(std::vector<std::string*>& rhythms, std::vector<std::string*>& pitches);
		std::string mergeRhythmAndPitchIntoNote(const std::string& r, const std::string& p);

		std::string getHeaderInfo(int index);
		std::string getFooterInfo(void);
		void        getDiatonicAccid(const std::string& pitch, int& d, int& a);
		void        prepareAccidentals(void);
		void        markEditorialAccidentals(GotScore::Measure& measure, int voice);
		void        checkForCautionaryAccidentals(int mindex, int vindex);
		void        cleanRhythmValues(std::vector<std::vector<std::string>>& rhythms);

		void        setNoEditorial(void);
		void        setCautionary(void);
		void        setNoForcedAccidentals(void);

	protected:
		void     parse                    (void);
		void     prepareCells             (void);
		bool     processSystemMeasures    (int barIndex, int system, std::ostream& out);
		void     splitMeasureTokens       (void);
		void     splitMeasureTokens       (GotScore::Measure& mdata);
		void     pairLeadingDots          (void);
		void     processDotsForMeasure    (GotScore::Measure& mdata);
		void     processPitchDotsByVoice  (std::vector<std::string*>& pitches);
		void     processDotTiedNotes      (void);
		void     buildVoiceEvents         (void);
		double   durationFromRhythmToken  (const std::string& token);
		std::vector<GotScore::EventAtTime> alignEventsByTimestamp(const GotScore::Measure& mdata);
		std::vector<std::string> convertGotToKernPitches (std::vector<std::string>& gotpitch);
		std::string convertGotToKernPitch (const std::string& gotpitch);
		void     storePitchHistograms     (std::vector<std::vector<std::string*>>& P);
		std::vector<std::string> generateVoiceClefs(void);
		std::string chooseClef(double mean, double min, double max);

		static int kernToBase40PC       (const std::string& kerndata);
		static int kernToAccidentalCount(const std::string& kerndata);
		static int kernToBase12PC       (const std::string& kerndata);
		static int kernToOctaveNumber   (const std::string& kerndata);
		static int kernToMidiNoteNumber (const std::string& kerndata);
		static int kernToDiatonicPC     (const std::string& kerndata);

		std::vector<std::string> tokenizeRhythmString (const std::string& input);
		std::vector<std::string> tokenizePitchString  (const std::string& input);
		std::vector<std::string> splitBySpaces        (const std::string& input);

	private:
		// m_voices == number of voices in score
		int m_voices = 0;

		// m_pitch_hist == used to calculate the clef for each voice.
		std::vector<std::vector<int>> m_pitch_hist;

		// m_textQ == true if input data has lyric text.
		bool m_textQ = false;

		// m_lines == input text lines, with whitespace removed from ends of lines.
		std::vector<std::string> m_lines;

		// m_cells == m_lines split by tab characters (i.e., TSV data):
		std::vector<std::vector<std::string>> m_cells;

		// m_measures == data organized by measure.
		std::vector<GotScore::Measure> m_measures;

		// m_debugQ == for printing debug statements
		bool m_debugQ = false;

		// m_error == for reporting errors.
		std::stringstream m_error;

		// m_got == for storing **got conversion.
		std::string m_got;

		// m_cautionary == add !!!RDF**kern: i = editorial accidental, paren
		bool m_cautionary;

		// m_kern == for storing **kern conversion.
		std::string m_kern;

		// conversion options:
		bool m_no_editorialQ = false;
		bool m_cautionaryQ   = false;
		bool m_modern_accQ   = false;

};


// END_MERGE

} // end namespace hum

#endif /* _HUMDRUMTOKEN_H_INCLUDED */



