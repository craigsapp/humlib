//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Apr  1 01:11:53 PDT 2024
// Last Modified: Mon Apr  1 23:38:26 PDT 2024
// Filename:      tool-sab2gs.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-sab2gs.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Converts Soprano/Alto/Bass staves into Grand-staff style.
//                The middle voice is converted into the second layer of the
//                top staff, and the clef of the middle voice is used to
//                assign to top or bottom staff.
//
//                Assumes no spine splits/merges in the three input **kern spines.
//
//                *clefF4 moves middle part to bottom staff, *clefG2 moves it to top staff.
//                Also *down/*Xdown work in a similar manner.
// Options:
//                -b char == RDF**kern down marking character (<) by default
//                -d      == Use only *down/*Xdown for controlling staff assignment
//                           for middle part (ignore clefs).

#include "tool-sab2gs.h"
#include "Convert.h"
#include "HumRegex.h"

#include <vector>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_sab2gs::Tool_sab2gs -- Set the recognized options for the tool.
//

Tool_sab2gs::Tool_sab2gs(void) {
	define("b|below=s:<", "Marker for displaying on next staff below");
	define("d|down=b",    "Use only *down/*Xdown interpretations");
}



/////////////////////////////////
//
// Tool_sab2gs::run -- Do the main work of the tool.
//

bool Tool_sab2gs::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_sab2gs::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_sab2gs::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_sab2gs::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_sab2gs::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_sab2gs::initialize(void) {
	m_belowMarker = getString("below");
	m_downQ       = getBoolean("down");
}



//////////////////////////////
//
// Tool_sab2gs::processFile --
//

void Tool_sab2gs::processFile(HumdrumFile& infile) {

	vector<HTp> spines;
	infile.getSpineStartList(spines);
	vector<HTp> kernSpines;
	for (int i=0; i<(int)spines.size(); i++) {
		if (spines[i]->isKern()) {
			kernSpines.push_back(spines[i]);
		}
	}
	if (kernSpines.size() != 3) {
		// Not valid for processing kern spines, so return original:
		m_humdrum_text << infile;
		return;
	}

	string belowMarker = hasBelowMarker(infile);
	if (!belowMarker.empty()) {
		m_hasBelowMarker = true;
		m_belowMarker = belowMarker;
	}

	adjustMiddleVoice(kernSpines[1]);
	printGrandStaff(infile, kernSpines);
}



/////////////////////////////
//
// Tool_sab2gs::hasBelowMarker -- Returns below marker if found; otherwise,
//     returns empty string.
//

string Tool_sab2gs::hasBelowMarker(HumdrumFile& infile) {
	string output;
	HumRegex hre;
	if (m_hasCrossStaff) {
		// Search backwards since if there is a below marker, it will be more
		// likely found at the bottom of the score.
		for (int i=infile.getLineCount()-1; i<=0; i--) {
			if (infile[i].hasSpines()) {
				continue;
			}
			if (hre.search(infile.token(i, 0), "^!!!RDF\\*\\*kern\\s*:\\s*([^\\s=]+)\\s*=\\s*below\\s*$")) {
				output = hre.getMatch(1);
				break;
			}
		}
	}
	return output;
}



///////////////////////////////
//
// Tool_sab2gs::printGrandStaff --
//

void Tool_sab2gs::printGrandStaff(HumdrumFile& infile, vector<HTp>& starts) {
	bool foundData = false;

	vector<int> ktracks(starts.size());
	for (int i=0; i<(int)starts.size(); i++) {
		ktracks.at(i) = starts.at(i)->getTrack();
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}
		if (!foundData && (infile[i].isData() || infile[i].isBarline())) {
			printSpineSplit(infile, i, ktracks);
			foundData = true;
		}
		if (*infile.token(i, 0) == "*-") {
			printSpineMerge(infile, i, ktracks);
			foundData = false;
			printReducedLine(infile, i, ktracks);
			if (m_hasCrossStaff && !m_hasBelowMarker) {
				m_humdrum_text << "!!!RDF**kern: " << m_belowMarker << " = below" << endl;
			}
			continue;
		}
		if (foundData) {
			printSwappedLine(infile, i, ktracks);
		} else {
			printReducedLine(infile, i, ktracks);
		}
	}
}


//////////////////////////////
//
// Tool_sab2gs::printSpineSplit -- Split second and third spines, moving non-kern spines
//    after the second one to the end of the line (null interpretations);
//

