//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  6 13:03:22 PST 2023
// Last Modified: Tue Jan 17 16:24:39 PST 2023
// Filename:      tool-deg.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-deg.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze scale degree information from a **kern spines.
// Documentation: https://doc.verovio.humdrum.org/humdrum/scale_degrees
//

#include "tool-deg.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

/////////////////////////////////
//
// Tool_deg::ScaleDegree static member variables:
//

bool Tool_deg::ScaleDegree::m_showTiesQ  = false;
bool Tool_deg::ScaleDegree::m_showZerosQ = false;
bool Tool_deg::ScaleDegree::m_octaveQ = false;
string Tool_deg::ScaleDegree::m_forcedKey = "";



/////////////////////////////////
//
// Tool_deg::Tool_deg -- Set the recognized options for the tool.
//

Tool_deg::Tool_deg(void) {
	define("above=b",                                    "display scale degrees above analyzed staff");
	define("arr|arrow|arrows=b",                         "display scale degree alterations as arrows");
	define("b|boxes|box=b",                              "display scale degrees in boxes");
	define("color=s",                                    "display color for scale degrees");
	define("c|circ|circles|circle=b",                    "display scale degrees in circles");
	define("hat|caret|circumflex=b",                     "display hats on scale degrees");
	define("solf|solfege=b",                             "display (relative) solfege syllables instead of scale degree numbers");
	define("I|no-input=b",                               "do not interleave **deg data with input score in output");
	define("kern=b",                                     "prefix composite rhythm **kern spine with -I option");
	define("k|kern-tracks=s",                            "process only the specified kern spines");
	define("kd|dk|key-default|default-key=s",            "default (initial) key if none specified in data");
	define("kf|fk|key-force|force-key|forced-key=s",     "use the given key for analysing deg data (ignore modulations)");
	define("o|octave|octaves|degree=b",                  "encode octave information int **degree spines");
	define("r|recip=b",                                  "prefix output data with **recip spine with -I option");
	define("t|ties=b",                                   "include scale degrees for tied notes");
	define("s|spine-tracks|spine|spines|track|tracks=s", "process only the specified spines");
	define("0|O|z|zero|zeros=b",                         "show rests as scale degree 0");
	define("resolve-null=b",                             "show scale degrees for tokens without attack");
}



/////////////////////////////////
//
// Tool_deg::run -- Do the main work of the tool.
//

bool Tool_deg::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_deg::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_deg::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_deg::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_deg::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_deg::initialize(void) {
	m_aboveQ   = getBoolean("above");
	m_arrowQ   = getBoolean("arrow");
	m_boxQ     = getBoolean("box");
	m_circleQ  = getBoolean("circle");
	m_colorQ   = getBoolean("color");
	m_hatQ     = getBoolean("hat");
	m_solfegeQ = getBoolean("solfege");

	if (m_colorQ) {
		m_color = getString("color");
	}

	m_degOnlyQ = getBoolean("no-input");
	m_kernQ    = getBoolean("kern");
	m_recipQ   = getBoolean("recip");
	if (m_kernQ) {
		m_recipQ = true;
	}
	m_degTiesQ = getBoolean("ties");
	m_resolveNullQ = getBoolean("resolve-null");
	Tool_deg::ScaleDegree::setShowOctaves(getBoolean("octave"));

	if (getBoolean("spine-tracks")) {
		m_spineTracks = getString("spine-tracks");
	} else if (getBoolean("kern-tracks")) {
		m_kernTracks = getString("kern-tracks");
	}

	if (getBoolean("default-key")) {
		m_defaultKey = getString("default-key");
		if (!m_defaultKey.empty()) {
			if (m_defaultKey[0] != '*') {
				m_defaultKey = "*" + m_defaultKey;
			}
			if (m_defaultKey.find(":") == string::npos) {
				m_defaultKey += ":";
			}
		}
	}

	if (getBoolean("forced-key")) {
		m_defaultKey.clear(); // override --default-key option

		m_forcedKey = getString("forced-key");
		if (!m_forcedKey.empty()) {
			if (m_forcedKey[0] != '*') {
				m_forcedKey = "*" + m_forcedKey;
			}
			if (m_forcedKey.find(":") == string::npos) {
				m_forcedKey += ":";
			}
			Tool_deg::ScaleDegree::setForcedKey(m_forcedKey);
		}
	}

	Tool_deg::ScaleDegree::setShowTies(m_degTiesQ);
	Tool_deg::ScaleDegree::setShowZeros(getBoolean("zeros"));
}



//////////////////////////////
//
// Tool_deg::processFile --
//

void Tool_deg::processFile(HumdrumFile& infile) {
	bool status = setupSpineInfo(infile);
	if (!status) {
		return;
	}

	// Create storage space for scale degree analyses:
	int kernCount = (int)m_selectedKernSpines.size();
	m_degSpines.resize(kernCount);
	for (int i=0; i<kernCount; i++) {
		prepareDegSpine(m_degSpines.at(i), m_selectedKernSpines.at(i), infile);
	}

	// Analyze the scale degrees in the score (for selected spines)
	if (m_degOnlyQ) {
		printDegScore(infile);
	} else {
		printDegScoreInterleavedWithInputScore(infile);
	}
}


//////////////////////////////
//
// Tool_deg::setupSpineInfo --
//

