//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 15 23:33:44 PDT 2024
// Last Modified: Tue Jul 16 08:29:14 PDT 2024
// Filename:      tool-rphrase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-rphrase.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of notes as phrases delimited by rests.
//

#include "tool-rphrase.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_rphrase::Tool_rphrase -- Set the recognized options for the tool.
//

Tool_rphrase::Tool_rphrase(void) {
	// add command-line options here
	define("a|average=b",            "calculate average length of rest-phrases by score");
	define("A|all-average=b",        "calculate average length of rest-phrases for all scores");
	define("c|composite|collapse=b", "collapse all voices into single part");
	define("f|filename=b",           "include filename in output analysis");
	define("F|full-filename=b",      "include full filename location in output analysis");
	define("l|longa=b",              "display minim length of longas");
	define("m|b|measure|barline=b",  "include barline numbers in output analysis");
	define("mark=b",                 "mark starts of phrases in score");
	define("s|sort=b",               "sort phrases by short to long length");
	define("S|reverse-sort=b",       "sort phrases by long to short length");
}



/////////////////////////////////
//
// Tool_rphrase::run -- Do the main work of the tool.
//

bool Tool_rphrase::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_rphrase::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rphrase::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rphrase::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_rphrase::finally --
//

void Tool_rphrase::finally(void) {
	if (!m_markQ) {
		if (m_allAverageQ) {
			if (m_collapseQ) {
				double average = m_sumCollapse / m_pcountCollapse;
				m_free_text << "Composite average phrase length: " << average << " minims" << endl;
			} else {
				double average = m_sum / m_pcount;
				m_free_text << "All average phrase length: " << average << " minims" << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_rphrase::initialize --
//

void Tool_rphrase::initialize(void) {
	m_averageQ      = getBoolean("average");
	m_barlineQ      = getBoolean("measure");
	m_allAverageQ   = getBoolean("all-average");
	#ifndef __EMSCRIPTEN__
		m_collapseQ     = !getBoolean("collapse");
	#else
		m_collapseQ     = getBoolean("collapse");
	#endif
	m_filenameQ     = getBoolean("filename");
	m_fullFilenameQ = getBoolean("full-filename");
	m_longaQ        = getBoolean("longa");
	#ifndef __EMSCRIPTEN__
		m_markQ         = getBoolean("mark");
	#else
		m_markQ         = !getBoolean("mark");
	#endif
	m_sortQ         = getBoolean("sort");
	m_reverseSortQ  = getBoolean("reverse-sort");
	m_longaQ        = getBoolean("longa");
}



//////////////////////////////
//
// Tool_rphrase::processFile --
//

void Tool_rphrase::processFile(HumdrumFile& infile) {
	if (m_filenameQ) {
		m_filename = infile.getFilename();
		HumRegex hre;
		hre.replaceDestructive(m_filename, "", ".*\\/");
		hre.replaceDestructive(m_filename, "", "\\.krn$");
	} else if (m_fullFilenameQ) {
		m_filename = infile.getFilename();
	}
	vector<HTp> kernStarts = infile.getKernSpineStartList();
	vector<Tool_rphrase::VoiceInfo> voiceInfo(kernStarts.size());
	Tool_rphrase::VoiceInfo collapseInfo;

	if (m_collapseQ) {
		fillCollapseInfo(collapseInfo, infile);
	} else {
		fillVoiceInfo(voiceInfo, kernStarts, infile);
	}


	if (m_longaQ) {
		markLongaDurations(infile);
	}

	if (!m_allAverageQ) {
		if (m_collapseQ) {
			printVoiceInfo(collapseInfo);
		} else {
			printVoiceInfo(voiceInfo);
		}
	}

	if (m_markQ) {
		outputMarkedFile(infile);
	}

}



//////////////////////////
//
// Tool_rphrase::markLongaDuratios --
//

void Tool_rphrase::markLongaDurations(HumdrumFile& infile) {
	string longrdf;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "RDF**kern") {
			continue;
		}
		string value = infile[i].getReferenceValue();
		HumRegex hre;
		if (hre.search(value, "^\\s*([^\\s=]+)\\s*=.*long")) {
			longrdf = hre.getMatch(1);
			break;
		}
	}

	if (longrdf.empty()) {
		return;
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->find(longrdf) != string::npos) {
				HumNum duration = token->getTiedDuration();
				stringstream value;
				value.str("");
				value << duration.getFloat() / 2.0;
				token->setValue("auto", "rphrase-longa", value.str());
			}
		}
	}
}



