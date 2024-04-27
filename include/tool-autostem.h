//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 20:36:38 PST 2016
// Last Modified: Wed Nov 30 20:36:41 PST 2016
// Filename:      tool-autostem.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-autostem.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for autostem tool.
//

#ifndef _TOOL_AUTOSTEM_H_INCLUDED
#define _TOOL_AUTOSTEM_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFileSet.h"

#include <string>
#include <vector>
#include <ostream>

namespace hum {

// START_MERGE


class Coord {
   public:
           Coord(void) { clear(); }
      void clear(void) { i = j = -1; }
      int i;
      int j;
};


class Tool_autostem : public HumTool {
	public:
		         Tool_autostem         (void);
		        ~Tool_autostem         () {};

		bool     run                   (HumdrumFileSet& infiles);
		bool     run                   (HumdrumFile& infile);
		bool     run                   (const std::string& indata, std::ostream& out);
		bool     run                   (HumdrumFile& infile, std::ostream& out);

	protected:
		void     initialize            (HumdrumFile& infile);
		void      example              (void);
		void      usage                (void);
		bool      autostem             (HumdrumFile& infile);
		void      getClefInfo          (std::vector<std::vector<int>>& baseline,
		                                HumdrumFile& infile);
		void      addStem              (std::string& input, const std::string& piece);
		void      processKernTokenStemsSimpleModel(HumdrumFile& infile,
		                                std::vector<std::vector<int>>& baseline,
		                                int row, int col);
		void      removeStems          (HumdrumFile& infile);
		void      removeStem2          (HumdrumFile& infile, int row, int col);
		int       getVoice             (HumdrumFile& infile, int row, int col);
		void      getNotePositions     (std::vector<std::vector<std::vector<int>>>& notepos,
		                                std::vector<std::vector<int>>& baseline,
		                                HumdrumFile& infile);
		void      printNotePositions   (HumdrumFile& infile,
		                                std::vector<std::vector<std::vector<int>>>& notepos);
		void      getVoiceInfo         (std::vector<std::vector<int>>& voice, HumdrumFile& infile);
		void      printVoiceInfo       (HumdrumFile& infile, std::vector<std::vector<int>>& voice);
		void      processKernTokenStems(HumdrumFile& infile,
		                                std::vector<std::vector<int>>& baseline, int row, int col);
		void      getMaxLayers         (std::vector<int>& maxlayer, std::vector<std::vector<int>>& voice,
		                                HumdrumFile& infile);
		bool      assignStemDirections (std::vector<std::vector<int>>& stemdir,
		                                std::vector<std::vector<int>> & voice,
		                                std::vector<std::vector<std::vector<int>>>& notepos,
		                                HumdrumFile& infile);
		void      assignBasicStemDirections(std::vector<std::vector<int>>& stemdir,
		                                std::vector<std::vector<int>>& voice,
		                                std::vector<std::vector<std::vector<int>>>& notepos,
		                                HumdrumFile& infile);
		int       determineChordStem   (std::vector<std::vector<int>>& voice,
		                                std::vector<std::vector<std::vector<int>>>& notepos,
		                                HumdrumFile& infile, int row, int col);
		void      insertStems          (HumdrumFile& infile,
		                                std::vector<std::vector<int>>& stemdir);
		void      setStemDirection     (HumdrumFile& infile, int row, int col,
		                                int direction);
		bool      getBeamState         (std::vector<std::vector<std::string >>& beams,
		                                HumdrumFile& infile);
		void      countBeamStuff       (const std::string& token, int& start, int& stop,
		                                int& flagr, int& flagl);
		void      getBeamSegments      (std::vector<std::vector<Coord>>& beamednotes,
		                                std::vector<std::vector<std::string >>& beamstates,
		                                HumdrumFile& infile, std::vector<int> maxlayer);
		int       getBeamDirection     (std::vector<Coord>& coords,
		                                std::vector<std::vector<int>>& voice,
		                                std::vector<std::vector<std::vector<int>>>& notepos);
		void      setBeamDirection     (std::vector<std::vector<int>>& stemdir,
		                                std::vector<Coord>& bnote, int direction);

	private:
		int    debugQ        = 0;       // used with --debug option
		int    removeQ       = 0;       // used with -r option
		int    noteposQ      = 0;       // used with -p option
		int    voiceQ        = 0;       // used with --voice option
		int    removeallQ    = 0;       // used with -R option
		int    overwriteQ    = 0;       // used with -o option
		int    overwriteallQ = 0;       // used with -O option
		int    Middle        = 4;       // used with -u option
		int    Borderline    = 0;       // really used with -u option
		int    notlongQ      = 0;       // used with -L option
		bool   m_quit        = false;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_AUTOSTEM_H_INCLUDED */



