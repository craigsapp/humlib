//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Sep 28 12:08:25 PDT 2020
// Last Modified: Mon May  2 21:56:16 PDT 2022
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
//                **mod-text == modern text (converts to **text with -m option)
//                **ori-text == original text (converts to **text with -o option)
//                **text-ori == original text (does not get suppressed with -m option)
//                **text-mod == modern text (does not get suppressed with -o option)
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

	define("C|no-clef|no-clefs=b", "Do not change clefs");
	define("K|no-key|no-keys=b", "Do not change key signatures");
	define("L|no-lyrics=b", "Do not change **text exclusive interpretations");
	define("M|no-mensuration|no-mensurations=b", "Do not change mensurations");
	define("R|no-references=b", "Do not change reference records keys");
	define("T|no-text=b",    "Do not change !LO:(TX|DY) layout parameters");
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
	m_nolotextQ      = getBoolean("no-text");
	m_nolyricsQ      = getBoolean("no-lyrics");
	m_norefsQ        = getBoolean("no-references");
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
	m_references.clear();
	m_lyrics.clear();
	m_lotext.clear();

	int maxtrack = infile.getMaxTrack();
	m_keys.resize(maxtrack+1);
	m_clefs.resize(maxtrack+1);
	m_mensurations.resize(maxtrack+1);
	m_references.reserve(1000);
	m_lyrics.reserve(1000);
	m_lotext.reserve(1000);

	HumRegex hre;
	int exinterpLine = -1;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isCommentLocal() || infile[i].isCommentGlobal()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				if (*token == "!") {
					continue;
				}
				if (hre.search(token, "^!!?LO:(TX|DY).*:mod=")) {
					m_lotext.push_back(token);
				} else if (hre.search(token, "^!!?LO:(TX|DY).*:ori=")) {
					m_lotext.push_back(token);
				}
				if (hre.search(token, "^!LO:MO:.*")) {
					m_lomo.push_back(token);
				}
			}
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		HumNum timeval = infile[i].getDurationFromStart();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isExclusiveInterpretation()) {
				exinterpLine = i;
				continue;
			}
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

	if (exinterpLine >= 0) {
		processExclusiveInterpretationLine(infile, exinterpLine);
	}

	storeModOriReferenceRecords(infile);

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
	printModoriOutput(infile);
}


//////////////////////////////
//
// Tool_modori::processExclusiveInterpretationLine --
//

void Tool_modori::processExclusiveInterpretationLine(HumdrumFile& infile, int line) {
	vector<HTp> staffish;
	vector<HTp> staff;
	vector<vector<HTp>> nonstaff;
	bool init = false;
	bool changed = false;

	if (!infile[line].isExclusive()) {
		return;
	}

	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isExclusiveInterpretation()) {
			continue;
		}
		if (token->isStaff()) {
			staff.push_back(token);
			nonstaff.resize(nonstaff.size() + 1);
			init = 1;
		} else {
			if (init) {
				nonstaff.back().push_back(token);
			}
		}
		if (token->isStaff()) {
			staffish.push_back(token);
		} else if (*token == "**mod-kern") {
			staffish.push_back(token);
		} else if (*token == "**mod-mens") {
			staffish.push_back(token);
		} else if (*token == "**ori-kern") {
			staffish.push_back(token);
		} else if (*token == "**ori-mens") {
			staffish.push_back(token);
		}
	}

	for (int i=0; i<(int)staff.size(); i++) {
		changed |= processStaffCompanionSpines(nonstaff[i]);
	}

	changed |= processStaffSpines(staffish);

	if (changed) {
		infile[line].createLineFromTokens();
	}
}



//////////////////////////////
//
// Tool_modori::processStaffSpines --
//

bool Tool_modori::processStaffSpines(vector<HTp>& tokens) {

	HumRegex hre;
	bool changed = false;
	for (int i=0; i<(int)tokens.size(); i++) {
		if (hre.search(tokens[i], "^\\*\\*(ori|mod)-(.*)")) {
			string newexinterp = "**" + hre.getMatch(2) + "-" + hre.getMatch(1);
			tokens[i]->setText(newexinterp);
			changed = true;
		} else if (hre.search(tokens[i], "^\\*\\*(.*?)-(ori|mod)$")) {
			string newexinterp = "**" + hre.getMatch(2) + "-" + hre.getMatch(1);
			tokens[i]->setText(newexinterp);
			changed = true;
		}
	}

	return changed;
}



//////////////////////////////
//
// Tool_modori::processStaffCompanionSpines --
//