//////////////////////////////
//
// Tool_rphrase::outputMarkedFile --
//

void Tool_rphrase::outputMarkedFile(HumdrumFile& infile) {
	m_free_text.clear();
	m_free_text.str("");
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			m_humdrum_text << infile[i] << endl;
		} else {
			printDataLine(infile, i);
		}
	}
}



//////////////////////////////
//
// Tool_rphrase::printDataLine --
//

void Tool_rphrase::printDataLine(HumdrumFile& infile, int index) {

	bool hasLonga = false;
	if (m_longaQ) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				continue;
			}
			string lotext = token->getValue("auto", "rphrase-longa");
			if (!lotext.empty()) {
				hasLonga = true;
				break;
			}
		}
	}


	bool hasLo = false;
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern()) {
			continue;
		}
		string lotext = token->getValue("auto", "rphrase-start");
		if (!lotext.empty()) {
			hasLo = true;
			break;
		}
	}

	if (hasLonga) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				m_humdrum_text << "!";
			} else {
				string value = token->getValue("auto", "rphrase-longa");
				if (value.empty()) {
					m_humdrum_text << "!";
				} else {
					m_humdrum_text << "!LO:TX:a:B:color=silver:t=" << value;
				}
			}
			if (j < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	if (hasLo) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				m_humdrum_text << "!";
			} else {
				string value = token->getValue("auto", "rphrase-start");
				if (value.empty()) {
					m_humdrum_text << "!";
				} else {
					m_humdrum_text << "!LO:TX:a:B:color=red:t=" << value;
				}
			}
			if (j < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	m_humdrum_text << infile[index] << endl;
}



//////////////////////////////
//
// Tool_rphrase::getCompositeStates --
//

void Tool_rphrase::getCompositeStates(vector<int>& noteStates, HumdrumFile& infile) {
	noteStates.resize(infile.getLineCount());
	fill(noteStates.begin(), noteStates.end(), -1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		int value = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			} else if (token->isNull()) {
				HTp resolve = token->resolveNull();
				if (!resolve) {
					continue;
				} else if (resolve->isRest()) {
					continue;
				} else {
					value = 1;
					break;
				}
			} else {
				value = 1;
				break;
			}
		}
		noteStates[i] = value;
	}
}



//////////////////////////////
//
// Tool_rphrase::printVoiceInfo --
//

void Tool_rphrase::printVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo) {
	for (int i=(int)voiceInfo.size() - 1; i>=0; i--) {
		printVoiceInfo(voiceInfo[i]);
	}
}


void Tool_rphrase::printVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo) {
	if (m_filenameQ) {
		m_free_text << m_filename << "\t";
	}
	m_free_text << voiceInfo.name << "\t";

	if (m_averageQ) {
		double sum = 0;
		int count = 0;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			count++;
			sum += voiceInfo.phraseDurs.at(i);
		}
		m_free_text << int(sum / count * 100.0 + 0.5)/100.0 << "\t";
	}

	if (m_sortQ || m_reverseSortQ) {
		vector<pair<double, int>> sortList;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			sortList.emplace_back(voiceInfo.phraseDurs[i], i);
		}
		if (m_sortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first < b.first;
			});
		} else if (m_reverseSortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first > b.first;
			});
		}

		for (int i=0; i<(int)sortList.size(); i++) {
			int ii = sortList[i].second;
			if (m_barlineQ) {
				m_free_text << "m" << voiceInfo.barStarts.at(ii) << ":";
			}
			m_free_text << voiceInfo.phraseDurs.at(ii);
			if (i < (int)sortList.size() - 1) {
				m_free_text << " ";
			}
		}
	} else {
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			if (voiceInfo.restsBefore.at(i) > 0) {
				m_free_text << "r:" << voiceInfo.restsBefore.at(i) << " ";
			}
			if (m_barlineQ) {
				m_free_text << "m" << voiceInfo.barStarts.at(i) << ":";
			}
			m_free_text << voiceInfo.phraseDurs.at(i);
			if (i < (int)voiceInfo.phraseDurs.size() - 1) {
				m_free_text << " ";
			}
		}
	}

	m_free_text << endl;
}



//////////////////////////////
//
// Tool_rphrase::fillCollapseInfo --
//

