//
// Programmer:    Alexander Morgan
// Creation Date: Fri Oct 16 2026
// Filename:      tool-metpos.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Append a **metpos spine giving the position of each data
//                line in the metric hierarchy (1 for the start of the
//                measure, larger numbers for weaker positions).  Based on
//                the Humdrum Toolkit metpos command by David Huron, but
//                calculated from note durations, so the input does not need
//                to be processed by timebase first.  Measures longer than
//                their time signature use the length from the barlines and
//                the organization from the mensuration sign, and music
//                without meter is measured from the (possibly hidden)
//                barlines with a semibreve pulse.
//

#ifndef _TOOL_METPOS_H
#define _TOOL_METPOS_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace hum {

// START_MERGE

class Tool_metpos : public HumTool {
	public:
		         Tool_metpos         (void);
		        ~Tool_metpos         () {};

		bool     run                 (HumdrumFileSet& infiles);
		bool     run                 (HumdrumFile& infile);
		bool     run                 (const std::string& indata, std::ostream& out);
		bool     run                 (HumdrumFile& infile, std::ostream& out);

		int      getMetricPosition   (HumNum position, HumNum duration = 0);
		bool     setMeter            (const std::string& meter, int lineNumber = 0);

	protected:
		void     initialize          (void);
		void     processFile         (HumdrumFile& infile);
		bool     processLine         (HumdrumFile& infile, int index);
		bool     processInterpretation(HumdrumFile& infile, int index,
		                              const std::vector<std::string>& fields);
		bool     processTandem       (HumdrumFile& infile, int index,
		                              const std::vector<std::string>& fields);
		void     processBarline      (HumdrumFile& infile, int index,
		                              const std::vector<std::string>& fields);
		bool     processData         (HumdrumFile& infile, int index);
		void     emit                (const std::string& text);
		void     printAnacrusis      (HumNum endTime);
		std::string getPositionText  (HumNum position, HumNum duration);
		int      getGridPosition     (HumNum position);
		HumNum   getShortestDuration (HumdrumFile& infile, int index);
		void     useHierarchy        (int beats, HumNum duration);
		void     updateMeasureHierarchy(HumNum length);
		void     usePulse            (HumNum duration);
		static int getSignBeats      (const std::string& sign);

		static std::vector<int> makeTemplate(int size, int base,
		                              const std::vector<int>& divisors);

	private:
		// Command-line meter (-m), such as "3/4".
		std::string m_meterOption;

		// Measure duration (in quarter notes) and number of beats of the
		// active time signature (0 when the meter is not handled).
		HumNum   m_signatureDuration;
		int      m_signatureBeats = 0;

		// Active original (*omet) and modern (*met) mensuration signs, the
		// text inside the parentheses.
		std::string m_omet;
		std::string m_met;

		// Hierarchy used by getMetricPosition(): the duration of the
		// measure (in quarter notes), and whether it is binary (2-beat and
		// 4-beat meters) or a semibreve pulse (measures without a regular
		// organization).  Otherwise m_template holds the hierarchy of the
		// measure, and all are empty for meters that are not handled.
		HumNum   m_measureDuration;
		bool     m_binaryQ = false;
		bool     m_pulseQ = false;
		const std::vector<int>* m_template = NULL;

		bool     m_firstBarYet = false;
		bool     m_firstInterpretationYet = false;
		HumNum   m_barStart;
		HumNum   m_barEnd;

		// Time of the next barline (or the end of the music) after each line.
		std::vector<HumNum> m_nextBarTime;

		// Records before the first barline, printed once the next barline
		// is known so that pickup notes count back from the end of the
		// measure.  The second value is the line index of a data record,
		// or -1 for other records.
		std::vector<std::pair<std::string, int>> m_anacrusis;
		HumdrumFile* m_infile = NULL;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_METPOS_H */


