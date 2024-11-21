//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  6 12:59:44 PST 2023
// Last Modified: Sun Jan 15 20:25:11 PST 2023
// Filename:      tool-deg.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-deg.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for deg(x) tool, which analyzes scale degrees.
//
// Options:       -I: do not interleave input data with output **deg spines.
//                -t: include scale degrees for tied notes.
//

#ifndef _TOOL_DEG_H_INCLUDED
#define _TOOL_DEG_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_deg : public HumTool {

	///////////////////////////////////////////////////////////////////
	//
	// Tool_deg::ScaleDegree --
	//

	public: // Tool_deg class
		class ScaleDegree;
		class ScaleDegree {
			public:  // ScaleDegree class
				ScaleDegree (void);
				~ScaleDegree ();

				void            setLinkedKernToken       (hum::HTp token, const std::string& mode, int b40tonic, bool unpitched = false, bool resolveNull = false);
				hum::HTp        getLinkedKernToken       (void) const;
				std::string     getDegToken              (void) const;

				hum::HumNum     getTimestamp             (void) const;
				hum::HumNum     getDuration              (void) const;
				hum::HumNum     getTiedDuration          (void) const;
				bool            hasSpines                (void) const;
				bool            isBarline                (void) const;
				std::string     getBarline               (void) const;
				bool            isExclusiveInterpretation(void) const;
				bool            isManipulator            (void) const;
				std::string     getManipulator           (void) const;
				bool            isInterpretation         (void) const;
				bool            isKeyDesignation         (void) const;
				bool            isLocalComment           (void) const;
				bool            isGlobalComment          (void) const;
				bool            isReferenceRecord        (void) const;
				bool            isUnpitched              (void) const;
				bool            isDataToken              (void) const;
				bool            isNullDataToken          (void) const;
				bool            isNonNullDataToken       (void) const;
				bool            isInMajorMode            (void) const;
				bool            isInMinorMode            (void) const;
				int             getBase40Tonic           (void) const;
				int             getSubtokenCount         (void) const;

				// output options:
				static void     setShowTies    (bool state) { m_showTiesQ = state;  }
				static void     setShowZeros   (bool state) { m_showZerosQ = state; }
				static void     setShowOctaves (bool state) { m_octaveQ = state; }
				static void     setForcedKey   (const std::string& key) { m_forcedKey = key; }

			protected:  // ScaleDegree class
				std::string     generateDegDataToken     (void) const;
				std::string     generateDegDataSubtoken  (int index) const;
				void            analyzeTokenScaleDegrees (void);

				void            setMajorMode             (int b40tonic);
				void            setMinorMode             (int b40tonic);
				void            setDorianMode            (int b40tonic);
				void            setPhrygianMode          (int b40tonic);
				void            setLydianMode            (int b40tonic);
				void            setMixolydianMode        (int b40tonic);
				void            setAeoleanMode           (int b40tonic);
				void            setLocrianMode           (int b40tonic);
				void            setIonianMode            (int b40tonic);

			private:  // ScaleDegree class
				// m_token: token in **kern data that links to this scale degree
				HTp m_linkedKernToken = NULL;

				// m_unpitched: true if unpitched (because in a percussion part)
				bool m_unpitched = false;

				// m_mode: the mode of the current key	(0 = none, 1 = major, 2 = minor)
				//
				// modal keys:
				// 3 = dorian (such as *c:dor)
				// 4 = phrygian (such as *c:phr)
				// 5 = lydian (such as *C:lyd)
				// 6 = mixolydian (such as *C:mix)
				// 7 = aeolean (such as *c:aeo)
				// 8 = locrian (such as *c:loc)
				// 9 = ionian (such as *C:ion)
				//
				int m_mode = 0;
				const int m_unknown_mode = 0;
				const int m_major_mode   = 1;
				const int m_minor_mode   = 2;
				const int m_dor_mode     = 3;
				const int m_phr_mode     = 4;
				const int m_lyd_mode     = 5;
				const int m_mix_mode     = 6;
				const int m_aeo_mode     = 7;
				const int m_loc_mode     = 8;
				const int m_ion_mode     = 9;

				// m_b40tonic: the tonic pitch of the key expressed as base-40
				int m_b40tonic = 0;

				// m_subtokens: Subtokens (of a chord)
				std::vector<std::string> m_subtokens;

				// m_degress: integer for scale degree (by subtoken)
				// 0 = rest; otherwise 1-7
				std::vector<int> m_degrees;

				// m_alters: chromatic alterations for scale degree
				std::vector<int> m_alters;

				// m_octaves: the octave number of the note
				// -1 = rest
				// 0-9 pitch octave (4 = middle C octave)
				std::vector<int> m_octaves;

				// Pointers to the next previous note (or chord) with which
				// to calculate a melodic contour approach or departure.
				ScaleDegree* m_prevNote = NULL;
				ScaleDegree* m_nextNote = NULL;

				// Pointers to the next/previous rest if there is a rest
				// between this note/chord and the next/previous note.
				// The pointer starts to the first rest if there is more
				// than one rest between the two notes.
				ScaleDegree* m_prevRest = NULL;
				ScaleDegree* m_nextRest = NULL;

				// ScaleDegree rendering options:
				static bool m_showTiesQ;
				static bool m_showZerosQ;
				static bool m_octaveQ;
				static std::string m_forcedKey;
		};


	/////////////////////////////////////////////////////////////////////////
	//
	// Tool_deg --
	//

	public:  // Tool_deg class
		      Tool_deg         (void);
		     ~Tool_deg         () {};

		bool  run              (HumdrumFileSet& infiles);
		bool  run              (HumdrumFile& infile);
		bool  run              (const std::string& indata, std::ostream& out);
		bool  run              (HumdrumFile& infile, std::ostream& out);

	protected: // Tool_deg class
		void            processFile              (HumdrumFile& infile);
		void            initialize               (void);

		bool            setupSpineInfo           (HumdrumFile& infile);
		void            prepareDegSpine          (std::vector<std::vector<ScaleDegree>>& degspine, HTp kernstart, HumdrumFile& infil);
		void            printDegScore            (HumdrumFile& infile);
		void            printDegScoreInterleavedWithInputScore(HumdrumFile& infile);
		std::string     createOutputHumdrumLine  (HumdrumFile& infile, int lineIndex);
      std::string     prepareMergerLine        (std::vector<std::vector<std::string>>& merge);
		void            calculateManipulatorOutputForSpine(std::vector<std::string>& lineout, std::vector<std::string>& linein);
		std::string     createRecipInterpretation(const std::string& starttok, int refLine);
		std::string     createDegInterpretation  (const std::string& degtok, int refLine, bool addPreSpine);
		std::string     printDegInterpretation   (const std::string& interp, HumdrumFile& infile, int lineIndex);
		void            getModeAndTonic          (std::string& mode, int& b40tonic, const std::string& token);

		bool            isDegAboveLine           (HumdrumFile& infile, int lineIndex);
		bool            isDegArrowLine           (HumdrumFile& infile, int lineIndex);
		bool            isDegBoxLine             (HumdrumFile& infile, int lineIndex);
		bool            isDegCircleLine          (HumdrumFile& infile, int lineIndex);
		bool            isDegColorLine           (HumdrumFile& infile, int lineIndex);
		bool            isDegHatLine             (HumdrumFile& infile, int lineIndex);
		bool            isDegSolfegeLine         (HumdrumFile& infile, int lineIndex);
		bool            isKeyDesignationLine     (HumdrumFile& infile, int lineIndex);

		void            checkAboveStatus         (std::string& value, bool arrowStatus);
		void            checkArrowStatus         (std::string& value, bool arrowStatus);
		void            checkBoxStatus           (std::string& value, bool arrowStatus);
		void            checkCircleStatus        (std::string& value, bool arrowStatus);
		void            checkColorStatus         (std::string& value, bool arrowStatus);
		void            checkHatStatus           (std::string& value, bool arrowStatus);
		void            checkSolfegeStatus       (std::string& value, bool arrowStatus);

		void            checkKeyDesignationStatus(std::string& value, int keyDesignationStatus);

	private: // Tool_deg class

		// m_degSpine: A three-dimensional list of **deg output spines.
		// This is a scratch pad to create **deg data for the input **kern
		//    spines.
		// First dimension is **kern spine enumeration in the input data,
		//    from left-to-right.
		// Second dimension is for the line in the Humdrum file, from top
		//    to bottom.
		// Third dimension is for the subspines (not subtokens, which are
		//    handled by Tool_deg::ScaleDegree class).
		std::vector<std::vector<std::vector<ScaleDegree>>> m_degSpines;

		// m_kernSpines: list of all **kern spines found in file.
		std::vector<HTp> m_kernSpines;

		// m_selectedKernSpines: list of only the **kern spines that will be analyzed.
		std::vector<HTp> m_selectedKernSpines;

		// m_degInsertTrack: the track number in the input file that an
		//	output **deg spine should be inserted before.  A track of -1 means
		// append the **deg spine after the last input spine.
		std::vector<int> m_degInsertTrack;

		// m_insertTracks: matches to m_degSpines first dimension.
		// It gives the track number for spines before which the corresponding
		// m_degSpine[x] spine should be inserted.  A -1 value at the last
		// position in m_insertTracks means append the **deg spine at the
		// end of the line.
		std::vector<int> m_insertTracks;

		bool m_aboveQ          = false;   // used with --above option
		bool m_arrowQ          = false;   // used with --arrow option
		bool m_boxQ            = false;   // used with --box option
		bool m_circleQ         = false;   // used with --circle option
		bool m_hatQ            = false;   // used with --hat option
		bool m_colorQ          = false;   // used with --color option
		std::string  m_color;             // used with --color option
		bool m_solfegeQ        = false;   // used with --solfege option

		bool m_degOnlyQ        = false;   // used with -I option
		bool m_recipQ          = false;   // used with -r option
		bool m_kernQ           = false;   // used with --kern option
		bool m_degTiesQ        = false;   // used with -t option
		bool m_resolveNullQ    = false;   // used with --resolve-null option
		bool m_forceKeyQ       = false;   // used with -K option

		std::string m_defaultKey  = "";    // used with --default-key option
		std::string m_forcedKey   = "";    // used with --forced-key option
		std::string m_kernSuffix  = "dR/"; // used with --kern option (currently hardwired)
		std::string m_spineTracks = "";    // used with -s option
		std::string m_kernTracks  = "";    // used with -k option

		std::vector<bool> m_processTrack;  // used with -k and -s option

		class InterleavedPrintVariables {
			public:
				bool foundData;
				bool hasDegSpines;
				bool foundAboveLine;
				bool foundArrowLine;
				bool foundBoxLine;
				bool foundCircleLine;
				bool foundColorLine;
				bool foundHatLine;
				bool foundKeyDesignationLine;
				bool foundSolfegeLine;

				InterleavedPrintVariables(void) { clear(); }
				void clear(void) {
					foundData       = false;
					hasDegSpines    = true;
					foundAboveLine  = false;
					foundArrowLine  = false;
					foundBoxLine    = false;
					foundCircleLine = false;
					foundColorLine  = false;
					foundHatLine    = false;
					foundKeyDesignationLine = false;
					foundSolfegeLine = false;
				}
		};
		InterleavedPrintVariables m_ipv;

};

std::ostream& operator<<(std::ostream& out, Tool_deg::ScaleDegree& degree);
std::ostream& operator<<(std::ostream& out, Tool_deg::ScaleDegree* degree);


// END_MERGE

} // end namespace hum

#endif /* _TOOL_DEG_H_INCLUDED */