bool Tool_modori::processStaffCompanionSpines(vector<HTp> tokens) {

	vector<HTp> mods;
	vector<HTp> oris;
	vector<HTp> other;

	for (int i=0; i<(int)tokens.size(); i++) {
		if (tokens[i]->find("**mod-") != string::npos) {
			mods.push_back(tokens[i]);
		} else if (tokens[i]->find("**ori-") != string::npos) {
			oris.push_back(tokens[i]);
		} else {
			other.push_back(tokens[i]);
		}
	}

	bool gchanged = false;

	if (mods.empty() && oris.empty()) {
		// Nothing to do.
		return false;
	}

	// mods and oris should not be mixed, so if there are no
	// other spines, then also give up:
	if (other.empty()) {
		return false;
	}


	if (m_modernQ) {
		bool changed = false;
		// Swap (**mod-XXX and **XXX) to (**XXX and **ori-XXX)

		for (int i=0; i<(int)other.size(); i++) {
			if (other[i] == NULL) {
				continue;
			}
			string target = "**mod-" + other[i]->substr(2);
			for (int j=0; j<(int)mods.size(); j++) {
				if (mods[j] == NULL) {
					continue;
				}
				if (*mods[j] != target) {
					continue;
				}
				mods[j]->setText(*other[i]);
				mods[j] = NULL;
				changed = true;
				gchanged = true;
			}
			if (changed) {
				string replacement = "**ori-" + other[i]->substr(2);
				other[i]->setText(replacement);
				other[i] = NULL;
			}
		}

	} else if (m_originalQ) {
		bool changed = false;
		// Swap (**ori-XXX and **XXX) to (**XXX and **mod-XXX)

		for (int i=0; i<(int)other.size(); i++) {
			if (other[i] == NULL) {
				continue;
			}
			string target = "**ori-" + other[i]->substr(2);
			for (int j=0; j<(int)oris.size(); j++) {
				if (oris[j] == NULL) {
					continue;
				}
				if (*oris[j] != target) {
					continue;
				}
				oris[j]->setText(*other[i]);
				oris[j] = NULL;
				changed = true;
				gchanged = true;
			}
			if (changed) {
				string replacement = "**mod-" + other[i]->substr(2);
				other[i]->setText(replacement);
				other[i] = NULL;
			}
		}
	}

	return gchanged;
}



//////////////////////////////
//
// Tool_modori::storeModOriReferenceRecors --
//

void Tool_modori::storeModOriReferenceRecords(HumdrumFile& infile) {
	m_references.clear();

	vector<HLp> refs = infile.getGlobalReferenceRecords();
	vector<string> keys(refs.size());
	for (int i=0; i<(int)refs.size(); i++) {
		string key = refs.at(i)->getReferenceKey();
		keys.at(i) = key;
	}

	vector<int> modernIndex;
	vector<int> originalIndex;

	HumRegex hre;
	for (int i=0; i<(int)keys.size(); i++) {
		if (m_modernQ || m_infoQ) {
			if (hre.search(keys[i], "-mod$")) {
				modernIndex.push_back(i);
			}
		} else if (m_originalQ || m_infoQ) {
			if (hre.search(keys[i], "-ori$")) {
				originalIndex.push_back(i);
			}
		}
	}

	if (m_modernQ || m_infoQ) {
		// Store *-mod reference records if there is a pairing:
		int pairing = -1;
		for (int i=0; i<(int)modernIndex.size(); i++) {
			int index = modernIndex[i];
			pairing = getPairedReference(index, keys);
			if (pairing >= 0) {
				m_references.push_back(make_pair(refs[index]->token(0), refs[pairing]->token(0)));
			}
		}
	}

	if (m_originalQ || m_infoQ) {
		// Store *-ori reference records if there is a pairing:
		int pairing = -1;
		string target;
		for (int i=0; i<(int)originalIndex.size(); i++) {
			int index = originalIndex[i];
			pairing = getPairedReference(index, keys);
			if (pairing >= 0) {
				target = keys[index];
				m_references.push_back(make_pair(refs[index]->token(0), refs[pairing]->token(0)));
			}
		}
	}
}



//////////////////////////////
//
// Tool_modori::getPairedReference --
//

