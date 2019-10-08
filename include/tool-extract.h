//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Dec  5 23:09:00 PST 2016
// Last Modified: Mon Dec  5 23:09:08 PST 2016
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
		bool     run                    (const string& indata, ostream& out);
		bool     run                    (HumdrumFile& infile, ostream& out);

	protected:

		// auto transpose functions:
		void     initialize             (HumdrumFile& infile);

		// function declarations
		void    processFile             (HumdrumFile& infile);
		void    excludeFields           (HumdrumFile& infile, vector<int>& field,
		                                 vector<int>& subfield, vector<int>& model);
		void    extractFields           (HumdrumFile& infile, vector<int>& field,
		                                 vector<int>& subfield, vector<int>& model);
		void    extractTrace            (HumdrumFile& infile, const string& tracefile);
		void    getInterpretationFields (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, HumdrumFile& infile,
		                                 string& interps, int state);
		//void    extractInterpretations  (HumdrumFile& infile, string& interps);
		void    example                 (void);
		void    usage                   (const string& command);
		void    fillFieldData           (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, string& fieldstring,
		                                 HumdrumFile& infile);
		void    processFieldEntry       (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, const string& astring,
		                                 HumdrumFile& infile);
		void    removeDollarsFromString (string& buffer, int maxtrack);
		int     isInList                (int number, vector<int>& listofnum);
		void    getTraceData            (vector<int>& startline,
		                                 vector<vector<int> >& fields,
		                                 const string& tracefile, HumdrumFile& infile);
		void    printTraceLine          (HumdrumFile& infile, int line,
		                                 vector<int>& field);
		void    dealWithSpineManipulators(HumdrumFile& infile, int line,
		                                 vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model);
		void    storeToken              (vector<string>& storage,
		                                 const string& string);
		void    storeToken              (vector<string>& storage, int index,
		                                 const string& string);
		void    printMultiLines         (vector<int>& vsplit, vector<int>& vserial,
		                                 vector<string>& tempout);
		void    reverseSpines           (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, HumdrumFile& infile,
		                                 const string& exinterp);
		void    getSearchPat            (string& spat, int target,
		                                 const string& modifier);
		void    expandSpines            (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, HumdrumFile& infile,
		                                 string& interp);
		void    dealWithSecondarySubspine(vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, int targetindex,
		                                 HumdrumFile& infile, int line, int spine,
		                                 int submodel);
		void    dealWithCospine         (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, int targetindex,
		                                 HumdrumFile& infile, int line, int cospine,
		                                 int comodel, int submodel,
		                                 const string& cointerp);
		void    printCotokenInfo        (int& start, HumdrumFile& infile, int line,
		                                 int spine, vector<string>& cotokens,
		                                 vector<int>& spineindex,
		                                 vector<int>& subspineindex);
		void    fillFieldDataByGrep     (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, const string& grepString,
		                                 HumdrumFile& infile, int state);
		vector<int> getNullDataTracks(HumdrumFile& infile);
		void fillFieldDataByEmpty       (vector<int>& field, vector<int>& subfield,
				                           vector<int>& model, HumdrumFile& infile, int negate);
		void fillFieldDataByNoEmpty     (vector<int>& field, vector<int>& subfield,
				                           vector<int>& model, HumdrumFile& infile, int negate);
		void fillFieldDataByNoRest      (vector<int>& field, vector<int>& subfield,
		                                 vector<int>& model, const string& searchstring,
		                                 HumdrumFile& infile, int state);

	private:

		// global variables
		int          excludeQ = 0;        // used with -x option
		int          expandQ  = 0;        // used with -e option
		string       expandInterp = "";   // used with -E option
		int          interpQ  = 0;        // used with -i option
		string       interps  = "";       // used with -i option
		int          debugQ   = 0;        // used with --debug option
		int          kernQ    = 0;        // used with -k option
		int          fieldQ   = 0;        // used with -f or -p option
		string       fieldstring = "";    // used with -f or -p option
		vector<int>  field;               // used with -f or -p option
		vector<int>  subfield;            // used with -f or -p option
		vector<int>  model;               // used with -p, or -e options and similar
		int          countQ   = 0;        // used with -C option
		int          traceQ   = 0;        // used with -t option
		string       tracefile = "";      // used with -t option
		int          reverseQ = 0;        // used with -r option
		string       reverseInterp = "**kern"; // used with -r and -R options.
		// sub-spine "b" expansion model: how to generate data for a secondary
		// spine if the primary spine is not divided.  Models are:
		//    'd': duplicate primary spine (or "a" subspine) data (default)
		//    'n': null = use a null token
		//    'r': rest = use a rest instead of a primary spine note (in **kern)
		//         data.  'n' will be used for non-kern spines when 'r' is used.
		int          submodel = 'd';       // used with -m option
		string editorialInterpretation = "yy";
		string      cointerp = "**kern";   // used with -c option
		int         comodel  = 0;          // used with -M option
		string subtokenseparator = " "; // used with a future option
		int         interpstate = 0;       // used -I or with -i
		int         grepQ       = 0;       // used with -g option
		string      grepString  = "";      // used with -g option
		string      blankName   = "**blank"; // used with -n option
		int         noEmptyQ    = 0;       // used with --no-empty option
		int         emptyQ      = 0;       // used with --empty option
		int         spineListQ  = 0;       // used with --spine option
		int         removerestQ = 0;       // used with --no-rest option

};


// END_MERGE

} // end namespace hum

#endif /* _TOOL_EXTRACT_H_INCLUDED */



