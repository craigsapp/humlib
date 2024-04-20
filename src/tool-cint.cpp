//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Dec 26 17:03:54 PST 2010
// Last Modified: Tue May 30 15:35:10 CEST 2017
// Filename:      tool-cint.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/tool-cint.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Calculates counterpoint interval modules in polyphonic
//                music.
//

#include "tool-cint.h"
#include "HumRegex.h"
#include "Convert.h"

#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE

#define EMPTY_ID ""
#define REST 0
#define RESTINT -1000000
#define RESTSTRING "R"
#define INTERVAL_HARMONIC 1
#define INTERVAL_MELODIC  2
#define MARKNOTES  1


/////////////////////////////////
//
// Tool_cint::Tool_cint -- Set the recognized options for the tool.
//

Tool_cint::Tool_cint(void) {
	define("base-40|base40|b40|40=b",             "display pitches/intervals in base-40");
	define("base-12|base12|b12|12=b",             "display pitches/intervals in base-12");
	define("base-7|base7|b7|7|diatonic=b",        "display pitches/intervals in base-7");
	define("g|grid|pitch|pitches=b",              "display pitch grid used to calculate modules");
	define("r|rhythm=b",                          "display rhythmic positions of notes");
	define("f|filename=b",                        "display filenames with --count");
	define("raw=b",                               "display only modules without formatting");
	define("raw2=b",                              "display only modules formatted for Vishesh");
	define("c|uncross=b",                         "uncross crossed voices when creating modules");
	define("k|koption=s:",                        "select only two spines to analyze");
	define("C|comma=b",                           "separate intervals by comma rather than space");
	define("retro|retrospective=b",               "retrospective module display in the score");
	define("suspension|suspensions=b",            "mark suspensions");
	define("rows|row=b",                          "display lattices in row form");
	define("dur|duration=b",                      "display durations appended to harmonic interval note attacks");
	define("id=b",                                "ids are echoed in module data");
	define("L|interleaved-lattice=b",             "display interleaved lattices");
	define("q|harmonic-parentheses=b",            "put square brackets around harmonic intervals");
	define("h|harmonic-marker=b",                 "put h character after harmonic intervals");
	define("m|melodic-marker=b",                  "put m character after melodic intervals");
	define("y|melodic-parentheses=b",             "put curly braces around melodic intervals");
	define("p|parentheses=b",                     "put parentheses around modules intervals");
	define("l|lattice=b",                         "calculate lattice");
	define("loc|location=b",                      "displayLocation");
	define("s|sustain=b",                         "display sustain/attack states of notes");
	define("o|octave=b",                          "reduce compound intervals to within an octave");
	define("H|no-harmonic=b",                     "don't display harmonic intervals");
	define("M|no-melodic=b",                      "don't display melodic intervals");
	define("t|top=b",                             "display top melodic interval of modules");
	define("T|top-only=b",                        "display only top melodic interval of modules");
	define("U|no-melodic-unisons=b",              "no melodic perfect unisons");
	define("attacks|attack=b",                    "start/stop module chains on pairs of note attacks");
	define("z|zero=b",                            "display diatonic intervals with 0 offset");
	define("N|note-marker=s:@",                   "pass-through note marking character");
	define("x|xoption=b",                         "display attack/sustain information on harmonic intervals only");
	define("n|chain=i:1",                         "number of sequential modules");
	define("R|no-rest|no-rests|norest|norests=b", "number of sequential modules");
	define("O|octave-all=b",                      "transpose all harmonic intervals to within an octave");
	define("chromatic=b",                         "display intervals as diatonic intervals with chromatic alterations");
	define("color=s:red",                         "color of marked notes");
	define("search=s:",                           "search string");
	define("mark=b",                              "mark matches notes from searches in data");
	define("count=b",                             "count matched modules from search query");
	define("debug=b",                             "determine bad input line number");
	define("author=b",                            "author of the program");
	define("version=b",                           "complation info");
	define("example=b",                           "example usages");
	define("help=b",                              "short description");
}



/////////////////////////////////
//
// Tool_cint::run -- Primary interfaces to the tool.
//

bool Tool_cint::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_cint::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_cint::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);

	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}

	return status;
}

//
// In-place processing of file:
//

