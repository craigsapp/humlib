//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Wolfgang Drescher
// Creation Date: Sun Dec 26 17:03:54 PST 2010
// Last Modifed:  Sun Dec 18 23:25:32 PST 2016 Ported to humlib
// Last Modifed:  Thu Feb  9 06:40:13 PST 2023 Added -l option
// Filename:      ...sig/examples/all/myank.cpp
// Filename:      tool-myank.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-myank.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Extract measures from input data.
//

#include "tool-myank.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_myank::Tool_myank -- Set the recognized options for the tool.
//

Tool_myank::Tool_myank(void) {
	define("v|verbose=b",    "Verbose output of data");
	define("debug=b",        "Debugging information");
	define("inlist=b",       "Show input measure list");
	define("outlist=b",      "Show output measure list");
	define("mark|marks=b",   "Yank measure with marked notes");
	define("T|M|bar-number-text=b", "print barnum with LO text above system ");
	define("d|double|dm|md|mdsep|mdseparator=b", "Put double barline between non-consecutive measure segments");
	define("m|b|measures|bars|measure|bar=s", "Measures to yank");
	define("l|lines|line-range=s", "Line numbers range to yank (e.g. 40-50)");
	define("I|i|instrument=b", "Include instrument codes from start of data");
	define("visible|not-invisible=b", "Do not make initial measure invisible");
	define("B|noendbar=b", "Do not print barline at end of data");
	define("max=b",  "print maximum measure number");
	define("min=b",  "print minimum measure number");
	define("section-count=b", "count the number of sections, JRP style");
	define("section=i:0", "extract given section number (indexed from 1");
	define("author=b",        "Program author");
	define("version=b",       "Program version");
	define("example=b",       "Program examples");
	define("h|help=b",        "Short description");
	define("hide-starting=b", "Prevent printStarting");
	define("hide-ending=b",   "Prevent printEnding");
}



/////////////////////////////////
//
// Tool_myank::run -- Primary interfaces to the tool.
//

bool Tool_myank::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_myank::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_myank::run(HumdrumFile& infile, ostream& out) {
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

bool Tool_myank::run(HumdrumFile& infile) {
	// Max track in enscripten is wrong for some reason,
	// so making a copy and forcing reanalysis:
	//perhaps not needed anymore:
	//stringstream ss;
	//ss << infile;
	//infile.read(ss);
	initialize(infile);
	processFile(infile);
	// Re-load the text for each line from their tokens.
	infile.createLinesFromTokens();
	return true;
}


///////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& out, MyCoord& value) {
	out << "(" << value.x << "," << value.y << ")";
	return out;
}


ostream& operator<<(ostream& out, MeasureInfo& info) {
	if (info.file == NULL) {
		return out;
	}
	HumdrumFile& infile = *(info.file);
	out << "================================== " << endl;
	out << "NUMBER      = " << info.num   << endl;
	out << "SEGMENT     = " << info.seg   << endl;
	out << "START       = " << info.start << endl;
	out << "STOP        = " << info.stop  << endl;
	out << "STOP_STYLE  = " << info.stopStyle << endl;
	out << "START_STYLE = " << info.startStyle << endl;

	for (int i=1; i<(int)info.sclef.size(); i++) {
		out << "TRACK " << i << ":" << endl;
		if (info.sclef[i].isValid()) {
			out << "   START CLEF    = " << infile.token(info.sclef[i].x, info.sclef[i].y)       << endl;
		}
		if (info.skeysig[i].isValid()) {
			out << "   START KEYSIG  = " << infile.token(info.skeysig[i].x, info.skeysig[i].y)   << endl;
		}
		if (info.skey[i].isValid()) {
			out << "   START KEY     = " << infile.token(info.skey[i].x, info.skey[i].y)         << endl;
		}
		if (info.stimesig[i].isValid()) {
			out << "   START TIMESIG = " << infile.token(info.stimesig[i].x, info.stimesig[i].y) << endl;
		}
		if (info.smet[i].isValid()) {
			out << "   START MET     = " << infile.token(info.smet[i].x, info.smet[i].y)         << endl;
		}
		if (info.stempo[i].isValid()) {
			out << "   START TEMPO   = " << infile.token(info.stempo[i].x, info.stempo[i].y)     << endl;
		}

		if (info.eclef[i].isValid()) {
			out << "   END CLEF    = " << infile.token(info.eclef[i].x, info.eclef[i].y)       << endl;
		}
		if (info.ekeysig[i].isValid()) {
			out << "   END KEYSIG  = " << infile.token(info.ekeysig[i].x, info.ekeysig[i].y)   << endl;
		}
		if (info.ekey[i].isValid()) {
			out << "   END KEY     = " << infile.token(info.ekey[i].x, info.ekey[i].y)         << endl;
		}
		if (info.etimesig[i].isValid()) {
			out << "   END TIMESIG = " << infile.token(info.etimesig[i].x, info.etimesig[i].y) << endl;
		}
		if (info.emet[i].isValid()) {
			out << "   END MET     = " << infile.token(info.emet[i].x, info.emet[i].y)         << endl;
		}
		if (info.etempo[i].isValid()) {
			out << "   END TEMPO   = " << infile.token(info.etempo[i].x, info.etempo[i].y)     << endl;
		}
	}

	return out;
}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// Tool_myank::initialize -- extract time signature lines for
//    each **kern spine in file.
//

void Tool_myank::initialize(HumdrumFile& infile) {
	// handle basic options:
	if (getBoolean("author")) {
		m_free_text << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, December 2010" << endl;
		return;
	} else if (getBoolean("version")) {
		m_free_text << getCommand() << ", version: 26 December 2010" << endl;
		m_free_text << "compiled: " << __DATE__ << endl;
		return;
	} else if (getBoolean("help")) {
		usage(getCommand());
		return;
	} else if (getBoolean("example")) {
		example();
		return;
	}

	m_debugQ        = getBoolean("debug");
	m_inlistQ       = getBoolean("inlist");
	m_outlistQ      = getBoolean("outlist");
	m_verboseQ      = getBoolean("verbose");
	m_maxQ          = getBoolean("max");
	m_minQ          = getBoolean("min");

	m_invisibleQ    = !getBoolean("not-invisible");
	m_instrumentQ   =  getBoolean("instrument");
	m_nolastbarQ    =  getBoolean("noendbar");
	m_markQ         =  getBoolean("mark");
	m_doubleQ       =  getBoolean("mdsep");
	m_barnumtextQ   =  getBoolean("bar-number-text");
	m_sectionCountQ =  getBoolean("section-count");
	m_section       =  getInteger("section");

	m_lineRange     = getString("lines");
	m_hideStarting  = getBoolean("hide-starting");
	m_hideEnding    = getBoolean("hide-ending");


	if (!m_section) {
		if (!(getBoolean("measures") || m_markQ) && !getBoolean("lines")) {
			// if -m option is not given, then --mark option presumed
			m_markQ = 1;
			// cerr << "Error: the -m option is required" << endl;
			// exit(1);
		}
	}

}



////////////////////////
//
// Tool_myank::processFile --
//

void Tool_myank::processFile(HumdrumFile& infile) {
	if (m_sectionCountQ) {
		int sections = getSectionCount(infile);
		m_humdrum_text << sections << endl;
		return;
	}

	getMetStates(m_metstates, infile);
	getMeasureStartStop(m_measureInList, infile);

	string measurestring = getString("measures");

	if (getBoolean("lines")) {
		int startLineNumber = getStartLineNumber();
		int endLineNumber = getEndLineNumber();
		if ((startLineNumber > endLineNumber) || (endLineNumber > infile.getLineCount())) {
			// Disallow when end line number is bigger then line count or when
			// start line number greather than end line number
			return;
		}
		m_barNumbersPerLine = analyzeBarNumbers(infile);
		int startBarNumber = getBarNumberForLineNumber(startLineNumber);
		int endBarNumber = getBarNumberForLineNumber(endLineNumber);
		measurestring = to_string(startBarNumber) + "-" + to_string(endBarNumber);
	}

	measurestring = expandMultipliers(measurestring);
	if (m_markQ) {
		stringstream mstring;
		getMarkString(mstring, infile);
		measurestring = mstring.str();
		if (m_debugQ) {
			m_free_text << "MARK STRING: " << mstring.str() << endl;
		}
	} else if (m_section) {
		string sstring;
		getSectionString(sstring, infile, m_section);
		measurestring = sstring;
	}
	if (m_debugQ) {
		m_free_text << "MARK MEASURES: " << measurestring << endl;
	}

	// expand to multiple measures later.
	expandMeasureOutList(m_measureOutList, m_measureInList, infile,
			measurestring);

	if (m_inlistQ) {
		m_free_text << "INPUT MEASURE MAP: " << endl;
		for (int i=0; i<(int)m_measureInList.size(); i++) {
			m_free_text << m_measureInList[i];
		}
	}
	if (m_outlistQ) {
		m_free_text << "OUTPUT MEASURE MAP: " << endl;
		for (int i=0; i<(int)m_measureOutList.size(); i++) {
			m_free_text << m_measureOutList[i];
		}
	}

	if (m_measureOutList.size() == 0) {
		// disallow processing files with no barlines
		return;
	}

	// move stopStyle to startStyle of next measure group.
	for (int i=(int)m_measureOutList.size()-1; i>0; i--) {
		m_measureOutList[i].startStyle = m_measureOutList[i-1].stopStyle;
		m_measureOutList[i-1].stopStyle = "";
	}

	myank(infile, m_measureOutList);
}



////////////////////////
//
// Tool_myank::analyzeBarNumbers -- Stores the bar number of each line in a vector
//

