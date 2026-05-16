//
// Programmer:    Alexander Morgan <alexanderpmorgan@gmail.com>
// Creation Date: Sat May 16 18:00:00 CEST 2026
// Last Modified: Sat May 16 19:00:00 CEST 2026
// Filename:      tool-extremis.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-extremis.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract a synthetic spine of the lowest and/or highest
//                sounding pitches in a score.
//

#ifndef _TOOL_EXTREMIS_H
#define _TOOL_EXTREMIS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_extremis : public HumTool {
	public:
		            Tool_extremis    (void);
		           ~Tool_extremis    () {};

		bool        run              (HumdrumFileSet& infiles);
		bool        run              (HumdrumFile& infile);
		bool        run              (const std::string& indata, std::ostream& out);
		bool        run              (HumdrumFile& infile, std::ostream& out);

	protected:

		// SynthEvent describes a single note (or rest) of the synthetic
		// extreme-pitch spine.  Each event spans a range of input data
		// lines that share the same extreme pitch, and which together
		// represent a single sustained occurrence of that pitch (i.e.,
		// without rearticulation by the source voice or a crossing of
		// a barline).
		struct SynthEvent {
			int    startLine = 0; // first input data line of the event
			int    endLine   = 0; // last input data line (inclusive)
			HumNum duration  = 0; // total duration of the event (in quarter notes)
			int    b40       = 0; // base-40 pitch (0 means rest)
			bool   isRest    = true;
		};

		void        initialize          (void);
		void        processFile         (HumdrumFile& infile);

		void        buildEvents         (HumdrumFile& infile,
		                                 std::vector<SynthEvent>& events,
		                                 bool wantHigh);
		void        computePitchAndAttack(HumdrumLine& line, bool wantHigh,
		                                 int& outB40, bool& outAttack);

		void        renderEvents        (HumdrumFile& infile,
		                                 const std::vector<SynthEvent>& events,
		                                 std::vector<std::string>& lineTokens);
		void        chunkEvent          (HumdrumFile& infile,
		                                 const SynthEvent& event,
		                                 std::vector<int>& chunkStartLines,
		                                 std::vector<HumNum>& chunkDurations);
		bool        isCleanKernDuration (HumNum dur);
		bool        lineHasMatchingBeamMarker(HumdrumFile& infile, int line,
		                                 int b40);
		std::string getSourceMarkers    (HumdrumFile& infile, int line,
		                                 int b40, std::string& outPitch);
		std::string stripRhythmAndTies  (const std::string& subtoken);

		std::string getBarlineToken     (HumdrumLine& line);
		std::string getKeySignature     (HumdrumLine& line);
		std::string getTimeSignature    (HumdrumLine& line);
		std::string getMeterSymbol      (HumdrumLine& line);
		std::string getKeyDesignation   (HumdrumLine& line);
		HTp         getFirstKernToken   (HumdrumLine& line);

	private:
		bool        m_lowQ   = true;   // -l option (default behavior)
		bool        m_highQ  = false;  // -h option
		bool        m_bothQ  = false;  // -b option

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_EXTREMIS_H */



