//
// Programmer:    Katherine Wong
// Creation Date: Mon Mar 13 23:47:52 PDT 2023
// Last Modified: Mon Mar 13 23:47:55 PDT 2023
// Filename:      tool-colorthirds.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-colorthirds.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color diatonic thirds in triadic sonorities:
//                    red   = root
//                    green = third
//                    blue  = fifth
//

#ifndef _TOOL_COLORTHIRDS_H
#define _TOOL_COLORTHIRDS_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_colorthirds : public HumTool {
	public:
		         Tool_colorthirds  (void);
		        ~Tool_colorthirds  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void             initialize        (void);
		void             processFile       (HumdrumFile& infile);
		std::vector<int> getMidiNotes(std::vector<HTp>& kernNotes);
		std::vector<int> getChordPositions(std::vector<int>& midiNotes);
		void             labelChordPositions(std::vector<HTp>& kernNotes, std::vector<int>& chordPositions);

	private:
		std::string m_root_marker = "@";
		std::string m_third_marker = "N";
		std::string m_fifth_marker = "Z";

		std::string m_root_color = "crimson";
		std::string m_third_color = "limegreen";
		std::string m_fifth_color = "royalblue";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COLORTHIRDS_H */