bool Tool_cint::run(HumdrumFile& infile) {
	processFile(infile);


	if (hasAnyText()) {
		// getAllText(cout);
	} else {
		// Re-load the text for each line from their tokens.
		cout << infile;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////
//
// NoteNode class functions:
//

NoteNode::NoteNode(const NoteNode& anode) {
	b40        = anode.b40;
	line       = anode.line;
	spine      = anode.spine;
	measure    = anode.measure;
	serial     = anode.serial;
	mark       = anode.mark;
	notemarker = anode.notemarker;
	beatsize   = anode.beatsize;
	duration   = 0;
	protected_id = anode.protected_id;
}


NoteNode& NoteNode::operator=(NoteNode& anode) {
	if (this == &anode) {
		return *this;
	}
	b40        = anode.b40;
	line       = anode.line;
	spine      = anode.spine;
	measure    = anode.measure;
	serial     = anode.serial;
	mark       = anode.mark;
	notemarker = anode.notemarker;
	beatsize   = anode.beatsize;
	duration   = anode.duration;
	protected_id = anode.protected_id;
	return *this;
}


void NoteNode::setId(const string& anid) {
	protected_id = anid;
}


NoteNode::~NoteNode(void) {
	// do nothing
}


void NoteNode::clear(void) {
	mark = measure = serial = b40 = 0;
	beatsize = 0.0;
	notemarker = "";
	line = spine = -1;
	protected_id = "";
}


string NoteNode::getId(void) {
	return protected_id;
}


///////////////////////////////////////////////////////////////////////////
//
// Tool_cint functions:
//


//////////////////////////////
//
// Tool_cint::processFile -- Do requested analysis on a given file.
//

int Tool_cint::processFile(HumdrumFile& infile) {

   initialize();

	vector<vector<NoteNode> > notes;
	vector<string> names;
	vector<int>    ktracks;
	vector<HTp>    kstarts;
	vector<int>    reverselookup;

	infile.getSpineStartList(kstarts, "**kern");
	ktracks.resize(kstarts.size());
	for (int i=0; i<(int)kstarts.size(); i++) {
		ktracks[i] = kstarts[i]->getTrack();
	}

	if (koptionQ) {
		adjustKTracks(ktracks, getString("koption"));
	}
	notes.resize(ktracks.size());
	reverselookup.resize(infile.getTrackCount()+1);
	fill(reverselookup.begin(), reverselookup.end(), -1);

	vector<vector<string> > retrospective;
	if (retroQ) {
		initializeRetrospective(retrospective, infile, ktracks);
	}

//	if (locationQ || rhythmQ || durationQ) {
//		infile.analyzeRhythm();
//	}

	int i;
	for (i=0; i<(int)ktracks.size(); i++) {
		reverselookup[ktracks[i]] = i;
		// notes[i].reserve(infile.getLineCount());
		notes[i].resize(0);
	}

	getNames(names, reverselookup, infile);
	HumRegex pre;
	extractNoteArray(notes, infile, ktracks, reverselookup);

	if (pitchesQ) {
		printPitchGrid(notes, infile);
		exit(0);
	}

	int count = 0;
	if (latticeQ) {
		printLattice(notes, infile, ktracks, reverselookup, Chaincount);
	} else if (interleavedQ) {
		printLatticeInterleaved(notes, infile, ktracks, reverselookup,
			Chaincount);
	} else if (suspensionsQ) {
		count = printCombinationsSuspensions(notes, infile, ktracks,
				reverselookup, Chaincount, retrospective);
	} else {
		count = printCombinations(notes, infile, ktracks, reverselookup,
				Chaincount, retrospective, SearchString);
	}


	// handle search results here
	if (markQ) {
		if (count > 0) {
			addMarksToInputData(infile, notes, ktracks, reverselookup);
		}
		infile.createLinesFromTokens();
		m_humdrum_text << infile;
		m_humdrum_text << "!!!RDF**kern: " << NoteMarker << " = matched note, color=\"" << MarkColor << "\"\n";
	}

	if (debugQ) {
		int j;
		for (i=0; i<(int)retrospective[0].size(); i++) {
			for (j=0; j<(int)retrospective.size(); j++) {
				m_humdrum_text << retrospective[j][i];
				if (j < (int)retrospective.size() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << "\n";
		}
	}

	return count;
}



//////////////////////////////
//
// Tool_cint::adjustKTracks -- Select only two spines to do analysis on.
//

void Tool_cint::adjustKTracks(vector<int>& ktracks, const string& koption) {
	HumRegex pre;
	if (!pre.search(koption, "(\\$|\\$?\\d*)[^\\$\\d]+(\\$|\\$?\\d*)")) {
		return;
	}
	int number1 = 0;
	int number2 = 0;
	HumRegex pre2;

	if (pre2.search(pre.getMatch(1), "\\d+")) {
		number1 = pre.getMatchInt(1);
		if (pre.getMatch(1).find('$') != string::npos) {
			number1 = (int)ktracks.size() - number1;
		}
	} else {
		number1 = (int)ktracks.size();
	}

	if (pre2.search(pre.getMatch(2), "\\d+")) {
		number2 = pre.getMatchInt(2);
		if (pre.getMatch(2).find('$') != string::npos) {
			number2 = (int)ktracks.size() - number2;
		}
	} else {
		number2 = (int)ktracks.size();
	}

	number1--;
	number2--;

	int track1 = ktracks[number1];
	int track2 = ktracks[number2];

	ktracks.resize(2);
	ktracks[0] = track1;
	ktracks[1] = track2;
}



//////////////////////////////
//
// Tool_cint::initializeRetrospective --
//

void Tool_cint::initializeRetrospective(vector<vector<string> >& retrospective,
		HumdrumFile& infile, vector<int>& ktracks) {

	int columns = (int)ktracks.size();
	columns = columns * (columns + 1) / 2; // triangle number of analysis cols.

	retrospective.resize(columns);
	int i, j;

	for (i=0; i<(int)retrospective.size(); i++) {
		retrospective[i].resize(infile.getLineCount());
	}

	string token;
	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isLocalComment()) {
			token = "!";
		} else if (infile[i].isGlobalComment()) {
			token = "!";
		} else if (infile[i].isReference()) {
			token = "!!";
		} else if (infile[i].isBarline()) {
			token = *infile.token(i, 0);
		} else if (infile[i].isData()) {
			token = ".";
		} else if (infile[i].isInterpretation()) {
			token = "*";
			if (infile[i].isExclusiveInterpretation()) {
				token = "**cint";
			}
		}

		for (j=0; j<(int)retrospective.size(); j++) {
			retrospective[j][i] = token;
		}
	}

	if (debugQ) {
		for (i=0; i<(int)retrospective[0].size(); i++) {
			for (j=0; j<(int)retrospective.size(); j++) {
				m_humdrum_text << retrospective[j][i];
				if (j < (int)retrospective.size() - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << "\n";
		}
	}
}



//////////////////////////////
//
// Tool_cint::printCombinationsSuspensions --
//
// have to do something with sbuffer
//

int  Tool_cint::printCombinationsSuspensions(vector<vector<NoteNode> >& notes,
		HumdrumFile& infile, vector<int>& ktracks, vector<int>& reverselookup,
		int n, vector<vector<string> >& retrospective) {

	string sbuffer;

	int oldcountQ = countQ;
	countQ = 1;             // mostly used to suppress intermediate output

	int countsum = 0;

	searchQ    = 1;               // turn on searching

	// Suspensions with length-2 modules
	n = 2;                        // -n 2
	xoptionQ   = 1;               // -x
	sbuffer = "";

	sbuffer += "^7xs 1 6sx -2 8xx$";
	sbuffer += "|^2sx -2 3xs 2 1xx$";
	sbuffer += "|^7xs 1 6sx 2 6xx$";
	sbuffer += "|^11xs 1 10sx -5 15xx$";
	sbuffer += "|^4xs 1 3sx -5 8xx$";
	sbuffer += "|^2sx -2 3xs 2 3xx$";

	// "9xs 1 8sx -2 10xx" archetype: Jos1405 m10 A&B
	sbuffer += "|^9xs 1 8sx -2 10xx$";
	// "4xs 1 3sx 5xx" archetype: Jos1713 m87-88 A&B
	sbuffer += "|^4xs 1 3sx -2 5xx$";
	// "11xs 1 10sx 4 8xx" archetype: Jos1402 m23-24 S&B
	sbuffer += "|^11xs 1 10sx 4 8xx$";

	countsum += printCombinations(notes, infile, ktracks, reverselookup, n,
			retrospective, sbuffer);

	// Suspensions with length-3 modules /////////////////////////////////
	n = 3;                        // -n 3
	xoptionQ   = 1;               // -x
	sbuffer = "";

	// "7xs 1 6sx 1 5sx 1 6sx" archetype: Jos2721 m27-78 S&T
	sbuffer += "^7xs 1 6sx 1 5sx 1 6sx$";
	// "7xs 1 6sx 1 6sx -2 8xx" archetype: Rue2018 m38-88 S&T
	sbuffer += "|^7xs 1 6sx 1 6sx -2 8xx$";
	// "11xs 1 10sx 1 10sx -5 15xx" archetype: Rue2018 m38-88 S&B
	sbuffer += "|^11xs 1 10sx 1 10sx -5 15xx$";

	countsum += printCombinations(notes, infile, ktracks, reverselookup, n,
							retrospective, sbuffer);

	// Suspensions with length-5 modules /////////////////////////////////
	n = 5;                        // -n 2
	xoptionQ   = 1;               // -x
	sbuffer = "";
	// "8xs 1 7sx 1 7sx 1 6sx 1 6sx 1 5sx -1 8xx" archetype: Duf3015a m94 S&T
	sbuffer += "^8xs 1 7sx 1 7sx 1 6sx 1 5sx -2 8xx$";

	countsum += printCombinations(notes, infile, ktracks, reverselookup, n,
							retrospective, sbuffer);

	// Suspensions with rests modules

	// done with multiple searches.  Mark the notes in the score if required.

	countQ = oldcountQ;

	return countsum;
}



//////////////////////////////
//
// Tool_cint::printCombinations --
//

int  Tool_cint::printCombinations(vector<vector<NoteNode> >& notes,
		HumdrumFile& infile, vector<int>& ktracks, vector<int>& reverselookup,
		int n, vector<vector<string> >& retrospective, const string& searchstring) {
	int i;
	int currentindex = 0;
	int matchcount   = 0;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			// print all lines here which do not contain spine
			// information.
			if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
				m_humdrum_text << infile[i] << "\n";
			}
			continue;
		}

		// At this point there are only four types of lines:
		//    (1) data lines
		//    (2) interpretation lines (lines starting with *)
		//    (3) local comment lines (lines starting with single !)
		//    (4) barlines

		if (infile[i].isInterpretation()) {
			string pattern = "*";
			if (infile.token(i, 0)->compare(0, 2, "**") == 0) {
				pattern = "**cint";
			} else if (*infile.token(i, 0) == "*-") {
				pattern = "*-";
			} else if (infile.token(i, 0)->compare(0, 2, "*>") == 0) {
				pattern = *infile.token(i, 0);
			}
			printAsCombination(infile, i, ktracks, reverselookup, pattern);
		} else if (infile[i].isLocalComment()) {
			printAsCombination(infile, i, ktracks, reverselookup, "!");
		} else if (infile[i].isBarline()) {
			printAsCombination(infile, i, ktracks, reverselookup, *infile.token(i, 0));
		} else {
			// print combination data
			currentindex = printModuleCombinations(infile, i, ktracks,
				reverselookup, n, currentindex, notes, matchcount, retrospective, searchstring);
		}
		if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
				m_humdrum_text << "\n";
		}
	}

	return matchcount;
}



//////////////////////////////
//
// Tool_cint::printModuleCombinations --
//

int Tool_cint::printModuleCombinations(HumdrumFile& infile, int line, vector<int>& ktracks,
		vector<int>& reverselookup, int n, int currentindex,
		vector<vector<NoteNode> >& notes, int& matchcount,
		vector<vector<string> >& retrospective, const string& searchstring) {

	int fileline = line;
	string filename = infile.getFilename();

	while ((currentindex < (int)notes[0].size())
			&& (fileline > notes[0][currentindex].line)) {
		currentindex++;
	}
	if (currentindex >= (int)notes[0].size()) {
		if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
			m_humdrum_text << ".";
			printAsCombination(infile, line, ktracks, reverselookup, ".");
		}
		return currentindex;
	}
	if (notes[0][currentindex].line != fileline) {
		// This section occurs when two voices are both sustaining
		// at the start of the module.  Print a "." to indicate that
		// the counterpoint module is continuing from a previous line.
		printAsCombination(infile, line, ktracks, reverselookup, ".");
		return currentindex;
	}

	// found the index into notes which matches to the current fileline.
	if (currentindex + n >= (int)notes[0].size()) {
		// asking for chain longer than rest of available data.
		printAsCombination(infile, line, ktracks, reverselookup, ".");
		return currentindex;
	}

	// printAsCombination(infile, line, ktracks, reverselookup, ".");
	// return currentindex;

	int tracknext;
	int track;
	int j, jj;
	int count = 0;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}
		track = infile.token(line, j)->getTrack();
		if (j < infile[line].getFieldCount() - 1) {
			tracknext = infile.token(line, j+1)->getTrack();
		} else {
			tracknext = -23525;
		}
		if (track == tracknext) {
			if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}

		// print the **kern spine, then check to see if there
		// is some **cint data to print
		if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
				m_humdrum_text << infile.token(line, j);
		}
		if ((track != ktracks.back()) && (reverselookup[track] >= 0)) {
			count = (int)ktracks.size() - reverselookup[track] - 1;
			for (jj = 0; jj<count; jj++) {
				if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
					m_humdrum_text << "\t";
				}
				int part1 = reverselookup[track];
				int part2 = part1+1+jj;
				// m_humdrum_text << part1 << "," << part2;
				matchcount += printCombinationModulePrepare(m_humdrum_text, filename,
						notes, n, currentindex, part1, part2, retrospective, infile,
						searchstring);
			}
		}

		if (!(raw2Q || rawQ || markQ || retroQ || countQ)) {
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
	}

	return currentindex;
}



