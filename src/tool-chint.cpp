//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May  2 20:00:46 PDT 2024
// Last Modified: Fri May  3 15:43:22 PDT 2024
// Filename:      tool-chint.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-chint.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color harmonic intervals between pairs of parts.
//

#include "tool-chint.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE

#define UNDEFINED_INTERVAL (-1000)
#define REST_INTERVAL (-1001)

/////////////////////////////////
//
// Tool_chint::Tool_chint -- Set the recognized options for the tool.
//

Tool_chint::Tool_chint(void) {
	define("b|bottom-part=i:1",      "bottom part number to colorize, 1-indexed");
	define("c|chromatic-coloring=b", "chromatic coloring");
	define("d|diatonic=b",           "diatonic intervals");
	define("m|middle=b",             "show diatonic intervals between staves");
	define("n|negative=b",           "show diatonic intervals for cross voices");
	define("t|top-part=i:2",         "top part number to colorize, 1-indexed");
	define("i|intervals=b",          "display interval names");
	define("B|no-color-bottom=b",    "do not color top analysis staff");
	define("T|no-color-top=b",       "do not color bottom analysis staff");
	define("8|preserve-octave=b",    "do not collapse P8 to P1");
}



/////////////////////////////////
//
// Tool_chint::run -- Do the main work of the tool.
//

bool Tool_chint::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_chint::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_chint::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_chint::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_chint::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_chint::initialize(void) {
	m_intervalQ = getBoolean("intervals");
	if (getBoolean("chromatic-coloring")) {
		chromaticColoring();
	} else {
		dissonanceColoring();
	}

	if (getBoolean("diatonic")) {
		fillIntervalNamesDiatonic();
	} else {
		fillIntervalNames();
	}

	m_noColorBotQ = getBoolean("no-color-bottom");
	m_noColorTopQ = getBoolean("no-color-top");
	m_negativeQ   = getBoolean("negative");
	m_octaveQ     = getBoolean("preserve-octave");
	m_middleQ     = getBoolean("middle");
}



//////////////////////////////
//
// Tool_chint::processFile --
//

void Tool_chint::processFile(HumdrumFile& infile) {
	initialize();

	vector<HTp> kernSpines;
	kernSpines = infile.getKernSpineStartList();

	int maxIndex = (int)kernSpines.size() - 1;
	int topIndex = -1;
	int botIndex = -1;

	if (getString("top-part") == "$") {
		topIndex = maxIndex;
	} else {
		topIndex = getInteger("top-part") - 1;
	}

	if (getString("bottom-part") == "$") {
		botIndex = maxIndex;
	} else {
		botIndex = getInteger("bottom-part") - 1;
	}

	if (topIndex < botIndex) {
		int temp = botIndex;
		botIndex = topIndex;
		topIndex = temp;
	}

	if ((topIndex < 0) | (botIndex < 0)) {
		return;
	}

	if ((topIndex > maxIndex) | (botIndex > maxIndex)) {
		return;
	}

	if (topIndex == botIndex) {
		return;
	}

	vector<int> topInterval;
	vector<int> botInterval;

	getPartIntervals(botInterval, topInterval, kernSpines[botIndex], kernSpines[topIndex], infile);

	int botTrack = kernSpines[botIndex]->getTrack();
	int topTrack = kernSpines[topIndex]->getTrack();
	insertPartColors(infile, botInterval, topInterval, botTrack, topTrack);
}



//////////////////////////////
//
// Tool_chint::getPartIntervals -- Assuming no *x
//