void Tool_sab2gs::printSpineSplit(HumdrumFile& infile, int index, vector<int>& ktracks) {
	// First print all non-kern spines at the start of the line:
	int nextIndex = 0;
	int fcount = 0;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << "*";
		nextIndex++;
	}
	// Must be on the first **kern spine:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Print the first **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << "*";
	nextIndex++;
	// Next print all non-kern spines after first **kern spine:
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		m_humdrum_text << "*";
		nextIndex++;
	}
	// Second **kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend B on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Ignore the second kern spine as it does not exist yet in the
	// output data.
	nextIndex++;
	// Then store any non-kern spines between the second and third kern spines to 
	// append to the end of the data line later.
	string postData;
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (!postData.empty()) {
			postData += "\t";
		}
		nextIndex++;
		postData += "*";
	}
	// Third kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend C on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Now print the third kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	nextIndex++;
	m_humdrum_text << "*^";
	// Now print the non-kern spines after the third **kern spine (or rather just
	// all spines including any other **kern spines, although current requirement
	// is that there are only three **kern spines.
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		// HTp token = infile.token(index, nextIndex);
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		nextIndex++;
		m_humdrum_text << "*";
	}
	// Finally print any non-kern spines after the second **kern spine:
	if (!postData.empty()) {
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << postData;
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_sab2gs::printSpineMerge -- Merge second and third spines, moving non-kern spines
//    after the second one to the end of the line (null interpretations);
//

void Tool_sab2gs::printSpineMerge(HumdrumFile& infile, int index, vector<int>& ktracks) {
	// First print all non-kern spines at the start of the line:
	int nextIndex = 0;
	int fcount = 0;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << "*";
		nextIndex++;
	}
	// Must be on the first **kern spine:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Print the first **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << "*";
	nextIndex++;
	// Next print all non-kern spines after first **kern spine:
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		m_humdrum_text << "*";
		nextIndex++;
	}
	// Second **kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend B on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Save the second kern spine as it does not exist yet in the
	// output data.
	// HTp savedKernToken = infile.token(index, nextIndex);
	nextIndex++;
	// Then store any non-kern spines between the second and third kern spines to 
	// append to the end of the data line later.
	string postData;
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (!postData.empty()) {
			postData += "\t";
		}
		nextIndex++;
		postData += "*";
	}
	// Third kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend C on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Now print the third kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << "*v";
	nextIndex++;
	// Now printed the saved second **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	m_humdrum_text << "*v";
	fcount++;
	// Now print the non-kern spines after the third **kern spine (or rather just
	// all spines including any other **kern spines, although current requirement
	// is that there are only three **kern spines.
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		// HTp token = infile.token(index, nextIndex);
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		nextIndex++;
		m_humdrum_text << "*";
	}
	// Finally print any non-kern spines after the second **kern spine:
	if (!postData.empty()) {
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << postData;
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_sab2gs::printSwappedLine -- move the second **kern spine immediately after
//    the third one, and move any non-kern spines after then end of the line.
//

void Tool_sab2gs::printSwappedLine(HumdrumFile& infile, int index, vector<int>& ktracks) {
	// First print all non-kern spines at the start of the line:
	int nextIndex = 0;
	int fcount = 0;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << token;
		nextIndex++;
	}
	// Must be on the first **kern spine:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Print the first **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << infile.token(index, nextIndex);
	nextIndex++;
	// Next print all non-kern spines after first **kern spine:
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		m_humdrum_text << token;
		nextIndex++;
	}
	// Second **kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend B on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Save the second kern spine as it does not exist yet in the
	// output data.
	HTp savedKernToken = infile.token(index, nextIndex++);
	// Then store any non-kern spines between the second and third kern spines to 
	// append to the end of the data line later.
	string postData;
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (!postData.empty()) {
			postData += "\t";
		}
		nextIndex++;
		postData += *token;
	}
	// Third kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend C on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Now print the third kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << infile.token(index, nextIndex++);
	// Now printed the saved second **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	m_humdrum_text << savedKernToken;
	fcount++;
	// Now print the non-kern spines after the third **kern spine (or rather just
	// all spines including any other **kern spines, although current requirement
	// is that there are only three **kern spines.
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, nextIndex);
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		nextIndex++;
		m_humdrum_text << token;
	}
	// Finally print any non-kern spines after the second **kern spine:
	if (!postData.empty()) {
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << postData;
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_sab2gs::printReducedLine -- remove the contents of the second **kern
//    spine, and move any non-kernspines after it to become after the third **kern spine
//

void Tool_sab2gs::printReducedLine(HumdrumFile& infile, int index, vector<int>& ktracks) {
	// First print all non-kern spines at the start of the line:
	int nextIndex = 0;
	int fcount = 0;
	for (int i=0; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << token;
		nextIndex++;
	}
	// Must be on the first **kern spine:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Print the first **kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << infile.token(index, nextIndex++);
	// Next print all non-kern spines after first **kern spine:
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		m_humdrum_text << token;
		nextIndex++;
	}
	// Second **kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend B on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Ignore the second kern spine as it does not exist yet in the
	// output data.
	nextIndex++;
	// Then store any non-kern spines between the second and third kern spines to 
	// append to the end of the data line later.
	string postData;
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, i);
		if (token->isKern()) {
			break;
		}
		if (!postData.empty()) {
			postData += "\t";
		}
		nextIndex++;
		postData += *token;
	}
	// Third kern spine must be **kern data:
	if (!infile.token(index, nextIndex)->isKern()) {
		cerr << "Something strange happend C on line " << index+1 << ": " << infile[index] << endl;
		return;
	}
	// Now print the third kern spine:
	if (fcount > 0) {
		m_humdrum_text << "\t";
	}
	fcount++;
	m_humdrum_text << infile.token(index, nextIndex++);
	// Now print the non-kern spines after the third **kern spine (or rather just
	// all spines including any other **kern spines, although current requirement
	// is that there are only three **kern spines.
	for (int i=nextIndex; i<infile[index].getFieldCount(); i++) {
		HTp token = infile.token(index, nextIndex);
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		nextIndex++;
		m_humdrum_text << token;
	}
	// Finally print any non-kern spines after the second **kern spine:
	if (!postData.empty()) {
		if (fcount > 0) {
			m_humdrum_text << "\t";
		}
		fcount++;
		m_humdrum_text << postData;
	}
	m_humdrum_text << endl;
}


