//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Nov 23 05:24:14 PST 2009
// Last Modified: Sun Feb  2 22:47:01 PST 2020
// Filename:      tool-rid.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-rid.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Remove various components of Humdrum file data structures.
//

#include "tool-rid.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_rid::Tool_rid -- Set the recognized options for the tool.
//

Tool_rid::Tool_rid(void) {
   // Humdrum Toolkit classic rid options:
   define("D|all-data=b", "remove all data records");
   define("d|null-data=b", "remove null data records");
   define("G|all-global=b", "remove all global comments");
   define("g|null-global=b", "remove null global comments");
   define("I|all-interpretation=b", "remove all interpretation records");
   define("i|null-interpretation=b", "remove null interpretation records");
   define("L|all-local-comment=b", "remove all local comments");
   define("l|1|null-local-comment=b", "remove null local comments");
   define("T|all-tandem-interpretation=b", "remove all tandem interpretations");
   define("U|u=b", "remove unnecessary (duplicate ex. interps.");
   define("k|consider-kern-only=b", "for -d, only consider **kern spines.");
   define("V=b","negate filtering effect of program.");
   define("H|no-humdrum-syntax=b", "equivalent to -GLIMd.");

   // additional options
   define("M|all-barlines=b", "remove measure lines");
   define("C|all-comments=b", "remove all comment lines");
   define("c=b", "remove global and local comment lines");
}



/////////////////////////////////
//
// Tool_rid::run -- Do the main work of the tool.
//

bool Tool_rid::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_rid::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rid::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rid::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_rid::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_rid::initialize(void) {
   option_D = getBoolean("D");
   option_d = getBoolean("d");
   option_G = getBoolean("G");
   option_g = getBoolean("g");
   option_I = getBoolean("I");
   option_i = getBoolean("i");
   option_L = getBoolean("L");
   option_l = getBoolean("l");
   option_T = getBoolean("T");
   option_U = getBoolean("U");
   option_M = getBoolean("M");
   option_C = getBoolean("C");
   option_c = getBoolean("c");
   option_k = getBoolean("k");
   option_V = getBoolean("V");

   if (getBoolean("no-humdrum-syntax")) {
      // remove all Humdrum file structure
      option_G = option_L = option_I = option_M = option_d = 1;
   }
}



//////////////////////////////
//
// Tool_rid::processFile --
//

void Tool_rid::processFile(HumdrumFile& infile) {
	int setcount = 1; // disabled for now.

   HumRegex hre;
   int revQ = option_V;

   // if bibliographic/reference records are not suppressed
   // print the !!!!SEGMENT: marker if present.
   if ((setcount > 1) && (!option_G)) {
      infile.printNonemptySegmentLabel(m_humdrum_text);
   }

   for (int i=0; i<infile.getLineCount(); i++) {
      if (option_D && (infile[i].isBarline() || infile[i].isData())) {
         // remove data lines if -D is specified
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_d) {
         // remove null data lines if -d is specified
         if (option_k && infile[i].isData() && 
               infile[i].equalFieldsQ("**kern", ".")) {
            // remove if only all **kern spines are null.
            if (revQ) {
               m_humdrum_text << infile[i] << "\n";
            }
            continue;
         } else if (!option_k && infile[i].isData() && 
               infile[i].isAllNull()) {
            // remove null data lines if all spines are null.
            if (revQ) {
               m_humdrum_text << infile[i] << "\n";
            }
            continue;
         }
      }
      if (option_G && (infile[i].isGlobalComment() || 
            infile[i].isReference())) {
         // remove global comments if -G is specified
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_g && hre.search(infile.token(i, 0), "^!!+\\s*$")) {
         // remove empty global comments if -g is specified
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_I && infile[i].isInterpretation()) {
         // remove all interpretation records
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_i && infile[i].isInterpretation() && 
            infile[i].isAllNull()) {
         // remove null interpretation records
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_L && infile[i].isLocalComment()) {
         // remove all local comments
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_l && infile[i].isLocalComment() && 
            infile[i].isAllNull()) {
         // remove null local comments
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_T && (infile[i].isInterpretation() && !infile[i].isManipulator())) {
         // remove tandem (non-manipulator) interpretations
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_U) {
         // remove unnecessary (duplicate exclusive) interpretations
         // HumdrumFile class does not allow duplicate ex. interps.
         // continue;
      }

      // non-classical options:

      if (option_M && infile[i].isBarline()) {
         // remove all measure lines
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_C && infile[i].isComment()) {
         // remove all comments (local & global)
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }
      if (option_c && (infile[i].isLocalComment() || 
            infile[i].isGlobalComment())) {
         // remove all comments (local & global)
         if (revQ) {
            m_humdrum_text << infile[i] << "\n";
         }
         continue;
      }

      // got past all test, so print the current line:
      if (!revQ) {
         m_humdrum_text << infile[i] << "\n";
      }
   }
}



// END_MERGE

} // end namespace hum



