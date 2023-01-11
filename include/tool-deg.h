//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  6 12:59:44 PST 2023
// Last Modified: Mon Jan  9 09:50:50 PST 2023
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

namespace hum {

// START_MERGE

class Tool_deg : public HumTool {

	///////////////////////////////////////////////////////////////////
	//
	// Tool_deg::ScaleDegree --
	//

	public:
		class ScaleDegree;
		class ScaleDegree {
			public:
				ScaleDegree (void);
				~ScaleDegree ();

				void            setLinkedKernToken       (hum::HTp token, const std::string& mode, int b40tonic, bool unpitched = false);
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
				static void     setShowTies              (bool state);

			protected:
				std::string     generateDegDataToken     (void) const;
				std::string     generateDegDataSubtoken  (int index) const;
				void            setMinorMode             (int b40tonic);
				void            setMajorMode             (int b40tonic);
				void            analyzeTokenScaleDegrees (void);

			private:
				// m_token: token in **kern data that links to this scale degree
				HTp m_linkedKernToken = NULL;

				// m_unpitched: true if unpitched (because in a percussion part)
				bool m_unpitched = false;

			   // m_mode: the mode of the current key	(0 = none, 1 = major, 2 = minor)
				int m_mode = 0;
				const int m_unknown_mode = 0;
				const int m_major_mode   = 1;
				const int m_minor_mode   = 2;

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

				// Rendering options:
				static bool m_showTiesQ;
		};


	/////////////////////////////////////////////////////////////////////////
	//
	// Tool_deg --
	//

	public:
		      Tool_deg         (void);
		     ~Tool_deg         () {};

		bool  run              (HumdrumFileSet& infiles);
		bool  run              (HumdrumFile& infile);
		bool  run              (const std::string& indata, std::ostream& out);
		bool  run              (HumdrumFile& infile, std::ostream& out);

	protected:
		void            processFile              (HumdrumFile& infile);
		void            initialize               (void);

		void            prepareDegSpine          (vector<vector<ScaleDegree>>& degspine, HTp kernstart, HumdrumFile& infil);
		void            printDegScore            (void);
		void            printDegScoreInterleavedWithInputScore(HumdrumFile& infile);
		std::string     createOutputHumdrumLine(HumdrumFile& infile, std::vector<int> insertTracks, int lineIndex);

	private:

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

		bool m_degOnlyQ = false;  // used with -I option
		bool m_degTiesQ = false;  // used with -t option

};

std::ostream& operator<<(std::ostream& out, Tool_deg::ScaleDegree& degree);
std::ostream& operator<<(std::ostream& out, Tool_deg::ScaleDegree* degree);


// END_MERGE

} // end namespace hum

#endif /* _TOOL_DEG_H_INCLUDED */



