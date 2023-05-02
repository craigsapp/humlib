//
// Programmer:    Katherine Wong
// Programmer:    Craig Stuart Sapp
// Creation Date: Mon Mar 13 23:45:00 PDT 2023
// Last Modified: Fri Mar 17 12:36:19 PDT 2023
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
	define("d|double=b", "highlight only doubled notes in triads");
	define("3|no-thirds=b", "do not color thirds");
	define("5|no-fifths=b", "do not color fifths");
	define("T|no-triads=b", "do not color full triads");
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
	m_colorThirds = !getBoolean("no-thirds");
	m_colorFifths = !getBoolean("no-thirds");
	m_colorTriads = !getBoolean("no-triads");
   m_doubleQ = getBoolean("double");
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
    vector<int> thirdPositions;
    vector<int> fifthPositions;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) { // if no notes in the line
			continue;
		}
		// iterate along the line looking at each field, and creating a '
		//     list of tokens that are notes.
		kernNotes.clear();
		midiNotes.clear();
		chordPositions.clear();
        thirdPositions.clear();
        fifthPositions.clear();
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

        if (m_colorThirds) { // thirds
            thirdPositions = getThirds(midiNotes);

            if (m_doubleQ) { // label only doubles if prompted
                keepOnlyDoubles(thirdPositions);
            }

            labelThirds(kernNotes, thirdPositions);
        }

        if (m_colorFifths) { // fifths
            fifthPositions = getFifths(midiNotes);

            if (m_doubleQ) { // label only doubles if prompted
                keepOnlyDoubles(fifthPositions);
            }

            labelFifths(kernNotes, fifthPositions);
        }

        if (m_colorTriads) { // triads
		    chordPositions = getChordPositions(midiNotes);

            if (m_doubleQ) { // label only doubles if prompted
                keepOnlyDoubles(chordPositions);
            }

		    labelChordPositions(kernNotes, chordPositions);
        }
	}

	infile.createLinesFromTokens();

	m_humdrum_text << infile;

    if (m_colorThirds) { // color thirds
        m_humdrum_text << "!!!RDF**kern: " << m_3rd_root_marker
            << " = marked note, root position, color=\"" << m_3rd_root_color << "\"" << endl;

        m_humdrum_text << "!!!RDF**kern: " << m_3rd_third_marker
            << " = marked note, third position, color=\"" << m_3rd_third_color << "\"" << endl;
    }

    if (m_colorFifths) { // color fifths
        m_humdrum_text << "!!!RDF**kern: " << m_5th_root_marker
            << " = marked note, root position, color=\"" << m_5th_root_color << "\"" << endl;

        m_humdrum_text << "!!!RDF**kern: " << m_5th_fifth_marker
            << " = marked note, fifth position, color=\"" << m_5th_fifth_color << "\"" << endl;
    }

    if (m_colorTriads) { // color full triads
	    m_humdrum_text << "!!!RDF**kern: " << m_root_marker
		    << " = marked note, root position, color=\"" << m_root_color << "\"" << endl;

	    m_humdrum_text << "!!!RDF**kern: " << m_third_marker
		    << " = marked note, third position, color=\"" << m_third_color << "\"" << endl;

	    m_humdrum_text << "!!!RDF**kern: " << m_fifth_marker
		    << " = marked note, third position, color=\"" << m_fifth_color << "\"" << endl;
    }
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
		string text = *kernNotes.at(i);
		text += label;
		kernNotes.at(i)->setText(text);
	}
}

//////////////////////////////
//
// Tool_colorthirds::labelThirds -- Mark the scale degree of notes:
//       0: not in interval sonority so no label.
//       1 == @: root (bottom of the interval)
//       3 == N: third (top of the interval)

void Tool_colorthirds::labelThirds(vector<HTp>& kernNotes, vector<int>& thirdPositions) {
    for (int i = 0; i < (int)kernNotes.size(); i++) {
        int position = thirdPositions.at(i);
        if (position == 0) {
            continue;
        }

        string label;
        switch (position) {
            case 1: label = m_3rd_root_marker; break;
            case 3: label = m_3rd_third_marker; break;
        }

        if (label.empty()) {
            continue;
        }
        string text = *kernNotes.at(i);
        text += label;
        kernNotes.at(i)->setText(text);
     }
}

//////////////////////////////
//
// Tool_colorthirds::labelFifths -- Mark the scale degree of notes:
//       0: not in interval sonority so no label.
//       1 == @: root (bottom of the interval)
//       5 == Z: fifth (top of the interval)

