//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Date: Mon Sep 16 13:53:47 PDT 2013
// Last Modified: Tue May 30 15:19:23 CEST 2017 Ported from Humdrum Extras
// Filename:      tool-cint.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-dissonant.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Calculates counterpoint interval modules in polyphonic
//                music.
//

#ifndef _TOOL_CINT_H
#define _TOOL_CINT_H

#include "HumTool.h"
#include "HumdrumFile.h"
#include "NoteGrid.h"
#include "HumRegex.h"

#include <vector>
#include <string>

namespace hum {

// START_MERGE

class NoteNode {
   public:
		int b40;         // base-40 pitch number or 0 if a rest, negative if tied
		int line;        // line number in original score of note
		int spine;       // spine number in original score of note
		int measure;     // measure number of note
		int serial;      // serial number 
		int mark;        // for marking search matches
		int notemarker;  // for pass-through of marks
		double beatsize; // time signature bottom value which or
		                 // 3 times the bottom if compound meter
		HumNum   duration;  // duration

		         NoteNode             (void) { clear(); }
		         NoteNode             (const NoteNode& anode);
		         NoteNode& operator=  (NoteNode& anode);
		        ~NoteNode             (void);
		void     clear                (void);
		int      isRest               (void) { return b40 == 0 ? 1 : 0; }
		int      isSustain            (void) { return b40 < 0 ? 1 : 0; }
		int      isAttack             (void) { return b40 > 0 ? 1 : 0; }
		int      getB40               (void) { return abs(b40); }
		void     setId                (const string& anid);
		string   getIdString          (void);
		string   getId                (void);

   protected:
		string  protected_id; // id number provided by data
};



class Tool_cint : public HumTool {
	public:
		         Tool_cint    (void);
		        ~Tool_cint    () {};

		bool     run                    (HumdrumFile& infile);
		bool     run                    (const string& indata, ostream& out);
		bool     run                    (HumdrumFile& infile, ostream& out);

	protected:

		void      initialize           (void);
		void      example              (void);
		void      usage                (const string& command);
		int       processFile          (HumdrumFile& infile);
		void      getKernTracks        (vector<int>& ktracks, HumdrumFile& infile);
		int       validateInterval     (vector<vector<NoteNode> >& notes, 
		                                int i, int j, int k);
		void      printIntervalInfo    (HumdrumFile& infile, int line, 
		                                int spine, vector<vector<NoteNode> >& notes, 
		                                int noteline, int noteindex, 
		                                vector<string >& abbr);
		void      getAbbreviations     (vector<string >& abbreviations, 
		                                vector<string >& names);
		void      getAbbreviation      (string& abbr, string& name);
		void      extractNoteArray     (vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile, vector<int>& ktracks, 
		                                vector<int>& reverselookup);
		int       onlyRests            (vector<NoteNode>& data);
		int       hasAttack            (vector<NoteNode>& data);
		int       allSustained         (vector<NoteNode>& data);
		void      printPitchGrid       (vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile);
		void      getNames             (vector<string >& names, 
		                                vector<int>& reverselookup, HumdrumFile& infile);
		void      printLattice         (vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile, vector<int>& ktracks, 
		                                vector<int>& reverselookup, int n);
		void      printSpacer          (ostream& out);
		int       printInterval        (ostream& out, NoteNode& note1, NoteNode& note2,
		                                int type, int octaveadjust = 0);
		int       printLatticeItem     (vector<vector<NoteNode> >& notes, int n, 
		                                int currentindex, int fileline);
		int       printLatticeItemRows (vector<vector<NoteNode> >& notes, int n, 
		                                int currentindex, int fileline);
		int       printLatticeModule   (ostream& out, vector<vector<NoteNode> >& notes, 
		                                int n, int startline, int part1, int part2);
		void      printInterleaved     (HumdrumFile& infile, int line, 
		                                vector<int>& ktracks, vector<int>& reverselookup, 
		                                const string& interstring);
		void      printLatticeInterleaved(vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile, vector<int>& ktracks, 
		                                vector<int>& reverselookup, int n);
		int       printInterleavedLattice(HumdrumFile& infile, int line, 
		                                vector<int>& ktracks, vector<int>& reverselookup,
		                                int n, int currentindex,
		                                vector<vector<NoteNode> >& notes);
		int       printCombinations    (vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile, vector<int>& ktracks, 
		                                vector<int>& reverselookup, int n,
		                                vector<vector<string> >& retrospective,
		                                const string& searchstring);
		void      printAsCombination   (HumdrumFile& infile, int line, 
		                                vector<int>& ktracks, vector<int>& reverselookup,
		                                const string& interstring);
		int       printModuleCombinations(HumdrumFile& infile, int line, 
		                                vector<int>& ktracks, vector<int>& reverselookup,
		                                int n, int currentindex, 
		                                vector<vector<NoteNode> >& notes, 
		                                int& matchcount, 
		                                vector<vector<string> >& retrospective,
		                                const string& searchstring);
		int       printCombinationsSuspensions(vector<vector<NoteNode> >& notes, 
		                                HumdrumFile& infile, vector<int>& ktracks, 
		                                vector<int>& reverselookup, int n,
		                                vector<vector<string> >& retrospective);
		int       printCombinationModule(ostream& out, const string& filename, 
		                                vector<vector<NoteNode> >& notes, 
		                                int n, int startline, int part1, int part2,
		                                vector<vector<string> >& retrospective,
		                                char& notemarker, int markstate = 0);
		int       printCombinationModulePrepare(ostream& out, const string& filename,
		                                vector<vector<NoteNode> >& notes, int n, 
		                                int startline, int part1, int part2,
		                                vector<vector<string> >& retrospective,
		                                HumdrumFile& infile, const string& searchstring);
		int       getOctaveAdjustForCombinationModule(vector<vector<NoteNode> >& notes, 
		                                int n, int startline, int part1, int part2);
		void      addMarksToInputData  (HumdrumFile& infile, 
		                                vector<vector<NoteNode> >& notes,
		                                vector<int>& ktracks,
		                                vector<int>& reverselookup);
		void      markNote              (HumdrumFile& infile, int line, int col);
		void      initializeRetrospective(vector<vector<string> >& retrospective, 
		                                HumdrumFile& infile, vector<int>& ktracks);
		int       getTriangleIndex(int number, int num1, int num2);
		void      adjustKTracks        (vector<int>& ktracks, const string& koption);
		int       getMeasure           (HumdrumFile& infile, int line);

