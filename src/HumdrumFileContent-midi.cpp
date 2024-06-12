//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Oct  5 23:15:44 PDT 2015
// Filename:      HumdrumFileContent-slur.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-midi.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   HumdrumFileContent functions related to MIDI data.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE



/////////////////////////////////
//
// HumdrumFileContent::fillMidiInfo -- Create a data structure that
//     organizes tokens by track/midi note number.
//     Input object to fill is firsted indexed the **kern track
//     number, then by MIDI note number, and then an array of pairs
//     <HTp, int> where int is the subtoken number in the token
//     for the given MIDI note.


void HumdrumFileContent::fillMidiInfo(vector<vector<vector<pair<HTp, int>>>>& trackMidi) {
	HumdrumFileContent& infile = *this;
	vector<HTp> ktracks = infile.getKernSpineStartList();
	trackMidi.clear();
	trackMidi.resize(ktracks.size());
	for (int i=0; i<(int)trackMidi.size(); i++) {
		trackMidi[i].resize(128);   // 0 used for rests
	}
	// each entry is trackMidi[track][key] is an array of <token, subtoken> pairs.
	// using trackMidi[track][0] for rests;
	
	vector<int> trackToKernIndex = infile.getTrackToKernIndex();

	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp sstart = infile.getStrandStart(i);
		if (!sstart->isKern()) {
			continue;
		}

		int track = sstart->getTrack();
		HTp send = infile.getStrandEnd(i);
		processStrandNotesForMidi(sstart, send, trackMidi[trackToKernIndex[track]]);
	}
}



/////////////////////////////////
//
// HumdrumFileContent::processStrandNotesForMidi -- store strand tokens/subtokens by MIDI note
//     in the midi track entry.
//    
//     First index if track info is the MIDI note number, second is a list
//     of tokens for that note number, with the second value of the pair
//     giving the subtoken index of the note in the token.
//

void HumdrumFileContent::processStrandNotesForMidi(HTp sstart, HTp send, vector<vector<pair<HTp, int>>>& trackInfo) {
	HTp current = sstart->getNextToken();
	while (current && (current != send)) {
		if (!current->isData() || current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		vector<string> subtokens = current->getSubtokens();
		for (int i=0; i<(int)subtokens.size(); i++) {
			if (subtokens[i] == ".") {
				// something strange happened (no null tokens expected)
				continue;
			}
			if (subtokens[i].find("r") != string::npos) {
				// rest, so store in MIDI[0]
				trackInfo.at(0).emplace_back(current, 0);
			} else if (subtokens[i].find("R") != string::npos) {
				// unpitched or quasi-pitched note, so store in MIDI[0]
				trackInfo.at(0).emplace_back(current, 0);
			} else {
				int keyno = Convert::kernToMidiNoteNumber(subtokens[i]);
				if ((keyno >= 0) && (keyno < 128)) {
					trackInfo.at(keyno).emplace_back(current, i);
				}
			}
		}
		current = current->getNextToken();
	}
}


// END_MERGE

} // end namespace hum



