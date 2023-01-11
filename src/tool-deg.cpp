//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  6 13:03:22 PST 2023
// Last Modified: Sat Jan  7 13:57:43 PST 2023
// Filename:      tool-deg.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-deg.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Analyze scale degree information from a **kern spines.
// Documentation: https://doc.verovio.humdrum.org/humdrum/scale_degrees
//

#include "tool-deg.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

/////////////////////////////////
//
// Tool_deg::ScaleDegree static member variables:
//

bool Tool_deg::ScaleDegree::m_showTiesQ = false;



/////////////////////////////////
//
// Tool_deg::Tool_deg -- Set the recognized options for the tool.
//

Tool_deg::Tool_deg(void) {
	define("I|no-input=b", "Do not interleave **deg data with input score in output");
	define("t|ties=b", "Include scale degrees for tied notes");
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
	m_degOnlyQ = getBoolean("no-input");
	m_degTiesQ = getBoolean("ties");
	Tool_deg::ScaleDegree::setShowTies(m_degTiesQ);
}



//////////////////////////////
//
// Tool_deg::processFile --
//

void Tool_deg::processFile(HumdrumFile& infile) {
	vector<HTp> kernstarts;
	infile.getKernSpineStartList(kernstarts);
	m_degSpines.clear();
	int kernCount = (int)kernstarts.size();
	if (kernCount == 0) {
		return;
	}
	m_degSpines.resize(kernCount);
	for (int i=0; i<kernCount; i++) {
		prepareDegSpine(m_degSpines.at(i), kernstarts.at(i), infile);
	}

	if (m_degOnlyQ) {
		printDegScore();
	} else {
		printDegScoreInterleavedWithInputScore(infile);
	}
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
	vector<int> insertTracks((int)kernStarts.size() - 1);
	for (int i=0; i<(int)kernStarts.size() - 1; i++) {
		insertTracks.at(i) = kernStarts.at(i+1)->getTrack();
	}
	insertTracks.push_back(-1);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
		} else {
			m_humdrum_text << createOutputHumdrumLine(infile, insertTracks, i) << endl;
		}
	}
}



//////////////////////////////
//
// Tool_deg::createOutputHumdrumLine --
//

string Tool_deg::createOutputHumdrumLine(HumdrumFile& infile, vector<int> insertTracks, int lineIndex) {
	int inputFieldCount = infile[lineIndex].getFieldCount();
	int currentDegIndex = 0;
	string output;
	for (int i=0; i<inputFieldCount; i++) {
		HTp token = infile.token(lineIndex, i);
		int track = token->getTrack();
		int dtrack = insertTracks.at(currentDegIndex);
		if (dtrack == track) {
			for (int k=0; k<(int)m_degSpines.at(currentDegIndex).at(lineIndex).size(); k++) {
				output += "\t";
				output += m_degSpines[currentDegIndex][lineIndex][k].getDegToken();
				currentDegIndex++;
			}
		}
		if (i > 0) {
			output += "\t";
		}
		output += *token;
	}
	output += "\t";
	output += m_degSpines.back().at(lineIndex).back().getDegToken();
	return output;
}



//////////////////////////////
//
// Tool_deg::printDegScore -- **deg spines without any input data.
//

void Tool_deg::printDegScore(void) {
	if (m_degSpines.empty()) {
		return;
	}
	int lineCount = (int)m_degSpines[0].size();
	int spineCount = (int)m_degSpines.size();
	for (int j=0; j<lineCount; j++) {
		if (!m_degSpines[0][j][0].hasSpines()) {
			m_free_text << m_degSpines[0][j][0].getLinkedKernToken() << endl;
			continue;
		}
		for (int i=0; i<spineCount; i++) {
			int hasSpines = 1;
			int subspineCount = (int)m_degSpines.at(i).at(j).size();
			for (int k=0; k<subspineCount; k++) {
				if ((i == 0) && (k > 0)) {
					m_free_text << "\t";
				} else if (i > 0) {
					m_free_text << "\t";
				}
				m_free_text << m_degSpines[i][j][k];
			}
			if (!hasSpines) {
				break;
			}
		}
		m_free_text << endl;
	}
}



//////////////////////////////
//
// Tool_deg::prepareDegSpine -- Convert one **kern spine into a **deg spine.
//

void Tool_deg::prepareDegSpine(vector<vector<ScaleDegree>>& degspine, HTp kernstart,
		HumdrumFile& infile) {
	string mode = "unknown";
	int b40tonic = -1;

	int lineCount = infile.getLineCount();

	degspine.resize(lineCount);
	int track = kernstart->getTrack();
	HTp current = kernstart;
	HumRegex hre;

	int middleC = Convert::kernToBase40("c");
	bool isUnpitched = false;

	while (current) {
		int line = current->getLineIndex();
		if (!current->getOwner()->hasSpines()) {
			degspine.at(line).resize(1);
			degspine.at(line).back().setLinkedKernToken(current, mode, b40tonic, isUnpitched);
			current = current->getNextToken();
			continue;
		}
		if (current->isKeyDesignation()) {
			if (hre.search(current, "\\*([A-Ga-g][-#]*):")) {
				string key = hre.getMatch(1);
				b40tonic = Convert::kernToBase40(key);
				if (b40tonic < middleC - 2) {
					mode = "major";
				} else {
					mode = "minor";
				}
			}
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
			degspine.at(line).back().setLinkedKernToken(curr, mode, b40tonic, isUnpitched);
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

void Tool_deg::ScaleDegree::setLinkedKernToken(HTp token, const string& mode, int b40tonic, bool unpitched) {
	m_linkedKernToken = token;
	m_unpitched = unpitched;
	if (!unpitched) {
		if (mode == "major") {
			setMajorMode(b40tonic);
		} else if (mode == "minor") {
			setMinorMode(b40tonic);
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
	int subtokCount = m_subtokens.size();

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
//     to a **deg token string.
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
		return "**deg";
	} else if (isManipulator()) {
		return getManipulator();
	} else if (isInterpretation()) {
		if (isKeyDesignation()) {
			return *token;
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

	int newCount = nontied.size();
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
// Tool_deg::ScaleDegree::createDegDataSubToken -- Convert the ScaleDegree
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
// Tool_deg::ScaleDegree::setMinorMode --
//

void Tool_deg::ScaleDegree::setMinorMode(int b40tonic) {
	m_mode = m_minor_mode;
	m_b40tonic = b40tonic;
}



//////////////////////////////
//
// Tool_deg::ScaleDegree::setMajorMode --
//

void Tool_deg::ScaleDegree::setMajorMode(int b40tonic) {
	m_mode = m_major_mode;
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
// Tool_deg::ScaleDegree::setShowTies --
//
void Tool_deg::ScaleDegree::setShowTies(bool state) {
	m_showTiesQ = state;
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



