//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Sep 28 12:08:25 PDT 2020
// Last Modified: Thu Dec 17 23:28:58 PST 2020
// Filename:      tool-modori.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-modori.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between original and modern 
//                clefs/keysig/mensuration/time sig.
//
//                *k[] == key signature (displayed)
//                *ok[] == original key signature (implies *k[] is modern)
//                *mk[] == modern key signature (implies *k[] is original)
//
//                *clefG2 == key signature (displayed)
//                *oclefG2 == original clef (implies *clefG2 is modern)
//                *mclefG2 == modern clef (implies *clefG2 is original)
//
//                *met(C) == mensuration (displayed)
//                *omet(C) == original mensuration (time sig. is instead)
//

#include "tool-modori.h"
#include "tool-shed.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_modori::Tool_modori -- Set the recognized options for the tool.
//

Tool_modori::Tool_modori(void) {
	define("m|modern=b",    "prepare score for modern style");
	define("o|original=b", "prepare score for original style");
	define("d|info=b", "display key/clef/mensuration information");
	define("K|no-key|no-keys=b", "Do not change key signatures");
	define("C|no-clef|no-clefs=b", "Do not change clefs");
	define("M|no-mensuration|no-mensurations=b", "Do not change mensurations");
}



/////////////////////////////////
//
// Tool_modori::run -- Do the main work of the tool.
//

bool Tool_modori::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_modori::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_modori::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_modori::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_modori::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_modori::initialize(void) {
	m_infoQ = getBoolean("info");
	m_modernQ = getBoolean("modern");
	m_originalQ = getBoolean("original");
	if (m_modernQ && m_originalQ) {
		// if both options are used, ignore -m:
		m_modernQ = false;
	}
	m_nokeyQ         = getBoolean("no-key");
	m_noclefQ        = getBoolean("no-clef");
	m_nomensurationQ = getBoolean("no-mensuration");
}



//////////////////////////////
//
// Tool_modori::processFile --
//

void Tool_modori::processFile(HumdrumFile& infile) {
	m_keys.clear();
	m_clefs.clear();
	m_mensurations.clear();

	int maxtrack = infile.getMaxTrack();
	m_keys.resize(maxtrack+1);
	m_clefs.resize(maxtrack+1);
	m_mensurations.resize(maxtrack+1);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HumNum timeval = infile[i].getDurationFromStart();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (*token == "*") {
				continue;
			}
			int track = token->getTrack();
			if (token->isKeySignature()) {
				m_keys[track][timeval].push_back(token);
			} else if (token->isOriginalKeySignature()) {
				m_keys[track][timeval].push_back(token);
			} else if (token->isModernKeySignature()) {
				m_keys[track][timeval].push_back(token);
			} else if (token->isClef()) {
				m_clefs[track][timeval].push_back(token);
			} else if (token->isOriginalClef()) {
				m_clefs[track][timeval].push_back(token);
			} else if (token->isModernClef()) {
				m_clefs[track][timeval].push_back(token);
			} else if (token->isMensuration()) {
				m_mensurations[track][timeval].push_back(token);
			} else if (token->isOriginalMensuration()) {
				m_mensurations[track][timeval].push_back(token);
			}
		}
	}

	if (m_infoQ) {
		if (m_modernQ || m_originalQ) {
			m_humdrum_text << infile;
		}
		printInfo();
	}

	if (!(m_modernQ || m_originalQ)) {
		// nothing to do
		return;
	}
	
	switchModernOriginal(infile);
}


//////////////////////////////
//
// Tool_modori::switchModernOriginal --
//

