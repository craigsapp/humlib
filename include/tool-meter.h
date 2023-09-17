//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Sep 10 00:29:24 PDT 2023
// Last Modified: Thu Sep 14 03:33:20 PDT 2023
// Filename:      tool-meter.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-meter.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Meter/beat data extraction tool.
//

#ifndef _TOOL_METER_H
#define _TOOL_METER_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_meter : public HumTool {

	public:
		         Tool_meter        (void);
		        ~Tool_meter        () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);


	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     getMeterData      (HumdrumFile& infile);
		void     processLine       (HumdrumLine& line,
		                            std::vector<HumNum>& curNum,
		                            std::vector<HumNum>& curDen, 
		                            std::vector<HumNum>& curBeat,
		                            std::vector<HumNum>& curBarTime);
		void     printMeterData    (HumdrumFile& infile);
		void     printHumdrumLine  (HumdrumLine& line, bool printLabels);
		int      printKernAndAnalysisSpine(HumdrumLine& line, int index, bool printLabels, bool forceInterpretation = false);
		HumNum   getHumNum          (HTp token, const std::string& parameter);
		std::string getHumNumString(HumNum input);
		bool     searchForLabels   (HumdrumLine& line);
		void     printLabelLine    (HumdrumLine& line);

	private:
		bool     m_commaQ       = false;
		bool     m_denominatorQ = false;
		bool     m_floatQ       = false;
		bool     m_joinQ        = false;
		bool     m_tsigQ        = false;
		bool     m_nobeatQ      = false;
		bool     m_nolabelQ     = false;
		bool     m_numeratorQ   = false;
		bool     m_restQ        = false;
		bool     m_zeroQ        = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_METER_H */



