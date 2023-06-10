//
// Programmer:    Katherine Wong
// Creation Date: Mon Mar 13 23:47:52 PDT 2023
// Last Modified: Tue Jun  6 19:23:10 PDT 2023
// Filename:      tool-tspos.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-tspos.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color diatonic thirds in triadic sonorities:
//                    red   = root
//                    green = third
//                    blue  = fifth
//

#ifndef _TOOL_TPOS_H
#define _TOOL_TPOS_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_tspos : public HumTool {
	public:
		         Tool_tspos  (void);
		        ~Tool_tspos  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void             initialize        (HumdrumFile& infile);
		void             processFile       (HumdrumFile& infile);
		std::vector<int> getMidiNotes(std::vector<HTp>& kernNotes);
		std::vector<int> getChordPositions(std::vector<int>& midiNotes);
		std::vector<int> getNoteMods(std::vector<int>& midiNotes);
		std::vector<int> getThirds(std::vector<int>& midiNotes);
		std::vector<int> getFifths(std::vector<int>& midiNotes);
		void             labelChordPositions(std::vector<HTp>& kernNotes, std::vector<int>& chordPositions);
		void             labelThirds(std::vector<HTp>& kernNotes, std::vector<int>& thirdPositions);
		void             labelFifths(std::vector<HTp>& kernNotes, std::vector<int>& fifthPositions);
		void             keepOnlyDoubles(std::vector<int>& output);
		void             checkForTriadicSonority(std::vector<int>& positions, int line);
		std::string      generateStatistics(HumdrumFile& infile);
		std::vector<std::string> getTrackNames(HumdrumFile& infile);
		int              getVectorSum(std::vector<int>& input);
		void             analyzeVoiceCount(HumdrumFile& infile);
		int              countVoicesOnLine(HumdrumFile& infile, int line);
		std::string      generateTable(HumdrumFile& infile, std::vector<std::string>& name);
		bool             hasFullTriadAttack(HumdrumLine& line);
		void             avoidRdfCollisions(HumdrumFile& infile);
		void             printUsedMarkers(void);

	private:
		std::string m_root_marker      = "@";
		std::string m_third_marker     = "N";
		std::string m_fifth_marker     = "Z";
		std::string m_3rd_root_marker  = "j";
		std::string m_3rd_third_marker = "l";
		std::string m_5th_root_marker  = "V";
		std::string m_5th_fifth_marker = "|";

		std::vector<int> m_used_markers;

		std::string m_root_color       = "#DC143C"; // crimson
		std::string m_third_color      = "#32CD32"; // limegreen
		std::string m_fifth_color      = "#4169E1"; // royalblue
		std::string m_3rd_root_color   = "#8B0000"; // darkred
		std::string m_3rd_third_color  = "#008000"; // green
		std::string m_5th_root_color   = "#8B0000"; // darkred
		std::string m_5th_fifth_color  = "#4682B4"; // steelblue

		bool m_colorThirds = true;   // used with -3 option (to negate)
		bool m_colorFifths = true;   // used with -5 option (to negate)
		bool m_colorTriads = true;   // used with -T option (to negate)
		bool m_doubleQ     = false;  // used with -d option

		bool m_topQ = false;         // used with --top option
		bool m_tableQ = false;       // used with -t option
		bool m_triadAttack = false;  // used with -x option

		// Statistical data variables:
		vector<bool> m_triadState;

		// m_partTriadPositions -- count the number of chordal positions by
		// voice.  The first dimention is the track number of the part, and
		// the second dimension is the counts for 7 categories:
		// 0 = count of root positions in full triadic chords
		// 1 = count of third positions in full triadic chords
		// 2 = count of fifth positions in full triadic chords
		// 3 = count of root positions in partial triadic chords ("open thirds")
		// 4 = count of third positions in partial triadic chords
		// 5 = count of root positions in partial triadic chords ("open fifths")
		// 6 = count of fifth positions in partial triadic chords
		std::vector<vector<int>> m_partTriadPositions;
		int m_positionCount = 7; // entries in 2nd dim. of m_partTriadPositions

		string m_toolName = "tspos";

		std::vector<int> m_voiceCount;
		// m_voice: used with -v option to limit analysis to sonorities that
		// have the given voice count. 0 means analyze any voice counts.
		int m_voice = 0;

		bool m_evenNoteSpacingQ = false;

		std::vector<string> m_fullNames;
};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_TPOS_H */



