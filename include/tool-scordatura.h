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

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_scordatura : public HumTool {
	public:
		         Tool_scordatura   (void);
		        ~Tool_scordatura   () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);
		void     getScordaturaRdfs (std::vector<HTp>& rdfs, HumdrumFile& infile);
		void     processScordatura (HumdrumFile& infile, HTp reference);
		void     processScordaturas(HumdrumFile& infile, std::vector<HTp>& rdfs);
		void     flipScordaturaInfo(HTp reference, int diatonic, int chromatic);
		void     transposeStrand   (HTp sstart, HTp sstop, const std::string& marker);
		void     transposeChord    (HTp token, const std::string& marker);
		std::string transposeNote     (const std::string& note);
		void     transposeMarker   (HumdrumFile& infile, const std::string& marker, int diatonic, int chromatic);
		std::set<int> parsePitches(const std::string& input);
		void     markPitches       (HumdrumFile& infile);
		void     markPitches       (HTp sstart, HTp sstop);
		void     markPitches       (HTp token);
		void     addMarkerRdf      (HumdrumFile& infile);
		void     prepareTranspositionInterval(void);

	private:
		bool           m_writtenQ    = false;
		bool           m_soundingQ   = false;
		bool           m_modifiedQ   = false;
		bool           m_IQ          = false;  // true: enbed marker in sounding score
		std::string    m_transposition;
		std::string    m_color;
		std::string    m_marker;
		std::set<int>  m_pitches;
		HumTransposer  m_transposer;
		int            m_diatonic;
		int            m_chromatic;
		std::string    m_interval;
		bool           m_cd;
		std::string    m_string;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SCORDATURA_H */