//////////////////////////////
//
// Tool_sab2gs::adjustMiddleVoice --
//

void Tool_sab2gs::adjustMiddleVoice(HTp spineStart) {
	HTp current = spineStart;
	// staff: +1 = top staff, -1 = bottom staff
	// when on top staff, force stem down, or on bottom staff, force stem up
	// when on bottom staff add "<" marker after pitch (or rest) to move to
	// bottom staff.  Staff choice is selected by clef: clefG2 is for top staff
	// and staffF4 is for bottom staff. Chords are not expected.
	int staff = 0; 
	string replacement = "$1" + m_belowMarker;
	HumRegex hre;
	while (current) {
		if (*current == "*-") {
			break;
		}
		if (!m_downQ && current->isClef()) {
			if (current->substr(0, 7) == "*clefG2") {
				staff = 1;
				// suppress clef:
				string text = "*x" + current->substr(1);
				current->setText(text);
			} else if (current->substr(0, 7) == "*clefF4") {
				staff = -1;
				// suppress clef:
				string text = "*x" + current->substr(1);
				current->setText(text);
			}
		} else if (current->isInterpretation()) {
			if (*current == "*down") {
				staff = -1;
			} else if (*current == "*Xdown") {
				staff = 1;
			}
		} else if ((staff != 0) && current->isData()) {
			if (current->isNull()) {
				// nothing to do with token
				current = current->getNextToken();
				continue;
			}
			if (staff > 0) {
				// force stems down or add stem down to non-rest notes
				if (hre.search(current, "[/\\\\]")) {
					string value = hre.replaceCopy(current, "\\", "/", "g");
					if (value != *current) {
						current->setText(value);
					}
					current = current->getNextToken();
					continue;
				} if (current->isRest()) {
					current = current->getNextToken();
					continue;
				} else {
					string value = *current;
					value += "\\";
					current->setText(value);
					current = current->getNextToken();
					continue;
				}

			} else if (staff < 0) {
				// force stems up or add stem up to non-rest notes
				if (hre.search(current, "[/\\\\]")) {
					string value = hre.replaceCopy(current, "\\", "/", "g");
					if (value != *current) {
						current->setText(value);
					}
					current = current->getNextToken();
					continue;
				} if (current->isRest()) {
					// Do not at stem direction to rests
				} else {
					// Force stem up (assuming not a chord, although it should not matter):
					string value = hre.replaceCopy(current, "/", "$");
					if (value != *current) {
						current->setText(value);
					}
				}
				// Add < after pitch (and accidental and qualifiers) to display
				// on staff below.
				m_hasCrossStaff = true;
				string output = hre.replaceCopy(current, replacement, "([A-Ga-gr]+[-#nXYxy]*)", "g");
				if (output != *current) {
					current->setText(output);
				}
			}
		}
		current = current->getNextToken();
	}
}


// END_MERGE

} // end namespace hum



