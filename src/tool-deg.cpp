//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Jan  6 13:03:22 PST 2023
// Last Modified: Sat Jan 14 02:14:32 PST 2023
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

bool Tool_deg::ScaleDegree::m_showTiesQ  = false;
bool Tool_deg::ScaleDegree::m_showZerosQ = false;



/////////////////////////////////
//
// Tool_deg::Tool_deg -- Set the recognized options for the tool.
//

Tool_deg::Tool_deg(void) {
	define("arr|arrow|arrows=b", "Display scale degree alterations as arrows");
	define("I|no-input=b", "Do not interleave **deg data with input score in output");
	define("kern=b", "Prefix composite rhythm **kern spine with -I option");
	define("r|recip=b", "Prefix output data with **recip spine with -I option");
	define("t|ties=b", "Include scale degrees for tied notes");
	define("0|z|zero|zeros=b", "Show rests as scale degree 0");
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
	m_arrowQ   = getBoolean("arrow");
	m_degOnlyQ = getBoolean("no-input");
	m_kernQ    = getBoolean("kern");
	m_recipQ   = getBoolean("recip");
	if (m_kernQ) {
		m_recipQ = true;
	}
	m_degTiesQ = getBoolean("ties");


	Tool_deg::ScaleDegree::setShowTies(m_degTiesQ);
	Tool_deg::ScaleDegree::setShowZeros(getBoolean("zeros"));
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
		printDegScore(infile);
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

	m_ipv.clear();

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
// Tool_deg::createOutputHumdrumLine -- Print **deg data within input score.  The **deg
//    spine will be placed in the right-most spine after a **kern spine (so just to
//    the left of the next **kern spine.  Perhaps change to staff-like spines such as
//    **mens.
//

string Tool_deg::createOutputHumdrumLine(HumdrumFile& infile, vector<int>& insertTracks, int lineIndex) {
	int inputFieldCount = infile[lineIndex].getFieldCount();
	int currentDegIndex = 0;
	string output;

	// tracks: this is for handling *v mergers gracefully for the inserted **deg data.
	vector<int> tracks;
	vector<string> tokens;
	bool hasMerger = false;
	bool hasDegMerger = false;
	int degNegativeTrack = -1;
	bool arrowStatus = false;

	// Keep track of an existing arrow styling line and if m_arrowQ is true, then
	// insert the arrow styling for new **deg spines here rather than just
	// before the first data line.
	if (m_arrowQ && !m_ipv.foundArrowLine && !m_ipv.foundData) {
		arrowStatus = isDegArrowLine(infile, lineIndex);
	}

	for (int i=0; i<inputFieldCount; i++) {
		HTp token = infile.token(lineIndex, i);
		int track = token->getTrack();
		int dtrack = insertTracks.at(currentDegIndex);
		if (dtrack == track) {
			// insert the
			int spineSize = (int)m_degSpines.at(currentDegIndex).at(lineIndex).size();
			for (int k=0; k<spineSize; k++) {
				string value = m_degSpines[currentDegIndex][lineIndex][k].getDegToken();

				if (arrowStatus && m_arrowQ && (!m_ipv.foundArrowLine) && (!m_ipv.foundData)) {
					if (value == "*") {
						value = "*arr";
					}
				}

				output += "\t";
				output += value;
				tracks.push_back(degNegativeTrack);
				tokens.push_back(value);
				if (value == "*v") {
					hasDegMerger = true;
				}
			}
			degNegativeTrack--;
			currentDegIndex++;
		}
		if (i > 0) {
			output += "\t";
		}
		output += *token;
		if (*token == "*v") {
			hasMerger = true;
		}
		tracks.push_back(track);
		tokens.push_back(*token);
	}

	// Output the last **deg spine at the end of the line:
	int kcount = (int)m_degSpines.back().at(lineIndex).size();
	for (int k=0; k<kcount; k++) {
		output += "\t";
		string value = m_degSpines.back().at(lineIndex).at(k).getDegToken();

		if (arrowStatus && m_arrowQ && (!m_ipv.foundArrowLine) && (!m_ipv.foundData)) {
			if (value == "*") {
				value = "*arr";
			}
		}

		if (value == "*v") {
			hasDegMerger = true;
		}
		output += value;
		tracks.push_back(degNegativeTrack);
		tokens.push_back(value);
	}

	if (arrowStatus) {
		m_ipv.foundArrowLine = true;
	}

	if ((!m_ipv.foundData) && infile[lineIndex].isData() && !m_ipv.foundArrowLine) {
		m_ipv.foundData = true;
		if (m_arrowQ && !m_ipv.foundArrowLine) {
			string line = printDegInterpretation("*arr", infile, lineIndex);
			if (!line.empty()) {
				output = line + "\n" + output;
			}
		}
	}

	if (!hasDegMerger) {
		return output;
	}

	// The output contains some spine mergers, so be careful and
	// place any adjacent mergers onto separate lines.

	output = prepareMergerLine(output, tracks, tokens, hasMerger, hasDegMerger);

	return output;
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
		if (!token->isDataType("**deg")) {
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
// Tool_deg::printDegInterpretation --
//

string Tool_deg::printDegInterpretation(const string& interp, HumdrumFile& infile, int lineIndex) {
	string output;
	int degIndex = 0;
	int kernCount = 0;
	for (int j=0; j<infile[lineIndex].getFieldCount(); j++) {
		HTp token = infile.token(lineIndex, j);
		if (!token->isKern()) {
			output += "*\t";
		} else {
			kernCount++;
			if (kernCount == 1) {
				output += "*\t";
			} else {
				// print **deg interps
				int kcount = m_degSpines.at(degIndex).at(lineIndex).size();
				for (int k=0; k<kcount; k++) {
					output += interp;
					output += "\t";
				}
				degIndex++;
				output += "*\t";
			}
		}
	}

	// print the last **deg spine:
	int kcount = m_degSpines.back().at(lineIndex).size();
	for (int k=0; k<kcount; k++) {
		output += interp;
		output += "\t";
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

string Tool_deg::prepareMergerLine(const string& input, vector<int>& tracks, vector<string>& tokens, bool inputMerger, bool outputMerger) {
	if (!outputMerger) {
		return input;
	}

	// group tokens by track
	vector<vector<string>> merge(1);
	vector<vector<int>> tks(1);
	merge[0].push_back(tokens[0]);
	tks[0].push_back(tracks[0]);
	for (int i=1; i<(int)tokens.size(); i++) {
		if (tracks[i-1] != tracks[i]) {
			merge.resize(merge.size() + 1);
			tks.resize(tks.size() + 1);
		}
		merge.back().push_back(tokens[i]);
		tks.back().push_back(tracks[i]);
	}

	// calculate result of spine manipulations
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
// Tool_deg::calculateManipulatorOutput -- Deal with *^ *v *- *+ manipulators
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
	bool printClef  = false;
	bool printColor = false;
	bool printStria = false;
	bool printStem  = false;
	bool foundData  = false;
	bool printArrow = !m_arrowQ;

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
				if (!printColor) {
					string line = createRecipInterpretation("*color:#fff0", i);
					m_humdrum_text << line << endl;
					printColor = true;
				}
				if (!printStem) {
					string line = createRecipInterpretation("*Xstem", i);
					m_humdrum_text << line << endl;
					printStem = true;
				}

			}

			if (!printArrow) {
				string line = createDegInterpretation("*arr", i, m_recipQ);
				m_humdrum_text << line << endl;
				printArrow = true;
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
			if (hre.search(current, "\\*([A-Ga-g][-#]*):(.*)")) {
				string key = hre.getMatch(1);
				string kmode = hre.getMatch(2);
				b40tonic = Convert::kernToBase40(key);
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