//////////////////////////////
//
// Tool_cint::printCombinationModulePrepare --
//

int Tool_cint::printCombinationModulePrepare(ostream& out, const string& filename,
		 vector<vector<NoteNode> >& notes, int n, int startline, int part1,
		 int part2, vector<vector<string> >& retrospective,
		HumdrumFile& infile, const string& searchstring) {
	int count = 0;
	HumRegex hre;
	stringstream tempstream;
	int match;
	string notemarker;
// ggg
	int status = printCombinationModule(tempstream, filename, notes,
			n, startline, part1, part2, retrospective, notemarker);
	if (status) {
		if (raw2Q || rawQ) {
			tempstream << "\n";
		}
		if ((!NoteMarker.empty()) && (notemarker == NoteMarker)) {
			out << NoteMarker;
		}
		if (searchQ) {
			// Check to see if the extracted module matches to the
			// search query.
			match = hre.search(tempstream.str(), searchstring);
			if (match) {
				count++;
				if (locationQ) {
					int line = notes[0][startline].line;
					double loc = infile[line].getDurationFromStart().getFloat() /
							infile[infile.getLineCount()-1].getDurationFromStart().getFloat();
					loc = int(100.0 * loc + 0.5)/100.0;
					m_humdrum_text << "!!LOCATION:"
							<< "\t"  << loc
							<< "\tm" << getMeasure(infile, line)
							<< "\tv" << ((int)notes.size() - part2)
							<< ":v"  << ((int)notes.size() - part1)
							<< "\t"  << infile.getFilename()
							<< endl;
				}
				if (raw2Q || rawQ) {
					out << tempstream.str();
					// newline already added somewhere previously.
					// m_humdrum_text << "\n";
				} else {
					// mark notes of the matched module(s) in the note array
					// for later marking in input score.
					status = printCombinationModule(tempstream, filename,
						 notes, n, startline, part1, part2, retrospective,
						 notemarker, MARKNOTES);
					if (status && (raw2Q || rawQ)) {
						tempstream << "\n";
					}
				}

			}
		} else {
			if (retroQ) {
				int column = getTriangleIndex((int)notes.size(), part1, part2);
				retrospective[column][status] = tempstream.str();
			} else {
				out << tempstream.str();
			}
		}
	} else {
		if (!(raw2Q || rawQ || markQ || retroQ || countQ || searchQ)) {
			out << ".";
		}
	}

	return count;
}



//////////////////////////////
//
// Tool_cint::getMeasure -- return the last measure number of the given line index.
//

int Tool_cint::getMeasure(HumdrumFile& infile, int line) {
	int measure = 0;
	HumRegex hre;

	for (int i=line; i>=0; i--) {
		if (!infile[i].isBarline()) {
			continue;
		}
		if (hre.search(*infile.token(i, 0), "=(\\d+)")) {
			measure = hre.getMatchInt(1);
			return measure;
		}
	}
	return 0;
}



//////////////////////////////
//
// Tool_cint::getTriangleIndex --
//

int Tool_cint::getTriangleIndex(int number, int num1, int num2) {
	// int triangle = number * (number + 1) / 2;
	// intermediate code, not active yet
	return 0;
}



//////////////////////////////
//
// Tool_cint::addMarksToInputData -- mark notes in the score which matched
//     to the search query.
//

void Tool_cint::addMarksToInputData(HumdrumFile& infile,
		vector<vector<NoteNode> >& notes, vector<int>& ktracks,
		vector<int>& reverselookup) {

	// first carry all marks from sustained portions of notes onto their
	// note attacks.
	int i, j;

	int mark = 0;
	int track = 0;
	int markpitch = -1;

	for (i=0; i<(int)notes.size(); i++) {
		mark = 0;
		for (j=(int)notes[i].size()-1; j>=0; j--) {
			if (mark && (-markpitch == notes[i][j].b40)) {
				// In the sustain region between a note
				// attack and the marked sustain. Mark the
				// sustained region as well (don't know
				// if this behavior might change in the
				// future.
				notes[i][j].mark = mark;
				continue;
			}
			if (mark && (markpitch == notes[i][j].b40)) {
				// At the start of a notes which was marked.
				// Mark the attack since only note attacks
				// will be marked in the score
				notes[i][j].mark = mark;
				mark = 0;
				continue;
			}
			if (mark && (markpitch != notes[i][j].b40)) {
				// something strange happened.  Probably
				// an open tie which was not started
				// properly, so just clear mark.
				mark = 0;
			}
			if (notes[i][j].mark) {
				mark = 1;
				markpitch = abs(notes[i][j].b40);
			} else {
				mark = 0;
			}

		}
	}

	// a forward loop here into notes array to continue
	// marks to end of sutained region of marked notes
	for (i=0; i<(int)notes.size(); i++)  {
		for (j=0; j<(int)notes[i].size(); j++) {
			if (notes[i][j].mark) {
				markpitch = -abs(notes[i][j].b40);
				continue;
			} else if (notes[i][j].b40 == markpitch) {
				notes[i][j].mark = 1;
				continue;
			} else {
				markpitch = -1;
			}
		}
	}

	// print mark information:
	// for (j=0; j<(int)notes[0].size(); j++) {
	//    for (i=0; i<(int)notes.size(); i++) {
	//       m_humdrum_text << notes[i][j].b40;
	//       if (notes[i][j].mark) {
	//          m_humdrum_text << "m";
	//       }
	//       m_humdrum_text << " ";
	//    }
	//    m_humdrum_text << "\n";
	// }


	// now go through the input score placing user-markers onto notes
	// which were marked in the note array.
	int currentindex = 0;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		while ((currentindex < (int)notes[0].size())
				&& (i > notes[0][currentindex].line)) {
			currentindex++;
		}
		if (currentindex >= (int)notes[0].size()) {
			continue;
		}
		if (notes[0][currentindex].line != i) {
			continue;
		}

		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (*infile.token(i, j) == ".") {
				// Don't mark null tokens.
				continue;
			}
			track = infile.token(i, j)->getTrack();
			if (reverselookup[track] < 0) {
				continue;
			}
			if (notes[reverselookup[track]][currentindex].mark != 0) {
				markNote(infile, i, j);
			}
		}
	}
}



//////////////////////////////
//
// Tool_cint::markNote --
//

void Tool_cint::markNote(HumdrumFile& infile, int line, int col) {
	HTp token = infile.token(line, col);
	string text = *token;
	text += NoteMarker;
	token->setText(text);
}



//////////////////////////////
//
// Tool_cint::getOctaveAdjustForCombinationModule -- Find the minim harmonic interval in
//      the module chain.  If it is greater than an octave, then move it down
//      below an octave.  If the minimum is an octave, then don't do anything.
//      Not considering crossed voices.
//