int Tool_modori::getPairedReference(int index, vector<string>& keys) {
	string key = keys.at(index);
	string tkey = key;
	if (tkey.size() > 4) {
		tkey.resize(tkey.size() - 4);
	} else {
		return -1;
	}

	for (int i=0; i<(int)keys.size(); i++) {
		int ii = index + i;
		if (ii < (int)keys.size()) {
			if (tkey == keys.at(ii)) {
				return ii;
			}
		}
		ii = index - i;
		if (ii >= 0) {
			if (tkey == keys.at(ii)) {
				return ii;
			}
		}
	}
	return -1;
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

	if (!m_nolyricsQ) {
		bool adjust = false;
		int line = -1;
		for (int i=0; i<(int)m_lyrics.size(); i++) {
			HTp token = m_lyrics[i];
			line = token->getLineIndex();
			if (m_modernQ) {
				if (*token == "**text") {
					adjust = true;
					token->setText("**ori-text");
				} else if (*token == "**mod-text") {
					adjust = true;
					token->setText("**text");
				}
			} else {
				if (*token == "**text") {
					adjust = true;
					token->setText("**mod-text");
				} else if (*token == "**ori-text") {
					adjust = true;
					token->setText("**text");
				}
			}
		}
		if (adjust && (line >= 0)) {
			infile[line].createLineFromTokens();
		}
	}

	if (!m_nolotextQ) {
		HumRegex hre;
		for (int i=0; i<(int)m_lotext.size(); i++) {
			HTp token = m_lotext[i];
			int line = token->getLineIndex();
			if (hre.search(token, "^!!?LO:(TX|DY).*:mod=")) {
				string text = *token;
				hre.replaceDestructive(text, ":ori=", ":t=");
				hre.replaceDestructive(text, ":t=", ":mod=");
				token->setText(text);
				changed.insert(line);
			} else if (hre.search(token, "^!!?LO:(TX|DY).*:ori=")) {
				string text = *token;
				hre.replaceDestructive(text, ":mod=", ":t=");
				hre.replaceDestructive(text, ":t=", ":ori=");
				token->setText(text);
				changed.insert(line);
			}
		}
	}

	if (!m_norefsQ) {
		HumRegex hre;
		for (int i=0; i<(int)m_references.size(); i++) {
			HTp first = m_references[i].first;
			HTp second = m_references[i].second;

			if (m_modernQ) {
				if (hre.search(first, "^!!![^:]*?-mod:")) {
					string text = *first;
					hre.replaceDestructive(text, ":", "-...:");
					first->setText(text);
					infile[first->getLineIndex()].createLineFromTokens();

					text = *second;
					hre.replaceDestructive(text, "-ori:", ":");
					second->setText(text);
					infile[second->getLineIndex()].createLineFromTokens();
				}
			} else if (m_originalQ) {
				if (hre.search(first, "^!!![^:]*?-ori:")) {
					string text = *first;
					hre.replaceDestructive(text, ":", "-...:");
					first->setText(text);
					infile[first->getLineIndex()].createLineFromTokens();

					text = *second;
					hre.replaceDestructive(text, "-mod:", ":");
					second->setText(text);
					infile[second->getLineIndex()].createLineFromTokens();
				}
			}

		}
	}

	// Mensurations are only used for "original" display.  It is possible
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

	updateLoMo(infile);
}


//////////////////////////////
//
// Tool_modori::printModoriOutput --
//

void Tool_modori::printModoriOutput(HumdrumFile& infile) {
	string state;
	if (m_modernQ) {

		// convert to modern
		for (int i=0; i<infile.getLineCount(); i++) {
			if (infile[i].isCommentGlobal()) {
				HTp token = infile.token(i, 0);
				if (*token == "!!LO:MO:mod") {
				   state = "mod";
					m_humdrum_text << token << endl;
					continue;
				} else if (*token == "!!LO:MO:ori") {
				   state = "ori";
					m_humdrum_text << token << endl;
					continue;
				} else if (*token == "!!LO:MO:end") {
					state = "";
					m_humdrum_text << token << endl;
					continue;
				}
			}
			if (state == "mod") {
				// Remove global comment prefix "!! ".  Complain if not there.
				if (infile[i].compare(0, 3, "!! ") != 0) {
					cerr << "Error: line does not start with \"!! \":\t" << infile[i] << endl;
				} else {
					m_humdrum_text << infile[i].substr(3) << endl;
				}
			} else if (state == "ori") {
				// Add global comment prefix "!! ".
				m_humdrum_text << "!! " << infile[i] << endl;
			} else {
				m_humdrum_text << infile[i] << endl;
			}
		}

	} else if (m_originalQ) {

		// convert to original
		for (int i=0; i<infile.getLineCount(); i++) {
			if (infile[i].isCommentGlobal()) {
				HTp token = infile.token(i, 0);
				if (*token == "!!LO:MO:mod") {
				   state = "mod";
					m_humdrum_text << token << endl;
					continue;
				} else if (*token == "!!LO:MO:ori") {
				   state = "ori";
					m_humdrum_text << token << endl;
					continue;
				} else if (*token == "!!LO:MO:end") {
					state = "";
					m_humdrum_text << token << endl;
					continue;
				}
			}
			if (state == "ori") {
				// Remove global comment prefix "!! ".  Complain if not there.
				if (infile[i].compare(0, 3, "!! ") != 0) {
					cerr << "Error: line does not start with \"!! \":\t" << infile[i] << endl;
				} else {
					m_humdrum_text << infile[i].substr(3) << endl;
				}
			} else if (state == "mod") {
				// Add global comment prefix "!! ".
				m_humdrum_text << "!! " << infile[i] << endl;
			} else {
				m_humdrum_text << infile[i] << endl;
			}
		}

	}
}