	private:

		int       debugQ       = 0;      // used with --debug option
		int       base40Q      = 0;      // used with --40 option
		int       base12Q      = 0;      // used with --12 option
		int       base7Q       = 0;      // used with -7 option
		int       pitchesQ     = 0;      // used with --pitches option
		int       rhythmQ      = 0;      // used with -r option and others
		int       durationQ    = 0;      // used with --dur option
		int       latticeQ     = 0;      // used with -l option
		int       interleavedQ = 0;      // used with -L option
		int       Chaincount   = 1;      // used with -n option
		int       chromaticQ   = 0;      // used with --chromatic option
		int       sustainQ     = 0;      // used with -s option
		int       zeroQ        = 0;      // used with -z option
		int       topQ         = 0;      // used with -t option
		int       toponlyQ     = 0;      // used with -T option
		int       hparenQ      = 0;      // used with -h option
		int       mparenQ      = 0;      // used with -y option
		int       locationQ    = 0;      // used with --location option
		int       koptionQ     = 0;      // used with -k option
		int       parenQ       = 0;      // used with -p option
		int       rowsQ        = 0;      // used with --rows option
		int       hmarkerQ     = 0;      // used with -h option
		int       mmarkerQ     = 0;      // used with -m option
		int       attackQ      = 0;      // used with --attacks option
		int       rawQ         = 0;      // used with --raw option
		int       raw2Q        = 0;      // used with --raw2 option
		int       xoptionQ     = 0;      // used with -x option
		int       octaveallQ   = 0;      // used with -O option
		int       octaveQ      = 0;      // used with -o option
		int       noharmonicQ  = 0;      // used with -H option
		int       nomelodicQ   = 0;      // used with -M option
		int       norestsQ     = 0;      // used with -R option
		int       nounisonsQ   = 0;      // used with -U option
		int       filenameQ    = 0;      // used with -f option
		int       searchQ      = 0;      // used with --search option
		int       markQ        = 0;      // used with --mark option
		int       countQ       = 0;      // used with --count option
		int       suspensionsQ = 0;      // used with --suspensions option
		int       uncrossQ     = 0;      // used with -c option
		int       retroQ       = 0;      // used with --retro option
		int       idQ          = 0;      // used with --id option
		vector<string> Ids;              // used with --id option
		char      NoteMarker   = '\0';   // used with -N option
		string    SearchString;
		string Spacer;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_CINT_H */