vector<int> Tool_myank::analyzeBarNumbers(HumdrumFile& infile) {
	vector<int> m_barnum;
	m_barnum.resize(infile.getLineCount());
	int current = -1;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_barnum.at(i) = current;
			continue;
		}
		if (hre.search(infile[i].token(0), "=(\\d+)")) {
			current = hre.getMatchInt(1);
		}
		m_barnum.at(i) = current;
	}
	return m_barnum;
}



////////////////////////
//
// Tool_myank::getBarNumberForLineNumber --
//

int Tool_myank::getBarNumberForLineNumber(int lineNumber) {
	return m_barNumbersPerLine[lineNumber-1];
}



////////////////////////
//
// Tool_myank::getStartLineNumber -- Get start line number from --lines
//

int Tool_myank::getStartLineNumber(void) {
	HumRegex hre;
	if (hre.search(m_lineRange, "^(\\d+)\\-(\\d+)$")) {
		return hre.getMatchInt(1);
	}
	return -1;
}



////////////////////////
//
// Tool_myank::getEndLineNumber -- Get end line number from --lines
//

int Tool_myank::getEndLineNumber(void) {
	HumRegex hre;
	if (hre.search(m_lineRange, "^(\\d+)\\-(\\d+)$")) {
		return hre.getMatchInt(2);
	}
	return -1;
}



//////////////////////////////
//
// Tool_myank::expandMultipliers -- 2*5 => 2,2,2,2,2
//    Limit of 100 times expansion
//

string Tool_myank::expandMultipliers(const string& inputstring) {
	HumRegex hre;
	if (!hre.search(inputstring, "\\*")) {
		return inputstring;
	}
	string outputstring = inputstring;
	while (hre.search(outputstring, "(\\d+)\\*([1-9]+[0-9]*)")) {
		string measurenum = hre.getMatch(1);
		int multiplier = hre.getMatchInt(2);
		if (multiplier > 100) {
			cerr << "Reducing multiplier from " << multiplier << " to 100" << endl;
			multiplier = 100;
		}
		string expansion = measurenum;
		for (int i=1; i<multiplier; i++) {
			expansion += ",";
			expansion += measurenum;
		}
		hre.replaceDestructive(outputstring, expansion, "(\\d+)\\*([1-9]+[0-9]*)");
	}
	return outputstring;
}


//////////////////////////////
//
// Tool_myank::getMetStates --  Store the current *met for every token
// in the score, keeping track of meter without metric symbols.
//

void Tool_myank::getMetStates(vector<vector<MyCoord> >& metstates,
		HumdrumFile& infile) {
	vector<MyCoord> current;
	current.resize(infile.getMaxTrack()+1);
	metstates.resize(infile.getLineCount());
	HumRegex hre;

	int track;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				track = infile.token(i, j)->getTrack();
				if (hre.search(infile.token(i, j), R"(^\*met\([^\)]+\))")) {
					current[track].x = i;
					current[track].y = j;
				} else if (hre.search(infile.token(i, j), R"(^\*M\d+\d+)")) {
					current[track] = getLocalMetInfo(infile, i, track);
				}
			}
		}

		// metstates[i].resize(infile[i].getFieldCount());
		// for (j=0; j<infile[i].getFieldCount(); j++) {
		//    track = infile.token(i, j)->getTrack();
		//    metstates[i][j] = current[track];
		// }
		metstates[i].resize(infile.getMaxTrack()+1);
		for (int j=1; j<=infile.getMaxTrack(); j++) {
			metstates[i][j] = current[j];
		}
	}

	if (m_debugQ) {
		for (int i=0; i<infile.getLineCount(); i++) {
			for (int j=1; j<(int)metstates[i].size(); j++) {
				if (metstates[i][j].x < 0) {
					m_humdrum_text << ".";
				} else {
					m_humdrum_text << infile.token(metstates[i][j].x, metstates[i][j].y);
				}
				m_humdrum_text << "\t";
			}
			m_humdrum_text << infile[i] << endl;
		}

	}
}



//////////////////////////////
//
// Tool_myank::getLocalMetInfo -- search in the non-data region indicated by the
// input row for a *met entry in the input track.  Return empty
// value if none found.
//

MyCoord Tool_myank::getLocalMetInfo(HumdrumFile& infile, int row, int track) {
	MyCoord output;
	int startline = -1;
	int stopline = -1;
	int i = row;
	int j;
	int xtrac;
	HumRegex hre;

	while (i>=0) {
		if (infile[i].isData()) {
			startline = i+1;
			break;
		}
		i--;
	}
	if (startline < 0) {
		startline = 0;
	}
	i = row;
	while (i<infile.getLineCount()){
		if (infile[i].isData()) {
			stopline = i-1;
			break;
		}
		i++;
	}
	if (stopline >= infile.getLineCount()) {
		stopline = infile.getLineCount()-1;
	}
	for (i=startline; i<=stopline; i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			xtrac = infile.token(i, j)->getTrack();
			if (track != xtrac) {
				continue;
			}
			if (hre.search(infile.token(i, j), R"(^\*met\([^\)]+\))")) {
				output.x = i;
				output.x = j;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_myank::getMarkString -- return a list of measures which contain marked
//    notes (primarily from search matches).
// This function scans for reference records in this form:
// !!!RDF**kern: @= matched note
// or
// !!!RDF**kern: i= marked note
// If it finds any lines like that, it will extract the character before
// the equals sign, and scan for it in the **kern data in the file.
// any measure which contains such a mark will be stored in the output
// string.
//

void Tool_myank::getMarkString(ostream& out, HumdrumFile& infile)  {
	string mchar; // list of characters which are marks
	char target;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		if (hre.search(infile.token(i, 0),
				R"(!!!RDF\*\*kern\s*:\s*([^=])\s*=\s*match)", "i")) {
			target = hre.getMatch(1)[0];
			mchar.push_back(target);
		} else if (hre.search(infile.token(i, 0),
				R"(!!!RDF\*\*kern\s*:\s*([^=])\s*=\s*mark)", "i")) {
			target = hre.getMatch(1)[0];
			mchar.push_back(target);
		}
	}

	if (m_debugQ) {
		for (int i=0; i<(int)mchar.size(); i++) {
			m_free_text << "\tMARK CHARCTER: " << mchar[i] << endl;
		}
	}

	if (mchar.size() == 0) {
		return;
	}

	// now search for measures which contains any of those character
	// in **kern data:
	int curmeasure = 0;
	int inserted = 0;
	int hasmark = 0;
	string str;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isBarline()) {
			if (hre.search(infile.token(i, 0), "^=.*?(\\d+)", "")) {
				curmeasure = stoi(hre.getMatch(1));
				hasmark = 0;
			}
		}
		if (hasmark) {
			continue;
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (infile.token(i, j)->isKern()) {
				int k=0;
				str = *infile.token(i, j);
				while (str[k] != '\0') {
					for (int m=0; m<(int)mchar.size(); m++) {
						if (str[k] == mchar[m]) {
							if (inserted) {
								out << ',';
							} else {
								inserted++;
							}
							out << curmeasure;
							hasmark = 1;
							goto outerforloop;
						}
					}
					k++;
				}
			}
		}
outerforloop: ;
	}
}



//////////////////////////////
//
// Tool_myank::myank -- yank the specified measures.
//