void Tool_chint::getPartIntervals(vector<int>& botInterval, vector<int>& topInterval,
		HTp botSpine, HTp topSpine, HumdrumFile& infile) {

	m_botPitch.resize(infile.getLineCount());
	m_topPitch.resize(infile.getLineCount());

	std::fill(m_botPitch.begin(), m_botPitch.end(), ".");
	std::fill(m_topPitch.begin(), m_topPitch.end(), ".");

	botInterval.resize(infile.getLineCount());
	topInterval.resize(infile.getLineCount());

	std::fill(botInterval.begin(), botInterval.end(), UNDEFINED_INTERVAL);
	std::fill(topInterval.begin(), topInterval.end(), UNDEFINED_INTERVAL);

	HumRegex hre;
	HTp current = botSpine->getNextToken();
	int ttrack = topSpine->getTrack();
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}

		HTp bot = current;
		int line = current->getLineIndex();
		HTp top = NULL;
		int botField = current->getFieldIndex();
		for (int i=botField+1; i<infile[line].getFieldCount(); i++) {
			HTp token = infile.token(line, i);
			int track = token->getTrack();
			if (track != ttrack) {
				continue;
			}
			top = token;
			break;
		}

		if (!top) {
			cerr << "TOP TOKEN IS NULL. BOTTOM TOKEN: " << bot << endl;
			return;
		}

		HTp botResolve = bot;
		HTp topResolve = top;
		if (botResolve->isNull()) {
			botResolve = botResolve->resolveNull();
		}
		if (topResolve->isNull()) {
			topResolve = topResolve->resolveNull();
		}
		if ((!botResolve) || botResolve->isNull()) {
			current = current->getNextToken();
			continue;
		}
		if ((!topResolve) || topResolve->isNull()) {
			current = current->getNextToken();
			continue;
		}

		if (botResolve->isRest()) {
			topInterval[line] = REST_INTERVAL;
			botInterval[line] = REST_INTERVAL;
			current = current->getNextToken();
			continue;
		}

		if (topResolve->isRest()) {
			botInterval[line] = REST_INTERVAL;
			topInterval[line] = REST_INTERVAL;
			current = current->getNextToken();
			continue;
		}

		m_botPitch[line] = hre.replaceDestructive(*botResolve, "", " .*");
		m_topPitch[line] = hre.replaceDestructive(*topResolve, "", " .*");

		int botB40 = abs(botResolve->getBase40Pitch());
		int topB40 = abs(topResolve->getBase40Pitch());

		int difference = topB40 - botB40;
		int negative = 1;
		if (difference < 0) {
			difference = -difference;
			negative = -1;
		}
		int difference2 = difference % 40;
		if (m_octaveQ && (difference2 == 0)) {
			if (difference != 0) {
				difference2 = 40;
			}
		}

		botInterval.at(line) = negative * difference2;
		topInterval.at(line) = negative * difference2;

		current = current->getNextToken();
	}
}



//////////////////////////////
//
// insertPartColors --
//

