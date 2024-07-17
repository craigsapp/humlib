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
	define("a|average=b",           "calculate average length of rest-phrases by score");
	define("A|all-average=b",       "calculate average length of rest-phrases for all scores");
	define("f|filename=b",          "include filename in output analysis");
	define("F|full-filename=b",     "include full filename location in output analysis");
	define("m|b|measure|barline=b", "include barline numbers in output analysis");
	define("s|sort=b",              "sort phrases by short to long length");
	define("S|reverse-sort=b",      "sort phrases by long to short length");
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
	if (m_allAverageQ) {
		double average = m_sum / m_pcount;
		m_free_text << "All average phrase length: " << average << " minims" << endl;
	}
}



//////////////////////////////
//
// Tool_rphrase::initialize --
//

void Tool_rphrase::initialize(void) {
	m_averageQ      = getBoolean("average");
	m_allAverageQ   = getBoolean("all-average");
	m_filenameQ     = getBoolean("filename");
	m_fullFilenameQ = getBoolean("full-filename");
	m_barlineQ      = getBoolean("measure");
	m_sortQ         = getBoolean("sort");
	m_reverseSortQ  = getBoolean("reverse-sort");
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
	
	fillVoiceInfo(voiceInfo, kernStarts);
	if (!m_allAverageQ) {
		printVoiceInfo(voiceInfo);
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
// Tool_rphrase::fillVoiceInfo --
//

void Tool_rphrase::fillVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo,
		vector<HTp>& kstarts) {
	for (int i=0; i<(int)kstarts.size(); i++) {
		fillVoiceInfo(voiceInfo.at(i), kstarts.at(i));
	}
}


void Tool_rphrase::fillVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo, HTp& kstart) {
	HTp current        = kstart;
	bool inPhraseQ     = false;
	int currentBarline = 0;
	int startBarline   = 1;
	HumNum startTime   = 0;
	while (current) {
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
				HumNum endTime = current->getDurationFromStart();
				HumNum duration = endTime - startTime;
				startTime = -1;
				inPhraseQ = false;
				voiceInfo.phraseDurs.push_back(duration.getFloat() / 2.0);
				voiceInfo.barStarts.push_back(startBarline);
				m_sum += duration.getFloat() / 2.0;
				m_pcount++;
			} else {
				// continuing a phrase, so do nothing
			}
		} else {
			// Not in phrase, so continue if rest; otherwise,
			// if a note, then record a phrase start.
			if (current->isRest()) {
				// continuing a non-phrase, so do nothing
			} else {
				startTime = current->getDurationFromStart();
				startBarline = currentBarline;
				inPhraseQ = true;
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
		m_sum += duration.getFloat() / 2.0;
		m_pcount++;
	}
}


// END_MERGE

} // end namespace hum