void Tool_myank::myank(HumdrumFile& infile, vector<MeasureInfo>& outmeasures) {

	if (outmeasures.size() > 0) {
		printStarting(infile);
	}

	int lastline = -1;
	int lastDataLine = -1;
	int h, i, j;
	int counter;
	int printed = 0;
	int mcount = 0;
	int measurestart = 1;
	int lastbarnum = -1;
	int barnum = -1;
	int datastart = 0;
	int bartextcount = 0;
	bool startLineHandled = false;

	int lastLineIndex = getBoolean("lines") ? getEndLineNumber() - 1 : outmeasures[outmeasures.size() - 1].stop;

	// Find the actual last line of the selected section that is a line with
	// data tokens
	while (infile.getLine(lastLineIndex)->isData() == false) {
		lastLineIndex--;
	}

	// Mapping with with the start token for each spine
	vector<int> lastLineResolvedTokenLineIndex;
	// Mapping with the later needed durations of the note that fits within the
	// selected section
	vector<HumNum> lastLineDurationsFromNoteStart;

	lastLineResolvedTokenLineIndex.resize(infile.getLine(lastLineIndex)->getTokenCount());
	lastLineDurationsFromNoteStart.resize(infile.getLine(lastLineIndex)->getTokenCount());

	for (int a = 0; a < infile.getLine(lastLineIndex)->getTokenCount(); a++) {
		HTp token = infile.token(lastLineIndex, a);
		// Get lineIndex for last data token with an attack
		lastLineResolvedTokenLineIndex[a] = infile.token(lastLineIndex, a)->resolveNull()->getLineIndex();
		// Get needed duration for this token until section end
		lastLineDurationsFromNoteStart[a] = token->getDurationFromNoteStart() + token->getLine()->getDuration();
	}

	int startLineNumber = getStartLineNumber();
	int endLineNumber = getEndLineNumber();

	if (getBoolean("lines")) {
		int firstDataLineIndex = -1;
		for (int b = startLineNumber - 1; b <= endLineNumber - 1; b++) {
			if (infile.getLine(b)->isData()) {
				firstDataLineIndex = b;
				break;
			}
		}
		if (firstDataLineIndex >= 0) {
			if (infile.getLine(firstDataLineIndex)->getDurationFromBarline() == 0) {
				for (int c = startLineNumber - 1; c >=  0; c--) {
					if (infile.getLine(c)->isBarline()) {
						startLineNumber = c + 1;
						break;
					}
				}
			}
		}
	}

	for (h=0; h<(int)outmeasures.size(); h++) {
		barnum = outmeasures[h].num;
		measurestart = 1;
		printed = 0;
		counter = 0;
		if (m_debugQ) {
			m_humdrum_text << "!! =====================================\n";
			m_humdrum_text << "!! processing " << outmeasures[h].num << endl;
		}
		if (h > 0) {
			reconcileSpineBoundary(infile, outmeasures[h-1].stop,
				outmeasures[h].start);
		} else {
			reconcileStartingPosition(infile, outmeasures[0].start);
		}
		int startLine = getBoolean("lines") ? std::max(startLineNumber-1, outmeasures[h].start)
			: outmeasures[h].start;
		int endLine = getBoolean("lines") ? std::min(endLineNumber, outmeasures[h].stop)
			: outmeasures[h].stop;
		for (i=startLine; i<endLine; i++) {
			counter++;
			if ((!printed) && ((mcount == 0) || (counter == 2))) {
				if ((datastart == 0) && outmeasures[h].num == 0) {
					// not ideal setup...
					datastart = 1;
				} else{
					// Fix adjustGlobalInterpretations when line is a global comment
					int nextLineIndexWithSpines = i;
					if (infile.getLine(i)->isCommentGlobal()) {
						for (int d = i; d <= endLineNumber - 1; d++) {
							if (!infile.getLine(d)->isCommentGlobal()) {
								nextLineIndexWithSpines = d;
								break;
							}
						}
					}
					adjustGlobalInterpretations(infile, nextLineIndexWithSpines, outmeasures, h);
					printed = 1;
				}
			}
			if (infile[i].isData() && (mcount == 0)) {
				mcount++;
			}
			if (infile[i].isBarline()) {
				mcount++;
			}
			if ((mcount == 1) && m_invisibleQ && infile[i].isBarline()) {
				printInvisibleMeasure(infile, i);
				measurestart = 0;
				if ((bartextcount++ == 0) && infile[i].isBarline()) {
					int barline = 0;
					sscanf(infile.token(i, 0)->c_str(), "=%d", &barline);
					if (m_barnumtextQ && (barline > 0)) {
						m_humdrum_text << "!!LO:TX:Z=20:X=-90:t=" << barline << endl;
					}
				}
			} else if (m_doubleQ && (lastbarnum > -1) && (abs(barnum - lastbarnum) > 1)) {
				printDoubleBarline(infile, i);
				measurestart = 0;
			} else if (measurestart && infile[i].isBarline()) {
				printMeasureStart(infile, i, outmeasures[h].startStyle);
				measurestart = 0;
			} else {
				printDataLine(infile.getLine(i), startLineHandled, lastLineResolvedTokenLineIndex, lastLineDurationsFromNoteStart);
				if (m_barnumtextQ && (bartextcount++ == 0) && infile[i].isBarline()) {
					int barline = 0;
					sscanf(infile.token(i, 0)->c_str(), "=%d", &barline);
					if (barline > 0) {
						m_humdrum_text << "!!LO:TX:Z=20:X=-25:t=" << barline << endl;
					}
				}
			}
			lastline = i;
			if (infile.getLine(i)->isData()) {
				lastDataLine = i;
			}
		}
		lastbarnum = barnum;
	}

	if (getBoolean("lines") && (lastDataLine >= 0) &&
			(infile.getLine(lastDataLine)->getDurationToBarline() > infile.getLine(lastDataLine)->getDuration())) {
		m_nolastbarQ = true;
	}

	HumRegex hre;
	string token;
	int lasti;
	if (outmeasures.size() > 0) {
		lasti = outmeasures.back().stop;
	} else {
		lasti = -1;
	}
	if ((!m_nolastbarQ) &&  (lasti >= 0) && infile[lasti].isBarline()) {
		for (j=0; j<infile[lasti].getFieldCount(); j++) {
			token = *infile.token(lasti, j);
			hre.replaceDestructive(token, outmeasures.back().stopStyle, "\\d+.*");
			// collapse final barlines
			hre.replaceDestructive(token, "==", "===+");
			if (m_doubleQ) {
				if (hre.search(token, "=(.+)")) {
					// don't add double barline, there is already
					// some style on the barline
				} else {
					// add a double barline
					hre.replaceDestructive(token, "||", "$");
				}
			}
			m_humdrum_text << token;
			if (j < infile[lasti].getFieldCount() - 1) {
				m_humdrum_text << '\t';
			}
		}
		m_humdrum_text << '\n';
	}

	collapseSpines(infile, lasti);

	if (m_debugQ) {
		m_free_text << "PROCESSING ENDING" << endl;
	}

	if (lastline >= 0) {
		printEnding(infile, outmeasures.back().stop, lasti);
	}
}



//////////////////////////////
//
// Tool_myank::collapseSpines -- Shrink all sub-spines to single spine.
//

void Tool_myank::collapseSpines(HumdrumFile& infile, int line) {
	if (line < 0) {
		return;
	}
	vector<int> counts(infile.getMaxTrack() + 1, 0);
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		int track = infile.token(line, i)->getTrack();
		counts.at(track)++;
	}
	for (int i=1; i<(int)counts.size(); i++) {
		if (counts[i] <= 1) {
			continue;
		}
		bool started = false;
		for (int j=1; j<(int)counts.size(); j++) {
			if (j < i) {
				if (started) {
					m_humdrum_text << "\t";
				}
				m_humdrum_text << "*";
				started = true;
				continue;
			} else if (j == i) {
				for (int k=0; k<counts[j]; k++) {
					if (started) {
						m_humdrum_text << "\t";
					}
					m_humdrum_text << "*v";
					started = true;
				}
			} else if (j > i) {
				for (int k=0; k<counts[j]; k++) {
					if (started) {
						m_humdrum_text << "\t";
					}
					m_humdrum_text << "*";
					started = true;
				}
			}
		}
		m_humdrum_text << "\n";
		counts[i] = 1;
	}
}



//////////////////////////////
//
// Tool_myank::adjustGlobalInterpretations --
//

