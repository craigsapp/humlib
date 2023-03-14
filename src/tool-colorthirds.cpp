//
// Programmer:    Katherine Wong
// Creation Date: Mon Mar 13 23:45:00 PDT 2023
// Last Modified: Mon Mar 13 23:45:05 PDT 2023
// Filename:      tool-colorthirds.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-colorthirds.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color the positions of notes in triads according to their thirds position:
//                   red   = root
//                   green = third
//                   blue  = fifth
//

#include "tool-colorthirds.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_colorthirds::Tool_colorthirds -- Set the recognized options for the tool.
//

Tool_colorthirds::Tool_colorthirds(void) {
	// define("filters=b", "print filter commands only");
}



/////////////////////////////////
//
// Tool_colorthirds::run -- Do the main work of the tool.
//

bool Tool_colorthirds::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_colorthirds::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colorthirds::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_colorthirds::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_colorthirds::initialize -- Setup to do before processing a file.
//

void Tool_colorthirds::initialize(void) {
	// do nothing
}



//////////////////////////////
//
// Tool_colorthirds::processFile -- Analyze an input file.
//

void Tool_colorthirds::processFile(HumdrumFile& infile) {
	// Algorithm go line by line in the infile, extracting the notes that are active
	// check to see if the list of notes form a triad
	// label the root third and fifth notes of the triad

	vector<HTp> kernNotes;
	vector<int> midiNotes;
	vector<int> chordPositions;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) { // if no notes in the line
			continue;
		}
		// iterate along the line looking at each field, and creating a '
		//     list of tokens that are notes.
		kernNotes.clear();
		midiNotes.clear();
		chordPositions.clear();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j); // get ptr to cell
			// if not a **kern token, then skip:
			if (!token->isKern()) {
				continue;
			}

			if (token->isRest()) {
				continue;
			}

			HTp resolvedToken = NULL;
			if (token->isNull()) {
				resolvedToken = token->resolveNull();
				if (!resolvedToken || resolvedToken->isRest()) {
					continue;
				}
			}
			if (resolvedToken) {
				kernNotes.push_back(resolvedToken);
			} else {
				kernNotes.push_back(token);
			}
		}
		midiNotes = getMidiNotes(kernNotes);
		chordPositions = getChordPositions(midiNotes);
		labelChordPositions(kernNotes, chordPositions);
	}

	infile.createLinesFromTokens();

	m_humdrum_text << infile;

	m_humdrum_text << "!!!RDF**kern: " << m_root_marker
		<< " = marked note, root position, color=\"" << m_root_color << "\"" << endl;

	m_humdrum_text << "!!!RDF**kern: " << m_third_marker
		<< " = marked note, third position, color=\"" << m_third_color << "\"" << endl;

	m_humdrum_text << "!!!RDF**kern: " << m_fifth_marker
		<< " = marked note, third position, color=\"" << m_fifth_color << "\"" << endl;
}



//////////////////////////////
//
// Tool_colorthirds::labelChordPositions -- Mark the scale degree of notes:
//       0: not in triadic sonority so no label.
//       1 == @: root (such as C in C major triad)
//       3 == N third (such as E in C major triad)
//       5 == Z fifth (such as G in C major triad)
//

void Tool_colorthirds::labelChordPositions(vector<HTp>& kernNotes, vector<int>& chordPositions) {
	for (int i=0; i<(int)kernNotes.size(); i++) {
		int position = chordPositions.at(i);
		if (position == 0) {
			continue;
		}
		string label;
		switch (position) {
			case 1: label = m_root_marker; break;
			case 3: label = m_third_marker; break;
			case 5: label = m_fifth_marker; break;
		}
		if (label.empty()) {
			continue;
		}
		string text = *kernNotes[i];
		text += label;
		kernNotes[i]->setText(text);
	}
}

//////////////////////////////
//
// Tool_colorthirds::getChordPositions -- Identify if the sonority is a triad, and if so, place
//    the position of the note in the chord:
//       0: not in triadic sonority.
//       1: root (such as C in C major triad)
//       3: third (such as E in C major triad)
//       5: fifth (such as G in C major triad)
//
vector<int> Tool_colorthirds::getChordPositions(vector<int>& midiNotes) {
	vector<int> output(midiNotes.size(), 0);
	if (midiNotes.empty()) {
		return output;
	}

	// create modulo 12 arr
	vector<int> pitchClasses(12, 0);
	for (int i = 0; i < (int)midiNotes.size(); i++) {
		pitchClasses[midiNotes[i] % 12]++; // add one to the pitch
	}

	vector<int> noteMods;

	for (int i = 0; i < (int)pitchClasses.size(); i++) {
		if (pitchClasses[i] != 0) { // not zero
			noteMods.push_back(i); // add the index
		}
	}

	if (noteMods.size() != 3) { // not a triad
		return output;
	}

	int bint = noteMods[1] - noteMods[0];
	int tint = noteMods[2] - noteMods[1];

	int rootClass = -1; // curr uninitialized
	int thirdClass = -1;
	int fifthClass = -1;

	// NOTE: right now, noteMods is not in the order that the og notes in the score are in
	// aka --> these inversions are not correct
	if ((bint == 3 && tint == 4) || (bint == 4 && tint == 3) || (bint == 3 && tint == 3)) { // root pos
		rootClass = noteMods[0];
		thirdClass = noteMods[1];
		fifthClass = noteMods[2];
	} else if ((bint == 4 && tint == 5) || (bint == 3 && tint == 5) || (bint == 3 && tint == 6)) { // first inv
		rootClass = noteMods[2];
		thirdClass = noteMods[0];
		fifthClass = noteMods[1];
	} else if ((bint == 5 && tint == 3) || (bint == 5 && tint == 4) || (bint == 6 && tint == 3)) { // sec inv
		rootClass = noteMods[1];
		thirdClass = noteMods[2];
		fifthClass = noteMods[0];
	}

	if (rootClass == -1) {
		return output;
	}

	for (int i = 0; i < (int)midiNotes.size(); i++) {
		if (midiNotes[i] % 12 == rootClass) {
			output[i] = 1;
		}
		else if (midiNotes[i] % 12 == thirdClass) {
			output[i] = 3;
		}
		else if (midiNotes[i] % 12 == fifthClass) {
			output[i] = 5;
		}
	}

	return output;
}


//////////////////////////////
//
// Tool_colorthirds::getMidiNotes -- Convert kern notes to MIDI note numbers.
//    If the note is sustaining, then make the MIDI note number negative.
//

vector<int> Tool_colorthirds::getMidiNotes(vector<HTp>& kernNotes) {
	vector<int> output(kernNotes.size());
	if (kernNotes.empty()) {
		return output;
	}
	for (int i=0; i<(int)kernNotes.size(); i++) {
		int midiNote = kernNotes[i]->getMidiPitch();
		output.at(i) = midiNote;
	}
	return output;
}



// END_MERGE

} // end namespace hum