void Tool_colorthirds::labelFifths(vector<HTp>& kernNotes, vector<int>& fifthPositions) {
    for (int i = 0; i < (int)kernNotes.size(); i++) {
        int position = fifthPositions.at(i);
        if (position == 0) {
            continue;
        }

        string label;
        switch (position) {
            case 1: label = m_5th_root_marker; break;
            case 5: label = m_5th_fifth_marker; break;
        }

        if (label.empty()) {
            continue;
        }
        string text = *kernNotes.at(i);
        text += label;
        kernNotes.at(i)->setText(text);
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

vector<int> Tool_colorthirds::getNoteMods(vector<int>& midiNotes) {
	// create modulo 12 arr
	vector<int> pitchClasses(12, 0);
	for (int i = 0; i < (int)midiNotes.size(); i++) {
		int index = midiNotes.at(i) % 12;
		pitchClasses.at(index)++; // add one to the pitch
	}

	vector<int> noteMods;

	for (int i = 0; i < (int)pitchClasses.size(); i++) {
		if (pitchClasses.at(i) != 0) { // not zero
			noteMods.push_back(i); // add the index
		}
    }

    return noteMods;
}

// Tool_colorthirds::getThirds -- Identify if the sonority is a third interval, and if so,
//    place the position of the note in the output.
//       0: not in the sonority
//       1: root (bottom of the interval)
//       3: third (top of the interval)

vector<int> Tool_colorthirds::getThirds(vector<int>& midiNotes) {
    vector<int> output(midiNotes.size(), 0);

    if (midiNotes.empty()) {
        return output;
    }

    vector<int> noteMods = getNoteMods(midiNotes); // we know noteMods is sorted
    if (noteMods.size() != 2) {
        return output;
    }

    int interval = noteMods[1] - noteMods[0];
    int rootClass = -1; // currently uninitialized
    int thirdClass = -1;

    if (interval == 3 || interval == 4) { // third is found
        rootClass = noteMods.at(0);
        thirdClass = noteMods.at(1);
    }
    else if (interval == 8 || interval == 9) { // third is found (inversion)
        rootClass = noteMods.at(1);
        thirdClass = noteMods.at(0);
    }

    if (rootClass == -1) { // third was not found
        return output;
    }

    // populate output
    for (int i = 0; i < (int)midiNotes.size(); i++) {
        if (midiNotes.at(i) % 12 == rootClass) {
            output.at(i) = 1;
        }
        else if (midiNotes.at(i) % 12 == thirdClass) {
            output.at(i) = 3;
        }
    }

    return output;
}

// Tool_colorthirds::getFifths -- Identify if the sonority is a fifth interval, and if so,
//    place the position of the note in the output.
//       0: not in the sonority
//       1: root (bottom of the interval)
//       5: fifth (top of the interval)

vector<int> Tool_colorthirds::getFifths(vector<int>& midiNotes) {
    vector<int> output(midiNotes.size(), 0);

    if (midiNotes.empty()) {
        return output;
    }

    vector<int> noteMods = getNoteMods(midiNotes);
    if (noteMods.size() != 2) {
        return output;
    }

    int interval = noteMods.at(1) - noteMods.at(0);
    int rootClass = -1; // currently uninitialized
    int fifthClass = -1;

    if ((interval == 7) || (interval == 6)) { // fifth found
        rootClass = noteMods.at(0);
        fifthClass = noteMods.at(1);
    }

    if (interval == 5) { // inverted fifth found
        rootClass = noteMods.at(1);
        fifthClass = noteMods.at(0);
    }

    if (rootClass == -1) { // fifth not found
        return output;
    }

    // populate output
    for (int i = 0; i < (int)midiNotes.size(); i++) {
        if (midiNotes.at(i) % 12 == rootClass) {
            output.at(i) = 1;
        }
        else if (midiNotes.at(i) % 12 == fifthClass) {
            output.at(i) = 5;
        }
    }

    return output;
}

vector<int> Tool_colorthirds::getChordPositions(vector<int>& midiNotes) {
	vector<int> output(midiNotes.size(), 0);
	if (midiNotes.empty()) {
		return output;
	}

	vector<int> noteMods = getNoteMods(midiNotes);

	if (noteMods.size() != 3) { // not a triad
		return output;
	}

	int bint = noteMods.at(1) - noteMods.at(0);
	int tint = noteMods.at(2) - noteMods.at(1);

	int rootClass = -1; // curr uninitialized
	int thirdClass = -1;
	int fifthClass = -1;

	if ((bint == 3 && tint == 4) || (bint == 4 && tint == 3) || (bint == 3 && tint == 3)) { // root pos
		rootClass = noteMods.at(0);
		thirdClass = noteMods.at(1);
		fifthClass = noteMods.at(2);
	} else if ((bint == 4 && tint == 5) || (bint == 3 && tint == 5) || (bint == 3 && tint == 6)) { // first inv
		rootClass = noteMods.at(2);
		thirdClass = noteMods.at(0);
		fifthClass = noteMods.at(1);
	} else if ((bint == 5 && tint == 3) || (bint == 5 && tint == 4) || (bint == 6 && tint == 3)) { // sec inv
		rootClass = noteMods.at(1);
		thirdClass = noteMods.at(2);
		fifthClass = noteMods.at(0);
	}

	if (rootClass == -1) {
		return output;
	}

	for (int i = 0; i < (int)midiNotes.size(); i++) {
		if (midiNotes.at(i) % 12 == rootClass) {
			output.at(i) = 1;
		}
		else if (midiNotes.at(i) % 12 == thirdClass) {
			output.at(i) = 3;
		}
		else if (midiNotes.at(i) % 12 == fifthClass) {
			output.at(i) = 5;
		}
	}

    if (m_doubleQ) {
        keepOnlyDoubles(output); // call some function that marks only doubles
    }

	return output;
}

void Tool_colorthirds::keepOnlyDoubles(vector<int>& output) {
    map<int, int> positionCounts = {{1, 0}, {3, 0}, {5, 0}};

    for (int i = 0; i < (int)output.size(); i++) { // create hashmap of counts
        if (output[i] == 1) {
            positionCounts[1]++;
        }
        else if (output[i] == 3) {
            positionCounts[3]++;
        }
        else if (output[i] == 5) {
            positionCounts[5]++;
        }
    }

    for (auto positionCount : positionCounts) {
        if (positionCount.second == 1) { // if only appears once
            replace(output.begin(), output.end(), positionCount.first, 0); // replace with 0
        }
    }

    return;
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
		int midiNote = kernNotes.at(i)->getMidiPitch();
		if (midiNote < 0) {
			// negative values indicate sustained note.
			midiNote = -midiNote;
		}
		output.at(i) = midiNote;
	}
	return output;
}


// END_MERGE

} // end namespace hum