void Tool_rphrase::fillCollapseInfo(Tool_rphrase::VoiceInfo& collapseInfo, HumdrumFile& infile) {
	collapseInfo.name = getCompositeLabel(infile);
	vector<int> noteStates;
	getCompositeStates(noteStates, infile);

	bool inPhraseQ       = false;
	int currentBarline   = 0;
	int startBarline     = 1;
	HumNum startTime     = 0;

	HumNum restBefore    = 0;
	HumNum startTimeRest = 0;

	HumNum scoreDur = infile.getScoreDuration();

	for (int i=0; i<infile.getLineCount(); i++) {

		// Split phrases at double barlines (medial cadences):
		if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			if (token->find("||") != string::npos) {
				HumNum tdur = token->getDurationFromStart();
				if (tdur != scoreDur) {
					// Only process if double barline is not at the end of the score.

					if (inPhraseQ) {
						// In phrase, so continue if still notes, otherwise
						// if a rest, then record the currently active phrase
						// that has ended.

						// ending a phrase
						HumNum endTime = infile[i].getDurationFromStart();
						HumNum duration = endTime - startTime;
						startTime = -1;
						inPhraseQ = false;
						collapseInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
						collapseInfo.barStarts.push_back(startBarline);
						m_sumCollapse += duration.getFloat() / 2.0;
						m_pcountCollapse++;
						collapseInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);

						// record rest start
						startTimeRest = endTime;
					} else {
						// Not in phrase, so not splitting a rest region.
						// This case should be rare (starting a medial cadence
						// with rests and potentially starting new section with rests.
					}

				}
			}
		}

		if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			HumRegex hre;
			if (hre.search(token, "(\\d+)")) {
				currentBarline = hre.getMatchInt(1);
				continue;
			}
		}


		if (!infile[i].isData()) {
			continue;
		}

		if (inPhraseQ) {
			// In phrase, so continue if still notes, otherwise
			// if a rest, then record the currently active phrase
			// that has ended.
			if (noteStates[i] == 0) {
				// ending a phrase
				HumNum endTime = infile[i].getDurationFromStart();
				HumNum duration = endTime - startTime;
				startTime = -1;
				inPhraseQ = false;
				collapseInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
				collapseInfo.barStarts.push_back(startBarline);
				m_sumCollapse += duration.getFloat() / 2.0;
				m_pcountCollapse++;
				collapseInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);
				// record rest start
				startTimeRest = endTime;
			} else {
				// continuing a phrase, so do nothing
			}
		} else {
			// Not in phrase, so continue if rest; otherwise,
			// if a note, then record a phrase start.
			if (noteStates[i] == 0) {
				// continuing a non-phrase, so do nothing
			} else {
				// starting a phrase
				startTime = infile[i].getDurationFromStart();
				startBarline = currentBarline;
				inPhraseQ = true;
				// check if there are rests before the phrase
				// The rest duration will be stored when the
				// end of the next phrase is encountered.
				if (startTimeRest >= 0) {
					restBefore = startTime - startTimeRest;
				} else {
					restBefore = 0;
				}
			}
		}

	}

	if (inPhraseQ) {
		// process last phrase
		HumNum endTime = infile.getScoreDuration();
		HumNum duration = endTime - startTime;
		collapseInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
		collapseInfo.barStarts.push_back(startBarline);
		m_sumCollapse += duration.getFloat() / 2.0;
		m_pcountCollapse++;
		collapseInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);
	}
}



//////////////////////////////
//
// Tool_rphrase::getCompositeLabel --
//

string Tool_rphrase::getCompositeLabel(HumdrumFile& infile) {
	string voices;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "voices") {
			continue;
		}
		voices = infile[i].getReferenceValue();
		break;
	}

	if (voices.empty()) {
		return "composite";
	}

	vector<HTp> kstarts = infile.getKernSpineStartList();

	string output = "composite ";
	output += voices;


	HumRegex hre;

	if (hre.search(voices, "^\\d+$")) {
		int vint = stoi(voices);
		if (vint != kstarts.size()) {
			output += "(";
			output += to_string(kstarts.size());
			output += ")";
		}
	} else {
		output += "(";
		output += to_string(kstarts.size());
		output += ")";
	}

	return output;
}



//////////////////////////////
//
// Tool_rphrase::fillVoiceInfo --
//

void Tool_rphrase::fillVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo,
		vector<HTp>& kstarts, HumdrumFile& infile) {
	for (int i=0; i<(int)kstarts.size(); i++) {
		fillVoiceInfo(voiceInfo.at(i), kstarts.at(i), infile);
	}
}



