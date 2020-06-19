//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jun 18 22:35:07 PDT 2020
// Last Modified: Thu Jun 18 22:35:10 PDT 2020
// Filename:      tool-scordatura.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-scordatura.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between scordatura encoding and corrected encoding.
//

#ifndef _TOOL_SCORDATURA_H
#define _TOOL_SCORDATURA_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "HumTransposer.h"

namespace hum {

// START_MERGE

class Tool_scordatura : public HumTool {
	public:
		         Tool_scordatura   (void);
		        ~Tool_scordatura   () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     getScordaturaRdfs (vector<HTp>& rdfs, HumdrumFile& infile);
		void     processScordatura (HumdrumFile& infile, HTp reference);
		void     processScordaturas(HumdrumFile& infile, vector<HTp>& rdfs);
		void     flipScordaturaInfo(HTp reference, int diatonic, int chromatic);
		void     transposeStrand   (HTp sstart, HTp sstop, const string& marker);
		void     transposeChord    (HTp token, const string& marker);
		std::string transposeNote     (const string& note);
		void     transposeMarker   (HumdrumFile& infile, const string& marker, int diatonic, int chromatic);

	private:
		bool          m_writtenQ    = false;
		bool          m_soundingQ   = false;
		bool          m_modifiedQ   = false;
		HumTransposer m_transposer;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SCORDATURA_H */