int Tool_cint::getOctaveAdjustForCombinationModule(vector<vector<NoteNode> >& notes, int n,
		int startline, int part1, int part2) {

	// if the current two notes are both sustains, then skip
	if ((notes[part1][startline].b40 <= 0) &&
		 (notes[part2][startline].b40 <= 0)) {
		return 0;
	}

	if (norestsQ) {
		if (notes[part1][startline].b40 == 0) {
			return 0;
		}
		if (notes[part2][startline].b40 == 0) {
			return 0;
		}
	}

	int i;
	int count = 0;
	int attackcount = 0;
	int hint;

	vector<int> hintlist;
	hintlist.reserve(1000);

	for (i=startline; i<(int)notes[0].size(); i++) {
		if ((notes[part1][i].b40 <= 0) && (notes[part2][i].b40 <= 0)) {
			// skip notes if both are sustained
			continue;
		}

		if (attackQ && ((notes[part1][i].b40 <= 0) ||
							 (notes[part2][i].b40 <= 0))) {
			if (attackcount == 0) {
				// not at the start of a pair of attacks.
				return 0;
			}
		}

		// consider  harmonic interval
		if ((notes[part2][i].b40 != 0) && (notes[part1][i].b40 != 0)) {
			hint = abs(notes[part2][i].b40) - abs(notes[part1][i].b40);
			if (uncrossQ && (hint < 0)) {
				hint = -hint;
			}
			hintlist.push_back(hint);
		}

		// if count matches n, then exit loop
		if ((count == n) && !attackQ) {
			break;
		}
		count++;

		if ((notes[part1][i].b40 > 0) && (notes[part2][i].b40 > 0)) {
			// keep track of double attacks
			if (attackcount >= n) {
				break;
			} else {
				attackcount++;
			}
		}

	}

	int minimum = 100000;

	for (i=0; i<(int)hintlist.size(); i++) {
		if (hintlist[i] < minimum) {
			minimum = hintlist[i];
		}
	}

	if (minimum > 1000) {
	  // no intervals found to consider
	  return 0;
	}

	if ((minimum >= 0) && (minimum <= 40)) {
		// nothing to do
		return 0;
	}

	if (minimum > 40) {
		return -(minimum/40);
	} else if (minimum < 0) {
		// don't go positive, this will invert the interval.
		return (-minimum)/40;
	}

	//int octaveadjust = -(minimum / 40);

	//if (attackQ && (attackcount == n)) {
	//   return octaveadjust;
	//} else if (count == n) {
	//   return octaveadjust;
	//} else {
	//   // did not find the required number of modules.
	//   return 0;
	//}

	return 0;
}



//////////////////////////////
//
// Tool_cint::printCombinationModule -- Similar to printLatticeModule, but harmonic
//      intervals will not be triggered by a pair of sustained notes.
//      Print a counterpoint module or module chain given the start notes
//      and pair of parts to calculate the module (chains) from.  Will not
//      print anything if the chain length is longer than the note array.
//      The n parameter will be ignored if --attacks option is used
//      (--attacks will gnereate a variable length module chain).
//

int Tool_cint::printCombinationModule(ostream& out, const string& filename,
		vector<vector<NoteNode> >& notes, int n, int startline, int part1,
		int part2, vector<vector<string> >& retrospective, string& notemarker,
		int markstate) {

	notemarker = "";

	if (norestsQ) {
		if (notes[part1][startline].b40 == 0) {
			return 0;
		}
		if (notes[part2][startline].b40 == 0) {
			return 0;
		}
	}

	stringstream idstream;

	// int crossing =  0;
	//int oldcrossing =  0;

	int octaveadjust = 0;   // used for -o option
	if (octaveQ) {
		octaveadjust = getOctaveAdjustForCombinationModule(notes, n, startline,
				part1, part2);
	}

	ostream *outp = &out;
	// if (rawQ && !searchQ) {
	//    outp = &m_humdrum_text;
	// }

	if (n + startline >= (int)notes[0].size()) { // [20150202]
		// definitely nothing to do
		return 0;
	}

	if ((int)notes.size() == 0) {
		// nothing to do
		return 0;
	}

	// if the current two notes are both sustains, then skip
	if ((notes[part1][startline].b40 <= 0) &&
		 (notes[part2][startline].b40 <= 0)) {
		return 0;
	}

	if (raw2Q) {
		// print pitch of first bottom note
		if (filenameQ) {
			(*outp) << "file_" << filename;
			(*outp) << " ";
		}

		(*outp) << "v_" << part1 << " v_" << part2 << " ";

		if (base12Q) {
			(*outp) << "base12_";
			(*outp) << Convert::base40ToMidiNoteNumber(abs(notes[part1][startline].b40));
		} else if (base40Q) {
			(*outp) << "base40_";
			(*outp) << abs(notes[part1][startline].b40);
		} else {
			(*outp) << "base7_";
			(*outp) << Convert::base40ToDiatonic(abs(notes[part1][startline].b40));
		}
		(*outp) << " ";
	}

	if (parenQ) {
		(*outp) << "(";
	}

	int i;
	int count = 0;
	int countm = 0;
	int attackcount = 0;
	int idstart = 0;

	int lastindex = -1;
	int retroline = 0;

	for (i=startline; i<(int)notes[0].size(); i++) {
		if ((notes[part1][i].b40 <= 0) && (notes[part2][i].b40 <= 0)) {
			// skip notes if both are sustained
			continue;
		}

		if (norestsQ) {
			if (notes[part1][i].b40 == 0) {
				return 0;
			}
			if (notes[part2][i].b40 == 0) {
				return 0;
			}
		}

		if (attackQ && ((notes[part1][i].b40 <= 0) ||
							 (notes[part2][i].b40 <= 0))) {
			if (attackcount == 0) {
				// not at the start of a pair of attacks.
				return 0;
			}
		}

		// print the melodic intervals (if not the first item in chain)
		if ((count > 0) && !nomelodicQ) {
			if (mparenQ) {
				(*outp) << "{";
			}

			if (nounisonsQ) {
				// suppress modules which contain melodic perfect unisons:
				if ((notes[part1][i].b40 != 0) &&
					(abs(notes[part1][i].b40) == abs(notes[part1][lastindex].b40))) {
					return 0;
				}
				if ((notes[part2][i].b40 != 0) &&
					(abs(notes[part2][i].b40) == abs(notes[part2][lastindex].b40))) {
					return 0;
				}
			}
			// bottom melodic interval:
			if (!toponlyQ) {
				printInterval((*outp), notes[part1][lastindex],
						notes[part1][i], INTERVAL_MELODIC);
				if (mmarkerQ) {
					(*outp) << "m";
				}
			}

			// print top melodic interval here if requested
			if (topQ || toponlyQ) {
				if (!toponlyQ) {
					printSpacer((*outp));
				}
				// top melodic interval:
				printInterval((*outp), notes[part2][lastindex],
						notes[part2][i], INTERVAL_MELODIC);
				if (mmarkerQ) {
					(*outp) << "m";
				}
			}

			if (mparenQ) {
				(*outp) << "}";
			}
			printSpacer((*outp));
		}

		countm++;

		// print harmonic interval
		if (!noharmonicQ) {
			if (hparenQ) {
			  (*outp) << "[";
			}
			if (markstate) {
				notes[part1][i].mark = 1;
				notes[part2][i].mark = 1;
			} else {
				// oldcrossing = crossing;
				//crossing = printInterval((*outp), notes[part1][i],
				//      notes[part2][i], INTERVAL_HARMONIC, octaveadjust);
				printInterval((*outp), notes[part1][i],
						notes[part2][i], INTERVAL_HARMONIC, octaveadjust);
			}

			if (durationQ) {
				if (notes[part1][i].isAttack()) {
					(*outp) << "D" << notes[part1][i].duration;
				}
				if (notes[part2][i].isAttack()) {
					(*outp) << "d" << notes[part1][i].duration;
				}
			}

			if (hmarkerQ) {
				(*outp) << "h";
			}
			if (hparenQ) {
			  (*outp) << "]";
			}
		}

		// prepare the ids string if requested
		if (idQ) {
		//   if (count == 0) {
				// insert both first two notes, even if sustain.
				if (idstart != 0) { idstream << ':'; }
				idstart++;
				idstream << notes[part1][i].getId() << ':'
							<< notes[part2][i].getId();
		//   } else {
		//      // only insert IDs if an attack
		//      if (notes[part1][i].b40 > 0) {
		//         if (idstart != 0) { idstream << ':'; }
		//         idstart++;
		//         idstream << notes[part1][i].getId();
		//      }
		//      if (notes[part2][i].b40 > 0) {
		//         if (idstart != 0) { idstream << ':'; }
		//         idstart++;
		//         idstream << notes[part2][i].getId();
		//      }
		//   }
		}

		// keep track of notemarker state
		if (notes[part1][i].notemarker == NoteMarker) {
			notemarker = NoteMarker;
		}
		if (notes[part2][i].notemarker == NoteMarker) {
			notemarker = NoteMarker;
		}

		// if count matches n, then exit loop
		if ((count == n) && !attackQ) {
			retroline = i;
			break;
		} else {
			if (!noharmonicQ) {
				printSpacer((*outp));
			}
		}
		lastindex = i;
		count++;

		if ((notes[part1][i].b40 > 0) && (notes[part2][i].b40 > 0)) {
			// keep track of double attacks
			if (attackcount >= n) {
				retroline = i;
				break;
			} else {
				attackcount++;
			}
		}

	}

	if (parenQ) {
		(*outp) << ")";
	}

	if (idQ && idstart) {
		idstream << ends;
		(*outp) << " ID:" << idstream.str();
	}

	if (attackQ && (attackcount == n)) {
		return retroline;
	} else if ((countm>1) && (count == n)) {
		return retroline;
	} else if (n == 0) {
		return retroline;
	} else {
		// did not print the required number of modules.
		return 0;
	}

	return 0;
}