void Tool_rphrase::fillVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo, HTp& kstart, HumdrumFile& infile) {
	HTp current = kstart;

	bool inPhraseQ       = false;
	int currentBarline   = 0;
	int startBarline     = 1;
	HumNum startTime     = 0;

	HumNum restBefore    = 0;
	HumNum startTimeRest = 0;

	HumNum scoreDur = infile.getScoreDuration();
	HTp phraseStartTok = NULL;

	while (current) {

		// Split phrases at double barlines (medial cadences):
		if (infile[current->getLineIndex()].isBarline()) {
			HTp token = infile.token(current->getLineIndex(), 0);
			if (token->find("||") != string::npos) {
				HumNum tdur = token->getDurationFromStart();
				if (tdur != scoreDur) {
					// Only process if double barline is not at the end of the score.

					if (inPhraseQ) {
						// In phrase, so continue if still notes, otherwise
						// if a rest, then record the currently active phrase
						// that has ended.

						HumNum endTime = current->getDurationFromStart();
						HumNum duration = endTime - startTime;
						startTime = -1;
						inPhraseQ = false;
						voiceInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
						voiceInfo.barStarts.push_back(startBarline);
						voiceInfo.phraseStartToks.push_back(phraseStartTok);
						phraseStartTok = NULL;
						m_sum += duration.getFloat() / 2.0;
						m_pcount++;
						voiceInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);

						// record rest start
						startTimeRest = endTime;
					} else {
						// Not in phrase, so not splitting a rest region.
						// This case should be rare (starting a medial cadence
						// with rests and potentially starting new section with rests.
					}

				}
			}
		}

		if (current->isBarline()) {
			HumRegex hre;
			if (hre.search(current, "(\\d+)")) {
				currentBarline = hre.getMatchInt(1);
				current = current->getNextToken();
				continue;
			}
		}

		if (current->isInstrumentName()) {
			voiceInfo.name = current->substr(3);
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		if (inPhraseQ) {
			// In phrase, so continue if still notes, otherwise
			// if a rest, then record the currently active phrase
			// that has ended.
			if (current->isRest()) {
				// ending a phrase
				HumNum endTime = current->getDurationFromStart();
				HumNum duration = endTime - startTime;
				startTime = -1;
				inPhraseQ = false;
				voiceInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
				voiceInfo.barStarts.push_back(startBarline);
				voiceInfo.phraseStartToks.push_back(phraseStartTok);
				phraseStartTok = NULL;
				m_sum += duration.getFloat() / 2.0;
				m_pcount++;
				voiceInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);
				// record rest start
				startTimeRest = endTime;
			} else {
				// continuing a phrase, so do nothing
			}
		} else {
			// Not in phrase, so continue if rest; otherwise,
			// if a note, then record a phrase start.
			if (current->isRest()) {
				// continuing a non-phrase, so do nothing
			} else {
				// starting a phrase
				startTime = current->getDurationFromStart();
				startBarline = currentBarline;
				inPhraseQ = true;
				// check if there are rests before the phrase
				// The rest duration will be stored when the
				// end of the next phrase is encountered.
				if (startTimeRest >= 0) {
					restBefore = startTime - startTimeRest;
				} else {
					restBefore = 0;
				}
				phraseStartTok = current;
			}
		}

		current = current->getNextToken();
	}
	if (inPhraseQ) {
		// process last phrase
		HumNum endTime = kstart->getLine()->getOwner()->getScoreDuration();
		HumNum duration = endTime - startTime;
		voiceInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
		voiceInfo.barStarts.push_back(startBarline);
		voiceInfo.phraseStartToks.push_back(phraseStartTok);
		m_sum += duration.getFloat() / 2.0;
		m_pcount++;
		voiceInfo.restsBefore.push_back(0.0);
		voiceInfo.restsBefore.push_back(restBefore.getFloat() / 2.0);
	}

	if (m_markQ) {
		markPhraseStartsInScore(infile, voiceInfo);
	}
}


//////////////////////////////
//
// Tool_rphrase::markPhraseStartsInScore --
//

void Tool_rphrase::markPhraseStartsInScore(HumdrumFile& infile, Tool_rphrase::VoiceInfo& voiceInfo) {
	stringstream buffer;
	for (int i=0; i<(int)voiceInfo.phraseStartToks.size(); i++) {
		HTp tok = voiceInfo.phraseStartToks.at(i);
		string measure = "";
		if (m_barlineQ) {
			measure = to_string(voiceInfo.barStarts.at(i));
		}
		double duration = voiceInfo.phraseDurs.at(i);
		buffer.str("");
		if (!measure.empty()) {
			buffer << "m" << measure << "&colon;";
		}
		buffer << duration;
		tok->setValue("auto", "rphrase-start", buffer.str());
	}
}


// END_MERGE

} // end namespace hum