void Tool_myank::adjustGlobalInterpretations(HumdrumFile& infile, int ii,
		vector<MeasureInfo>& outmeasures, int index) {

	if (index <= 0) {
		adjustGlobalInterpretationsStart(infile, ii, outmeasures, index);
		return;
	}

	// the following lines will not work when non-contiguous measures are
	// elided.
	//   if (!infile[ii].isInterpretation()) {
	//      return;
	//   }

	int clefQ    = 0;
	int keysigQ  = 0;
	int keyQ     = 0;
	int timesigQ = 0;
	int metQ     = 0;
	int tempoQ   = 0;

	int x, y;
	int xo, yo;

	int tracks = infile.getMaxTrack();

	// these lines may cause bugs, but they get rid of zeroth measure
	// problem.
// ggg
//   if ((outmeasures.size() > 1) && (outmeasures[index-1].num == 0)) {
//      return;
//   }
//   if ((outmeasures.size() > 0) && (outmeasures[index].num == 0)) {
//      return;
//   }

	for (int i=1; i<=tracks; i++) {
		if (!clefQ && (outmeasures[index].sclef.size() > 0)) {
			x  = outmeasures[index].sclef[i].x;
			y  = outmeasures[index].sclef[i].y;
			xo = outmeasures[index-1].eclef[i].x;
			yo = outmeasures[index-1].eclef[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					clefQ = 1;
				}
			}
		}

		if (!keysigQ && (outmeasures[index].skeysig.size() > 0)) {
			x  = outmeasures[index].skeysig[i].x;
			y  = outmeasures[index].skeysig[i].y;
			xo = outmeasures[index-1].ekeysig[i].x;
			yo = outmeasures[index-1].ekeysig[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					keysigQ = 1;
				}
			}
		}

		if (!keyQ && (outmeasures[index].skey.size() > 0)) {
			x  = outmeasures[index].skey[i].x;
			y  = outmeasures[index].skey[i].y;
			xo = outmeasures[index-1].ekey[i].x;
			yo = outmeasures[index-1].ekey[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					keyQ = 1;
				}
			}
		}

		if (!timesigQ && (outmeasures[index].stimesig.size() > 0)) {
			x  = outmeasures[index].stimesig[i].x;
			y  = outmeasures[index].stimesig[i].y;
			xo = outmeasures[index-1].etimesig[i].x;
			yo = outmeasures[index-1].etimesig[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					timesigQ = 1;
				}
			}
		}

		if (!metQ && (outmeasures[index].smet.size() > 0)) {
			x  = outmeasures[index].smet[i].x;
			y  = outmeasures[index].smet[i].y;
			xo = outmeasures[index-1].emet[i].x;
			yo = outmeasures[index-1].emet[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					metQ = 1;
				}
			}
		}

		if (!tempoQ && (outmeasures[index].stempo.size() > 0)) {
			x  = outmeasures[index].stempo[i].x;
			y  = outmeasures[index].stempo[i].y;
			xo = outmeasures[index-1].etempo[i].x;
			yo = outmeasures[index-1].etempo[i].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					tempoQ = 1;
				}
			}
		}
	}

	int track;

	if (clefQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].sclef[track].x;
			y  = outmeasures[index].sclef[track].y;
			xo = outmeasures[index-1].eclef[track].x;
			yo = outmeasures[index-1].eclef[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (keysigQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].skeysig[track].x;
			y  = outmeasures[index].skeysig[track].y;
			xo = outmeasures[index-1].ekeysig[track].x;
			yo = outmeasures[index-1].ekeysig[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (keyQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].skey[track].x;
			y  = outmeasures[index].skey[track].y;
			xo = outmeasures[index-1].ekey[track].x;
			yo = outmeasures[index-1].ekey[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (timesigQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].stimesig[track].x;
			y  = outmeasures[index].stimesig[track].y;
			xo = outmeasures[index-1].etimesig[track].x;
			yo = outmeasures[index-1].etimesig[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (metQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].smet[track].x;
			y  = outmeasures[index].smet[track].y;
			xo = outmeasures[index-1].emet[track].x;
			yo = outmeasures[index-1].emet[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (tempoQ) {
		for (int i=0; i<infile[ii].getFieldCount(); i++) {
			track = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].stempo[track].x;
			y  = outmeasures[index].stempo[track].y;
			xo = outmeasures[index-1].etempo[track].x;
			yo = outmeasures[index-1].etempo[track].y;
			if ((x>=0)&&(y>=0)&&(xo>=0)&&(yo>=0)) {
				if (*infile.token(x, y) != *infile.token(xo, yo)) {
					m_humdrum_text << infile.token(x, y);
				} else {
					m_humdrum_text << "*";
				}
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

}



//////////////////////////////
//
// Tool_myank::adjustGlobalInterpretationsStart --
//

void Tool_myank::adjustGlobalInterpretationsStart(HumdrumFile& infile, int ii,
		vector<MeasureInfo>& outmeasures, int index) {
	if (index != 0) {
		cerr << "Error in adjustGlobalInterpetationsStart" << endl;
		exit(1);
	}

	int i;

	int clefQ    = 0;
	int keysigQ  = 0;
	int keyQ     = 0;
	int timesigQ = 0;
	int metQ     = 0;
	int tempoQ   = 0;

	int x, y;

	// ignore the zeroth measure
	// (may not be proper).
// ggg
	if (outmeasures[index].num == 0) {
		return;
	}

	int tracks = infile.getMaxTrack();

	for (i=1; i<=tracks; i++) {

		if (!clefQ) {
			x  = outmeasures[index].sclef[i].x;
			y  = outmeasures[index].sclef[i].y;

			if ((x>=0)&&(y>=0)) {
				clefQ = 1;
			}
		}

		if (!keysigQ) {
			x  = outmeasures[index].skeysig[i].x;
			y  = outmeasures[index].skeysig[i].y;
			if ((x>=0)&&(y>=0)) {
				keysigQ = 1;
			}
		}

		if (!keyQ) {
			x  = outmeasures[index].skey[i].x;
			y  = outmeasures[index].skey[i].y;
			if ((x>=0)&&(y>=0)) {
				keyQ = 1;
			}
		}

		if (!timesigQ) {
			x  = outmeasures[index].stimesig[i].x;
			y  = outmeasures[index].stimesig[i].y;
			if ((x>=0)&&(y>=0)) {
				timesigQ = 1;
			}
		}

		if (!metQ) {
			x  = outmeasures[index].smet[i].x;
			y  = outmeasures[index].smet[i].y;
			if ((x>=0)&&(y>=0)) {
				metQ = 1;
			}
		}

		if (!tempoQ) {
			x  = outmeasures[index].stempo[i].x;
			y  = outmeasures[index].stempo[i].y;
			if ((x>=0)&&(y>=0)) {
				tempoQ = 1;
			}
		}
	}

	int ptrack;

	if (clefQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].sclef[ptrack].x;
			y  = outmeasures[index].sclef[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (keysigQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].skeysig[ptrack].x;
			y  = outmeasures[index].skeysig[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (keyQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].skey[ptrack].x;
			y  = outmeasures[index].skey[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (timesigQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].stimesig[ptrack].x;
			y  = outmeasures[index].stimesig[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}
	if (metQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].smet[ptrack].x;
			y  = outmeasures[index].smet[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}

	if (tempoQ) {
		for (i=0; i<infile[ii].getFieldCount(); i++) {
			ptrack = infile.token(ii, i)->getTrack();
			x  = outmeasures[index].stempo[ptrack].x;
			y  = outmeasures[index].stempo[ptrack].y;
			if ((x>=0)&&(y>=0)) {
				m_humdrum_text << infile.token(x, y);
			} else {
				m_humdrum_text << "*";
			}
			if (i < infile[ii].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << "\n";
	}
}



//////////////////////////////
//
// Tool_myank::printDataLine -- Print line with data tokens of selected section
//

void Tool_myank::printDataLine(HLp line,
		bool& startLineHandled,
		const vector<int>& lastLineResolvedTokenLineIndex,
		const vector<HumNum>& lastLineDurationsFromNoteStart) {
	bool lineChange = false;
	string recipRegex = R"re(([\d%.]+))re";
	// Handle cutting the previeous token of a note that hangs into the selected
	// section
	if (startLineHandled == false) {
		if (line->isData()) {
			vector<HTp> tokens;
			line->getTokens(tokens);
			for (HTp token : tokens) {
				if (token->isKern() && token->isNull()) {
					HTp resolvedToken = token->resolveNull();
					if (resolvedToken->isNull()) {
						continue;
					}
					HumRegex hre;
					string recip = Convert::durationToRecip(token->getDurationToNoteEnd());
					vector<string> subtokens = resolvedToken->getSubtokens();
					string tokenText;
					for (int i=0; i<(int)subtokens.size(); i++) {
						if (hre.search(subtokens[i], recipRegex)) {
							string before = hre.getPrefix();
							string after = hre.getSuffix();
							hre.replaceDestructive(after, "", recipRegex, "g");
							string subtokenText;
							// Replace the old duration with the clipped one
							subtokenText += before + recip + after;
							// Add a tie end if not already in a tie group
							if (!hre.search(subtokens[i], "[_\\]]")) {
									subtokenText += "]";
							}
							tokenText += subtokenText;
							if (i < (int)subtokens.size() - 1) {
								tokenText += " ";
							}
						}
					}
					token->setText(tokenText);
					lineChange = true;
				}
			}
			startLineHandled = true;
		}
	// Handle cutting the last attacked note of the selected section
	} else {
		// Check if line has a note that needs to be handled
		if (find(lastLineResolvedTokenLineIndex.begin(), lastLineResolvedTokenLineIndex.end(), line->getLineIndex()) !=
				lastLineResolvedTokenLineIndex.end()) {
			for (int i = 0; i < line->getTokenCount(); i++) {
				HTp token = line->token(i);
				// Check if token need the be handled and is of type **kern
				if (token->isKern() && (lastLineResolvedTokenLineIndex[i] == line->getLineIndex())) {
					HTp resolvedToken = token->resolveNull();
					if (resolvedToken->isNull()) {
						continue;
					}
					HumNum dur = lastLineDurationsFromNoteStart[i];
					HumRegex hre;
					string recip = Convert::durationToRecip(dur);
					vector<string> subtokens = resolvedToken->getSubtokens();
					for (int i=0; i<(int)subtokens.size(); i++) {
						if (hre.search(subtokens[i], recipRegex)) {
							string before = hre.getPrefix();
							string after = hre.getSuffix();
							hre.replaceDestructive(after, "", recipRegex, "g");
							string subtokenText;
							if (resolvedToken->getDuration() > dur) {
								// Add a tie start if not already in a tie group
								if (!hre.search(subtokens[i], "[_\\[]")) {
										subtokenText += "[";
								}
							}
							// Replace the old duration with the clipped one
							subtokenText += before + recip + after;
							token->replaceSubtoken(i, subtokenText);
							lineChange = true;
						}
					}
				}
			}
		}
	}
	if (lineChange) {
		line->createLineFromTokens();
	}
	m_humdrum_text << line << "\n";
}



//////////////////////////////
//
// Tool_myank::printMeasureStart -- print a starting measure of a segment.
//

void Tool_myank::printMeasureStart(HumdrumFile& infile, int line, const string& style) {
	if (!infile[line].isBarline()) {
		m_humdrum_text << infile[line] << "\n";
		return;
	}

	HumRegex hre;
	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (hre.search(infile.token(line, j), "=(\\d*)(.*)", "")) {
			if (style == "==") {
				m_humdrum_text << "==";
				m_humdrum_text << hre.getMatch(1);
			} else {
				m_humdrum_text << "=";
				m_humdrum_text << hre.getMatch(1);
				m_humdrum_text << style;
			}
		} else {
			if (style == "==") {
				m_humdrum_text << "==";
			} else {
				m_humdrum_text << "=" << style;
			}
		}
		if (j < infile[line].getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

	if (m_barnumtextQ) {
		int barline = 0;
		sscanf(infile.token(line, 0)->c_str(), "=%d", &barline);
		if (barline > 0) {
			m_humdrum_text << "!!LO:TX:Z=20:X=-25:t=" << barline << endl;
		}
	}
}


//////////////////////////////
//
// Tool_myank::printDoubleBarline --
//

void Tool_myank::printDoubleBarline(HumdrumFile& infile, int line) {

	if (!infile[line].isBarline()) {
		m_humdrum_text << infile[line] << "\n";
		return;
	}

	HumRegex hre;
	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (hre.search(infile.token(line, j), "(=\\d*)(.*)", "")) {
			m_humdrum_text << hre.getMatch(1);
			m_humdrum_text << "||";
		} else {
			m_humdrum_text << "=||";
		}
		if (j < infile[line].getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

	if (m_barnumtextQ) {
		int barline = 0;
		sscanf(infile.token(line, 0)->c_str(), "=%d", &barline);
		if (barline > 0) {
			m_humdrum_text << "!!LO:TX:Z=20:X=-25:t=" << barline << endl;
		}
	}

}



//////////////////////////////
//
// Tool_myank::printInvisibleMeasure --
//

void Tool_myank::printInvisibleMeasure(HumdrumFile& infile, int line) {
	if (!infile[line].isBarline()) {
		m_humdrum_text << infile[line] << "\n";
		return;
	}

	HumRegex hre;
	int j;
	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (infile.token(line, j)->find('-') != string::npos) {
			m_humdrum_text << infile.token(line, j);
			if (j < infile[line].getFieldCount()-1) {
				m_humdrum_text << "\t";
			}
			continue;
		}
		if (hre.search(infile.token(line, j), "(=\\d*)(.*)", "")) {
			m_humdrum_text << hre.getMatch(1);
			// m_humdrum_text << "-";
			m_humdrum_text << hre.getMatch(2);
		} else {
			m_humdrum_text << infile.token(line, j);
		}
		if (j < infile[line].getFieldCount()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_myank::reconcileSpineBoundary -- merge spines correctly between segments.
//    will not be able to handle all permutations of spine manipulators.
//    So don't expect exotic manipulators to work...
//

void Tool_myank::reconcileSpineBoundary(HumdrumFile& infile, int index1, int index2) {

	if (m_debugQ) {
		m_humdrum_text << "RECONCILING LINES " << index1+1 << " and " << index2+1 << endl;
		m_humdrum_text << "FIELD COUNT OF " << index1+1 << " is "
			            << infile[index1].getFieldCount() << endl;
		m_humdrum_text << "FIELD COUNT OF " << index2+1 << " is "
			            << infile[index2].getFieldCount() << endl;
	}

	// check to see if any changes need reconciling; otherwise, exit function
	int i, j;
	if (infile[index1].getFieldCount() == infile[index2].getFieldCount()) {
		int same = 1;
		for (i=0; i<infile[index1].getFieldCount(); i++) {
			if (infile.token(index1,i)->getSpineInfo() != infile.token(index2, i)->getSpineInfo()) {
				same = 0;
			}
		}
		if (same != 0) {
			return;
		}
	}

	// handle splits all at once
	string buff1;
	string buff2;

	vector<int> splits(infile[index1].getFieldCount());
	fill(splits.begin(), splits.end(), 0);

	int hassplit = 0;
	for (i=0; i<infile[index1].getFieldCount(); i++) {
		buff1 = "(";
		buff1 += infile.token(index1, i)->getSpineInfo();
		buff1 += ")";
		buff2 = buff1;
		buff1 += "a";
		buff2 += "b";
		for (j=0; j<infile[index2].getFieldCount()-1; j++) {
			if ((buff1 == infile.token(index2, j)->getSpineInfo()
					&& (buff2 == infile.token(index2,j+1)->getSpineInfo()))) {
				splits[i] = 1;
				hassplit++;
			}
		}
	}

	if (hassplit) {
		for (i=0; i<(int)splits.size(); i++) {
			if (splits[i]) {
				m_humdrum_text << "*^";
			} else {
				m_humdrum_text << '*';
			}
			if (i < (int)splits.size()-1) {
				m_humdrum_text << '\t';
			}
		}
		m_humdrum_text << '\n';
	}

	// make splits cumulative;
	//for (i=1; i<(int)splits.size(); i++) {
	//   splits[i] += splits[i-1];
	//}

	HumRegex hre1;
	HumRegex hre2;
	// handle joins one at a time, only look for binary joins at the moment.
	// assuming that no *x has been used to mix the voices up.
	for (i=0; i<infile[index1].getFieldCount()-1; i++) {
		if (!hre1.search(infile.token(index1, i)->getSpineInfo(), "\\((.*)\\)a")) {
			continue;
		}
		if (!hre2.search(infile.token(index1, i+1)->getSpineInfo(), "\\((.*)\\)b")) {
			continue;
		}
		if (hre1.getMatch(1) != hre2.getMatch(1)) {
			// spines are not split from same source
			continue;
		}

		// found an "a" and "b" portion of a spine split, now search
		// through the target line for a joint of those two sub-spines
		for (j=0; j<infile[index2].getFieldCount(); j++) {
			if (infile.token(index2, j)->getSpineInfo() != hre1.getMatch(1)) {
				continue;
			}
			// found a simple binary spine join: emmit a spine manipulator line
			printJoinLine(splits, i, 2);
		}
	}

	// handle *x switches, not perfect since ordering might need to be
	// handled between manipulators...

}



//////////////////////////////
//
// Tool_myank::printJoinLine -- count is currently ignored, but may in the future
//    allow for more than two spines to join at the same time.
//

void Tool_myank::printJoinLine(vector<int>& splits, int index, int count) {
	int i;
	for (i=0; i<(int)splits.size(); i++) {
		if (i == index) {
			m_humdrum_text << "*v\t*v";
			i+=count-1;
		} else {
			m_humdrum_text << "*";
		}
		if (i<(int)splits.size()-1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";

	// merge splits by one element
	for (i=index+1; i<(int)splits.size()-1; i++) {
		splits[i] = splits[i+1];
	}
	splits.resize(splits.size()-1);
}



//////////////////////////////
//
// Tool_myank::reconcileStartingPosition -- merge spines from start of data and
//    first measure in output.
//

void Tool_myank::reconcileStartingPosition(HumdrumFile& infile, int index2) {
	int i;
	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			reconcileSpineBoundary(infile, i, index2);
			break;
		}
	}
}



//////////////////////////////
//
// Tool_myank::printStarting -- print header information before start of data.
//

void Tool_myank::printStarting(HumdrumFile& infile) {
	int i, j;
	int exi = -1;
	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			// the first interpretation is the exclusive one
			m_humdrum_text << infile[i] << "\n";
			exi = i;
			break;
		}
		if (!m_hideStarting) {
			m_humdrum_text << infile[i] << "\n";
		} else {
			if (infile[i].rfind("!!!RDF", 0) == 0) {
				m_humdrum_text << infile[i] << "\n";
			}
		}
	}

	int hasI = 0;

	if (m_instrumentQ) {
		// print any tandem interpretations which start with *I found
		// at the start of the data before measures, notes, or any
		// spine manipulator lines
		for (i=exi+1; i<infile.getLineCount(); i++) {
			if (infile[i].isData()) {
				break;
			}
			if (infile[i].isBarline()) {
				break;
			}
			if (!infile[i].isInterpretation()) {
				continue;
			}
			if (infile[i].isManipulator()) {
				break;
			}
			hasI = 0;
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (infile.token(i, j)->compare(0, 2, "*I") == 0) {
					hasI = 1;
					break;
				}
			}
			if (hasI) {
				for (j=0; j<infile[i].getFieldCount(); j++) {
					if (infile.token(i, j)->compare(0, 2, "*I") == 0) {
						m_humdrum_text << infile.token(i, j);
					} else {
						m_humdrum_text << "*";
					}
					if (j < infile[i].getFieldCount() - 1) {
						m_humdrum_text << "\t";
					}
				}
				m_humdrum_text << "\n";
			}
		}
	}

}



//////////////////////////////
//
// Tool_myank::printEnding -- print the spine terminators and any
//     content after the end of the data.
//

void Tool_myank::printEnding(HumdrumFile& infile, int lastline, int adjlin) {
	if (m_debugQ) {
		m_humdrum_text << "IN printEnding" << endl;
	}
	int ending = -1;
	int marker = -1;
	int i;
	for (i=infile.getLineCount()-1; i>=0; i--) {
		if (infile[i].isInterpretation() && (ending <0)
				&& (*infile.token(i, 0) == "*-")) {
			ending = i;
		}
		if (infile[i].isData()) {
			marker = i+1;
			break;
		}
		if (infile[i].isBarline()) {
			marker = i+1;
			break;
		}
	}

	if (ending >= 0) {
		reconcileSpineBoundary(infile, adjlin, ending);
	}

	int startline  = ending;
	if (marker >= 0) {
		// capture any comment which occur after the last measure
		// line in the data.
		startline = marker;
	}

	// reconcileSpineBoundary(infile, lastline, startline);

	if (startline >= 0) {
		for (i=startline; i<infile.getLineCount(); i++) {
			if (m_hideEnding && (i > ending)) {
				if (infile[i].rfind("!!!RDF", 0) == 0) {
					m_humdrum_text << infile[i] << "\n";
				}
			} else {
				m_humdrum_text << infile[i] << "\n";
			}
		}
	}

}



//////////////////////////////
//
// Tool_myank::getMeasureStartStop --  Get a list of the (numbered) measures in the
//    input file, and store the start/stop lines for those measures.
//    All data before the first numbered measure is in measure 0.
//    although, if the first measure is not labeled, then ...
//

void Tool_myank::getMeasureStartStop(vector<MeasureInfo>& measurelist, HumdrumFile& infile) {
	measurelist.reserve(infile.getLineCount());
	measurelist.resize(0);

	MeasureInfo current;
	int i, ii;
	int lastend = -1;
	int dataend = -1;
	int barnum1 = -1;
	int barnum2 = -1;
	HumRegex hre;

	insertZerothMeasure(measurelist, infile);

	for (i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			if (*infile.token(i, 0) == "*-") {
				dataend = i;
				break;
			}
		}
		if (!infile[i].isBarline()) {
			continue;
		}
		//if (!hre.search(infile.token(i, 0), "^=.*(\\d+)")) {
		//   continue;
		//}
		//barnum1 = stoi(hre.getMatch(1));
		if (!sscanf(infile.token(i, 0)->c_str(), "=%d", &barnum1)) {
			continue;
		}
		current.clear();
		current.start = i;
		current.num   = barnum1;
		for (ii=i+1; ii<infile.getLineCount(); ii++) {
			if (!infile[ii].isBarline()) {
				continue;
			}
			//if (hre.search(infile.token(ii, 0), "^=.*(\\d+)")) {
			//   barnum2 = stoi(hre.getMatch(1));
			//   current.stop = ii;
			//   lastend = ii;
			//   i = ii - 1;
			//   measurelist.push_back(current);
			//   break;
			//}
			if (hre.search(infile.token(ii, 0), "=[^\\d]*(\\d+)")) {
			// if (sscanf(infile.token(ii, 0), "=%d", &barnum2)) {
				barnum2 = stoi(hre.getMatch(1));
				current.stop = ii;
				lastend = ii;
				i = ii - 1;
				current.file = &infile;
				measurelist.push_back(current);
				break;
			} else {
				if (atEndOfFile(infile, ii)) {
					break;
				}
			}
		}
	}

	int lastdata    = -1;   // last line in file with data
	int lastmeasure = -1;   // last line in file with measure

	for (i=infile.getLineCount()-1; i>=0; i--) {
		if ((lastdata < 0) && infile[i].isData()) {
			lastdata = i;
		}
		if ((lastmeasure < 0) && infile[i].isBarline()) {
			lastmeasure = i;
		}
		if ((lastmeasure >= 0) && (lastdata >= 0)) {
			break;
		}
	}

	if (lastmeasure < lastdata) {
		// no final barline, so set to ignore
		lastmeasure = -1;
		lastdata    = -1;
	}

	if ((barnum2 >= 0) && (lastend >= 0) && (dataend >= 0)) {
		current.clear();
		current.num = barnum2;
		current.start = lastend;
		current.stop = dataend;
		if (lastmeasure > lastdata) {
			current.stop = lastmeasure;
		}
		current.file = &infile;
		measurelist.push_back(current);
	}


}



//////////////////////////////
//
// Tool_myank::getSectionCount -- Count the number of sections in a file according to
//     JRP rules: sections are defined by double barlines. There may be some
//     corner cases to consider.
//

int Tool_myank::getSectionCount(HumdrumFile& infile) {
	int i;
	int count = 0;
	int dataQ = 0;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!dataQ && infile[i].isData()) {
			dataQ = 1;
			count++;
			continue;
		}
		if (infile[i].isBarline()) {
			if (infile.token(i, 0)->find("||") != string::npos) {
				dataQ = 0;
			}
		}
	}
	return count;
}



//////////////////////////////
//
// Tool_myank::getSectionString -- return the measure range of a section.
//

void Tool_myank::getSectionString(string& sstring, HumdrumFile& infile, int sec) {
	int i;
	int first = -1;
	int second = -1;
	int barnum = 0;
	int count = 0;
	int dataQ = 0;
	HumRegex hre;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!dataQ && infile[i].isData()) {
			dataQ = 1;
			count++;
			if (count == sec) {
				first = barnum;
			} else if (count == sec+1) {
				second = barnum - 1;
			}
			continue;
		}
		if (infile[i].isBarline()) {
			if (infile.token(i, 0)->find("||") != string::npos) {
				dataQ = 0;
			}
			if (hre.search(infile.token(i, 0), "(\\d+)")) {
				barnum = hre.getMatchInt(1);
			}
		}
	}
	if (second < 0) {
		second = barnum;
	}
	sstring = to_string(first);
	sstring += "-";
	sstring += to_string(second);
}



//////////////////////////////
//
// Tool_myank::atEndOfFile --
//

int Tool_myank::atEndOfFile(HumdrumFile& infile, int line) {
	int i;
	for (i=line+1; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			return 0;
		}
	}

	return 1;
}



//////////////////////////////
//
// Tool_myank::insertZerothMeasure --
//

void Tool_myank::insertZerothMeasure(vector<MeasureInfo>& measurelist,
		HumdrumFile& infile) {

	HumRegex hre;
	int exinterpline = -1;
	int startline = -1;
	int stopline = -1;
	int i;
	for (i=9; i<infile.getLineCount(); i++) {
		if ((exinterpline < 0) && infile[i].isInterpretation()) {
			exinterpline = i;
		}
		if ((startline < 0) && (infile[i].isData())) {
			startline = i;
		}
		if (infile[i].isBarline() && hre.search(infile.token(i, 0), "^=.*\\d+", "")) {
			stopline = i;
			break;
		}
	}

	if (exinterpline < 0) {
		// somethind weird happend, just return
		return;
	}
	if (startline < 0) {
		// no zeroth measure;
		return;
	}
	if (stopline < 0) {
		// strange situation, no measure numbers
		// consider what to do later...
		return;
	}

	MeasureInfo current;
	current.clear();
	current.num = 0;
	// current.start = startline;
	current.start = exinterpline+1;
	current.stop = stopline;
	measurelist.push_back(current);
}



//////////////////////////////
//
// Tool_myank::expandMeasureOutList -- read the measure list for the sequence of measures
//     to extract.
//

void Tool_myank::expandMeasureOutList(vector<MeasureInfo>& measureout,
		vector<MeasureInfo>& measurein, HumdrumFile& infile,
		const string& optionstring) {

	HumRegex hre;
	// find the largest measure number in the score
	int maxmeasure = -1;
	int minmeasure = -1;
	for (int i=0; i<(int)measurein.size(); i++) {
		if (maxmeasure < measurein[i].num) {
			maxmeasure = measurein[i].num;
		}
		if ((minmeasure == -1) || (minmeasure > measurein[i].num)) {
			minmeasure = measurein[i].num;
		}
	}
	if (maxmeasure <= 0) {
		cerr << "Error: There are no measure numbers present in the data" << endl;
		exit(1);
	}
	if (maxmeasure > 1123123) {
		cerr << "Error: ridiculusly large measure number: " << maxmeasure << endl;
		exit(1);
	}
	if (m_maxQ) {
		if (measurein.size() == 0) {
			m_humdrum_text << 0 << endl;
		} else {
			m_humdrum_text << maxmeasure << endl;
		}
		exit(0);
	} else if (m_minQ) {
		for (int ii=0; ii<infile.getLineCount(); ii++) {
			if (infile[ii].isBarline()) {
				if (hre.search(infile.token(ii, 0), "=\\d", "")) {
					break;
				} else {
					m_humdrum_text << 0 << endl;
					exit(0);
				}
			}
			if (infile[ii].isData()) {
				m_humdrum_text << 0 << endl;
				exit(0);
			}
		}
		if (measurein.size() == 0) {
			m_humdrum_text << 0 << endl;
		} else {
			m_humdrum_text << minmeasure << endl;
		}
		exit(0);
	}

	// create reverse-lookup list
	vector<int> inmap(maxmeasure+1);
	fill(inmap.begin(), inmap.end(), -1);
	for (int i=0; i<(int)measurein.size(); i++) {
		inmap[measurein[i].num] = i;
	}

	fillGlobalDefaults(infile, measurein, inmap);
	string ostring = optionstring;
	removeDollarsFromString(ostring, maxmeasure);

	if (m_debugQ) {
		m_free_text << "Option string expanded: " << ostring << endl;
	}

	hre.replaceDestructive(ostring, "", "\\s+", "g");  // remove any spaces between items.
	hre.replaceDestructive(ostring, "-", "--+", "g");  // remove extra dashes
	int value = 0;
	int start = 0;
	vector<MeasureInfo>& range = measureout;
	range.reserve(10000);
	string searchexp = "^([\\d$-]+[^\\d$-]*)";
	value = hre.search(ostring, searchexp);
	while (value != 0) {
		start += value - 1;
		start += (int)hre.getMatch(1).size();
		processFieldEntry(range, hre.getMatch(1), infile, maxmeasure, measurein, inmap);
		value = hre.search(ostring, start, searchexp);
	}
}



//////////////////////////////
//
// Tool_myank::fillGlobalDefaults -- keep track of the clef, key signature, key, etc.
//

void Tool_myank::fillGlobalDefaults(HumdrumFile& infile, vector<MeasureInfo>& measurein,
		vector<int>& inmap) {
	int i, j;
	HumRegex hre;

	int tracks = infile.getMaxTrack();
	// cerr << "MAX TRACKS " << tracks << " ===============================" << endl;

	vector<MyCoord> currclef(tracks+1);
	vector<MyCoord> currkeysig(tracks+1);
	vector<MyCoord> currkey(tracks+1);
	vector<MyCoord> currtimesig(tracks+1);
	vector<MyCoord> currmet(tracks+1);
	vector<MyCoord> currtempo(tracks+1);

	MyCoord undefMyCoord;
	undefMyCoord.clear();

	fill(currclef.begin(), currclef.end(), undefMyCoord);
	fill(currkeysig.begin(), currkeysig.end(), undefMyCoord);
	fill(currkey.begin(), currkey.end(), undefMyCoord);
	fill(currtimesig.begin(), currtimesig.end(), undefMyCoord);
	fill(currmet.begin(), currmet.end(), undefMyCoord);
	fill(currtempo.begin(), currtempo.end(), undefMyCoord);

	int currmeasure = -1;
	int lastmeasure = -1;
	int datafound   = 0;
	int track;
	int thingy = 0;

	for (i=0; i<infile.getLineCount(); i++) {
		if ((currmeasure == -1) && (thingy == 0) && infile[i].isData()) {
			currmeasure = 0;
		}
		if (infile[i].isBarline()) {
			if (!hre.search(infile.token(i, 0), "(\\d+)", "")) {
				continue;
			}
			thingy = 1;

			// store state of global music values at end of measure
			if (currmeasure >= 0) {
				measurein[inmap[currmeasure]].eclef    = currclef;
				measurein[inmap[currmeasure]].ekeysig  = currkeysig;
				measurein[inmap[currmeasure]].ekey     = currkey;
				measurein[inmap[currmeasure]].etimesig = currtimesig;
				measurein[inmap[currmeasure]].emet     = currmet;
				measurein[inmap[currmeasure]].etempo   = currtempo;
			}

			lastmeasure = currmeasure;
			currmeasure = hre.getMatchInt(1);

			if (currmeasure < (int)inmap.size()) {
				// [20120818] Had to compensate for last measure being single
				// and un-numbered.
				if (inmap[currmeasure] < 0) {
					// [20111008] Had to compensate for "==85" barline
					datafound = 0;
					break;
				}
// cerr << "CURRCLEF: ";
// for (int z=0; z<(int)currclef.size(); z++) {
// cerr << "(" << currclef[z].x << "," << currclef[z].y << ") ";
// }
// cerr << endl;
				measurein[inmap[currmeasure]].sclef    = currclef;
				measurein[inmap[currmeasure]].skeysig  = currkeysig;
				measurein[inmap[currmeasure]].skey     = currkey;
				measurein[inmap[currmeasure]].stimesig = currtimesig;
				// measurein[inmap[currmeasure]].smet     = metstates[i];
				measurein[inmap[currmeasure]].smet     = currmet;
				measurein[inmap[currmeasure]].stempo   = currtempo;
			}

			datafound   = 0;
			continue;
		}
		if (infile[i].isInterpretation()) {
			for (j=0; j<infile[i].getFieldCount(); j++) {
				if (!infile.token(i, j)->isKern()) {
					continue;
				}
				track = infile.token(i, j)->getTrack();

				if ((datafound == 0) && (lastmeasure >= 0)) {
					if (infile.token(i, j)->compare(0, 5, "*clef") == 0) {
						measurein[inmap[currmeasure]].sclef[track].x = -1;
						measurein[inmap[currmeasure]].sclef[track].y = -1;
					} else if (hre.search(infile.token(i, j), "^\\*k\\[.*\\]", "")) {
						measurein[inmap[currmeasure]].skeysig[track].x = -1;
						measurein[inmap[currmeasure]].skeysig[track].y = -1;
					} else if (hre.search(infile.token(i, j), "^\\*[A-G][#-]?:", "i")) {
						measurein[inmap[currmeasure]].skey[track].x = -1;
						measurein[inmap[currmeasure]].skey[track].y = -1;
					} else if (hre.search(infile.token(i, j), R"(^\*M\d+/\d+)")) {
						measurein[inmap[currmeasure]].stimesig[track].x = -1;
						measurein[inmap[currmeasure]].stimesig[track].y = -1;
					} else if (hre.search(infile.token(i, j), R"(^\*met\(.*\))")) {
						measurein[inmap[currmeasure]].smet[track].x = -1;
						measurein[inmap[currmeasure]].smet[track].y = -1;
					} else if (hre.search(infile.token(i, j), "^\\*MM\\d+", "i")) {
						measurein[inmap[currmeasure]].stempo[track].x = -1;
						measurein[inmap[currmeasure]].stempo[track].y = -1;
					}
				}

				if (infile.token(i, j)->compare(0, 5, "*clef") == 0) {
					currclef[track].x = i;
					currclef[track].y = j;
					continue;
				}
				if (hre.search(infile.token(i, j), R"(^\*k\[.*\])")) {
					currkeysig[track].x = i;
					currkeysig[track].y = j;
					continue;
				}
				if (hre.search(infile.token(i, j), "^\\*[A-G][#-]?:", "i")) {
					currkey[track].x = i;
					currkey[track].y = j;
					continue;
				}
				if (hre.search(infile.token(i, j), R"(^\*M\d+/\d+)")) {
					currtimesig[track].x = i;
					currtimesig[track].y = j;
					continue;
				}
				if (hre.search(infile.token(i, j), R"(^\*met\(.*\))")) {
					currmet[track].x = i;
					currmet[track].y = j;
					continue;
				}
				if (hre.search(infile.token(i, j), R"(^\*MM[\d.]+)")) {
					currtempo[track].x = i;
					currtempo[track].y = j;
					continue;
				}

			}
		}
		if (infile[i].isData()) {
			datafound = 1;
		}
	}

	// store state of global music values at end of music
	if ((currmeasure >= 0) && (currmeasure < (int)inmap.size())
			&& (inmap[currmeasure] >= 0)) {
		measurein[inmap[currmeasure]].eclef    = currclef;
		measurein[inmap[currmeasure]].ekeysig  = currkeysig;
		measurein[inmap[currmeasure]].ekey     = currkey;
		measurein[inmap[currmeasure]].etimesig = currtimesig;
		measurein[inmap[currmeasure]].emet     = currmet;
		measurein[inmap[currmeasure]].etempo   = currtempo;
	}

	// go through the measure list and clean up start/end states
	for (i=0; i<(int)measurein.size()-2; i++) {

		if (measurein[i].sclef.size() == 0) {
			measurein[i].sclef.resize(tracks+1);
			fill(measurein[i].sclef.begin(), measurein[i].sclef.end(), undefMyCoord);
		}
		if (measurein[i].eclef.size() == 0) {
			measurein[i].eclef.resize(tracks+1);
			fill(measurein[i].eclef.begin(), measurein[i].eclef.end(), undefMyCoord);
		}
		if (measurein[i+1].sclef.size() == 0) {
			measurein[i+1].sclef.resize(tracks+1);
			fill(measurein[i+1].sclef.begin(), measurein[i+1].sclef.end(), undefMyCoord);
		}
		if (measurein[i+1].eclef.size() == 0) {
			measurein[i+1].eclef.resize(tracks+1);
			fill(measurein[i+1].eclef.begin(), measurein[i+1].eclef.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].sclef.size(); j++) {
			if (!measurein[i].eclef[j].isValid()) {
				if (measurein[i].sclef[j].isValid()) {
					measurein[i].eclef[j] = measurein[i].sclef[j];
				}
			}
			if (!measurein[i+1].sclef[j].isValid()) {
				if (measurein[i].eclef[j].isValid()) {
					measurein[i+1].sclef[j] = measurein[i].eclef[j];
				}
			}
		}

		if (measurein[i].skeysig.size() == 0) {
			measurein[i].skeysig.resize(tracks+1);
			fill(measurein[i].skeysig.begin(), measurein[i].skeysig.end(), undefMyCoord);
		}
		if (measurein[i].ekeysig.size() == 0) {
			measurein[i].ekeysig.resize(tracks+1);
			fill(measurein[i].ekeysig.begin(), measurein[i].ekeysig.end(), undefMyCoord);
		}
		if (measurein[i+1].skeysig.size() == 0) {
			measurein[i+1].skeysig.resize(tracks+1);
			fill(measurein[i+1].skeysig.begin(), measurein[i+1].skeysig.end(), undefMyCoord);
		}
		if (measurein[i+1].ekeysig.size() == 0) {
			measurein[i+1].ekeysig.resize(tracks+1);
			fill(measurein[i+1].ekeysig.begin(), measurein[i+1].ekeysig.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].skeysig.size(); j++) {
			if (!measurein[i].ekeysig[j].isValid()) {
				if (measurein[i].skeysig[j].isValid()) {
					measurein[i].ekeysig[j] = measurein[i].skeysig[j];
				}
			}
			if (!measurein[i+1].skeysig[j].isValid()) {
				if (measurein[i].ekeysig[j].isValid()) {
					measurein[i+1].skeysig[j] = measurein[i].ekeysig[j];
				}
			}
		}

		if (measurein[i].skey.size() == 0) {
			measurein[i].skey.resize(tracks+1);
			fill(measurein[i].skey.begin(), measurein[i].skey.end(), undefMyCoord);
		}
		if (measurein[i].ekey.size() == 0) {
			measurein[i].ekey.resize(tracks+1);
			fill(measurein[i].ekey.begin(), measurein[i].ekey.end(), undefMyCoord);
		}
		if (measurein[i+1].skey.size() == 0) {
			measurein[i+1].skey.resize(tracks+1);
			fill(measurein[i+1].skey.begin(), measurein[i+1].skey.end(), undefMyCoord);
		}
		if (measurein[i+1].ekey.size() == 0) {
			measurein[i+1].ekey.resize(tracks+1);
			fill(measurein[i+1].ekey.begin(), measurein[i+1].ekey.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].skey.size(); j++) {
			if (!measurein[i].ekey[j].isValid()) {
				if (measurein[i].skey[j].isValid()) {
					measurein[i].ekey[j] = measurein[i].skey[j];
				}
			}
			if (!measurein[i+1].skey[j].isValid()) {
				if (measurein[i].ekey[j].isValid()) {
					measurein[i+1].skey[j] = measurein[i].ekey[j];
				}
			}
		}

		if (measurein[i].stimesig.size() == 0) {
			measurein[i].stimesig.resize(tracks+1);
			fill(measurein[i].stimesig.begin(), measurein[i].stimesig.end(), undefMyCoord);
		}
		if (measurein[i].etimesig.size() == 0) {
			measurein[i].etimesig.resize(tracks+1);
			fill(measurein[i].etimesig.begin(), measurein[i].etimesig.end(), undefMyCoord);
		}
		if (measurein[i+1].stimesig.size() == 0) {
			measurein[i+1].stimesig.resize(tracks+1);
			fill(measurein[i+1].stimesig.begin(), measurein[i+1].stimesig.end(), undefMyCoord);
		}
		if (measurein[i+1].etimesig.size() == 0) {
			measurein[i+1].etimesig.resize(tracks+1);
			fill(measurein[i+1].etimesig.begin(), measurein[i+1].etimesig.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].stimesig.size(); j++) {
			if (!measurein[i].etimesig[j].isValid()) {
				if (measurein[i].stimesig[j].isValid()) {
					measurein[i].etimesig[j] = measurein[i].stimesig[j];
				}
			}
			if (!measurein[i+1].stimesig[j].isValid()) {
				if (measurein[i].etimesig[j].isValid()) {
					measurein[i+1].stimesig[j] = measurein[i].etimesig[j];
				}
			}
		}

		if (measurein[i].smet.size() == 0) {
			measurein[i].smet.resize(tracks+1);
			fill(measurein[i].smet.begin(), measurein[i].smet.end(), undefMyCoord);
		}
		if (measurein[i].emet.size() == 0) {
			measurein[i].emet.resize(tracks+1);
			fill(measurein[i].emet.begin(), measurein[i].emet.end(), undefMyCoord);
		}
		if (measurein[i+1].smet.size() == 0) {
			measurein[i+1].smet.resize(tracks+1);
			fill(measurein[i+1].smet.begin(), measurein[i+1].smet.end(), undefMyCoord);
		}
		if (measurein[i+1].emet.size() == 0) {
			measurein[i+1].emet.resize(tracks+1);
			fill(measurein[i+1].emet.begin(), measurein[i+1].emet.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].smet.size(); j++) {
			if (!measurein[i].emet[j].isValid()) {
				if (measurein[i].smet[j].isValid()) {
					measurein[i].emet[j] = measurein[i].smet[j];
				}
			}
			if (!measurein[i+1].smet[j].isValid()) {
				if (measurein[i].emet[j].isValid()) {
					measurein[i+1].smet[j] = measurein[i].emet[j];
				}
			}
		}

		if (measurein[i].stempo.size() == 0) {
			measurein[i].stempo.resize(tracks+1);
			fill(measurein[i].stempo.begin(), measurein[i].stempo.end(), undefMyCoord);
		}
		if (measurein[i].etempo.size() == 0) {
			measurein[i].etempo.resize(tracks+1);
			fill(measurein[i].etempo.begin(), measurein[i].etempo.end(), undefMyCoord);
		}
		if (measurein[i+1].stempo.size() == 0) {
			measurein[i+1].stempo.resize(tracks+1);
			fill(measurein[i+1].stempo.begin(), measurein[i+1].stempo.end(), undefMyCoord);
		}
		if (measurein[i+1].etempo.size() == 0) {
			measurein[i+1].etempo.resize(tracks+1);
			fill(measurein[i+1].etempo.begin(), measurein[i+1].etempo.end(), undefMyCoord);
		}
		for (j=1; j<(int)measurein[i].stempo.size(); j++) {
			if (!measurein[i].etempo[j].isValid()) {
				if (measurein[i].stempo[j].isValid()) {
					measurein[i].etempo[j] = measurein[i].stempo[j];
				}
			}
			if (!measurein[i+1].stempo[j].isValid()) {
				if (measurein[i].etempo[j].isValid()) {
					measurein[i+1].stempo[j] = measurein[i].etempo[j];
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_myank::processFieldEntry --
//   3-6 expands to 3 4 5 6
//   $   expands to maximum spine track
//   $0  expands to maximum spine track
//   $1  expands to maximum spine track minus 1, etc.
//   2-$1 expands to 2 through the maximum minus one.
//   6-3 expands to 6 5 4 3
//   $2-5 expands to the maximum minus 2 down through 5.
//   Ignore negative values and values which exceed the maximum value.
//

void Tool_myank::processFieldEntry(vector<MeasureInfo>& field,
		const string& str, HumdrumFile& infile, int maxmeasure,
		vector<MeasureInfo>& inmeasures, vector<int>& inmap) {

	MeasureInfo current;

	HumRegex hre;
	string buffer = str;

	// remove any comma left at end of input string (or anywhere else)
	hre.replaceDestructive(buffer, "", ",", "g");

	string measureStyling = "";
	if (hre.search(buffer, "([|:!=]+)$")) {
		measureStyling = hre.getMatch(1);
		hre.replaceDestructive(buffer, "", "([|:!=]+)$");
	}

	if (hre.search(buffer, "^(\\d+)[a-z]?-(\\d+)[a-z]?$")) {
		// processing a measure range
		int firstone = hre.getMatchInt(1);
		int lastone  = hre.getMatchInt(2);

		// limit the range to 0 to maxmeasure
		if (firstone > maxmeasure) { firstone = maxmeasure; }
		if (lastone  > maxmeasure) { lastone  = maxmeasure; }
		if (firstone < 0         ) { firstone = 0         ; }
		if (lastone  < 0         ) { lastone  = 0         ; }

		if ((firstone < 1) && (firstone != 0)) {
			cerr << "Error: range token: \"" << str << "\""
				  << " contains too small a number at start: " << firstone << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			exit(1);
		}
		if ((lastone < 1) && (lastone != 0)) {
			cerr << "Error: range token: \"" << str << "\""
				  << " contains too small a number at end: " << lastone << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			exit(1);
		}

		if (firstone > lastone) {
			// reverse the order of the measures
			for (int i=firstone; i>=lastone; i--) {
				if (inmap[i] >= 0) {
					current.clear();
					current.file = &infile;
					current.num = i;
					current.start = inmeasures[inmap[i]].start;
					current.stop = inmeasures[inmap[i]].stop;

					current.sclef    = inmeasures[inmap[i]].sclef;
					current.skeysig  = inmeasures[inmap[i]].skeysig;
					current.skey     = inmeasures[inmap[i]].skey;
					current.stimesig = inmeasures[inmap[i]].stimesig;
					current.smet     = inmeasures[inmap[i]].smet;
					current.stempo   = inmeasures[inmap[i]].stempo;

					current.eclef    = inmeasures[inmap[i]].eclef;
					current.ekeysig  = inmeasures[inmap[i]].ekeysig;
					current.ekey     = inmeasures[inmap[i]].ekey;
					current.etimesig = inmeasures[inmap[i]].etimesig;
					current.emet     = inmeasures[inmap[i]].emet;
					current.etempo   = inmeasures[inmap[i]].etempo;

					field.push_back(current);
				}
			}
		} else {
			// measure range not reversed
			for (int i=firstone; i<=lastone; i++) {
				if (inmap[i] >= 0) {
					current.clear();
					current.file = &infile;
					current.num = i;
					current.start = inmeasures[inmap[i]].start;
					current.stop = inmeasures[inmap[i]].stop;

					current.sclef    = inmeasures[inmap[i]].sclef;
					current.skeysig  = inmeasures[inmap[i]].skeysig;
					current.skey     = inmeasures[inmap[i]].skey;
					current.stimesig = inmeasures[inmap[i]].stimesig;
					current.smet     = inmeasures[inmap[i]].smet;
					current.stempo   = inmeasures[inmap[i]].stempo;

					current.eclef    = inmeasures[inmap[i]].eclef;
					current.ekeysig  = inmeasures[inmap[i]].ekeysig;
					current.ekey     = inmeasures[inmap[i]].ekey;
					current.etimesig = inmeasures[inmap[i]].etimesig;
					current.emet     = inmeasures[inmap[i]].emet;
					current.etempo   = inmeasures[inmap[i]].etempo;

					field.push_back(current);
				}
			}
		}
	} else if (hre.search(buffer, "^(\\d+)([a-z]*)")) {
		// processing a single measure number
		int value = hre.getMatchInt(1);
		// do something with letter later...

		if ((value < 1) && (value != 0)) {
			cerr << "Error: range token: \"" << str << "\""
				  << " contains too small a number at end: " << value << endl;
			cerr << "Minimum number allowed is " << 1 << endl;
			exit(1);
		}
		if (inmap[value] >= 0) {
			current.clear();
			current.file = &infile;
			current.num = value;
			current.start = inmeasures[inmap[value]].start;
			current.stop = inmeasures[inmap[value]].stop;

			current.sclef    = inmeasures[inmap[value]].sclef;
			current.skeysig  = inmeasures[inmap[value]].skeysig;
			current.skey     = inmeasures[inmap[value]].skey;
			current.stimesig = inmeasures[inmap[value]].stimesig;
			current.smet     = inmeasures[inmap[value]].smet;
			current.stempo   = inmeasures[inmap[value]].stempo;

			current.eclef    = inmeasures[inmap[value]].eclef;
			current.ekeysig  = inmeasures[inmap[value]].ekeysig;
			current.ekey     = inmeasures[inmap[value]].ekey;
			current.etimesig = inmeasures[inmap[value]].etimesig;
			current.emet     = inmeasures[inmap[value]].emet;
			current.etempo   = inmeasures[inmap[value]].etempo;

			field.push_back(current);
		}
	}

	field.back().stopStyle = measureStyling;

}



//////////////////////////////
//
// Tool_myank::removeDollarsFromString -- substitute $ sign for maximum track count.
//

void Tool_myank::removeDollarsFromString(string& buffer, int maxx) {
	HumRegex hre;
	HumRegex hre2;
	string tbuf;
	string obuf;
	int outval;
	int value;

	if (m_debugQ) {
		m_free_text << "MEASURE STRING BEFORE DOLLAR REMOVAL: " << buffer << endl;
	}

	while (hre.search(buffer, "(\\$\\d*)", "")) {
		tbuf = hre.getMatch(1);
		if (hre2.search(tbuf, "(\\$\\d+)")) {
		  sscanf(hre2.getMatch(1).c_str(), "$%d", &value);
		  outval = maxx - value;
		} else {
			outval = maxx;
		}

		if (outval < 0) {
			outval = 0;
		}

		tbuf = to_string(outval);
		obuf = "\\";
		obuf += hre.getMatch(1);
		hre.replaceDestructive(buffer, tbuf, obuf);
	}
	if (m_debugQ) {
		m_free_text << "DOLLAR EXPAND: " << buffer << endl;
	}
}






//////////////////////////////
//
// Tool_myank::example -- example function calls to the program.
//

void Tool_myank::example(void) {


}



//////////////////////////////
//
// Tool_myank::usage -- command-line usage description and brief summary
//

void Tool_myank::usage(const string& ommand) {

}


// END_MERGE

} // end namespace hum