//////////////////////////////
//
// Tool_cint::printAsCombination --
//

void Tool_cint::printAsCombination(HumdrumFile& infile, int line, vector<int>& ktracks,
	 vector<int>& reverselookup, const string& interstring) {

	if (raw2Q || rawQ || markQ || retroQ || countQ) {
		return;
	}

	vector<int> done(ktracks.size(), 0);
	int track;
	int tracknext;
	int count;

	int j, jj;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			m_humdrum_text << infile.token(line, j);
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		track = infile.token(line, j)->getTrack();
		if (j < infile[line].getFieldCount() - 1) {
			tracknext = infile.token(line, j+1)->getTrack();
		} else {
			tracknext = -23525;
		}
		if (track == tracknext) {
			m_humdrum_text << infile.token(line, j);
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
			continue;
		}

		// print the **kern spine, then check to see if there
		// is some **cint data to print
		// ggg
		m_humdrum_text << infile.token(line, j);

		if (reverselookup[track] >= 0) {
			count = (int)ktracks.size() - reverselookup[track] - 1;
			for (jj=0; jj<count; jj++) {
				m_humdrum_text << "\t" << interstring;
			}
		}

		if (j < infile[line].getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
}



//////////////////////////////
//
// Tool_cint::printLatticeInterleaved --
//

void Tool_cint::printLatticeInterleaved(vector<vector<NoteNode> >& notes,
		HumdrumFile& infile, vector<int>& ktracks, vector<int>& reverselookup,
		int n) {
	int currentindex = 0;
	int i;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			// print all lines here which do not contain spine
			// information.
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << infile[i] << "\n";
			}
			continue;
		}

		// At this point there are only four types of lines:
		//    (1) data lines
		//    (2) interpretation lines (lines starting with *)
		//    (3) local comment lines (lines starting with single !)
		//    (4) barlines

		if (infile[i].isInterpretation()) {
			string pattern = "*";
			if (infile.token(i, 0)->compare(0, 2, "**") == 0) {
				pattern = "**cint";
			} else if (infile.token(i, 0)->compare("*-") == 0) {
				pattern = "*-";
			} else if (infile.token(i, 0)->compare(0, 2, "*>") == 0) {
				pattern = *infile.token(i, 0);
			}
			printInterleaved(infile, i, ktracks, reverselookup, pattern);
		} else if (infile[i].isLocalComment()) {
			printInterleaved(infile, i, ktracks, reverselookup, "!");
		} else if (infile[i].isBarline()) {
			printInterleaved(infile, i, ktracks, reverselookup, *infile.token(i, 0));
		} else {
			// print interleaved data
			currentindex = printInterleavedLattice(infile, i, ktracks,
				reverselookup, n, currentindex, notes);
		}
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << "\n";
		}
	}
}



//////////////////////////////
//
// Tool_cint::printInterleavedLattice --
//

int Tool_cint::printInterleavedLattice(HumdrumFile& infile, int line, vector<int>& ktracks,
		vector<int>& reverselookup, int n, int currentindex,
		vector<vector<NoteNode> >& notes) {

	int fileline = line;

	while ((currentindex < (int)notes[0].size())
			&& (fileline > notes[0][currentindex].line)) {
		currentindex++;
	}
	if (currentindex >= (int)notes[0].size()) {
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << ".";
			printInterleaved(infile, line, ktracks, reverselookup, ".");
		}
		return currentindex;
	}
	if (notes[0][currentindex].line != fileline) {
		// should never get here.
		printInterleaved(infile, line, ktracks, reverselookup, "?");
		return currentindex;
	}

	// found the index into notes which matches to the current fileline.
	if (currentindex + n >= (int)notes[0].size()) {
		// asking for chain longer than rest of available data.
		printInterleaved(infile, line, ktracks, reverselookup, ".");
		return currentindex;
	}

	int tracknext;
	int track;
	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}
		track = infile.token(line, j)->getTrack();
		if (j < infile[line].getFieldCount() - 1) {
			tracknext = infile.token(line, j+1)->getTrack();
		} else {
			tracknext = -23525;
		}
		if (track == tracknext) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}

		// print the **kern spine, then check to see if there
		// is some **cint data to print
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << infile.token(line, j);
		}
		if ((track != ktracks.back()) && (reverselookup[track] >= 0)) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t";
			}
			int part1 = reverselookup[track];
			int part2 = part1+1;
			// m_humdrum_text << part1 << "," << part2;
			printLatticeModule(m_humdrum_text, notes, n, currentindex, part1, part2);
		}

		if (!(rawQ || raw2Q)) {
			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
	}

	return currentindex;
}



//////////////////////////////
//
// Tool_cint::printInterleaved --
//

void Tool_cint::printInterleaved(HumdrumFile& infile, int line, vector<int>& ktracks,
	 vector<int>& reverselookup, const string& interstring) {

	vector<int> done(ktracks.size(), 0);
	int track;
	int tracknext;

	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (!infile.token(line, j)->isKern()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}
		track = infile.token(line, j)->getTrack();
		if (j < infile[line].getFieldCount() - 1) {
			tracknext = infile.token(line, j+1)->getTrack();
		} else {
			tracknext = -23525;
		}
		if (track == tracknext) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << infile.token(line, j);
				if (j < infile[line].getFieldCount() - 1) {
					m_humdrum_text << "\t";
				}
			}
			continue;
		}

		// print the **kern spine, then check to see if there
		// is some **cint data to print
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << infile.token(line, j);

			if ((track != ktracks.back()) && (reverselookup[track] >= 0)) {
				m_humdrum_text << "\t" << interstring;
			}

			if (j < infile[line].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
	}
}



//////////////////////////////
//
// Tool_cint::printLattice --
//

void Tool_cint::printLattice(vector<vector<NoteNode> >& notes, HumdrumFile& infile,
		vector<int>& ktracks, vector<int>& reverselookup, int n) {

	int i;
	int ii = 0;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << infile[i];
		}
		HTp ltok = infile.token(i, 0);
		if (ltok->compare(0, 2, "**") == 0) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t**cint\n";
			}
		} else if (*ltok == "*-") {
				m_humdrum_text << "\t*-\n";
		} else if (infile[i].isData()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t";
			}
			if (rowsQ) {
				ii = printLatticeItemRows(notes, n, ii, i);
			} else {
				ii = printLatticeItem(notes, n, ii, i);
			}
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\n";
			}
		} else if (infile[i].isBarline()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t" << ltok << "\n";
			}
		} else if (infile[i].isInterpretation()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t*\n";
			}
		} else if (infile[i].isLocalComment()) {
			if (!(rawQ || raw2Q)) {
				m_humdrum_text << "\t!\n";
			}
		} else {
			// null, global comment or reference record
			m_humdrum_text << "\n";
		}
	}

}


//////////////////////////////
//
// Tool_cint::printLatticeModule -- print a counterpoint module or module chain given
//      the start notes and pair of parts to calculate the module
//      (chains) from.  Will not print anything if the chain length
//      is longer than the note array.
//

int Tool_cint::printLatticeModule(ostream& out, vector<vector<NoteNode> >& notes, int n,
		int startline, int part1, int part2) {

	if (n + startline >= (int)notes[0].size()) {
		return 0;
	}

	if (parenQ) {
		out << "(";
	}

	int i;
	for (i=0; i<n; i++) {
		// print harmonic interval
		if (hparenQ) {
			out << "[";
		}
		printInterval(out, notes[part1][startline+i],
			notes[part2][startline+i], INTERVAL_HARMONIC);
		if (hmarkerQ) {
			out << "h";
		}
		if (hparenQ) {
			out << "]";
		}
		printSpacer(out);

		// print melodic interal(s)
		if (mparenQ) {
			out << "{";
		}
		// bottom melodic interval:
		if (!toponlyQ) {
			printInterval(out, notes[part1][startline+i],
							  notes[part1][startline+i+1], INTERVAL_MELODIC);
		}

		// print top melodic interval here if requested
		if (topQ || toponlyQ) {
			if (!toponlyQ) {
				printSpacer(out);
			}
			// top melodic interval:
			printInterval(out, notes[part2][startline+i],
							  notes[part2][startline+i+1], INTERVAL_MELODIC);
			if (mmarkerQ) {
				out << "m";
			}
		}

		if (mparenQ) {
			out << "}";
		}
		printSpacer(out);
	}

	// print last harmonic interval
	if (hparenQ) {
	  out << "[";
	}
	printInterval(out, notes[part1][startline+n],
			notes[part2][startline+n], INTERVAL_HARMONIC);
	if (hmarkerQ) {
		out << "h";
	}
	if (hparenQ) {
	  out << "]";
	}

	if (parenQ) {
		out << ")";
	}

	return 1;
}



