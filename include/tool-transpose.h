//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  5 23:09:00 PST 2016
// Last Modified: Mon Dec  5 23:09:08 PST 2016
// Filename:      tool-transpose.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-transpose.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for transpose tool.
//

#ifndef _TOOL_TRANSPOSE_H_INCLUDED
#define _TOOL_TRANSPOSE_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>


namespace hum {

// START_MERGE

class Tool_transpose : public HumTool {
	public:
		         Tool_transpose  (void);
		        ~Tool_transpose  () {};

		bool     run             (HumdrumFileSet& infiles);
		bool     run             (HumdrumFile& infile);
		bool     run             (const std::string& indata, std::ostream& out);
		bool     run             (HumdrumFile& infile, std::ostream& out);

	protected:

		// auto transpose functions:
		void     initialize             (HumdrumFile& infile);
		void     convertScore           (HumdrumFile& infile, int style);
		void     processFile            (HumdrumFile& infile,
		                                 std::vector<bool>& spineprocess);
		void     convertToConcertPitches(HumdrumFile& infile, int line,
		                                 std::vector<int>& tvals);
		void     convertToWrittenPitches(HumdrumFile& infile, int line,
		                                 std::vector<int>& tvals);
		void     printNewKeySignature   (const std::string& keysig, int trans);
		void     processInterpretationLine(HumdrumFile& infile, int line,
		                                 std::vector<int>& tvals, int style);
		int      isKeyMarker            (const std::string& str);
		void     printNewKeyInterpretation(HumdrumLine& aRecord,
		                                 int index, int transval);
		int      hasTrMarkers           (HumdrumFile& infile, int line);
		void     printHumdrumKernToken  (HumdrumLine& record, int index,
		                                 int transval);
		void     printHumdrumMxhmToken(HumdrumLine& record, int index,
		                                 int transval);
		int      checkForDeletedLine    (HumdrumFile& infile, int line);
		int      getBase40ValueFromInterval(const std::string& string);
		void     example                (void);
		void     usage                  (const std::string& command);
		void     printHumdrumDataRecord (HumdrumLine& record,
		                                 std::vector<bool>& spineprocess);

		double   pearsonCorrelation     (int size, double* x, double* y);
		void     doAutoTransposeAnalysis(HumdrumFile& infile);
		void     addToHistogramDouble   (std::vector<std::vector<double> >& histogram,
		                                 int pc, double start, double dur,
		                                 double tdur, int segments);
		double   storeHistogramForTrack (std::vector<std::vector<double> >& histogram,
		                                 HumdrumFile& infile, int track,
		                                 int segments);
		void     printHistograms        (int segments, std::vector<int> ktracks,
		                                std::vector<std::vector<std::vector<double> > >&
		                                 trackhist);
		void     doAutoKeyAnalysis      (std::vector<std::vector<std::vector<double> > >&
		                                 analysis, int level, int hop, int count,
		                                 int segments, std::vector<int>& ktracks,
		                                 std::vector<std::vector<std::vector<double> > >&
		                                 trackhist);
		void     doTrackKeyAnalysis     (std::vector<std::vector<double> >& analysis,
		                                 int level, int hop, int count,
		                                 std::vector<std::vector<double> >& trackhist,
		                                 std::vector<double>& majorweights,
		                                 std::vector<double>& minorweights);
		void     identifyKeyDouble      (std::vector<double>& correls,
		                                 std::vector<double>& histogram,
		                                 std::vector<double>& majorweights,
		                                 std::vector<double>& minorweights);
		void     fillWeightsWithKostkaPayne(std::vector<double>& maj,
		                                 std::vector<double>& min);
		void     printRawTrackAnalysis  (std::vector<std::vector<std::vector<double> > >&
		                                 analysis, std::vector<int>& ktracks);
		void     doSingleAnalysis       (std::vector<double>& analysis,
		                                 int startindex, int length,
		                                 std::vector<std::vector<double> >& trackhist,
		                                 std::vector<double>& majorweights,
		                                 std::vector<double>& minorweights);
		void     identifyKey            (std::vector<double>& correls,
		                                 std::vector<double>& histogram,
		                                 std::vector<double>& majorweights,
		                                 std::vector<double>& minorweights);
		void     doTranspositionAnalysis(std::vector<std::vector<std::vector<double> > >&
		                                 analysis);
		int      calculateTranspositionFromKey(int targetkey,
		                                 HumdrumFile& infile);
		void     printTransposedToken   (HumdrumFile& infile, int row, int col,
		                                 int transval);
		void     printTransposeInformation(HumdrumFile& infile,
		                                 std::vector<bool>& spineprocess,
		                                 int line, int transval);
		int      getTransposeInfo       (HumdrumFile& infile, int row, int col);
		void     printNewKernString     (const std::string& string, int transval);

	private:
		int      transval     = 0;   // used with -b option
		int      ssettonicQ   = 0;   // used with -k option
		int      ssettonic    = 0;   // used with -k option
		int      currentkey   = 0;
		int      autoQ        = 0;   // used with --auto option
		int      debugQ       = 0;   // used with --debug option
		std::string   spinestring  = "";  // used with -s option
		int      octave       = 0;   // used with -o option
		int      concertQ     = 0;   // used with -C option
		int      writtenQ     = 0;   // used with -W option
		int      quietQ       = 0;   // used with -q option
		int      instrumentQ  = 0;   // used with -I option
};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_TRANSPOSE_H_INCLUDED */