bool Tool_deg::setupSpineInfo(HumdrumFile& infile) {
	infile.getKernSpineStartList(m_kernSpines);

	if (m_kernSpines.empty()) {
		return false;
	}

	// Create a list of only the spine starts that are selected with the -s or -k option.
	// -s spines that are not **kern spines will be ignored, and spine numbers outside
	// of the range of **kern spines in the file will be ignored (such as spine 5 in a
	// file containing four **kern spines).
	m_selectedKernSpines.clear();

	if (!m_kernTracks.empty()) {
		vector<int> tracks = Convert::extractIntegerList(m_kernTracks, (int)m_kernSpines.size());
		// don't allow out-of-sequence values for the tracks list:
		sort(tracks.begin(),tracks.end());
		tracks.erase(unique(tracks.begin(), tracks.end()), tracks.end());
		if (tracks.empty()) {
			return false;
		}
		for (int i=0; i<(int)tracks.size(); i++) {
			int index = tracks.at(i) - 1;
			if ((index < 0) || (index > (int)m_kernSpines.size() - 1)) {
				continue;
			}
			m_selectedKernSpines.push_back(m_kernSpines.at(index));
		}
	} else if (!m_spineTracks.empty()) {
		int maxTrack = infile.getMaxTrack();
		vector<int> tracks = Convert::extractIntegerList(m_spineTracks, maxTrack);
		sort(tracks.begin(),tracks.end());
		tracks.erase(unique(tracks.begin(), tracks.end()), tracks.end());
		if (tracks.empty()) {
			return false;
		}
		for (int i=0; i<(int)tracks.size(); i++) {
			int track = tracks.at(i);
			if ((track < 1) || (track > maxTrack)) {
				continue;
			}
			for (int j=0; j<(int)m_kernSpines.size(); j++) {
				int ktrack = m_kernSpines.at(j)->getTrack();
				if (ktrack == track) {
					m_selectedKernSpines.push_back(m_kernSpines.at(j));
				}
			}
		}
	} else {
		// analyzing all **kern tracks
		m_selectedKernSpines = m_kernSpines;
	}

	if (m_selectedKernSpines.empty()) {
		return false;
	}


	// Finally, store the insertion track for added **deg analysis spines,
	// which is the track number of the next **kern spine (not the next
	// selected **kern spine).  A track of -1 means append the last **deg
	// spine to the end of data lines.
	m_degInsertTrack.resize(m_selectedKernSpines.size());
	for (int i=0; i<(int)m_selectedKernSpines.size(); i++) {
		HTp target = m_selectedKernSpines.at(i);
		for (int j=0; j<(int)m_kernSpines.size(); j++) {
			if (m_kernSpines.at(j) != target) {
				continue;
			}
			if (j < (int)m_kernSpines.size() - 1) {
				m_degInsertTrack.at(i) = m_kernSpines.at(j+1)->getTrack();

			} else {
				m_degInsertTrack.at(i) = -1;
			}
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_deg::printDegScoreInterleavedWithInputScore --
//

void Tool_deg::printDegScoreInterleavedWithInputScore(HumdrumFile& infile) {
	vector<HTp> kernStarts = infile.getKernSpineStartList();
	if (kernStarts.empty()) {
		return;
	}

	m_ipv.clear();

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
		} else {
			m_humdrum_text << createOutputHumdrumLine(infile, i) << endl;
		}
	}
}



//////////////////////////////
//
// Tool_deg::createOutputHumdrumLine -- Print **deg data within input score.  The **deg
//    spine will be placed in the right-most spine after a **kern spine (so just to
//    the left of the next **kern spine.  Perhaps change to staff-like spines such as
//    **mens.
//

string Tool_deg::createOutputHumdrumLine(HumdrumFile& infile, int lineIndex) {

	// Styling interpretation tracking variables:
	bool aboveStatus          = false;
	bool arrowStatus          = false;
	bool boxStatus            = false;
	bool circleStatus         = false;
	bool colorStatus          = false;
	bool hatStatus            = false;
	bool keyDesignationStatus = false;
	bool solfegeStatus        = false;

	// Keep track of an existing styling line and if such a line is found,
	// then insert a styling interpretation for the new **deg spines here
	// rather than just before the first data line.
	if (!m_ipv.foundData) {
		if (!m_defaultKey.empty() && !keyDesignationStatus && !m_ipv.foundKeyDesignationLine) {
			keyDesignationStatus = isKeyDesignationLine(infile, lineIndex);
		}
		if (!m_forcedKey.empty() && !keyDesignationStatus && !m_ipv.foundKeyDesignationLine) {
			keyDesignationStatus = isKeyDesignationLine(infile, lineIndex);
		}
		if (m_aboveQ && !m_ipv.foundAboveLine) {
			aboveStatus = isDegAboveLine(infile, lineIndex);
		}
		if (m_arrowQ && !m_ipv.foundArrowLine) {
			arrowStatus = isDegArrowLine(infile, lineIndex);
		}
		if (m_boxQ && !m_ipv.foundBoxLine) {
			boxStatus = isDegBoxLine(infile, lineIndex);
		}
		if (m_circleQ && !m_ipv.foundCircleLine) {
			circleStatus = isDegCircleLine(infile, lineIndex);
		}
		if (m_colorQ && !m_ipv.foundColorLine) {
			colorStatus = isDegColorLine(infile, lineIndex);
		}
		if (m_hatQ && !m_ipv.foundHatLine) {
			hatStatus = isDegHatLine(infile, lineIndex);
		}
		if (m_solfegeQ && !m_ipv.foundSolfegeLine) {
			solfegeStatus = isDegSolfegeLine(infile, lineIndex);
		}
	}

	// spineData: The output Humdrum data line stored in a 2D vector.
	// The first index for the primary track/spine, and the second is for
	// the subtracks for each track/spine.  This data will be checked
	// for adjacemtn mergers happening between two different tracks
	// (which for backwards compatibility with the Humdrum Toolkit, is not
	// allowed).
	vector<vector<string>> spineData;

	// Interleave the **deg spines into the input line.  The **deg spines
	// are stored just before the matching index of the spine in m_degSpines
	// in the vector m_degInsertTrack.  If the track in m_degInsertTrack
	// is -1 (only possible in the last position of m_degInsertTrack), then
	// that will be handled append at the end of the line (outside of the
	// following loop).

	int  curDegIndex  = 0;
	bool hasDegMerger = false;
	int  track        = -1000;
	int  lasttrack    = -1000;

	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		lasttrack = track;
		track = token->getTrack();

		if ((curDegIndex < (int)m_degSpines.size()) && (track == m_degInsertTrack.at(curDegIndex))) {
			// insert the next **deg spine into spineData
			spineData.resize(spineData.size() + 1);
			for (int j=0; j<(int)m_degSpines.at(curDegIndex).at(lineIndex).size(); j++) {
				string value = m_degSpines.at(curDegIndex).at(lineIndex).at(j).getDegToken();
				checkKeyDesignationStatus(value, keyDesignationStatus);
				checkAboveStatus(value, aboveStatus);
				checkArrowStatus(value, arrowStatus);
				checkBoxStatus(value, boxStatus);
				checkCircleStatus(value, circleStatus);
				checkColorStatus(value, colorStatus);
				checkHatStatus(value, hatStatus);
				checkSolfegeStatus(value, hatStatus);
				spineData.back().push_back(value);
				if (value == "*v") {
					hasDegMerger = true;
				}
			}
			curDegIndex++;
		}

		if (track != lasttrack) {
			spineData.resize(spineData.size() + 1);
		}

		spineData.back().push_back(*token);
	}

	// Add the last **deg spine if necessary:
	if (!m_degInsertTrack.empty() && m_degInsertTrack.back() == -1) {
		spineData.resize(spineData.size() + 1);
		for (int i=0; i<(int)m_degSpines.back().at(lineIndex).size(); i++) {
			string value = m_degSpines.back().at(lineIndex).at(i).getDegToken();
			checkKeyDesignationStatus(value, keyDesignationStatus);
			checkAboveStatus(value, aboveStatus);
			checkArrowStatus(value, arrowStatus);
			checkBoxStatus(value, boxStatus);
			checkCircleStatus(value, circleStatus);
			checkColorStatus(value, colorStatus);
			checkHatStatus(value, hatStatus);
			checkSolfegeStatus(value, hatStatus);
			spineData.back().push_back(value);
			if (value == "*v") {
				hasDegMerger = true;
			}
		}
		curDegIndex++;
	}

	// Keep track of cases where the styling interpretations can be
	// stored in the header (to avoid creating a new line in the
	// output score just before the data to store such interpretations).
	if (keyDesignationStatus) {
		m_ipv.foundKeyDesignationLine = true;
	}
	if (aboveStatus) {
		m_ipv.foundAboveLine = true;
	}
	if (arrowStatus) {
		m_ipv.foundArrowLine = true;
	}
	if (boxStatus) {
		m_ipv.foundBoxLine = true;
	}
	if (circleStatus) {
		m_ipv.foundCircleLine = true;
	}
	if (colorStatus) {
		m_ipv.foundColorLine = true;
	}
	if (hatStatus) {
		m_ipv.foundHatLine = true;
	}
	if (solfegeStatus) {
		m_ipv.foundSolfegeLine = true;
	}


	// if styling interpretation lines were not found before the data,
	// add them just before the data (or change to also before first
	// barline and before first spine manipulator other than exclusive
	// interpretations.
	vector<string> extraLines;
	if (!m_ipv.foundData && infile[lineIndex].isData()) {
		m_ipv.foundData = true;

		if (!m_ipv.foundHatLine) {
			if (m_hatQ && !m_ipv.foundHatLine) {
				string line = printDegInterpretation("*hat", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundColorLine) {
			if (m_colorQ && !m_ipv.foundColorLine) {
				string interp = "*color:";
				interp += m_color;
				string line = printDegInterpretation(interp, infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundBoxLine) {
			if (m_boxQ && !m_ipv.foundBoxLine) {
				string line = printDegInterpretation("*box", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundCircleLine) {
			if (m_circleQ && !m_ipv.foundCircleLine) {
				string line = printDegInterpretation("*circ", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundArrowLine) {
			if (m_arrowQ && !m_ipv.foundArrowLine) {
				string line = printDegInterpretation("*arr", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundAboveLine) {
			if (m_aboveQ && !m_ipv.foundAboveLine) {
				string line = printDegInterpretation("*above", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundSolfegeLine) {
			if (m_solfegeQ && !m_ipv.foundSolfegeLine) {
				string line = printDegInterpretation("*solf", infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundKeyDesignationLine) {
			if (!m_defaultKey.empty() && !m_ipv.foundKeyDesignationLine) {
				string line = printDegInterpretation(m_defaultKey, infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

		if (!m_ipv.foundKeyDesignationLine) {
			if (!m_forcedKey.empty() && !m_ipv.foundKeyDesignationLine) {
				string line = printDegInterpretation(m_forcedKey, infile, lineIndex);
				if (!line.empty()) {
					extraLines.push_back(line);
				}
			}
		}

	}

	if (!hasDegMerger) {
		string output;
		for (int i=0; i<(int)spineData.size(); i++) {
			for (int j=0; j<(int)spineData[i].size(); j++) {
				output += spineData[i][j];
				output += "\t";
			}
		}
		if (!output.empty()) {
			output.resize(output.size() - 1);
		}
		if (extraLines.empty()) {
			return output;
		} else {
			extraLines.push_back(output);
			string newoutput;
			for (int i=0; i<(int)extraLines.size(); i++) {
				newoutput += extraLines[i];
				if (i < (int)extraLines.size() - 1) {
					newoutput += "\n";
				}
			}
			return newoutput;
		}
	}

	// The output contains some spine mergers, so be careful and
	// place any adjacent mergers onto separate lines.
	string output = prepareMergerLine(spineData);
	return output;
}



//////////////////////////////
//
// Tool_deg::checkKeyDesignationStatus --
//

void Tool_deg::checkKeyDesignationStatus(string& value, int keyDesignationStatus) {
	if (keyDesignationStatus && (!m_ipv.foundKeyDesignationLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			if (!m_defaultKey.empty()) {
				value = m_defaultKey;
			} else if (!m_forcedKey.empty()) {
				value = m_forcedKey;
			}
		}
	}
}



//////////////////////////////
//
// Tool_deg::checkAboveStatus -- Add *above interpretation to spine if needed.
//

void Tool_deg::checkAboveStatus(string& value, bool aboveStatus) {
	if (aboveStatus && m_aboveQ && (!m_ipv.foundAboveLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*above";
		}
	}
}



//////////////////////////////
//
// Tool_deg::checkArrowStatus -- Add *arr interpretation to spine if needed.
//

void Tool_deg::checkArrowStatus(string& value, bool arrowStatus) {
	if (arrowStatus && m_arrowQ && (!m_ipv.foundArrowLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*arr";
		}
	}
}



//////////////////////////////
//
// Tool_deg::checkBoxStatus -- Add *box interpretation to spine if needed.
//

void Tool_deg::checkBoxStatus(string& value, bool boxStatus) {
	if (boxStatus && m_boxQ && (!m_ipv.foundBoxLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*box";
		}
	}
}



//////////////////////////////
//
// Tool_deg::checkCircleStatus -- Add *circ interpretation to spine if needed.
//

void Tool_deg::checkCircleStatus(string& value, bool circleStatus) {
	if (circleStatus && m_circleQ && (!m_ipv.foundCircleLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*circ";
		}
	}
}


//////////////////////////////
//
// Tool_deg::checkColorStatus -- Add *color interpretation to spine if needed.
//

void Tool_deg::checkColorStatus(string& value, bool colorStatus) {
	if (colorStatus && m_colorQ && (!m_ipv.foundColorLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*color:";
			value += m_color;
		}
	}
}


//////////////////////////////
//
// Tool_deg::checkHatStatus -- Add *hat interpretation to spine if needed.
//

void Tool_deg::checkHatStatus(string& value, bool hatStatus) {
	if (hatStatus && m_hatQ && (!m_ipv.foundHatLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*hat";
		}
	}
}



//////////////////////////////
//
// Tool_deg::checkSolfegeStatus -- Add *solfege interpretation to spine if needed.
//

void Tool_deg::checkSolfegeStatus(string& value, bool hatStatus) {
	if (hatStatus && m_solfegeQ && (!m_ipv.foundSolfegeLine) && (!m_ipv.foundData)) {
		if (value == "*") {
			value = "*solf";
		}
	}
}



//////////////////////////////
//
// Tool_deg::isKeyDesignationLine -- Returns true if any spine on the line
//    looks like a key designation (such as *G#:loc).
//

bool Tool_deg::isKeyDesignationLine(HumdrumFile& infile, int lineIndex) {
	if (!infile[lineIndex].hasSpines()) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	}
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (token->isKeyDesignation()) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_deg::isDegAboveLine -- Return true if **deg spines only
//     include *above, *below, interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegAboveLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (*token == "*above")  { return true; }
		if (*token == "*below") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegArrowLine -- Return true if **deg spines only
//     include *arr, *Xarr, *acc, *Xacc interpretations
//     and "*" (but not all "*").
//

bool Tool_deg::isDegArrowLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (*token == "*arr")  { return true; }
		if (*token == "*Xarr") { return true; }
		if (*token == "*acc")  { return true; }
		if (*token == "*Xacc") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegBoxLine -- Return true if **deg spines includes
//     any *box, or *Xbox, interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegBoxLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (*token == "*box")  { return true; }
		if (*token == "*Xbox") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegCircleLine -- Return true if **deg spines includes
//     any *circ, or *Xcirc, interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegCircleLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (*token == "*circ")  { return true; }
		if (*token == "*Xcirc") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegColorLine -- Return true if **deg spines only
//     include *color interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegColorLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (token->compare(0, 7, "*color:") == 0)  { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegHatLine -- Return true if **deg spines includes
//     any *hat, or *Xhat, interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegHatLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!(token->isDataType("**deg") || token->isDataType("**degree"))) {
			continue;
		}
		degCount++;
		if (*token == "*hat")  { return true; }
		if (*token == "*Xhat") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::isDegSolfegeLine -- Return true if **deg spines includes
//     any *solf, or *Xsolf, interpretations and "*" (but not all "*").
//

bool Tool_deg::isDegSolfegeLine(HumdrumFile& infile, int lineIndex) {
	// If there are no **deg spines, then don't bother searching for them.
	if (!m_ipv.hasDegSpines) {
		return false;
	}
	if (!infile[lineIndex].isInterpretation()) {
		return false;
	} if (infile[lineIndex].isManipulator()) {
		return false;
	}

	int degCount = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		if (!token->isDataType("**deg")) {
			continue;
		}
		degCount++;
		if (*token == "*solf")  { return true; }
		if (*token == "*Xsolf") { return true; }
	}
	if (degCount == 0) {
		m_ipv.hasDegSpines = false;
	}

	return false;
}



//////////////////////////////
//
// Tool_deg::printDegInterpretation --
//

string Tool_deg::printDegInterpretation(const string& interp, HumdrumFile& infile, int lineIndex) {
	string output;

	int curDegIndex = 0;
	for (int i=0; i<infile[lineIndex].getFieldCount(); i++) {
		HTp token = infile.token(lineIndex, i);
		int track = token->getTrack();

		if ((curDegIndex < (int)m_degSpines.size()) && (track == m_degInsertTrack.at(curDegIndex))) {
			// insert the next **deg spine into spineData
			for (int j=0; j<(int)m_degSpines.at(curDegIndex).at(lineIndex).size(); j++) {
				output += interp;
				output += "\t";
			}
			curDegIndex++;
		}
		output += "*";
		output += "\t";
	}

	// Add the last **deg spine if necessary:
	if (!m_degInsertTrack.empty() && m_degInsertTrack.back() == -1) {
		for (int i=0; i<(int)m_degSpines.back().at(lineIndex).size(); i++) {
			output += interp;
			output += "\t";
		}
		curDegIndex++;
	}
	if (!output.empty()) {
		output.resize(output.size() - 1);
	}
	return output;
}



//////////////////////////////
//
// Tool_deg::prepareMergerLine --
//

string Tool_deg::prepareMergerLine(vector<vector<string>>& merge) {

	// Calculate result of spine manipulations:
	vector<vector<string>> after(merge.size());
	for (int i=0; i<(int)merge.size(); i++) {
		calculateManipulatorOutputForSpine(after.at(i), merge.at(i));
	}

	vector<vector<string>> before(merge.size());
	for (int i=0; i<(int)merge.size(); i++) {
		for (int j=0; j<(int)merge[i].size(); j++) {
			before[i].push_back("*");
		}
	}

	vector<vector<string>> line1(merge.size());
	vector<vector<string>> line2(merge.size());

	// before = spine states before merger
	// merge  = unprocessed merger line
	// after  = spine states after merger
	// Simple example:
	//  (*  * ) (*  * )  A: before line
	//  (*v *v) (*v *v)  B: merge line
	//  (*    ) (*    )  C: after line
	// Output should be:
	// B(*v *v) A(*   * )  line1
	// C(*    ) B(*v	*v)  line2
	//
	// More complicated example:
	//  (*  * ) (*  * ) (*  * ) A
	//  (*v *v) (*v *v) (*v *v) B
	//  (*    ) (*    ) (*    ) C
	//  Output should be:
	//  B(*v *v) A(*   * ) B(*v *v) line1
	//  C(*    ) B(*v  *v) C(*    ) line2
	//
	// Algorithm:
	//   If the current merger is not adjacent to the previous merger
	//       (or there is no previous merger), then copy merger to line1
	//       and after to line2
	//
	//   If the current merger is adjacent to the previous merger
	//       then copy merger to line2 and copy before to line1.
	//
	//   If the current merger is adjacent and the previous merger
	//       was delayed, then copy merger to line1 and after to line2.
	//       (same as first if statement)

	// Keep track delay of each track's merger line for above algorithm.
	vector<bool> delayed;

	for (int i=0; i<(int)merge.size(); i++) {
		if (merge.at(i).empty()) {
			// a track should not have an empty marger spine.
			cerr << "STRANGE CASE 1" << endl;
			continue;
		}
		if ((i > 0) && merge.at(i-1).empty()) {
			cerr << "STRANGE CASE 2" << endl;
			continue;
		}

		if (i == 0) {
			line1.at(i) = merge.at(i);
			line2.at(i) = after.at(i);
			delayed.push_back(false);
		} else if ((merge.at(i).at(0) == "*v") && (merge.at(i-1).back() == "*v")) {
			// the current merge needs to be offset from the previous merge.
			if (delayed.back()) {
				// last spine was already displayed so undelay the next spine:
				line1.at(i) = merge.at(i);
				line1.at(i) = after.at(i);
			} else {
				// last spine was not delayed, so delay this spine:
				line1.at(i) = before.at(i);
				line2.at(i) = merge.at(i);
			}
			delayed.push_back(!delayed.back());
		} else {
			// no delay is needed
			line1.at(i) = merge.at(i);
			line2.at(i) = after.at(i);
			delayed.push_back(false);
		}
	}

	string output;
	for (int i=0; i<(int)line1.size(); i++) {
		for (int j=0; j<(int)line1[i].size(); j++) {
			output += line1[i][j];
			output += "\t";
		}
	}
	output.back() = '\n';
	for (int i=0; i<(int)line2.size(); i++) {
		for (int j=0; j<(int)line2[i].size(); j++) {
			output += line2[i][j];
			output += "\t";
		}
	}
	if (!output.empty()) {
		output.resize(output.size() - 1);
	}
	return output;
}



//////////////////////////////
//
// Tool_deg::calculateManipulatorOutputForSpine -- Deal with *^ *v *- *+ manipulators
//

void Tool_deg::calculateManipulatorOutputForSpine(vector<string>& lineout,
		vector<string>& linein) {

	lineout.clear();
	for (int i=0; i<(int)linein.size(); i++) {
		if (linein[i] == "*^") {
			lineout.push_back("*");
			lineout.push_back("*");
		} else if (linein[i] == "*v") {
			if (i==0) {
				lineout.push_back("*");
			} else if ((i > 0) && (linein[i-1] == "*v")) {
				// do nothing for secondary merger manipulators
			} else {
				// strange data (merging two separate spines?)
				lineout.push_back(linein[i]);
			}
		} else if (linein[i] == "*-") {
			// do nothing
		} else if (linein[i] == "*+") {
			// rare so not well tested (and next line should have an exinterp.)
			lineout.push_back("*");
			lineout.push_back("*");
		} else {
			lineout.push_back(linein[i]);
		}
	}
}



//////////////////////////////
//
// Tool_deg::printDegScore -- **deg spines without any input data.
//

void Tool_deg::printDegScore(HumdrumFile& infile) {
	if (m_degSpines.empty()) {
		return;
	}

	// Variables to keep track of printing interpretations in **recip spine
	HumNum printTimeSignature = -1;
	HumNum printMetricSignature = -1;
	bool foundData  = false;
	bool printClef  = false;
	bool printRecipColor = false;
	bool printStria = false;
	bool printStem  = false;

	// input styling options
	bool printAbove  = !m_aboveQ;
	bool printArrow  = !m_arrowQ;
	bool printBox    = !m_boxQ;
	bool printCircle = !m_circleQ;
	bool printColor  = !m_colorQ;
	bool printHat    = !m_hatQ;
	bool printSolfege = !m_solfegeQ;

	int lineCount = (int)m_degSpines[0].size();
	int spineCount = (int)m_degSpines.size();

	for (int i=0; i<lineCount; i++) {
		if (!m_degSpines[0][i][0].hasSpines()) {
			m_humdrum_text << m_degSpines[0][i][0].getLinkedKernToken() << endl;
			continue;
		}

		// check for **recip spine display options before
		// first data line.
		if (!foundData && m_degSpines[0][i][0].isDataToken()) {
			foundData = true;

			// Add **recip styling options that have not yet been
			// given in the spine.
			if (m_recipQ) {
				if (!printClef) {
					string line = createRecipInterpretation("*clefXyy", i);
					m_humdrum_text << line << endl;
					printClef = true;
				}
				if (!printStria) {
					string line = createRecipInterpretation("*stria0", i);
					m_humdrum_text << line << endl;
					printStria = true;
				}
				if (!printRecipColor) {
					string line = createRecipInterpretation("*color:#fff0", i);
					m_humdrum_text << line << endl;
					printRecipColor = true;
				}
				if (!printStem) {
					string line = createRecipInterpretation("*Xstem", i);
					m_humdrum_text << line << endl;
					printStem = true;
				}

			}

			if (!printAbove) {
				string line = createDegInterpretation("*above", i, m_recipQ);
				m_humdrum_text << line << endl;
				printAbove = true;
			}

			if (!printArrow) {
				string line = createDegInterpretation("*arr", i, m_recipQ);
				m_humdrum_text << line << endl;
				printArrow = true;
			}

			if (!printBox) {
				string line = createDegInterpretation("*box", i, m_recipQ);
				m_humdrum_text << line << endl;
				printBox = true;
			}

			if (!printCircle) {
				string line = createDegInterpretation("*circ", i, m_recipQ);
				m_humdrum_text << line << endl;
				printCircle = true;
			}

			if (!printColor) {
				string interp = "*color:";
				interp += m_color;
				string line = createDegInterpretation(interp, i, m_recipQ);
				m_humdrum_text << line << endl;
				printColor = true;
			}

			if (!printHat) {
				string line = createDegInterpretation("*hat", i, m_recipQ);
				m_humdrum_text << line << endl;
				printHat = true;
			}

			if (!printSolfege) {
				string line = createDegInterpretation("*solf", i, m_recipQ);
				m_humdrum_text << line << endl;
				printSolfege = true;
			}

		}

		for (int j=0; j<spineCount; j++) {

			// recip spine generation:
			if (m_recipQ && (j == 0)) {
				HTp token = infile.token(i, 0);
				if (infile[i].isExclusiveInterpretation()) {
					if (m_kernQ) {
						m_humdrum_text << "**kern";
					} else {
						m_humdrum_text << "**recip";
					}
				} else if (infile[i].isManipulator()) {
					if (*token == "*-") {
						m_humdrum_text << "*-";
					} else {
						m_humdrum_text << "*";
					}
				} else if (infile[i].isInterpretation()) {
					HumNum timestamp = infile[i].getDurationFromStart();
					string timesig;
					string metersig;
					string clef;

					if (timestamp != printTimeSignature) {
						for (int jj=0; jj<infile[i].getFieldCount(); jj++) {
							HTp token = infile.token(i, jj);
							if (!token->isKern()) {
								continue;
							}
							if (token->isTimeSignature()) {
								timesig = *token;
								break;
							}
						}
					}

					if (timestamp != printMetricSignature) {
						for (int jj=0; jj<infile[i].getFieldCount(); jj++) {
							HTp token = infile.token(i, jj);
							if (!token->isKern()) {
								continue;
							}
							if (token->isMeterSignature()) {
								metersig = *token;
								break;
							}
						}
					}

					if (!printClef) {
						for (int jj=0; jj<infile[i].getFieldCount(); jj++) {
							HTp token = infile.token(i, jj);
							if (!token->isKern()) {
								continue;
							}
							if (token->isClef()) {
								clef = "*clefXyy";
								break;
							}
						}
					}

					if (!timesig.empty()) {
						m_humdrum_text << timesig;
						printTimeSignature = timestamp;
					} else if (!metersig.empty()) {
						m_humdrum_text << metersig;
						printMetricSignature = timestamp;
					} else if (!clef.empty()) {
						m_humdrum_text << clef;
						printClef = true;
					} else {
						m_humdrum_text << "*";
					}
				} else if (infile[i].isBarline()) {
					m_humdrum_text << token;
				} else if (infile[i].isLocalComment()) {
					m_humdrum_text << "!";
				} else if (infile[i].isData()) {
					m_humdrum_text << Convert::durationToRecip(infile[i].getDuration());
					if (m_kernQ) {
						m_humdrum_text << m_kernSuffix;
					}
				}
			}
			// end of recip spine generation

			// Print deg spines
			int subspineCount = (int)m_degSpines.at(j).at(i).size();
			for (int k=0; k<subspineCount; k++) {
				if ((j == 0) && (k == 0)) {
					if (m_recipQ) {
						m_humdrum_text << "\t";
					}
				} else if ((j == 0) && (k > 0)) {
					m_humdrum_text << "\t";
				} else if (j > 0) {
					m_humdrum_text << "\t";
				}
				m_humdrum_text << m_degSpines[j][i][k];
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_deg::createRecipInterpretation -- Add a new line to the output of a **deg-only
//    score for a styling option in the **recip (**kern) spine at the start of the
//    line.  The **deg spines all get null interpretations.
//    i = the line index in the proto **deg score that will be used to count the
//        number of interpretations that need to be added.
//

string Tool_deg::createRecipInterpretation(const string& starttok,  int refLine) {
	string output = starttok;
	int count = 0;
	for (int j=0; j < (int)m_degSpines.size(); j++) {
		count += (int)m_degSpines.at(j).at(refLine).size();
	}
	for (int i=0; i<count; i++) {
		output += "\t*";
	}
	return output;
}



//////////////////////////////
//
// Tool_deg::createDegInterpretation -- Add a new line to the output of a **deg-only
//    score for a styling option in the **deg spines.  The
//    line.  The **deg spines all get null interpretations.
//    i = the line index in the proto **deg score that will be used to count the
//        number of interpretations that need to be added.
//

string Tool_deg::createDegInterpretation(const string& degtok,  int refLine, bool addPreSpine) {
	string output;
	if (addPreSpine) {
		output += "*\t";
	}
	int count = 0;
	for (int j=0; j < (int)m_degSpines.size(); j++) {
		count += (int)m_degSpines.at(j).at(refLine).size();
	}
	for (int i=0; i<count; i++) {
		if (i != 0) {
			output += "\t";
		}
		output += degtok;
	}

	return output;
}



//////////////////////////////
//
// Tool_deg::prepareDegSpine -- Convert one **kern spine into a **deg spine.
//

void Tool_deg::prepareDegSpine(vector<vector<ScaleDegree>>& degspine, HTp kernstart,
		HumdrumFile& infile) {
	string mode = "unknown";
	int b40tonic = -1;

	if (!m_defaultKey.empty()) {
		getModeAndTonic(mode, b40tonic, m_defaultKey);
	} else if (!m_forcedKey.empty()) {
		getModeAndTonic(mode, b40tonic, m_forcedKey);
	}

	int lineCount = infile.getLineCount();

	degspine.resize(lineCount);
	int track = kernstart->getTrack();
	HTp current = kernstart;

	bool isUnpitched = false;

	while (current) {
		int line = current->getLineIndex();
		if (!current->getOwner()->hasSpines()) {
			degspine.at(line).resize(1);
			degspine.at(line).back().setLinkedKernToken(current, mode, b40tonic, isUnpitched, m_resolveNullQ);
			current = current->getNextToken();
			continue;
		}
		if (current->isKeyDesignation()) {
			getModeAndTonic(mode, b40tonic, *current);

		}
		if (current->isClef()) {
			if (*current == "*clefX") {
				isUnpitched = true;
			} else {
				isUnpitched = false;
			}
		}

		HTp curr = current;
		while (curr) {
			int ttrack = curr->getTrack();
			if (ttrack != track) {
				break;
			}
			degspine.at(line).resize((int)degspine.at(line).size() + 1);
			degspine.at(line).back().setLinkedKernToken(curr, mode, b40tonic, isUnpitched, m_resolveNullQ);
			curr = curr->getNextFieldToken();
		}
		current = current->getNextToken();
	}

	// Go back and fill in the non-spine tokens.  This is used to print **deg output
	// without input data.
	for (int i=0; i<lineCount; i++) {
		if (!infile[i].hasSpines()) {
			if (degspine.at(i).empty()) {
				degspine.at(i).resize(1);
			}
			degspine.at(i).back().setLinkedKernToken(infile.token(i, 0), "unknown", 0, true);
		}
	}

	// process melodic contours, etc.

}


//////////////////////////////
//
// Tool_deg::getModeAndTonic -- Convert key designation interpretation into a mode
//     string and a base-40 tonic pitch (162 = middle C)
//

void Tool_deg::getModeAndTonic(string& mode, int& b40tonic, const string& token) {
	string newtoken = token;
	if (!m_forcedKey.empty()) {
		newtoken = m_forcedKey;
	}
	HumRegex hre;
	if (hre.search(newtoken, "^\\*?([A-Ga-g][-#]*):?(.*)$")) {
		string key = hre.getMatch(1);
		string kmode = hre.getMatch(2);
		b40tonic = Convert::kernToBase40(key);
		int middleC = 162; // Convert::kernToBase40("c");
		if (b40tonic < middleC - 2) {
			mode = "major";
		} else {
			mode = "minor";
		}
		if (!kmode.empty()) {
			if      (kmode == "dor") { mode = "dor"; }
			else if (kmode == "phr") { mode = "phr"; }
			else if (kmode == "lyd") { mode = "lyd"; }
			else if (kmode == "mix") { mode = "mix"; }
			else if (kmode == "aeo") { mode = "aeo"; }
			else if (kmode == "loc") { mode = "loc"; }
			else if (kmode == "ion") { mode = "ion"; }
		}
	}
}


///////////////////////////////////////////////////////////////////////////
//
// ScaleDegree helper class within Tool_deg:
//


//////////////////////////////
//
// Tool_deg::ScaleDegree:ScaleDegree -- Constructor
//

Tool_deg::ScaleDegree::ScaleDegree (void) {
	// do nothing
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:~ScaleDegree -- Destructor
//
Tool_deg::ScaleDegree::~ScaleDegree () {
	// do nothing
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:setLinkedKernToken -- Return the
//    **kern token that this ScaleDegree is linked to.
//    NULL token means that there is no link (initialized
//    state when creating a ScaleDegree).   If unpitched
//    is true, this means the **kern token is in a percussion
//    part (*clefX  for percussion clef).
//       default value: unpitched = false
//    Unpitched scale degrees will be output as null data tokens.
//

void Tool_deg::ScaleDegree::setLinkedKernToken(HTp token, const string& mode, int b40tonic, bool unpitched, bool resolveNull) {
	m_linkedKernToken = resolveNull ? token->resolveNull() : token;
	m_unpitched = unpitched;
	if (!unpitched) {
		if (mode == "major") {
			setMajorMode(b40tonic);
		} else if (mode == "minor") {
			setMinorMode(b40tonic);
		} else if (mode == "dor") {
			setDorianMode(b40tonic);
		} else if (mode == "phr") {
			setPhrygianMode(b40tonic);
		} else if (mode == "lyd") {
			setLydianMode(b40tonic);
		} else if (mode == "mix") {
			setMixolydianMode(b40tonic);
		} else if (mode == "aeo") {
			setAeoleanMode(b40tonic);
		} else if (mode == "loc") {
			setLocrianMode(b40tonic);
		} else if (mode == "ion") {
			setIonianMode(b40tonic);
		}
		analyzeTokenScaleDegrees();
	} else {
		m_mode = m_unknown_mode;
		m_b40tonic = -1;
	}

}



//////////////////////////////
//
// Tool_deg::ScaleDegree::analyzeTokenScaleDegrees -- Extract scale degrees from
//    stored **kern token.  The token can be non data, in which case nothing
//    should be done, or it can be a chord, in which case multiple degrees could
//    be extracted.
//

void Tool_deg::ScaleDegree::analyzeTokenScaleDegrees(void) {
	m_subtokens.clear();
	m_degrees.clear();
	m_alters.clear();
	m_octaves.clear();

	if (!m_linkedKernToken) {
		return;
	}
	if (!m_linkedKernToken->isNonNullData()) {
		return;
	}

	// Only processing non-null data from here.
	m_subtokens = m_linkedKernToken->getSubtokens();
	int subtokCount = (int)m_subtokens.size();

	m_degrees.resize(subtokCount);
	fill(m_degrees.begin(), m_degrees.end(), -1);

	m_octaves.resize(subtokCount);
	fill(m_octaves.begin(), m_octaves.end(), -1);

	m_alters.resize(subtokCount);
	fill(m_alters.begin(), m_alters.end(), 0);

	HumRegex hre;
	for (int i=0; i<(int)subtokCount; i++) {
		if (m_unpitched) {
			// Unpitched note (in percussion clef)
			m_degrees[i] = -1;
			m_octaves[i] = -1;
		}
		if (m_subtokens[i].find('r') != string::npos) {
			if (subtokCount == 1) {
				// rest
				m_degrees[i] = 0;
			m_octaves[i] = -1;
			} else {
				// Rest in a chord: a non-sounding harmonic note.
				m_degrees[i] = -1;
				m_octaves[i] = -1;
			}
			continue;
		}
		if (m_subtokens[i].find("R") != string::npos) {
			// Semi-pitch note, so ignore it
			m_degrees[i] = -1;
			m_octaves[i] = -1;
			continue;
		}
		if (!hre.search(m_subtokens[i], "([A-Ga-g]+[-#]*)")) {
			// No pitch information (rhythm only?)
		}
		string kernPitch = hre.getMatch(1);
		int b40 = Convert::kernToBase40(kernPitch);
		// check for negative values
		m_octaves[i] = b40 / 40;
		int sd = ((b40 - m_b40tonic) + 400) % 40;
		switch (sd) {

			case 38: m_degrees[i] = 1; m_alters[i] = -2; break;
			case 39: m_degrees[i] = 1; m_alters[i] = -1; break;
			case  0: m_degrees[i] = 1; m_alters[i] =  0; break;
			case  1: m_degrees[i] = 1; m_alters[i] = +1; break;
			case  2: m_degrees[i] = 1; m_alters[i] = +2; break;

			case  4: m_degrees[i] = 2; m_alters[i] = -2; break;
			case  5: m_degrees[i] = 2; m_alters[i] = -1; break;
			case  6: m_degrees[i] = 2; m_alters[i] =  0; break;
			case  7: m_degrees[i] = 2; m_alters[i] = +1; break;
			case  8: m_degrees[i] = 2; m_alters[i] = +2; break;

			case 10: m_degrees[i] = 3; m_alters[i] = -2; break;
			case 11: m_degrees[i] = 3; m_alters[i] = -1; break;
			case 12: m_degrees[i] = 3; m_alters[i] =  0; break;
			case 13: m_degrees[i] = 3; m_alters[i] = +1; break;
			case 14: m_degrees[i] = 3; m_alters[i] = +2; break;

			case 15: m_degrees[i] = 4; m_alters[i] = -2; break;
			case 16: m_degrees[i] = 4; m_alters[i] = -1; break;
			case 17: m_degrees[i] = 4; m_alters[i] =  0; break;
			case 18: m_degrees[i] = 4; m_alters[i] = +1; break;
			case 19: m_degrees[i] = 4; m_alters[i] = +2; break;

			case 21: m_degrees[i] = 5; m_alters[i] = -2; break;
			case 22: m_degrees[i] = 5; m_alters[i] = -1; break;
			case 23: m_degrees[i] = 5; m_alters[i] =  0; break;
			case 24: m_degrees[i] = 5; m_alters[i] = +1; break;
			case 25: m_degrees[i] = 5; m_alters[i] = +2; break;

			case 27: m_degrees[i] = 6; m_alters[i] = -2; break;
			case 28: m_degrees[i] = 6; m_alters[i] = -1; break;
			case 29: m_degrees[i] = 6; m_alters[i] =  0; break;
			case 30: m_degrees[i] = 6; m_alters[i] = +1; break;
			case 31: m_degrees[i] = 6; m_alters[i] = +2; break;

			case 33: m_degrees[i] = 7; m_alters[i] = -2; break;
			case 34: m_degrees[i] = 7; m_alters[i] = -1; break;
			case 35: m_degrees[i] = 7; m_alters[i] =  0; break;
			case 36: m_degrees[i] = 7; m_alters[i] = +1; break;
			case 37: m_degrees[i] = 7; m_alters[i] = +2; break;

			default: m_degrees[i] = -1; m_alters[i] = -1;
		}

		if (m_mode == m_minor_mode) {
			if ((m_degrees[i] == 3) || (m_degrees[i] == 6) || (m_degrees[i] == 7)) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_dor_mode) {
			if ((m_degrees[i] == 3) || (m_degrees[i] == 7)) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_phr_mode) {
			if ((m_degrees[i] == 2) || (m_degrees[i] == 3) || (m_degrees[i] == 6) || (m_degrees[i] == 7)) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_lyd_mode) {
			if (m_degrees[i] == 4) {
				m_alters[i]--;
			}
		}

		if (m_mode == m_mix_mode) {
			if (m_degrees[i] == 7) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_aeo_mode) {
			if ((m_degrees[i] == 3) || (m_degrees[i] == 6) || (m_degrees[i] == 7)) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_loc_mode) {
			if ((m_degrees[i] == 2) || (m_degrees[i] == 3) || (m_degrees[i] == 5) || (m_degrees[i] == 6) || (m_degrees[i] == 7)) {
				m_alters[i]++;
			}
		}

		if (m_mode == m_ion_mode) {
			// nothing to do
		}
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:getLinkedKernToken -- Return the **kern
//    token that this ScaleDegree is linked to.
//    Returns NULL if it has not been linked.
//

HTp Tool_deg::ScaleDegree::getLinkedKernToken(void) const {
	return m_linkedKernToken;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:getDegToken -- Convert the ScaleDegre
//     to a **deg token string (or **degree token if including an octave).
//

string Tool_deg::ScaleDegree::getDegToken(void) const {
	HTp token = getLinkedKernToken();
	if (!hasSpines()) {
		if (token) {
			return *token;
		} else {
			return "ERROR1";
		}
	}

	if (isExclusiveInterpretation()) {
		if (m_octaveQ) {
			return "**degree";
		} else {
			return "**deg";
		}
	} else if (isManipulator()) {
		return getManipulator();
	} else if (isInterpretation()) {
		if (isKeyDesignation()) {
			if (m_forcedKey.empty()) {
				return *token;
			} else{
				return "*";
			}
		} else {
			return "*";
		}
	} else if (isLocalComment()) {
		return "!";
	} else if (isNullDataToken()) {
		return ".";
	} else if (isBarline()) {
		return getBarline();
	}

	if (m_unpitched) {
		return ".";
	}

	return generateDegDataToken();
}


//////////////////////////////
//
// Tool_deg::ScaleDegree::generateDegDataToken -- Convert a ScaleDegree
//     object into a **deg data token.  This version of generateDegDataToken()
//     will assemble individual subtokens for chordtones into a single
//     token.
//

string Tool_deg::ScaleDegree::generateDegDataToken(void) const {
	if (!isDataToken()) {
		return "ERROR2 (not a data token)";
	}

	if (isNullDataToken()) {
		return ".";
	}

	// Convert internal data into **deg data token (which may be a chord)

	// scale degrees:
	int subtokenCount = getSubtokenCount();
	if (subtokenCount == 0) {
		return "ERROR3";
	}

	vector<string> subtokens(subtokenCount);;
	for (int i=0; i<subtokenCount; i++) {
		subtokens.at(i) = generateDegDataSubtoken(i);
	}


	// Include tied notes or suppress them:

	if (m_showTiesQ)  {
		// Secondary tied notes scale degrees should be shown.
		string output;
		for (int i=0; i<subtokenCount; i++) {
			output += subtokens[i];
			if (i < subtokenCount - 1) {
				output += " ";
			}
		}
		return output;
	}

	// Removed scale degress for secondary tied notes.
	vector<string> nontied(subtokens.size());
	nontied.clear();
	for (int i=0; i<(int)subtokens.size(); i++) {
		if (subtokens[i].find('_') == string::npos) {
			nontied.push_back(subtokens[i]);
		}
	}

	if (nontied.empty()) {
		return ".";
	}

	int newCount = (int)nontied.size();
	string output;
	for (int i=0; i<newCount; i++) {
		output += nontied[i];
		if (i < newCount - 1) {
			output += " ";
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::generateDegDataSubtoken -- Convert the ScaleDegree
//		subtoken (chord note) into **deg data.
//

string Tool_deg::ScaleDegree::generateDegDataSubtoken(int index) const {
	if (!isNonNullDataToken()) {
		return "ERROR5";
	}
	if (index < 0) {
		return "ERROR6";
	}
	if (index >= getSubtokenCount()) {
		return "ERROR7";
	}

	string output;

	// Is the note a secondary tied note? If so, then prefix the scale degree by and underscore.
	bool tiedQ = (m_subtokens.at(index).find('_') != string::npos) || (m_subtokens.at(index).find(']') != string::npos);
	if (tiedQ) {
		output += "_";
	}

	// Add the scale degree:
	int degree = m_degrees.at(index);
	if (degree == 0) {
		output += "r";
		if (m_showZerosQ) {
			output += "0";
		}
   } else if ((degree > 0) && (degree <= 7)) {
		output += to_string(degree);
	} else {
		return "ERROR8";
	}

	// Add the scale degree chromatic alteration:
	if (m_alters.at(index) < 0) {
		for (int i=m_alters.at(index); i<0; i++) {
			output += "-";
		}
	} if (m_alters.at(index) > 0) {
		for (int i=0; i<m_alters.at(index); i++) {
			output += "+";
		}
	}

	// Add octave information if requested:
	if (m_octaveQ && (degree != 0)) {
		output += "/";
		output += to_string(m_octaves.at(index));
	}

	return output;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:getTimestamp -- An absolute value
//    that represents the starting time of the ScaleDegree
//    from the start of the score (which is at time 0).
//    Units are in quarter-note durations.
//

HumNum Tool_deg::ScaleDegree::getTimestamp(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->getDurationFromStart();
	} else {
		return -1;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:getDuration -- Duration of the
//     linked note for the ScaleDegree.
//

HumNum Tool_deg::ScaleDegree::getDuration(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->getDuration();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:getTiedDuration -- Duration of
//     the linked note including subsequent tied notes.
//

HumNum Tool_deg::ScaleDegree::getTiedDuration(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->getTiedDuration();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isExclusiveInterpretation -- Returns
//     true if the ScaleDegree is on an exclusive interpretation
//     line.
//

bool Tool_deg::ScaleDegree::isExclusiveInterpretation(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isExclusiveInterpretation();
	} else {
		return false;
	}
}




//////////////////////////////
//
// Tool_deg::ScaleDegree::isNonNullDataToken -- Returns true
//   if the ScaleDegree is a non-null data token.

bool Tool_deg::ScaleDegree::isNonNullDataToken(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isNonNullData();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isNullDataToken -- Returns true
//     if the ScaleDegree is a null data token (".").
//

bool Tool_deg::ScaleDegree::isNullDataToken(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isNullData();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isDataToken -- Returns true
//     if the ScaleDegree is a data token (either null or
//     contains actual data).
//

bool Tool_deg::ScaleDegree::isDataToken(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isData();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isBarline -- Returns true
//     if the ScaleDegree is in a barline.
//

bool Tool_deg::ScaleDegree::isBarline(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isBarline();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isInterpretation -- Returns true
//     if the ScaleDegree is in an interpretation line.
//

bool Tool_deg::ScaleDegree::isInterpretation(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		if (token->isExclusiveInterpretation()) {
			return false;
		} else {
			return token->isInterpretation();
		}
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isLocalComment -- Returns true
//     if the ScaleDegree is on a local comment line.
//

bool Tool_deg::ScaleDegree::isLocalComment(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isLocalComment();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isGlobalComment -- Returns true
//     if the ScaleDegree is on a global comment line
//

bool Tool_deg::ScaleDegree::isGlobalComment(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		return token->isGlobalComment();
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:isReferenceRecord -- Returns true
//    if the ScaleDegree is on a reference record line.
//

bool Tool_deg::ScaleDegree::isReferenceRecord(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		HLp line = token->getOwner();
		if (line) {
			return line->isReference();
		} else {
			return false;
		}
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree:hasSpines --  Retuns true
//    if the line that the ScaleDegree is on is on a spined
//    line (not global reference record, nor global comment
//    nor an empty line).
//

bool Tool_deg::ScaleDegree::hasSpines(void) const {
	HTp token = getLinkedKernToken();
	if (token) {
		HLp line = token->getOwner();
		if (line) {
			return line->hasSpines();
		} else {
			return false;
		}
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::getManipulator -- such as *^, ^-, *+, * (null manipulator),
//     but not exclusive interpretation (starting with **).
//

string Tool_deg::ScaleDegree::getManipulator(void) const {
	HTp token = getLinkedKernToken();
	if (!token) {
		return "ERROR4";
	}
	if (token->isManipulator()) {
		return *token;
	} else {
		return "*";
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isKeyDesignation -- Returns true if is a key designation.
//

bool Tool_deg::ScaleDegree::isKeyDesignation(void) const {
	HTp token = getLinkedKernToken();
	if (token && token->isInterpretation() && token->isKeyDesignation()) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::getBarline -- return **kern barline if available.
//

string Tool_deg::ScaleDegree::getBarline(void) const {
	HTp token = getLinkedKernToken();
	if (token && token->isBarline()) {
		return *token;
	} else {
		return "=";
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isManipulator -- such as *^, ^-, *+, * (null
//     manipulator), but not exclusive interpretation (starting with **).
//

bool Tool_deg::ScaleDegree::isManipulator(void) const {
	HTp token = getLinkedKernToken();
	if (!token) {
		return false;
	} else if (token->isExclusiveInterpretation()) {
		return false;
	} else if (token->isManipulator()) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::getSubtokenCount -- Return the number
//      of subtokens (chord notes).
//

int Tool_deg::ScaleDegree::getSubtokenCount(void) const {
	return (int)m_degrees.size();
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setMajorMode -- CDEFGABC
//

void Tool_deg::ScaleDegree::setMajorMode(int b40tonic) {
	m_mode = m_major_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setMinorMode -- ABCDEFGA
//

void Tool_deg::ScaleDegree::setMinorMode(int b40tonic) {
	m_mode = m_minor_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setDorianMode -- DEFGABCD
//

void Tool_deg::ScaleDegree::setDorianMode(int b40tonic) {
	m_mode = m_dor_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setPhrygianMode -- EFGABCDE
//

void Tool_deg::ScaleDegree::setPhrygianMode(int b40tonic) {
	m_mode = m_phr_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setLydianMode -- FGABCDEF
//

void Tool_deg::ScaleDegree::setLydianMode(int b40tonic) {
	m_mode = m_lyd_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setMixolydianMode -- GABCDEFG
//

void Tool_deg::ScaleDegree::setMixolydianMode(int b40tonic) {
	m_mode = m_mix_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setAeoleanMode -- ABCDEFGA
//

void Tool_deg::ScaleDegree::setAeoleanMode(int b40tonic) {
	m_mode = m_aeo_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setLocrianMode -- BCDEFGAB
//

void Tool_deg::ScaleDegree::setLocrianMode(int b40tonic) {
	m_mode = m_loc_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setIonianMode -- CDEFGABC
//

void Tool_deg::ScaleDegree::setIonianMode(int b40tonic) {
	m_mode = m_ion_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isInMinorMode --
//

bool Tool_deg::ScaleDegree::isInMinorMode(void) const {
	return m_mode == m_minor_mode;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::isInMajorMode --
//

bool Tool_deg::ScaleDegree::isInMajorMode(void) const {
	return m_mode == m_major_mode;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::getBase40Tonic --
//

int  Tool_deg::ScaleDegree::getBase40Tonic(void) const {
	return m_b40tonic;
}



//////////////////////////////
//
// operator<<ScaleDegree -- print converted **deg string.
//

ostream& operator<<(ostream& out, Tool_deg::ScaleDegree& degree) {
	out << degree.getDegToken();
	return out;
}

ostream& operator<<(ostream& out, Tool_deg::ScaleDegree* degree) {
	if (degree == NULL) {
		out << "{null}";
		return out;
	} else {
		out << (*degree);
		return out;
	}
}



// END_MERGE

} // end namespace hum