//////////////////////////////
//
// Tool_cint::printLatticeItemRows -- Row form of the lattice.
//

int Tool_cint::printLatticeItemRows(vector<vector<NoteNode> >& notes, int n,
		int currentindex, int fileline) {

	while ((currentindex < (int)notes[0].size())
			&& (fileline > notes[0][currentindex].line)) {
		currentindex++;
	}
	if (currentindex >= (int)notes[0].size()) {
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << ".";
		}
		return currentindex;
	}
	if (notes[0][currentindex].line != fileline) {
		// should never get here.
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << "?";
		}
		return currentindex;
	}

	// found the index into notes which matches to the current fileline.
	if (currentindex + n >= (int)notes[0].size()) {
		// asking for chain longer than rest of available data.
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << ".";
		}
		return currentindex;
	}

	stringstream tempstream;
	int j;
	int counter = 0;

	for (j=0; j<(int)notes.size()-1; j++) {
		// iterate through each part, printing the module
		// for adjacent parts.
		counter += printLatticeModule(tempstream, notes, n, currentindex, j, j+1);
		if (j < (int)notes.size()-2) {
			printSpacer(tempstream);
		}
	}

	if (!(rawQ || raw2Q)) {
		if (counter == 0) {
			m_humdrum_text << ".";
		} else {
			m_humdrum_text << tempstream.str();
		}
	}

	return currentindex;
}



//////////////////////////////
//
// Tool_cint::printLatticeItem --
//

int Tool_cint::printLatticeItem(vector<vector<NoteNode> >& notes, int n, int currentindex,
		int fileline) {
	while ((currentindex < (int)notes[0].size())
			&& (fileline > notes[0][currentindex].line)) {
		currentindex++;
	}
	if (currentindex >= (int)notes[0].size()) {
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << ".";
		}
		return currentindex;
	}
	if (notes[0][currentindex].line != fileline) {
		// should never get here.
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << "??";
		}
		return currentindex;
	}

	// found the index into notes which matches to the current fileline.
	if (currentindex + n >= (int)notes[0].size()) {
		// asking for chain longer than rest of available data.
		if (!(rawQ || raw2Q)) {
			m_humdrum_text << ".";
		}
		return currentindex;
	}

	int count;
	int melcount;
	int j;
	if (parenQ) {
		m_humdrum_text << "(";
	}
	for (count = 0; count < n; count++) {
		// print harmonic intervals
		if (hparenQ) {
			m_humdrum_text << "[";
		}
		for (j=0; j<(int)notes.size()-1; j++) {
			printInterval(m_humdrum_text, notes[j][currentindex+count],
					notes[j+1][currentindex+count], INTERVAL_HARMONIC);
			if (j < (int)notes.size()-2) {
				printSpacer(m_humdrum_text);
			}
		}
		if (hparenQ) {
			m_humdrum_text << "]";
		}
		printSpacer(m_humdrum_text);

		// print melodic intervals
		if (mparenQ) {
			m_humdrum_text << "{";
		}
		melcount = (int)notes.size()-1;
		if (topQ) {
			melcount++;
		}
		for (j=0; j<melcount; j++) {
			printInterval(m_humdrum_text, notes[j][currentindex+count],
					notes[j][currentindex+count+1], INTERVAL_MELODIC);
			if (j < melcount-1) {
				printSpacer(m_humdrum_text);
			}
		}
		if (mparenQ) {
			m_humdrum_text << "}";
		}
		printSpacer(m_humdrum_text);

	}
	// print last sequence of harmonic intervals
	if (hparenQ) {
		m_humdrum_text << "[";
	}
	for (j=0; j<(int)notes.size()-1; j++) {
		printInterval(m_humdrum_text, notes[j][currentindex+n],
				notes[j+1][currentindex+n], INTERVAL_HARMONIC);
		if (j < (int)notes.size()-2) {
			printSpacer(m_humdrum_text);
		}
	}
	if (hparenQ) {
		m_humdrum_text << "]";
	}
	if (parenQ) {
		m_humdrum_text << ")";
	}

	if ((rawQ || raw2Q)) {
		m_humdrum_text << "\n";
	}

	return currentindex;
}



//////////////////////////////
//
// Tool_cint::printInterval --
//

int Tool_cint::printInterval(ostream& out, NoteNode& note1, NoteNode& note2,
		int type, int octaveadjust) {
	if ((note1.b40 == REST) || (note2.b40 == REST)) {
		out << RESTSTRING;
		return 0;
	}
	int cross = 0;
	int pitch1 = abs(note1.b40);
	int pitch2 = abs(note2.b40);
	int interval = pitch2 - pitch1;

	if ((type == INTERVAL_HARMONIC) && (interval < 0)) {
		cross = 1;
		if (uncrossQ) {
			interval = -interval;
		}
	} else {
		interval = interval + octaveadjust  * 40;
	}

	if ((type == INTERVAL_HARMONIC) && (octaveallQ)) {
		if (interval <= -40) {
			interval = interval + 4000;
		}
		if (interval > 40) {
			if (interval % 40 == 0) {
				interval = 40;
			} else {
				interval = interval % 40;
			}
		} else if (interval < 0) {
			interval = interval + 40;
		}
	}
	if (base12Q && !chromaticQ) {
		interval = Convert::base40ToMidiNoteNumber(interval + 40*4 + 2) - 12*5;
		if ((type == INTERVAL_HARMONIC) && (octaveallQ)) {
			if (interval <= -12) {
				interval = interval + 1200;
			}
			if (interval > 12) {
				if (interval % 12 == 0) {
					interval = 12;
				} else {
					interval = interval % 12;
				}
			} else if (interval < 0) {
				interval = interval + 12;
			}
		}
		interval = interval + octaveadjust  * 12;
	} else if (base7Q && !chromaticQ) {
		interval = Convert::base40ToDiatonic(interval + 40*4 + 2) - 7*4;
		if ((type == INTERVAL_HARMONIC) && (octaveallQ)) {
			if (interval <= -7) {
				interval = interval + 700;
			}
			if (interval > 7) {
				if (interval % 7 == 0) {
					interval = 7;
				} else {
					interval = interval % 7;
				}
			} else if (interval < 0) {
				interval = interval + 7;
			}
		}
		interval = interval + octaveadjust  * 7;
	}


	if (chromaticQ) {
		out << Convert::base40ToIntervalAbbr(interval);
	} else {
		int negative = 1;
		if (interval < 0) {
			negative = -1;
			interval = -interval;
		}
		if (base7Q && !zeroQ) {
			out << negative * (interval+1);
		} else {
			out << negative * interval;
		}
	}

	if (sustainQ || ((type == INTERVAL_HARMONIC) && xoptionQ)) {
		// print sustain/attack information of intervals.
		if (note1.b40 < 0) {
			out << "s";
		} else {
			out << "x";
		}
		if (note2.b40 < 0) {
			out << "s";
		} else {
			out << "x";
		}
	}

	return cross;
}



//////////////////////////////
//
// Tool_cint::printSpacer -- space or comma...
//

void Tool_cint::printSpacer(ostream& out) {
	out << Spacer;
}



//////////////////////////////
//
// Tool_cint::printPitchGrid -- print the pitch grid from which all counterpoint
//      modules are calculated.
//