//////////////////////////////
//
// Tool_modori::updateLoMo --
//

void Tool_modori::updateLoMo(HumdrumFile& infile) {
	for (int i=0; i<(int)m_lomo.size(); i++) {
		processLoMo(m_lomo[i]);
	}
}



//////////////////////////////
//
// Tool_modori::processLoMo --
//

void Tool_modori::processLoMo(HTp lomo) {
	HumRegex hre;

	if (m_modernQ) {
		string text = lomo->getText();
		string modtext;
		string oritext;
		string base;
		string rest;
		if (hre.search(text, "(.*):mod=([^:]*)(.*)")) {
			base = hre.getMatch(1);
			modtext = hre.getMatch(2);
			rest = hre.getMatch(3);
			hre.replaceDestructive(modtext, ":", "&colon;", "g");
			HTp current = lomo->getNextToken();
			// null parameter allows next following null token
			// to be swapped out
			bool nullQ = hre.search(text, ":null:");
			if (!nullQ) {
				while (current) {
					if (current->isNull()) {
						current = current->getNextToken();
						continue;
					}
					break;
				}
			}
			if (current) {
				string oritext = current->getText();
				hre.replaceDestructive(oritext, "&colon;", ":", "g");
				current->setText(modtext);
				string newtext = base;
				newtext += ":ori=";
				newtext += oritext;
				newtext += rest;
				lomo->setText(newtext);
				lomo->getLine()->createLineFromTokens();
				current->getLine()->createLineFromTokens();
			}
		}

	} else if (m_originalQ) {
		string text = lomo->getText();
		string modtext;
		string oritext;
		string base;
		string rest;
		if (hre.search(text, "(.*):ori=([^:]*)(.*)")) {
			base = hre.getMatch(1);
			oritext = hre.getMatch(2);
			rest = hre.getMatch(3);
			hre.replaceDestructive(oritext, ":", "&colon;", "g");
			HTp current = lomo->getNextToken();
			// null parameter allows next following null token
			// to be swapped out
			bool nullQ = hre.search(text, ":null:");
			if (nullQ) {
				while (current) {
					if (current->isNull()) {
						current = current->getNextToken();
						continue;
					}
					break;
				}
			}
			if (current) {
				string modtext = current->getText();
				hre.replaceDestructive(modtext, "&colon;", ":", "g");
				current->setText(oritext);
				string newtext = base;
				newtext += ":mod=";
				newtext += modtext;
				newtext += rest;
				lomo->setText(newtext);
				lomo->getLine()->createLineFromTokens();
				current->getLine()->createLineFromTokens();
			}
		}
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
			m_humdrum_text << "!!\t" << it->first;
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
			m_humdrum_text << "!!\t" << it->first;
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
			m_humdrum_text << "!!\t" << it->first;
			for (int j=0; j<(int)it->second.size(); j++) {
				m_humdrum_text << '\t' << it->second.at(j);
			}
			m_humdrum_text << endl;
		}
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! LYRICS:" << endl;

	for (int i=0; i<(int)m_lyrics.size(); i++) {
		HTp token = m_lyrics[i];
		m_humdrum_text << "!!\t";
		m_humdrum_text << token;
		m_humdrum_text << endl;
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! TEXT:" << endl;

	for (int i=0; i<(int)m_lotext.size(); i++) {
		m_humdrum_text << "!!\t" << m_lotext[i] << endl;
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	m_humdrum_text << "!! REFERENCES:" << endl;

	for (int i=0; i<(int)m_references.size(); i++) {
		m_humdrum_text << "!!\t" << m_references[i].first << endl;
		m_humdrum_text << "!!\t" << m_references[i].second << endl;
		m_humdrum_text << "!!\n";
	}

	m_humdrum_text << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
}


// END_MERGE

} // end namespace hum



