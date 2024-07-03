//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  5 23:09:00 PST 2016
// Last Modified: Wed Jul  3 21:00:27 CEST 2024
// Filename:      tool-extract.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-extract.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for extract tool.
//

#ifndef _TOOL_EXTRACT_H_INCLUDED
#define _TOOL_EXTRACT_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_extract : public HumTool {
	public:
		         Tool_extract           (void);
		        ~Tool_extract           () {};

		bool     run                    (HumdrumFileSet& infiles);
		bool     run                    (HumdrumFile& infile);
		bool     run                    (const std::string& indata, std::ostream& out);
		bool     run                    (HumdrumFile& infile, std::ostream& out);

	protected:

		// auto transpose functions:
		void     initialize             (HumdrumFile& infile);

		// function declarations
		void    processFile             (HumdrumFile& infile);
		void    excludeFields           (HumdrumFile& infile, std::vector<int>& field,
		                                 std::vector<int>& subfield, std::vector<int>& model);
		void    extractFields           (HumdrumFile& infile, std::vector<int>& field,
		                                 std::vector<int>& subfield, std::vector<int>& model);
		void    extractTrace            (HumdrumFile& infile, const std::string& tracefile);
		void    getInterpretationFields (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, HumdrumFile& infile,
		                                 std::string& interps, int state);
		//void    extractInterpretations  (HumdrumFile& infile, std::string& interps);
		void    example                 (void);
		void    usage                   (const std::string& command);
		std::string reverseFieldString(const std::string& input, int maxval);
		void    fillFieldData           (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, std::string& fieldstring,
		                                 HumdrumFile& infile);
		void    processFieldEntry       (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, const std::string& astring,
		                                 HumdrumFile& infile);
		void    removeDollarsFromString (std::string& buffer, int maxtrack);
		int     isInList                (int number, std::vector<int>& listofnum);
		void    getTraceData            (std::vector<int>& startline,
		                                 std::vector<std::vector<int> >& fields,
		                                 const std::string& tracefile, HumdrumFile& infile);
		void    printTraceLine          (HumdrumFile& infile, int line,
		                                 std::vector<int>& field);
		void    dealWithSpineManipulators(HumdrumFile& infile, int line,
		                                 std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model);
		void    storeToken              (std::vector<std::string>& storage,
		                                 const std::string& string);
		void    storeToken              (std::vector<std::string>& storage, int index,
		                                 const std::string& string);
		void    printMultiLines         (std::vector<int>& vsplit, std::vector<int>& vserial,
		                                 std::vector<std::string>& tempout);
		void    reverseSpines           (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, HumdrumFile& infile,
		                                 const std::string& exinterp);
		void    getSearchPat            (std::string& spat, int target,
		                                 const std::string& modifier);
		void    expandSpines            (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, HumdrumFile& infile,
		                                 std::string& interp);
		void    dealWithSecondarySubspine(std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, int targetindex,
		                                 HumdrumFile& infile, int line, int spine,
		                                 int submodel);
		void    dealWithCospine         (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, int targetindex,
		                                 HumdrumFile& infile, int line, int cospine,
		                                 int comodel, int submodel,
		                                 const std::string& cointerp);
		void    printCotokenInfo        (int& start, HumdrumFile& infile, int line,
		                                 int spine, std::vector<std::string>& cotokens,
		                                 std::vector<int>& spineindex,
		                                 std::vector<int>& subspineindex);
		void    fillFieldDataByGrep     (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, const std::string& grepString,
		                                 HumdrumFile& infile, int state);
		std::vector<int> getNullDataTracks(HumdrumFile& infile);
		void fillFieldDataByEmpty       (std::vector<int>& field, std::vector<int>& subfield,
				                           std::vector<int>& model, HumdrumFile& infile, int negate);
		void fillFieldDataByNoEmpty     (std::vector<int>& field, std::vector<int>& subfield,
				                           std::vector<int>& model, HumdrumFile& infile, int negate);
		void fillFieldDataByNoRest      (std::vector<int>& field, std::vector<int>& subfield,
		                                 std::vector<int>& model, const std::string& searchstring,
		                                 HumdrumFile& infile, int state);
		void printInterpretationForKernSpine(HumdrumFile& infile, int index);

	private:

		// global variables
		bool        excludeQ     = false;     // used with -x option
		bool        expandQ      = false;     // used with -e option
		std::string expandInterp = "";        // used with -E option
		bool        interpQ      = false;     // used with -i option
		std::string interps      = "";        // used with -i option
		bool        debugQ       = false;     // used with --debug option
		bool        kernQ        = false;     // used with -k option
		bool        rkernQ       = false;     // used with -K option
		bool        fieldQ       = false;     // used with -f or -p option
		std::string fieldstring  = "";        // used with -f or -p option
		std::vector<int> field;               // used with -f or -p option
		std::vector<int> subfield;            // used with -f or -p option
		std::vector<int> model;               // used with -p, or -e options and similar
		bool        countQ        = false;    // used with -C option
		bool        traceQ        = false;    // used with -t option
		std::string tracefile     = "";       // used with -t option
		bool        reverseQ      = false;    // used with -r option
		std::string reverseInterp = "**kern"; // used with -r and -R options.
		// sub-spine "b" expansion model: how to generate data for a secondary
		// spine if the primary spine is not divided.  Models are:
		//    'd': duplicate primary spine (or "a" subspine) data (default)
		//    'n': null = use a null token
		//    'r': rest = use a rest instead of a primary spine note (in **kern)
		//         data.  'n' will be used for non-kern spines when 'r' is used.
		int          submodel = 'd';       // used with -m option
		std::string editorialInterpretation = "yy";
		std::string cointerp    = "**kern";  // used with -c option
		int         comodel     = 0;         // used with -M option
		std::string subtokenseparator = " "; // used with a future option
		int         interpstate = 0;         // used -I or with -i
		bool        grepQ       = false;     // used with -g option
		std::string grepString  = "";        // used with -g option
		std::string blankName   = "**blank"; // used with -n option
 		bool        addRestsQ   = false;     // used with -n option
		bool        noEmptyQ    = false;     // used with --no-empty option
		bool        emptyQ      = false;     // used with --empty option
		bool        spineListQ  = false;     // used with --spine option
		bool        removerestQ = false;     // used with --no-rest option

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_EXTRACT_H_INCLUDED */