void Tool_modori::switchModernOriginal(HumdrumFile& infile) {

	set<int> changed;

	if (!m_nokeyQ) {
		for (int t=1; t<(int)m_keys.size(); ++t) {
			for (auto it = m_keys.at(t).begin(); it != m_keys.at(t).end(); ++it) {
				if (it->second.size() != 2) {
					continue;
				}
				bool status = swapKeyStyle(it->second.at(0), it->second.at(1));
				if (status) {
					int line = it->second.at(0)->getLineIndex();
					changed.insert(line);
					line = it->second.at(1)->getLineIndex();
					changed.insert(line);
				}
			}
		}
	}

	if (!m_noclefQ) {
		for (int t=1; t<(int)m_clefs.size(); ++t) {
			for (auto it = m_clefs.at(t).begin(); it != m_clefs.at(t).end(); ++it) {
				if (it->second.size() != 2) {
					continue;
				}
				bool status = swapClefStyle(it->second.at(0), it->second.at(1));
				if (status) {
					int line = it->second.at(0)->getLineIndex();
					changed.insert(line);
					line = it->second.at(1)->getLineIndex();
					changed.insert(line);
				}
			}
		}
	}

	// mensurations are only used for "original" display.  It is possible
	// to use a modern metric signature (common time or cut time) but these
	// are not currently allowed.  Only one *met at a given time position
	// is allowed.

	if (!m_nomensurationQ) {
		for (int t=1; t<(int)m_mensurations.size(); ++t) {
			for (auto it = m_mensurations.at(t).begin(); it != m_mensurations.at(t).end(); ++it) {
				if (it->second.size() != 1) {
					continue;
				}
				bool status = flipMensurationStyle(it->second.at(0));
				if (status) {
					int line = it->second.at(0)->getLineIndex();
					changed.insert(line);
				}
			}
		}
	}

	for (auto it = changed.begin(); it != changed.end(); ++it) {
		int line = *it;
		infile[line].createLineFromTokens();
	}

}



//////////////////////////////
//
// Tool_modori::flipMensurationStyle -- Returns true if swapped.
//

bool Tool_modori::flipMensurationStyle(HTp token) {
	bool output = false;
	HumRegex hre;
	string text;
	if (token->isMensuration()) {
		// switch to invisible mensuration
		text = "*omet";
		text += token->substr(4);
		token->setText(text);
		output = true;
	} else if (token->isOriginalMensuration()) {
		// switch to visible mensuration
		text = "*met";
		text += token->substr(5);
		token->setText(text);
		output = true;
	}

	return output;
}



//////////////////////////////
//
// Tool_modori::swapKeyStyle -- Returns true if swapped.
//

