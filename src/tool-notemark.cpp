//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Mon Oct 14 19:24:00 UTC 2024
// Filename:      tool-notemark.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-notemark.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Adds note markers for specific lines and voices
//

#include <iostream>
#include <vector>
#include <algorithm> 

#include "tool-notemark.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_notemark::Tool_notemark -- Set the recognized options for the tool.
//

Tool_notemark::Tool_notemark(void) {
	define("k|kern-tracks=s",      "process only the specified kern spines");
	define("s|spine-tracks=s",     "process only the specified spines");
	define("l|lines|line-range=s", "line numbers range to yank (e.g. 40-50)");
	define("signifier=s",          "signifier to mark the notes (default: @)");
	define("color=s",              "color for marked notes (default: #ef4444)");

}



//////////////////////////////
//
// Tool_notemark::run -- Do the main work of the tool.
//

bool Tool_notemark::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_notemark::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_notemark::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_notemark::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_notemark::initialize --
//

void Tool_notemark::initialize(void) {
	m_kernTracks = getString("kern-tracks");
	m_spineTracks = getString("spine-tracks");
	m_lineRange = getString("lines");
	m_signifier = getBoolean("signifier") ? getString("signifier") : "@";
	m_color = getBoolean("color") ? getString("color") : "#ef4444";
}



//////////////////////////////
//
// Tool_notemark::processFile --
//

void Tool_notemark::processFile(HumdrumFile& infile) {

	std::vector<int> selectedSpineIndices = setupSpineInfo(infile);

	bool hasMarkers = false;

	if (getBoolean("lines")) {
		int startLineNumber = getStartLineNumber();
		int endLineNumber = getEndLineNumber();
		if ((startLineNumber > endLineNumber) || (endLineNumber > infile.getLineCount())) {
			// Disallow when end line number is bigger then line count or when
			// start line number greather than end line number
			return;
		}

		for (int i = startLineNumber; i <= endLineNumber; i++) {
			HLp line = infile.getLine(i-1);
			if (line->isData()) {
				vector<HTp> tokens;
				line->getTokens(tokens);
				for (int j = 0; j < tokens.size(); j++) {
					HTp token = tokens[j];
					if (token->isNonNullData()) {
						if (std::find(selectedSpineIndices.begin(), selectedSpineIndices.end(), token->getSpineIndex()) != selectedSpineIndices.end() || selectedSpineIndices.size() == 0) {
							token->setText(token->getText() + m_signifier);
							hasMarkers = true;
						}
					}
				}
				line->createLineFromTokens();
			}
		}
	}

	if (hasMarkers) {
		infile.appendLine("!!!RDF**kern: " + m_signifier + " = marked note color=\"" + m_color + "\"");
	}


	m_humdrum_text << infile;

}



////////////////////////
//
// Tool_notemark::getStartLineNumber -- Get start line number from --lines
//

int Tool_notemark::getStartLineNumber(void) {
	HumRegex hre;
	if (hre.search(m_lineRange, "^(\\d+)\\-(\\d+)$")) {
		return hre.getMatchInt(1);
	}
	return -1;
}



////////////////////////
//
// Tool_notemark::getEndLineNumber -- Get end line number from --lines
//

int Tool_notemark::getEndLineNumber(void) {
	HumRegex hre;
	if (hre.search(m_lineRange, "^(\\d+)\\-(\\d+)$")) {
		return hre.getMatchInt(2);
	}
	return -1;
}



//////////////////////////////
//
// Tool_notemark::setupSpineInfo --
//

std::vector<int> Tool_notemark::setupSpineInfo(HumdrumFile& infile) {

	infile.getKernSpineStartList(m_kernSpines);

	m_selectedKernSpines.clear();

	if (!m_kernTracks.empty()) {
		vector<int> tracks = Convert::extractIntegerList(m_kernTracks, (int)m_kernSpines.size());
		// don't allow out-of-sequence values for the tracks list:
		sort(tracks.begin(),tracks.end());
		tracks.erase(unique(tracks.begin(), tracks.end()), tracks.end());
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

 	std::vector<int> spineIndices(m_selectedKernSpines.size());
	std::transform(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), spineIndices.begin(),
        [](const HTp token) {
            return token->getSpineIndex();
        });

	return spineIndices;
}



// END_MERGE

} // end namespace hum