void Tool_chint::insertPartColors(HumdrumFile& infile, vector<int>& botInterval,
		vector<int>& topInterval, int botTrack, int topTrack) {

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		vector<string> output;
		output.clear();
		int fields = infile[i].getFieldCount();
		bool botUsed = false;
		bool topUsed = false;
		bool intervalUsed = false;
		for (int j = fields - 1; j >= 0; j--) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (!botUsed && (track == botTrack)) {
				botUsed = true;
				if (!m_noColorBotQ) {
					if (token->isNull()) {
						output.push_back(".");
					} else {
						output.push_back(getColorToken(botInterval[i], infile, i, token));
					}
				}
			}
			if (!topUsed && (track == topTrack)) {
				topUsed = true;
				if (!m_noColorTopQ) {
					output.push_back(getColorToken(topInterval[i], infile, i, token));
				}
				if ((!intervalUsed) && m_intervalQ) {
					intervalUsed = true;
					if (token->isNull()) {
						output.push_back(".");
					} else {
						output.push_back(getIntervalToken(topInterval[i], infile, i));
					}
				}
			}
			output.push_back(*token);
		}
		for (int i=(int)output.size() - 1; i>=0; i--) {
			m_humdrum_text << output[i];
			if (i > 0) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_chint::getColorToken --
//

string Tool_chint::getColorToken(int interval, HumdrumFile& infile, int line, HTp token) {
	int absinterval = interval;
	if (absinterval > UNDEFINED_INTERVAL) {
		absinterval = abs(absinterval);
	}
	if (infile[line].isData()) {
		if (interval == REST_INTERVAL) {
			return "black";
		}
		if (interval == UNDEFINED_INTERVAL) {
			return ".";
		}
		if (token->isNull()) {
			return ".";
		}
		if (absinterval > 40) {
			return ".";
		} else {
			return m_color.at(absinterval);
		}
	}
	if (infile[line].isLocalComment()) {
		return "!";
	}
	HTp firstToken = infile.token(line, 0);
	if (firstToken->compare(0, 2, "**") == 0) {
		return "**color";
	}
	if (*firstToken == "*-") {
		return "*-";
	}
	if (firstToken->compare(0, 1, "*") == 0) {
		return "*";
	}
	if (firstToken->isBarline()) {
		return *firstToken;
	}
	return "ERROR";
}



//////////////////////////////
//
// Tool_chint::getIntervalToken --
//

string Tool_chint::getIntervalToken(int interval, HumdrumFile& infile, int line) {
	int absinterval = interval;
	if (interval > UNDEFINED_INTERVAL) {
		absinterval = abs(absinterval);
	}
	HumRegex hre;
	if (infile[line].isData()) {
		if (absinterval < 0) {
			return ".";
		}
		bool botTieQ = false;
		bool topTieQ = false;
		if (hre.search(m_botPitch[line], "[\\]_]")) {
			botTieQ = true;
		}
		if (hre.search(m_topPitch[line], "[\\]_]")) {
			topTieQ = true;
		}
		if (botTieQ && topTieQ) {
			return ".";
		}

		if (interval > 40) {
			// Above an octave is not handled.
			return ".";
		} else {
			if (m_negativeQ) {
				if (interval < 0) {
					return "-" + m_intervals.at(absinterval);
				} else {
					return m_intervals.at(absinterval);
				}
			} else {
				return m_intervals.at(absinterval);
			}
		}
	}
	if (infile[line].isLocalComment()) {
		return "!";
	}
	HTp firstToken = infile.token(line, 0);
	if (firstToken->compare(0, 2, "**") == 0) {
		if (!m_middleQ) {
			#ifdef __EMSCRIPTEN__
			return "**adata=hint";
			#else
			return "**hint";
			#endif
		} else {
			#ifdef __EMSCRIPTEN__
			return "**bdata=hint";
			#else
			return "**hint";
			#endif
		}
	}
	if (*firstToken == "*-") {
		return "*-";
	}
	if (firstToken->compare(0, 1, "*") == 0) {
		return "*";
	}
	if (firstToken->isBarline()) {
		return *firstToken;
	}
	return "ERROR";
}



//////////////////////////////
//
// Tool_chint::ChromaticColoring --
//

void Tool_chint::chromaticColoring(void) {

	m_color.resize(41);

	m_color[0]   = "gray";          // P1
	m_color[1]   = "lightgray";     // A1
	m_color[2]   = "gainsboro";     // AA1
	m_color[3]   = "white";         // unused
	m_color[4]   = "navy";          // d2
	m_color[5]   = "darkblue";      // m2
	m_color[6]   = "mediumblue";    // M2
	m_color[7]   = "royalblue";     // A2
	m_color[8]   = "steelblue";     // AA2
	m_color[9]   = "white";         // unused
	m_color[10]  = "darkgreen";     // dd3
	m_color[11]  = "green";         // d3
	m_color[12]  = "limegreen";     // m3
	m_color[13]  = "lawngreen";     // M3
	m_color[14]  = "lightgreen";    // A3
	m_color[15]  = "brown";         // dd4
	m_color[16]  = "darkorange";    // d4
	m_color[17]  = "orange";        // P4
	m_color[18]  = "gold";          // A4
	m_color[19]  = "yellow";        // AA4
	m_color[20]  = "white";         // unused
	m_color[21]  = "mistyrose";     // dd5
	m_color[22]  = "hotpink";       // d5
	m_color[23]  = "red";           // P5
	m_color[24]  = "crimson";       // A5
	m_color[25]  = "firebrick";     // AA5
	m_color[26]  = "white";         // unused
	m_color[27]  = "darkturquoise"; // d6
	m_color[28]  = "turquoise";     // m6
	m_color[29]  = "deepskyblue";   // M6
	m_color[30]  = "lightblue";     // A6
	m_color[31]  = "powderblue";    // AA6
	m_color[32]  = "white";         // unused
	m_color[33]  = "indigo";        // d7
	m_color[34]  = "purple";        // m7
	m_color[35]  = "darkmagenta";   // M7
	m_color[36]  = "mediumorchid";  // A7
	m_color[37]  = "mediumpurple";  // AA7
	m_color[38]  = "slategray";     // dd1
	m_color[39]  = "dimgray";       // d1
	m_color[40]  = "gray";          // P1

}



//////////////////////////////
//
// Tool_chint::dissonanceColoring --
//    gray          = P1 (unison)
//    indigo        = P5 (perfect intervals)
//    darkviolet    = P4 (other perfect intervals)
//    dodgerblue    = M3, M6 (major 3, 6)
//    darkturquoise = m3, m6 (minor 3, 6)
//    limegreen     = M2, m6 (weak dissonance)
//    limegreen     = M2, m6 (weak dissonance)
//    orange        = m2, M6 (strong dissonance)
//    gold          = A4 (strong dissonance)
//    crimson       = d5 (strong dissonance)
//    red           = other
//

void Tool_chint::dissonanceColoring(void) {

	m_color.resize(41);

	m_color[0]   = "gray";          // P1
	m_color[1]   = "red";           // A1
	m_color[2]   = "red";           // AA1
	m_color[3]   = "white";         // unused
	m_color[4]   = "red";           // dd2
	m_color[5]   = "orange";        // m2
	m_color[6]   = "lawngreen";     // M2
	m_color[7]   = "royalblue";     // A2
	m_color[8]   = "steelblue";     // AA2
	m_color[9]   = "white";         // AA1
	m_color[10]  = "red";           // d3
	m_color[11]  = "darkturquoise"; // m3
	m_color[12]  = "dodgerblue";    // M3
	m_color[13]  = "red";           // A3
	m_color[14]  = "red";           // AA3
	m_color[15]  = "red";           // dd4
	m_color[16]  = "red";           // d4
	m_color[17]  = "blueviolet";    // P4
	m_color[18]  = "gold";          // A4
	m_color[19]  = "red";           // AA4
	m_color[20]  = "white";         // unused
	m_color[21]  = "red";           // dd5
	m_color[22]  = "hotpink";       // d5
	m_color[23]  = "purple";        // P5
	m_color[24]  = "red";           // A5
	m_color[25]  = "red";           // AA5
	m_color[26]  = "white";         // unused
	m_color[27]  = "red";           // d6
	m_color[28]  = "darkturquoise"; // m6
	m_color[29]  = "dodgerblue";    // M6
	m_color[30]  = "red";           // A6
	m_color[31]  = "red";           // AA6
	m_color[32]  = "white";         // unused
	m_color[33]  = "chocolate";     // d7
	m_color[34]  = "limegreen";     // m7
	m_color[35]  = "darkorange";    // M7
	m_color[36]  = "red";           // A7
	m_color[37]  = "red";           // AA7
	m_color[38]  = "red";           // dd1
	m_color[39]  = "red";           // d1
	m_color[40]  = "gray";          // P1

}



//////////////////////////////
//
// Tool_chint::fillIntervalNames --
//

void Tool_chint::fillIntervalNames(void) {

	m_intervals.resize(41);

	m_intervals[0]   = "P1";      // C
	m_intervals[1]   = "A1";
	m_intervals[2]   = "AA1";
	m_intervals[3]   = "ERROR";
	m_intervals[4]   = "d2";
	m_intervals[5]   = "m2";
	m_intervals[6]   = "M2";      // D
	m_intervals[7]   = "A2";
	m_intervals[8]   = "AA2";
	m_intervals[9]   = "ERROR";
	m_intervals[10]  = "d3";
	m_intervals[11]  = "m3";
	m_intervals[12]  = "M3";      // E
	m_intervals[13]  = "A3";
	m_intervals[14]  = "AA3";
	m_intervals[15]  = "dd4";
	m_intervals[16]  = "d4";
	m_intervals[17]  = "P4";      // F
	m_intervals[18]  = "A4";
	m_intervals[19]  = "AA4";
	m_intervals[20]  = "ERROR";
	m_intervals[21]  = "dd5";
	m_intervals[22]  = "d5";
	m_intervals[23]  = "P5";      // G
	m_intervals[24]  = "A5";
	m_intervals[25]  = "AA5";
	m_intervals[26]  = "ERROR";
	m_intervals[27]  = "d6";
	m_intervals[28]  = "m6";
	m_intervals[29]  = "M6";      // A
	m_intervals[30]  = "A6";
	m_intervals[31]  = "AA6";
	m_intervals[32]  = "ERROR";
	m_intervals[33]  = "d7";
	m_intervals[34]  = "m7";
	m_intervals[35]  = "M7";      // B
	m_intervals[36]  = "A7";
	m_intervals[37]  = "AA7";
	m_intervals[38]  = "dd1";
	m_intervals[39]  = "d1";
	m_intervals[40]  = "P8";

}



//////////////////////////////
//
// Tool_chint::fillIntervalNamesDiatonic --
//

void Tool_chint::fillIntervalNamesDiatonic(void) {

	m_intervals.resize(41);

	m_intervals[0]   = "1";        // C
	m_intervals[1]   = "A1";
	m_intervals[2]   = "AA1";
	m_intervals[3]   = "ERROR";
	m_intervals[4]   = "d2";
	m_intervals[5]   = "2";
	m_intervals[6]   = "2";        // D
	m_intervals[7]   = "A2";
	m_intervals[8]   = "AA2";
	m_intervals[9]   = "ERROR";
	m_intervals[10]  = "d3";
	m_intervals[11]  = "3";
	m_intervals[12]  = "3";        // E
	m_intervals[13]  = "A3";
	m_intervals[14]  = "AA3";
	m_intervals[15]  = "dd4";
	m_intervals[16]  = "d4";
	m_intervals[17]  = "4";        // F
	m_intervals[18]  = "A4";
	m_intervals[19]  = "AA4";
	m_intervals[20]  = "ERROR";
	m_intervals[21]  = "dd5";
	m_intervals[22]  = "d5";
	m_intervals[23]  = "5";        // G
	m_intervals[24]  = "A5";
	m_intervals[25]  = "AA5";
	m_intervals[26]  = "ERROR";
	m_intervals[27]  = "d6";
	m_intervals[28]  = "6";
	m_intervals[29]  = "6";        // A
	m_intervals[30]  = "A6";
	m_intervals[31]  = "AA6";
	m_intervals[32]  = "ERROR";
	m_intervals[33]  = "d7";
	m_intervals[34]  = "7";
	m_intervals[35]  = "7";        // B
	m_intervals[36]  = "A7";
	m_intervals[37]  = "AA7";
	m_intervals[38]  = "dd1";
	m_intervals[39]  = "d1";
	m_intervals[40]  = "8";

}



// END_MERGE

} // end namespace hum