bool Tool_modori::swapKeyStyle(HTp one, HTp two) {
	bool mtype1 = false;
	bool mtype2 = false;
	bool otype1 = false;
	bool otype2 = false;
	bool ktype1 = false;
	bool ktype2 = false;
	bool output = false;

	if (one->isKeySignature()) {
		ktype1 = true;
	} else if (one->isModernKeySignature()) {
		mtype1 = true;
	} else if (one->isOriginalKeySignature()) {
		otype1 = true;
	}

	if (two->isKeySignature()) {
		ktype2 = true;
	} else if (two->isModernKeySignature()) {
		mtype2 = true;
	} else if (two->isOriginalKeySignature()) {
		otype2 = true;
	}

	if (m_modernQ) {
		// Show the modern key signature.  If one key is *mk and the
		// other is *k then change *mk to *k and *k to *ok respectively.
		if (ktype1 && mtype2) {
			convertKeySignatureToOriginal(one);
			convertKeySignatureToRegular(two);
			output = true;
		} else if (mtype1 && ktype2) {
			convertKeySignatureToRegular(one);
			convertKeySignatureToOriginal(two);
			output = true;
		}
	} else if (m_originalQ) {
		// Show the original key.  If one key is *ok and the
		// other is *k then change *ok to *k and *k to *mk respectively.
		if (ktype1 && otype2) {
			convertKeySignatureToModern(one);
			convertKeySignatureToRegular(two);
			output = true;
		} else if (otype1 && ktype2) {
			convertKeySignatureToRegular(one);
			convertKeySignatureToModern(two);
			output = true;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_modori::swapClefStyle -- Returns true if swapped.
//

bool Tool_modori::swapClefStyle(HTp one, HTp two) {
	bool mtype1 = false;
	bool mtype2 = false;
	bool otype1 = false;
	bool otype2 = false;
	bool ktype1 = false;
	bool ktype2 = false;
	bool output = false;

	if (one->isClef()) {
		ktype1 = true;
	} else if (one->isModernClef()) {
		mtype1 = true;
	} else if (one->isOriginalClef()) {
		otype1 = true;
	}

	if (two->isClef()) {
		ktype2 = true;
	} else if (two->isModernClef()) {
		mtype2 = true;
	} else if (two->isOriginalClef()) {
		otype2 = true;
	}

	if (m_modernQ) {
		// Show the modern key signature.  If one key is *mk and the
		// other is *k then change *mk to *k and *k to *ok respectively.
		if (ktype1 && mtype2) {
			convertClefToOriginal(one);
			convertClefToRegular(two);
			output = true;
		} else if (mtype1 && ktype2) {
			convertClefToRegular(one);
			convertClefToOriginal(two);
			output = true;
		}
	} else if (m_originalQ) {
		// Show the original key.  If one key is *ok and the
		// other is *k then change *ok to *k and *k to *mk respectively.
		if (ktype1 && otype2) {
			convertClefToModern(one);
			convertClefToRegular(two);
			output = true;
		} else if (otype1 && ktype2) {
			convertClefToRegular(one);
			convertClefToModern(two);
			output = true;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_modori::convertKeySignatureToModern --
//

void Tool_modori::convertKeySignatureToModern(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?k(.*)")) {
		string text = "*mk";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



//////////////////////////////
//
// Tool_modori::convertKeySignatureToOriginal --
//

void Tool_modori::convertKeySignatureToOriginal(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?k(.*)")) {
		string text = "*ok";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



//////////////////////////////
//
// Tool_modori::convertKeySignatureToRegular --
//

void Tool_modori::convertKeySignatureToRegular(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?k(.*)")) {
		string text = "*k";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



//////////////////////////////
//
// Tool_modori::convertClefToModern --
//

void Tool_modori::convertClefToModern(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?clef(.*)")) {
		string text = "*mclef";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



//////////////////////////////
//
// Tool_modori::convertClefToOriginal --
//

void Tool_modori::convertClefToOriginal(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?clef(.*)")) {
		string text = "*oclef";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



//////////////////////////////
//
// Tool_modori::convertClefToRegular --
//

void Tool_modori::convertClefToRegular(HTp token) {
	HumRegex hre;
	if (hre.search(token, "^\\*[mo]?clef(.*)")) {
		string text = "*clef";
		text += hre.getMatch(1);
		token->setText(text);
	}
}



////////////////////
//
// Tool_modori::printInfo --
//

void Tool_modori::printInfo(void) {
	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! KEYS:" << endl;

	for (int t=1; t<(int)m_keys.size(); ++t) {
		for (auto it = m_keys.at(t).begin(); it != m_keys.at(t).end(); ++it) {
			m_humdrum_text << "!!    " << it->first;
			for (int j=0; j<(int)it->second.size(); ++j) {
				m_humdrum_text << '\t' << it->second.at(j);
		}
			m_humdrum_text << endl;
		}
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! CLEFS:" << endl;

	for (int t=1; t<(int)m_keys.size(); ++t) {
		for (auto it = m_clefs.at(t).begin(); it != m_clefs.at(t).end(); ++it) {
			m_humdrum_text << "!!    " << it->first;
			for (int j=0; j<(int)it->second.size(); ++j) {
				m_humdrum_text << '\t' << it->second.at(j);
			}
			m_humdrum_text << endl;
		}
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! MENSURATIONS:" << endl;

	for (int t=1; t<(int)m_mensurations.size(); ++t) {
		for (auto it = m_mensurations.at(t).begin(); it != m_mensurations.at(t).end(); ++it) {
			m_humdrum_text << "!!    " << it->first;
			for (int j=0; j<(int)it->second.size(); j++) {
				m_humdrum_text << '\t' << it->second.at(j);
			}
			m_humdrum_text << endl;
		}
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

}


// END_MERGE

} // end namespace hum