void Tool_cint::printPitchGrid(vector<vector<NoteNode> >& notes, HumdrumFile& infile) {
	int i = 0;
	int j = 0;
	int pitch;
	int abspitch;
	int newpitch;
	int partcount;
	int line;
	double beat;

	if (base40Q) {
		partcount = (int)notes.size();

		if (rhythmQ) {
			m_humdrum_text << "**absq\t";
			m_humdrum_text << "**bar\t";
			m_humdrum_text << "**beat\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "**b40";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
		for (i=0; i<(int)notes[0].size(); i++) {
			if (rhythmQ) {
				line = notes[0][i].line;
				beat = infile[line].getDurationFromBarline().getFloat() * notes[0][i].beatsize + 1;
				m_humdrum_text << infile[line].getDurationFromStart().getFloat() << "\t";
				m_humdrum_text << notes[0][i].measure << "\t";
				m_humdrum_text << beat << "\t";
			}
			for (j=0; j<(int)notes.size(); j++) {
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				m_humdrum_text << notes[j][i].b40;
				if (j < (int)notes.size()-1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
		if (rhythmQ) {
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "*-";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	} else if (base7Q) {
		partcount = (int)notes.size();

		if (rhythmQ) {
			m_humdrum_text << "**absq\t";
			m_humdrum_text << "**bar\t";
			m_humdrum_text << "**beat\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "**b7";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;

		for (i=0; i<(int)notes[0].size(); i++) {
			if (rhythmQ) {
				line = notes[0][i].line;
				beat = infile[line].getDurationFromBarline().getFloat() * notes[0][i].beatsize + 1;
				m_humdrum_text << infile[line].getDurationFromStart().getFloat() << "\t";
				m_humdrum_text << notes[0][i].measure << "\t";
				m_humdrum_text << beat << "\t";
			}
			for (j=0; j<(int)notes.size(); j++) {
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				pitch = notes[j][i].b40;
				abspitch = abs(pitch);
				if (pitch == 0) {
					// print rest
					m_humdrum_text << 0;
				} else {
					newpitch = Convert::base40ToDiatonic(abspitch);
					if (pitch < 0) {
						newpitch = -newpitch;
					}
					m_humdrum_text << newpitch;
				}
				if (j < (int)notes.size()-1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
		if (rhythmQ) {
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "*-";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	} else if (base12Q) {
		partcount = (int)notes.size();

		if (rhythmQ) {
			m_humdrum_text << "**absq\t";
			m_humdrum_text << "**bar\t";
			m_humdrum_text << "**beat\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "**b12";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;

		for (i=0; i<(int)notes[0].size(); i++) {
			if (rhythmQ) {
				line = notes[0][i].line;
				beat = infile[line].getDurationFromBarline().getFloat() * notes[0][i].beatsize + 1;
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				m_humdrum_text << infile[line].getDurationFromStart() << "\t";
				m_humdrum_text << notes[0][i].measure << "\t";
				m_humdrum_text << beat << "\t";
			}
			for (j=0; j<(int)notes.size(); j++) {
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				pitch = notes[j][i].b40;
				if (pitch == 0) {
					// print rest
					m_humdrum_text << 0;
				} else {
					abspitch = abs(pitch);
					newpitch = Convert::base40ToMidiNoteNumber(abspitch);
					if (pitch < 0) {
						newpitch = -newpitch;
					}
					m_humdrum_text << newpitch;
				}
				if (j < (int)notes.size()-1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
		if (rhythmQ) {
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "*-";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	} else {
		// print as Humdrum **kern data
		partcount = (int)notes.size();

		if (rhythmQ) {
			m_humdrum_text << "**absq\t";
			m_humdrum_text << "**bar\t";
			m_humdrum_text << "**beat\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "**kern";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;

		for (i=0; i<(int)notes[0].size(); i++) {
			if (rhythmQ) {
				line = notes[0][i].line;
				beat = infile[line].getDurationFromBarline().getFloat() * notes[0][i].beatsize + 1;
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				m_humdrum_text << infile[line].getDurationFromStart() << "\t";
				m_humdrum_text << notes[0][i].measure << "\t";
				m_humdrum_text << beat << "\t";
			}
			for (j=0; j<(int)notes.size(); j++) {
				if (!notes[j][i].notemarker.empty()) {
					m_humdrum_text << notes[j][i].notemarker;
				}
				pitch = notes[j][i].b40;
				abspitch = abs(pitch);
				if (pitch == 0) {
					m_humdrum_text << "r";
				} else {
					if ((pitch > 0) && (i<(int)notes[j].size()-1) &&
						 (notes[j][i+1].b40 == -abspitch)) {
						// start of a note which continues into next
						// sonority.
						m_humdrum_text << "[";
					}
					m_humdrum_text << Convert::base40ToKern(abspitch);
					// print tie continue/termination as necessary.
					if (pitch < 0) {
						if ((i < (int)notes[j].size() - 1) &&
							 (notes[j][i+1].b40 == notes[j][i].b40)) {
						  // note sustains further
						  m_humdrum_text << "_";
						} else {
						  // note does not sustain any further.
						  m_humdrum_text << "]";
						}
					}
				}
				if (j < (int)notes.size()-1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}

		if (rhythmQ) {
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
			m_humdrum_text << "*-\t";
		}
		for (i=0; i<partcount; i++) {
			m_humdrum_text << "*-";
			if (i < partcount - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_cint::extractNoteArray --
//

void Tool_cint::extractNoteArray(vector<vector<NoteNode> >& notes, HumdrumFile& infile,
		vector<int>& ktracks, vector<int>& reverselookup) {

	HumRegex hre;

	Ids.resize(infile.getTrackCount()+1);
	int i, j, ii, jj;
	for (i=0; i<(int)Ids.size(); i++) {
		Ids[i] = EMPTY_ID;
	}

	vector<NoteNode> current(ktracks.size());
	vector<double> beatsizes(infile.getTrackCount()+1, 1);

	int sign;
	int track = 0;
	int index;

	int snum = 0;
	int measurenumber = 0;
	int tempmeasurenum = 0;
	double beatsize = 1.0;
	int topnum, botnum;

	for (i=0; i<infile.getLineCount(); i++) {
		if (debugQ) {
			m_humdrum_text << "PROCESSING LINE: " << i << "\t" << infile[i] << endl;
		}
		if (infile[i].isBarline()) {
			tempmeasurenum = infile.getMeasureNumber(i);
			if (tempmeasurenum >= 0) {
				measurenumber = tempmeasurenum;
			}
		}
		for (j=0; j<(int)current.size(); j++) {
			current[j].clear();
			current[j].measure = measurenumber;
			current[j].line = i;
		}

		if (infile[i].isBarline() && (infile.token(i, 0)->find("||") != string::npos)) {
			// double barline (terminal for Josquin project), so add a row
			// of rests to prevent cint melodic interval identification between
			// adjacent notes in different sections.
			for (j=0; j<(int)notes.size(); j++) {
				notes[j].push_back(current[j]);
			}
		} else if (infile[i].isInterpretation()) {
			// search for time signatures from which to extract beat information.
			for (j=0; j<infile[i].getFieldCount(); j++) {
				track = infile.token(i, j)->getTrack();
				if (hre.search(*infile.token(i, j), "^\\*M(\\d+)/(\\d+)")) {
					// deal with 3%2 in denominator later...
					topnum = hre.getMatchInt(1);
					botnum = hre.getMatchInt(2);
					beatsize = botnum;
					if (((topnum % 3) == 0) && (topnum > 3) && (botnum > 1)) {
						// compound meter
						// fix later
						beatsize = botnum / 3;
					}
					beatsizes[track] = beatsize / 4.0;
				} else if (*infile.token(i, j) == "*met(C|)") {
					// MenCutC, use 2 as the "beat"
					beatsizes[track] = 2.0 / 4.0;
				}
			}
		} else if (idQ && infile[i].isLocalComment()) {
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (hre.search(*infile.token(i, j), "^!ID:\\s*([^\\s]*)")) {
					int track = infile.token(i, j)->getTrack();
					Ids[track] = hre.getMatch(1);
				}
			}
		}

		if (!infile[i].isData()) {
			continue;
		}

		for (j=0; j<infile[i].getFieldCount(); j++) {
			sign = 1;
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			track = infile.token(i, j)->getTrack();
			index = reverselookup[track];
			if (index < 0) {
				continue;
			}
			if (idQ) {
				current[index].getId() = Ids[track];
				Ids[track] = "";  // don't assign to next item;
			}
			current[index].line  = i;
			current[index].spine = j;
			current[index].beatsize = beatsizes[track];
			if (infile.token(i, j)->isNull()) {
				sign = -1;
				HTp nullx = infile.token(i, j)->resolveNull();
				if (nullx == NULL) {
					ii = jj = -1;
				} else {
					ii = nullx->getLineIndex();
					jj = nullx->getFieldIndex();
				}
			} else {
				ii = i;
				jj = j;
			}
			if (infile.token(ii, jj)->find(NoteMarker) != string::npos) {
				current[index].notemarker = NoteMarker;
			}
			if (infile.token(ii, jj)->find('r') != string::npos) {
				current[index].b40 = 0;
				current[index].serial = ++snum;
				continue;
			}
			if (*infile.token(ii, jj) == ".") {
				current[index].b40 = 0;
				current[index].serial = snum;
			}
			current[index].b40 = Convert::kernToBase40(*infile.token(ii, jj));
			if (infile.token(ii, jj)->find('_') != string::npos) {
				sign = -1;
				current[index].serial = snum;
			}
			if (infile.token(ii, jj)->find(']') != string::npos) {
				sign = -1;
				current[index].serial = snum;
			}
			current[index].b40 *= sign;
			if (sign > 0) {
				current[index].serial = ++snum;
				if (durationQ) {
					current[index].duration = infile.token(ii, jj)->getTiedDuration();
				}
			}
		}
		if (onlyRests(current) && onlyRests(notes.back())) {
			// don't store more than one row of rests in the data array.
			continue;
		}
		if (allSustained(current)) {
			// don't store sonorities which are purely sutained
			// (may need to be updated with a --sustain option implementation)
			continue;
		}
		for (j=0; j<(int)notes.size(); j++) {
			notes[j].push_back(current[j]);
		}
	}

	// attach ID tag to all sustain sections of notes
	if (idQ) {
		for (j=0; j<(int)notes.size(); j++) {
			for (i=1; i<(int)notes[j].size(); i++) {
				if (notes[j][i].isAttack()) {
					continue;
				}
				if ((int)notes[j][i].getId().size() > 0) {
					// allow for Ids on sustained notes which probably means
					// that there is a written tied note in the music.
					continue;
				}
				if (notes[j][i].getB40() == notes[j][i-1].getB40()) {
					notes[j][i].getId() = notes[j][i-1].getId();
				}
			}
		}
	}

}



//////////////////////////////
//
// Tool_cint::onlyRests -- returns true if all NoteNodes are for rests
//

int Tool_cint::onlyRests(vector<NoteNode>& data) {
	int i;
	for (i=0; i<(int)data.size(); i++) {
		if (!data[i].isRest()) {
			return 0;
		}
	}
	return 1;
}



//////////////////////////////
//
// Tool_cint::hasAttack -- returns true if at least one NoteNode has
//   has an attack.
//

int Tool_cint::hasAttack(vector<NoteNode>& data) {
	int i;
	for (i=0; i<(int)data.size(); i++) {
		if (data[i].isAttack()) {
			return 1;
		}
	}
	return 0;
}



//////////////////////////////
//
// Tool_cint::allSustained -- returns true if all NoteNodes are sustains
//    or rests (but not all rests).
//

int Tool_cint::allSustained(vector<NoteNode>& data) {
	int i;
	int hasnote = 0;
	for (i=0; i<(int)data.size(); i++) {
		if (data[i].b40 != 0) {
			hasnote = 1;
		}
		if (data[i].isAttack()) {
			return 0;
		}
	}
	if (hasnote == 0) {
		return 0;
	}
	return 1;
}



//////////////////////////////
//
// Tool_cint::getAbbreviations --
//

void Tool_cint::getAbbreviations(vector<string>& abbreviations,
		vector<string>& names) {
	abbreviations.resize(names.size());
	for (int i=0; i<(int)names.size(); i++) {
		getAbbreviation(abbreviations[i], names[i]);
	}
}



//////////////////////////////
//
// Tool_cint::getAbbreviation --
//

void Tool_cint::getAbbreviation(string& abbr, string& name) {
	HumRegex hre;
	hre.replaceDestructive(abbr, "(?<=[a-zA-Z])[a-zA-Z]*", "");
	hre.tr(abbr, "123456789", "abcdefghi");
}



//////////////////////////////
//
// Tool_cint::getKernTracks -- return a list of track number for **kern spines.
//

void Tool_cint::getKernTracks(vector<int>& ktracks, HumdrumFile& infile) {
	int i, j;
	ktracks.reserve(infile.getTrackCount()+1);
	ktracks.resize(0);
	int track;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (infile.token(i, j)->isKern()) {
				track = infile.token(i, j)->getTrack();
				ktracks.push_back(track);
			}
		}
		break;
	}
}



//////////////////////////////
//
// Tool_cint::getNames -- get the names of each column if they have one.
//

void Tool_cint::getNames(vector<string>& names, vector<int>& reverselookup,
		HumdrumFile& infile) {

	names.resize((int)reverselookup.size()-1);
	char buffer[1024] = {0};
	int value;
	HumRegex pre;
	int i;
	int j;
	int track;

	for (i=0; i<(int)names.size(); i++) {
		value = (int)reverselookup.size() - i;
		snprintf(buffer, 1024, "%d", value);
		names[i] = buffer;
	}

	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			// stop looking for instrument name after the first data line
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (reverselookup[infile.token(i, j)->getTrack()] < 0) {
				continue;
			}
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			if (pre.search(*infile.token(i, j), "^\\*I\"(.*)")) {
				track = infile.token(i, j)->getTrack();
				names[reverselookup[track]] = pre.getMatch(1);
			}
		}
	}

	if (debugQ) {
		for (i=0; i<(int)names.size(); i++) {
			m_humdrum_text << i << ":\t" <<  names[i] << endl;
		}
	}

}



//////////////////////////////
//
// Tool_cint::initialize -- validate and process command-line options.
//

void Tool_cint::initialize(void) {

	// handle basic options:
	if (getBoolean("author")) {
		m_humdrum_text << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, September 2013" << endl;
		exit(0);
	} else if (getBoolean("version")) {
		m_humdrum_text << getCommand() << ", version: 16 March 2022" << endl;
		m_humdrum_text << "compiled: " << __DATE__ << endl;
		exit(0);
	} else if (getBoolean("help")) {
		usage(getCommand());
		exit(0);
	} else if (getBoolean("example")) {
		example();
		exit(0);
	}

	koptionQ = getBoolean("koption");

	if (getBoolean("comma")) {
		Spacer = ",";
	} else {
		Spacer = " ";
	}

	// dispay as base-7 by default
	base7Q = 1;

	base40Q    = getBoolean("base-40");
	base12Q    = getBoolean("base-12");
	chromaticQ = getBoolean("chromatic");
	zeroQ      = getBoolean("zero");

	if (base40Q) {
		base12Q = 0;
		base7Q = 0;
		zeroQ = 0;
	}

	if (base12Q) {
		base40Q = 0;
		base7Q = 0;
		zeroQ = 0;
	}

	pitchesQ     = getBoolean("pitches");
	debugQ       = getBoolean("debug");
	rhythmQ      = getBoolean("rhythm");
	durationQ    = getBoolean("duration");
	latticeQ     = getBoolean("lattice");
	sustainQ     = getBoolean("sustain");
	topQ         = getBoolean("top");
	toponlyQ     = getBoolean("top-only");
	hparenQ      = getBoolean("harmonic-parentheses");
	mparenQ      = getBoolean("melodic-parentheses");
	parenQ       = getBoolean("parentheses");
	rowsQ        = getBoolean("rows");
	hmarkerQ     = getBoolean("harmonic-marker");
	interleavedQ = getBoolean("interleaved-lattice");
	mmarkerQ     = getBoolean("melodic-marker");
	attackQ      = getBoolean("attacks");
	rawQ         = getBoolean("raw");
	raw2Q        = getBoolean("raw2");
	xoptionQ     = getBoolean("x");
	octaveallQ   = getBoolean("octave-all");
	octaveQ      = getBoolean("octave");
	noharmonicQ  = getBoolean("no-harmonic");
	nomelodicQ   = getBoolean("no-melodic");
	norestsQ     = getBoolean("no-rests");
	nounisonsQ   = getBoolean("no-melodic-unisons");
	Chaincount   = getInteger("n");
	searchQ      = getBoolean("search");
	markQ        = getBoolean("mark");
	idQ          = getBoolean("id");
	countQ       = getBoolean("count");
	filenameQ    = getBoolean("filename");
	suspensionsQ = getBoolean("suspensions");
	uncrossQ     = getBoolean("uncross");
	locationQ    = getBoolean("location");
	retroQ       = getBoolean("retrospective");
	MarkColor    = getString("color");
	NoteMarker   = "";
	if (getBoolean("note-marker")) {
		NoteMarker = getString("note-marker");
	}
	if (searchQ) {
		NoteMarker = getString("note-marker");
	}
	if (Chaincount < 0) {
		Chaincount = 0;
	}

	if (searchQ) {
		// Automatically assume marking of --search is used
		// (may change in the future).
		markQ = 1;
	}
	if (countQ) {
		searchQ = 1;
		markQ   = 0;
	}

	if (raw2Q) {
		norestsQ = 1;
	}

	if (searchQ) {
		SearchString = getString("search");
	}

}



//////////////////////////////
//
// Tool_cint::example -- example usage of the quality program
//

void Tool_cint::example(void) {
	m_humdrum_text <<
	"                                                                         \n"
	<< endl;
}



//////////////////////////////
//
// Tool_cint::usage -- gives the usage statement for the meter program
//

void Tool_cint::usage(const string& command) {
	m_humdrum_text <<
	"                                                                         \n"
	<< endl;
}


// END_MERGE

} // end namespace hum



