//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Fri Mar  5 10:27:09 PST 2021
// Filename:      tool-composite.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-composite.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Generate composite rhythm spine for music.
//

#include "tool-composite.h"
#include "tool-extract.h"
#include "tool-autobeam.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

// Note state variables for grouping:
#define TYPE_UNDEFINED           9 /* for inactive groupings */
#define TYPE_NoteSustainAttack   3
#define TYPE_NoteAttack          2
#define TYPE_RestAttack          1
#define TYPE_NONE                0
#define TYPE_RestSustain        -1
#define TYPE_NoteSustain        -2
#define TYPE_NoteSustainSustain -3

#define COMPOSITE_TREMOLO_MARKER "||"

/////////////////////////////////
//
// Tool_composite::Tool_composite -- Set the recognized options for the tool.
//

Tool_composite::Tool_composite(void) {
	define("a|append=b",    "append data to end of line (top of system)");
	define("g|grace=b",     "include grace notes in composite rhythm");
	define("x|extract=b",   "only output composite rhythm spines");
	define("B|no-beam=b",   "do not apply automatic beaming");
	define("G|no-groups=b", "do not split composite rhythm into separate streams by group markers");
	define("pitch=s:eR",    "pitch to display for composite rhythm");
	define("debug=b",       "print debugging information");
}



/////////////////////////////////
//
// Tool_composite::run -- Do the main work of the tool.
//

bool Tool_composite::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_composite::run(const string& indata, ostream& out) {
	HumdrumFile infile;
	infile.readStringNoRhythm(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_composite::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	infile.createLinesFromTokens();
	// need to convert to text for now:
	m_humdrum_text << infile;
	return true;
}



//////////////////////////////
//
// Tool_composite::initialize --
//

void Tool_composite::initialize(void) {
	m_pitch     = getString("pitch");
	m_extractQ  = getBoolean("extract");
	m_nogroupsQ = getBoolean("no-groups");
	m_graceQ    = getBoolean("grace");
	m_appendQ   = getBoolean("append");
	m_debugQ    = getBoolean("debug");
	if (m_extractQ) {
		m_appendQ = false;
	}
}



//////////////////////////////
//
// Tool_composite::processFile --
//

void Tool_composite::processFile(HumdrumFile& infile) {
	reduceTremolos(infile);
	bool autogroup = false;
	if (!m_nogroupsQ) {
		autogroup = hasGroupInterpretations(infile);
	}
	if (autogroup) {
		prepareMultipleGroups(infile);
	} else {
		prepareSingleGroup(infile);
	}
}



//////////////////////////////
//
// Tool_composite::hasGroupInterpretations --
//

bool Tool_composite::hasGroupInterpretations(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 5, "*grp:") == 0) {
				return true;
			}
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_composite::prepareMultipleGroups --
//

void Tool_composite::prepareMultipleGroups(HumdrumFile& infile) {
	Tool_extract extract;

	// add two columns, one for each rhythm stream:
	if (!m_appendQ) {
		extract.setModified("s", "0,0,1-$");
	} else {
		extract.setModified("s", "1-$,0,0");
	}

	vector<HumNum> durations(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		durations[i] = infile[i].getDuration();
	}

	vector<bool> isRest(infile.getLineCount(), false);
	vector<bool> isNull(infile.getLineCount(), false);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		bool allnull = true;
		bool allrest = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			string value = tok->getValue("auto", "ignoreTremoloNote");
			if (value == "1") {
				continue;
			}
			if (tok->isNull()) {
				continue;
			}
			allnull = false;
			if (!tok->isKern()) {
				continue;
			}
			if (tok->isNote()) {
				allrest = false;
				break;
			}
			if (tok->isRest()) {
				allnull = false;
			}
		}
		if (allrest) {
			isRest[i] = true;
		}
		if (allnull) {
			isNull[i] = true;
		}
	}

	string pstring = getString("pitch");

	HumRegex hre;

	bool wroteA = false;
	bool wroteB = false;

	stringstream sstream;
	sstream << infile;
	HumdrumFile originalfile;
	originalfile.readString(sstream.str());
	extract.run(originalfile);
	infile.readString(extract.getAllText());
	// need to redo tremolo analyses...
	reduceTremolos(infile);

	assignGroups(infile);
	analyzeLineGroups(infile);
	if (m_debugQ) {
		printGroupAssignments(infile);
	}
	vector<vector<int>> groupstates;
	getGroupStates(groupstates, infile);
	vector<vector<HumNum>> groupdurs;
	getGroupDurations(groupdurs, groupstates, infile);
	vector<vector<string>> rhythms;
	getGroupRhythms(rhythms, groupdurs, groupstates, infile);

	HTp token = NULL;
	HTp token2 = NULL;
	for (int i=0; i<infile.getLineCount(); i++) {
		token = NULL;
		token2 = NULL;
		if (infile[i].isInterpretation()) {
			if (m_appendQ) {
				token = infile.token(i, infile[i].getFieldCount() - 2);
				token2 = infile.token(i, infile[i].getFieldCount() - 1);
			} else {
				token = infile.token(i, 0);
				token2 = infile.token(i, 1);
			}
			if (token && token->compare("**blank") == 0) {
				token->setText("**kern");
			}
			if (token2 && token2->compare("**blank") == 0) {
				token2->setText("**kern");
			}
			// continue;

			// copy time signature and tempos
			for (int j=2; j<infile[i].getFieldCount(); j++) {
				HTp stok = infile.token(i, j);
				if (stok->isTempo()) {
					token->setText(*stok);
					token2->setText(*stok);
				} else if (stok->isTimeSignature()) {
					token->setText(*stok);
					token2->setText(*stok);
				} else if (stok->isMensurationSymbol()) {
					token->setText(*stok);
					token2->setText(*stok);
				} else if (stok->isKeySignature()) {
					// Don't transfer key signature, but maybe add as an option.
					// token->setText(*stok);
					// token2->setText(*stok);
				} else if (stok->isClef()) {
					token->setText("*clefX");
					token2->setText("*clefX");
				} else if (stok->compare(0, 5, "*grp:") == 0) {
					if (!wroteA) {
						token->setText("*grp:A");
						wroteA = true;
					}
					if (!wroteB) {
						token2->setText("*grp:B");
						wroteB = true;
					}
				}
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!m_graceQ) {
				continue;
			}
			// otherwise, borrow the view of the first grace note found on the line
			// (beaming, visual duration) and apply the target pitch to the grace note.
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				if (tok->isNull()) {
					continue;
				}
				if (tok->isGrace()) {
					string q;
					string beam;
					string recip;
					if (hre.search(tok, "(\\d+%?\\d*\\.*)")) {
						recip = hre.getMatch(1);
					}
					if (hre.search(tok, "([LJk]+)")) {
						beam = hre.getMatch(1);
					}
					if (hre.search(tok, "(q+)")) {
						q = hre.getMatch(1);
					}
					string full;
					full += recip;
					full += q;
					full += pstring;
					full += beam;
					HTp targettok = NULL;
					HTp targettok2 = NULL;
					if (m_appendQ) {
						targettok = infile.token(i, infile[i].getFieldCount()-2);
						targettok2 = infile.token(i, infile[i].getFieldCount()-1);
					} else {
						targettok = infile.token(i, 0);
						targettok2 = infile.token(i, 1);
					}

					string group = infile.token(i, j)->getValue("auto", "group");
					if (group == "A") {
						targettok->setText(full);
					} else if (group == "B") {
						targettok2->setText(full);
					}
					break;
				}
			}
			continue;
		}

		HumNum duration = getLineDuration(infile, i, isNull);
		string recip = rhythms[0][i];
		string recip2 = rhythms[1][i];
		if (recip.empty()) {
			recip = ".";
		} else {
			if (groupstates[0][i] == TYPE_RestAttack) {
				recip += "rRb";
			}
			else if (groupstates[0][i] == TYPE_UNDEFINED) {
				// make invisible rest (rest not part of group)
				recip += "ryy";
			} else {
				recip += m_pitch;
			}
		}
		if (recip2.empty()) {
			// null group: add invisible rest to rhythm
			// HumNum linedur = infile[i].getDuration();
			// recip2 = Convert::durationToRecip(linedur);
			recip2 += ".";
		} else {
			if (groupstates[1][i] == TYPE_RestAttack) {
				recip2 += "rRb";
			}
			else if (groupstates[1][i] == TYPE_UNDEFINED) {
				// make invisible rest (rest not part of group)
				recip2 += "ryy";
			} else {
				recip2 += m_pitch;
			}
		}

		HTp token2 = NULL;
		if (m_appendQ) {
			token = infile.token(i, infile[i].getFieldCount()-2);
			token2 = infile.token(i, infile[i].getFieldCount()-1);
		} else {
			token = infile.token(i, 0);
			token2 = infile.token(i, 1);
		}

		token->setText(recip);
		token2->setText(recip2);
	}

	if (m_extractQ) {
		Tool_extract extract2;
		extract2.setModified("s", "1-2");
		extract2.run(infile);
		infile.readString(extract2.getAllText());
	}

	if (m_appendQ) {
		addLabels(infile, -2);
	} else {
		addLabels(infile, +2);
	}

	if (!getBoolean("no-beam")) {
		Tool_autobeam autobeam;

		if (m_appendQ) {
			int trackcount =  infile.getTrackCount();
			string tstring = to_string(trackcount-1);
			tstring += ",";
			tstring += to_string(trackcount);
			autobeam.setModified("t", tstring);
		} else {
			autobeam.setModified("t", "1,2");
		}

		if (m_appendQ) {
			int trackcount =  infile.getTrackCount();
			string tstring = to_string(trackcount);
			autobeam.setModified("t", tstring);
		} else {
			autobeam.setModified("t", "1");
		}

		// need to analyze structure for some reason:
//		infile.analyzeStrands();
//		autobeam.run(infile);

	}
}


//////////////////////////////
//
// Tool_compare::getGrouprhythms --
//

void Tool_composite::getGroupRhythms(vector<vector<string>>& rhythms,
		vector<vector<HumNum>>& groupdurs, vector<vector<int>>& groupstates,
		HumdrumFile& infile) {
	rhythms.resize(groupdurs.size());
	for (int i=0; i<(int)rhythms.size(); i++) {
		getGroupRhythms(rhythms[i], groupdurs[i], groupstates[i], infile);
	}
}


void Tool_composite::getGroupRhythms(vector<string>& rhythms, vector<HumNum>& durs,
		vector<int>& states, HumdrumFile& infile) {
	rhythms.clear();
	rhythms.resize(durs.size());
	int lastnotei = -1;
	for (int i=0; i<(int)rhythms.size(); i++) {
		if (states[i] <= 0) {
			continue;
		}
		string prefix = "";
		string postfix = "";
		for (int j=i+1; j<(int)rhythms.size(); j++) {
			if ((states[j]) > 0 && (states[j] < 5)) {
				if ((states[i] == TYPE_NoteAttack) && (states[j] == TYPE_NoteSustainAttack)) {
					prefix = "[";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_NoteSustainAttack)) {
					postfix = "_";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_NoteAttack)) {
					postfix = "]";
				} else if ((states[i] == TYPE_NoteSustainAttack) && (states[j] == TYPE_RestAttack)) {
					postfix = "]";
				}
				lastnotei = j;
				break;
			}
		}
		string value = Convert::durationToRecip(durs[i]);
		rhythms[i] = prefix + value + postfix;
	}
	if (lastnotei >= 0) {
		if (states[lastnotei] == TYPE_NoteSustainAttack) {
			rhythms[lastnotei] = rhythms[lastnotei] + "]";
		}
	}

	if (m_debugQ) {
		cerr << "=========================================" << endl;
		cerr << "RECIP FOR GROUP: " << endl;
		for (int i=0; i<(int)rhythms.size(); i++) {
			cerr << rhythms[i] << "\t" << durs[i] << "\t" << states[i] << "\t" << infile[i] << endl;
		}
		cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
	}

}



//////////////////////////////
//
// Tool_composite::getGroupDurations --
//

void Tool_composite::getGroupDurations(vector<vector<HumNum>>& groupdurs,
		vector<vector<int>>& groupstates, HumdrumFile& infile) {
	groupdurs.resize(groupstates.size());
	for (int i=0; i<(int)groupstates.size(); i++) {
		getGroupDurations(groupdurs[i], groupstates[i], infile);
	}
}

void Tool_composite::getGroupDurations(vector<HumNum>& groupdurs,
		vector<int>& groupstates, HumdrumFile& infile) {
	HumNum enddur = infile.getScoreDuration();
	groupdurs.resize(groupstates.size());
	fill(groupdurs.begin(), groupdurs.end(), -1);
	int eventi = -1;
	HumNum lasttime = 0;
	for (int i=0; i<(int)groupdurs.size(); i++) {
		if (groupstates[i] > 0) {
			if (eventi >= 0) {
				HumNum eventtime = infile[i].getDurationFromStart();
				HumNum duration = eventtime - lasttime;
				groupdurs[eventi] = duration;
				lasttime = eventtime;
				eventi = i;
				continue;
			} else {
				eventi = i;
			}
		}
	}
	if (eventi >= 0) {
		HumNum duration = enddur - lasttime;
		groupdurs[eventi] = duration;
	}
}



/////////////////////////////
//
// Tool_composite::getGroupStates -- Pull out the group note states for each
//    composite rhytm stream.
//
//    group:A:type = "note"   if there is at least one note attack in group A on the line.
//    group:A:type = "ncont"  if there is no attack but at least one note sustain in group A.
//    group:A:type = "snote"  there is a printed note which is part of a tie group sustain note.
//    group:A:type = "scont"  continuation of a tie group sutain note.
//    group:A:type = "rest"   if there is no note attack or sustain but there is a rest start.
//    group:A:type = "rcont"  if there is a rest continuing in group A on the line.
//    group:A:type = "none"   if there is no activity for group A on the line.
//
//    Numeric equivalents:
//     9 = TYPE_UNDEFINED           = "undefined"
//     3 = TYPE_NoteSustainAttack   = "snote"
//     2 = TYPE_NoteAttack          = "note"
//     1 = TYPE_RestAttack          = "rest"
//     0 = TYPE_NONE                = "none"
//    -1 = TYPE_RestSustain         = "rcont"
//    -2 = TYPE_NoteSustain         = "ncont"
//    -3 = TYPE_NoteSustainSustain  = "scont"
//

void Tool_composite::getGroupStates(vector<vector<int>>& groupstates, HumdrumFile& infile) {
	groupstates.resize(2);
	groupstates[0].resize(infile.getLineCount());
	groupstates[1].resize(infile.getLineCount());
	fill(groupstates[0].begin(), groupstates[0].end(), 0);
	fill(groupstates[1].begin(), groupstates[1].end(), 0);

	for (int i=0; i<infile.getLineCount(); i++) {
		for (int j=0; j<(int)groupstates.size(); j++) {
			char groupname = 'A' + j;
			string name;
			name.clear();
			name += groupname;
			string state = infile[i].getValue("group", name, "type");
			int typenum = typeStringToInt(state);
			groupstates[j][i] = typenum;
		}
	}
}



//////////////////////////////
//
// Tool_composite::typeStringToInt -- Convert between numeric and string state forms.
//

int Tool_composite::typeStringToInt(const string& value) {
	if (value == "snote") { return TYPE_NoteSustainAttack;  }
	if (value == "note")  { return TYPE_NoteAttack;         }
	if (value == "rest")  { return TYPE_RestAttack;         }
	if (value == "none")  { return TYPE_NONE;               }
	if (value == "rcont") { return TYPE_RestSustain;        }
	if (value == "ncont") { return TYPE_NoteSustain;        }
	if (value == "scont") { return TYPE_NoteSustainSustain; }
	return TYPE_UNDEFINED;
}



//////////////////////////////
//
// Tool_composite::prepareSingleGroup --
//

void Tool_composite::prepareSingleGroup(HumdrumFile& infile) {
	Tool_extract extract;

	if (m_appendQ) {
		extract.setModified("s", "1-$,0");
	} else {
		extract.setModified("s", "0,1-$");
	}

	vector<HumNum> durations(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		durations[i] = infile[i].getDuration();
	}

	vector<bool> isRest(infile.getLineCount(), false);
	vector<bool> isNull(infile.getLineCount(), false);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (durations[i] == 0) {
			continue;
		}
		bool allnull = true;
		bool allrest = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			if (tok->isNull()) {
				continue;
			}
			allnull = false;
			if (!tok->isKern()) {
				continue;
			}
			if (tok->isNote()) {
				allrest = false;
				break;
			}
			if (tok->isRest()) {
				allnull = false;
			}
		}
		if (allrest) {
			isRest[i] = true;
		}
		if (allnull) {
			isNull[i] = true;
		}
	}

	string pstring = getString("pitch");

	HumRegex hre;

	extract.run(infile);
	infile.readString(extract.getAllText());
	// need to redo tremolo analyses...
	reduceTremolos(infile);

	HTp token;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			if (m_appendQ) {
				token = infile.token(i, infile[i].getFieldCount() - 1);
			} else {
				token = infile.token(i, 0);
			}
			if (token->compare("**blank") == 0) {
				token->setText("**kern");
				continue;
			}
			// copy time signature and tempos
			for (int j=1; j<infile[i].getFieldCount(); j++) {
				HTp stok = infile.token(i, j);
				if (stok->isTempo()) {
					token->setText(*stok);
				} else if (stok->isTimeSignature()) {
					token->setText(*stok);
				} else if (stok->isMensurationSymbol()) {
					token->setText(*stok);
				} else if (stok->isKeySignature()) {
					// token->setText(*stok);
				} else if (stok->isClef()) {
					token->setText("*clefX");
				} else if (stok->isKeyDesignation()) {
					// token->setText(*stok);
				} else if (stok->compare(0, 5, "*grp:") == 0) {
					// Don't transfer any group tags to the composite rhythm.
					token->setText("*");
				}
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!m_graceQ) {
				continue;
			}
			// otherwise, borrow the view of the first grace note found on the line
			// (beaming, visual duration) and apply the target pitch to the grace note.
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				if (tok->isNull()) {
					continue;
				}
				if (tok->isGrace()) {
					string q;
					string beam;
					string recip;
					if (hre.search(tok, "(\\d+%?\\d*\\.*)")) {
						recip = hre.getMatch(1);
					}
					if (hre.search(tok, "([LJk]+)")) {
						beam = hre.getMatch(1);
					}
					if (hre.search(tok, "(q+)")) {
						q = hre.getMatch(1);
					}
					string full;
					full += recip;
					full += q;
					full += pstring;
					full += beam;
					HTp targettok = infile.token(i, 0);
					if (m_appendQ) {
						targettok = infile.token(i, infile[i].getFieldCount() - 1);
					}
					targettok->setText(full);
					break;
				}
			}
			continue;
		}
		HumNum duration = getLineDuration(infile, i, isNull);
		string recip;
		if (isNull[i]) {
			recip = ".";
		} else {
			recip = Convert::durationToRecip(duration);
		}

		if (onlyAuxTremoloNotes(infile, i)) {
			// mark auxiliary notes so that they can be merged
			// with a preceding note later.
			recip += COMPOSITE_TREMOLO_MARKER;
		}

		if (m_appendQ) {
			token = infile.token(i, infile[i].getFieldCount() - 1);
		} else {
			token = infile.token(i, 0);
		}
		if (isRest[i]) {
			if (!isNull[i]) {
				recip += "r";
			}
		} else {
			recip += pstring;
		}
		token->setText(recip);
	}

	if (m_extractQ) {
		Tool_extract extract2;
		extract2.setModified("s", "1");
		extract2.run(infile);
		infile.readString(extract2.getAllText());
	}

	if (!getBoolean("no-beam")) {
		Tool_autobeam autobeam;

		if (m_appendQ) {
			int trackcount =  infile.getTrackCount();
			string tstring = to_string(trackcount-1);
			tstring += ",";
			tstring += to_string(trackcount);
			autobeam.setModified("t", tstring);
		} else {
			autobeam.setModified("t", "1,2");
		}

		if (m_appendQ) {
			int trackcount =  infile.getTrackCount();
			string tstring = to_string(trackcount);
			autobeam.setModified("t", tstring);
		} else {
			autobeam.setModified("t", "1");
		}

		// need to analyze structure for some reason:
		//	 infile.analyzeStrands();
		//	 autobeam.run(infile);
	}

	removeAuxTremolosFromCompositeRhythm(infile);
}



//////////////////////////////
//
// Tool_composite::removeAuxTremolosFromCompositeRhythm --
//

void Tool_composite::removeAuxTremolosFromCompositeRhythm(HumdrumFile& infile) {
	vector<HTp> starts = infile.getKernSpineStartList();
	vector<HTp> stops;
	infile.getSpineStopList(stops);

	if (stops.empty()) {
		return;
	}
	HTp current = NULL;
	if (m_appendQ) {
		current = stops.back();
	} else {
		current = stops[0];
	}
	if (current == NULL) {
		return;
	}
	current = current->getPreviousToken();
	HumNum accumulator = 0;
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (*current == ".") {
			current = current->getPreviousToken();
			continue;
		}
		if (current->find(COMPOSITE_TREMOLO_MARKER) != string::npos) {
			string text = current->getText();
			accumulator += Convert::recipToDuration(text);
			current->setText(".");
		} else if (accumulator > 0) {
			string text = current->getText();
			HumNum totaldur = Convert::recipToDuration(text);
			totaldur += accumulator;
			accumulator = 0;
			string newrhy = Convert::durationToRecip(totaldur);
			HumRegex hre;
			hre.replaceDestructive(text, newrhy, "\\d+%?\\d*\\.*");
			current->setText(text);
		}
		current = current->getPreviousToken();
	}
}



//////////////////////////////
//
// Tool_composite::onlyAuxTremoloNotes -- True if note attacks on line are only for
//     auxiliary tremolo notes.
//

bool Tool_composite::onlyAuxTremoloNotes(HumdrumFile& infile, int line) {
	int attackcount = 0;
	int sustaincount = 0;
	int auxcount = 0;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		if (token->isRest()) {
			continue;
		}
		bool attack = token->isNoteAttack();
		if (!attack) {
			sustaincount++;
			continue;
		}
		attackcount++;
		string value = token->getValue("auto", "ignoreTremoloNote");
		if (value == "1") {
			auxcount++;
		}
	}

	if ((auxcount > 0) && (auxcount == attackcount)) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Tool_composite::analyzeLineGroups -- Look at each line for Group A and B and determine if
//    And one of five activity types are possible for the line:
//        group:A:type = "note"   if there is at least one note attack in group A on the line.
//        group:A:type = "ncont"  if there is no attack but at least one note sustain in group A.
//        group:A:type = "rest"   if there is no note attack or sustain but there is a rest start.
//        group:A:type = "rcont"  if there is a rest continuing in group A on the line.
//        group:A:type = "empty"  if there is no activity for group A on the line.
//

void Tool_composite::analyzeLineGroups(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].data()) {
			continue;
		}
		analyzeLineGroup(infile, i, "A");
		analyzeLineGroup(infile, i, "B");
	}
}



//////////////////////////////
//
// Tool_composite::analyzeLineGroup --
//
//     9 = TYPE_UNDEFINED           = "undefined"
//     3 = TYPE_NoteSustainAttack   = "snote"
//     2 = TYPE_NoteAttack          = "note"
//     1 = TYPE_RestAttack          = "rest"
//     0 = TYPE_NONE                = "none"
//    -1 = TYPE_RestSustain         = "rcont"
//    -2 = TYPE_NoteSustain         = "ncont"
//    -3 = TYPE_NoteSustainSustain  = "scont"
//

void Tool_composite::analyzeLineGroup(HumdrumFile& infile, int line, const string& target) {
	int groupstate = getGroupNoteType(infile, line, target);
	switch (groupstate) {
		case TYPE_NoteSustainAttack:
			infile[line].setValue("group", target, "type", "snote");
			break;
		case TYPE_NoteAttack:
			infile[line].setValue("group", target, "type", "note");
			break;
		case TYPE_RestAttack:
			infile[line].setValue("group", target, "type", "rest");
			break;
		case TYPE_RestSustain:
			infile[line].setValue("group", target, "type", "rcont");
			break;
		case TYPE_NoteSustain:
			infile[line].setValue("group", target, "type", "ncont");
			break;
		case TYPE_NoteSustainSustain:
			infile[line].setValue("group", target, "type", "scont");
			break;
		case TYPE_NONE:
			infile[line].setValue("group", target, "type", "none");
			break;
		default:
			infile[line].setValue("group", target, "type", "undefined");
			break;
	}
}



//////////////////////////////
//
// Tool_composite::getGroupNoteType --
//
//  9 = TYPE_UNDEFINED
//  3 = TYPE_NoteSustainAttack
//  2 = TYPE_NoteAttack
//  1 = TYPE_RestAttack
//  0 = TYPE_NONE
// -1 = TYPE_RestSustain
// -2 = TYPE_NoteSustain
// -3 = TYPE_NoteSustainSustain
//

int Tool_composite::getGroupNoteType(HumdrumFile& infile, int line, const string& group) {
	if (!infile[line].isData()) {
		return TYPE_NONE;
	}

	vector<HTp> grouptokens;
	for (int i=0; i<infile[line].getFieldCount(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		string tgroup = token->getValue("auto", "group");
		if (group == tgroup) {
			grouptokens.push_back(token);
		}
	}

	if (grouptokens.empty()) {
		return TYPE_UNDEFINED;
	}

	bool hasRestAttack    = false;
	bool hasRestSustain   = false;
	bool hasNoteAttack    = false;
	bool hasNoteSustain   = false;
	bool hasNoteSAttack   = false;
	bool hasNoteSSustain  = false;

	for (int i=0; i<(int)grouptokens.size(); i++) {
		HTp token = grouptokens[i];
		string value = token->getValue("auto", "ignoreTremoloNote");
		if (value == "1") {
			hasNoteSustain = true;
			// need to check for tie on head note...
			continue;
		}
		if (token->isNull()) {
			HTp resolved = token->resolveNull();
			if (resolved && !resolved->isNull()) {
				if (resolved->isRest()) {
					hasRestSustain = true;
				} else {
					if (resolved->isNoteAttack()) {
						hasNoteSustain = true;
					} else if (resolved->isNoteSustain()) {
						hasNoteSSustain = true;
					}
				}
			}
			continue;
		}
		if (token->isRest()) {
			hasRestAttack = true;
			continue;
		}
		if (token->isNoteAttack()) {
			string value = token->getValue("auto", "ignoreTremoloNote");
			if (value != "1") {
				hasNoteAttack = true;
			}
			continue;
		}
		if (token->isNoteSustain()) {
			hasNoteSAttack = true;
		}
	}

	//  3 = TYPE_NoteSustainAttack
	//  2 = TYPE_NoteAttack
	//  1 = TYPE_RestAttack
	//  0 = TYPE_NONE
	// -1 = TYPE_RestSustain
	// -2 = TYPE_NoteSustain
	// -3 = TYPE_NoteSustainSustain

	if (hasNoteAttack) {
		return TYPE_NoteAttack;
	}
	if (hasNoteSAttack) {
		return TYPE_NoteSustainAttack;
	}
	if (hasNoteSustain) {
		return TYPE_NoteSustain;
	}
	if (hasNoteSSustain) {
		return TYPE_NoteSustainSustain;
	}
	if (hasRestAttack) {
		return TYPE_RestAttack;
	}
	if (hasRestSustain) {
		return TYPE_RestSustain;
	}

	cerr << "Warning: no category for line " << infile[line] << endl;

	return 0;
}


//////////////////////////////
//
// Tool_composite::getLineDuration -- Return the duration of the line, but return
//    0 if the line only contains nulls.  Also add the duration of any subsequent
//    lines that are null lines before any data content lines.

HumNum Tool_composite::getLineDuration(HumdrumFile& infile, int index, vector<bool>& isNull) {
	if (isNull[index]) {
		return 0;
	}
	if (!infile[index].isData()) {
		return 0;
	}
	HumNum output = infile[index].getDuration();
	for (int i=index+1; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (isNull[i]) {
			output += infile[i].getDuration();
		} else {
			break;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_composite::assignGroups -- Add a parameter auto:grouping = "A" or "B" depending on the
//    group.  This can be generalized later to more letters, or arbitrary strings perhaps.
//    This comes from an interpretation such as *grp:A or *grp:B in the data.  If *grp: is found
//    without a letter, than that group will be null group.
//

void Tool_composite::assignGroups(HumdrumFile& infile) {

	int maxtrack = infile.getMaxTrack();
	vector<vector<string>> current;
	current.resize(maxtrack);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		int lasttrack = 1;
		int subtrack = 1;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (track != lasttrack) {
				subtrack = 1;
			} else {
				subtrack++;
			}
			if (subtrack > (int)current.at(track-1).size()) {
				current.at(track-1).resize(subtrack);
			}
			if (*token == "*grp:A") {
  				current.at(track-1).at(subtrack-1) = "A";
			}
			if (*token == "*grp:B") {
  				current.at(track-1).at(subtrack-1) = "B";
			}
			if (*token == "*grp:") {
				// clear a group:
  				current.at(track-1).at(subtrack-1) = "";
			}
         string group = getGroup(current, track-1, subtrack-1);
			token->setValue("auto", "group", group);
// cerr << "$$$ SETTING TOKEN " << token << " GROUP TO " << token->getValue("auto", "group") << endl;
		}
	}
}



//////////////////////////////
//
// Tool_composite::getGroup --
//

string Tool_composite::getGroup(vector<vector<string>>& current, int spine, int subspine) {
	if (spine >= (int)current.size()) {
		return "";
	}
	if (subspine >= (int)current.at(spine).size()) {
		return "";
	}
	string output = current[spine][subspine];
	if (!output.empty()) {
		return output;
	}
	// Grouping is empty for the subspine, so walk backwards through the
	// subspine array to find any non-empty grouping, which will be assumed
	// to apply to the current subspine.
	for (int i=subspine-1; i>=0; --i) {
		if (!current[spine][i].empty()) {
			return current[spine][i];
		}
	}

	return "";
}



//////////////////////////////
//
// Tool_composite::printGroupAssignments -- for debugging of group assignments.
//

void Tool_composite::printGroupAssignments(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			cerr << infile[i] << endl;
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			string value = token->getValue("auto", "group");
			cerr << token;
			if (!value.empty()) {
				cerr << "{" << value << "}";
			}
			if (j < infile[i].getFieldCount() - 1) {
				cerr << "\t";
			}
		}
		cerr << endl;
	}
}



//////////////////////////////
//
// Tool_composite::reduceTremolos --  Does not do split parallel tremolo states.
//

void Tool_composite::reduceTremolos(HumdrumFile& infile) {
	int maxtrack = infile.getMaxTrack();
	vector<bool> tstates(maxtrack + 1, false);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isInterpretation()) {
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp token = infile.token(i, j);
				int track = token->getTrack();
				if (*token == "*tremolo") {
					tstates[track] = true;
				} else if (*token == "*Xtremolo") {
					tstates[track] = false;
				}
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			int track = token->getTrack();
			if (!tstates[track]) {
				continue;
			}
			if (token->find("L") != string::npos) {
				checkForTremoloReduction(infile, i, j);
			}
		}
	}
}



//////////////////////////////
//
// Tool_composite::checkForTremoloReduction -- 
//

void Tool_composite::checkForTremoloReduction(HumdrumFile& infile, int line, int field) {
	HTp token = infile.token(line, field);
	vector<HTp> notes;
	getBeamedNotes(notes, token);

	if (notes.empty()) {
		return;
	}
	if (notes.size() == 1) {
		return;
	}

	vector<HumNum> durations(notes.size(), 0);
	vector<vector<int>> pitches(notes.size());
	for (int i=0; i<(int)notes.size(); i++) {
		durations[i] = notes[i]->getDuration();
		getPitches(pitches[i], notes[i]);

	}

	vector<int> tgroup(notes.size(), 0);
	int curgroup = 0;
	for (int i=1; i<(int)notes.size(); i++) {
		if (durations[i] != durations[i-1]) {
			tgroup[i] = ++curgroup;
			continue;
		}
		if (!pitchesEqual(pitches[i], pitches[i-1])) {
			tgroup[i] = ++curgroup;
			continue;
		}
		tgroup[i] = curgroup;
	}

	int groupcount = tgroup.back() + 1;
	for (int i=0; i<groupcount; i++) {
		mergeTremoloGroup(notes, tgroup, i);
	}
}



//////////////////////////////
//
// Tool_composite::mergeTremoloGroup --
//

void Tool_composite::mergeTremoloGroup(vector<HTp>& notes, vector<int> groups, int group) {
	vector<int> tindex;
	for (int i=0; i<(int)notes.size(); i++) {
		if (groups[i] == group) {
			tindex.push_back(i);
		}
	}
	if (tindex.empty()) {
		return;
	}
	if (tindex.size() == 1) {
		return;
	}
	// Consider complicated groups that should not be grouped into a single note (such as 5 16ths).
	// These cases need LO information in any case.

	int starti = tindex[0];
	int endi = tindex.back();

	// Remove (repeating) tremolo notes.
	HumNum starttime = notes[starti]->getDurationFromStart();
	HumNum endtime = notes[endi]->getDurationFromStart();
	HumNum lastdur = notes[endi]->getDuration();
	HumNum duration = endtime - starttime + lastdur;
	string recip = Convert::durationToRecip(duration);
	notes[starti]->setValue("auto", "tremoloRhythm", recip);
	// string text = *notes[0];
	// HumRegex hre;
	// hre.replaceDestructive(text, recip, "\\d+%?\\d*\\.*", "g");
	// hre.replaceDestructive(text, "", "[LJkK]+", "g");
	// notes[0]->setText(text);
	for (int i=starti+1; i<=endi; i++) {
		notes[i]->setValue("auto", "ignoreTremoloNote", 1);
	}
}



//////////////////////////////
//
// Tool_composite::pitchesEqual -- also consider ties...
//

bool Tool_composite::pitchesEqual(vector<int>& pitches1, vector<int>& pitches2) {
	if (pitches1.size() != pitches2.size()) {
		return false;
	}
	for (int i=0; i<(int)pitches1.size(); i++) {
		if (pitches1[i] != pitches2[i]) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// Tool_composite::areAllEqual -- 
//

bool Tool_composite::areAllEqual(vector<HTp>& notes) {
	if (notes.empty()) {
		return false;
	}
	vector<int> pitches;
	getPitches(pitches, notes[0]);
	vector<int> others;
	for (int i=1; i<(int)notes.size(); i++) {
		getPitches(others, notes[i]);
		if (others.size() != pitches.size()) {
			return false;
		}
		for (int j=0; j<(int)others.size(); j++) {
			if (others[j] != pitches[j]) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// Tool_composite::getPitches -- 
//

void Tool_composite::getPitches(vector<int>& pitches, HTp token) {
	vector<string> subtokens;
	subtokens = token->getSubtokens();
	pitches.clear();
	pitches.resize(subtokens.size());
	fill(pitches.begin(), pitches.end(), 0);
	for (int i=0; i<(int)subtokens.size(); i++) {
		if (subtokens[i].find("r") != string::npos) {
			continue;
		}
		pitches[i] = Convert::kernToBase40(subtokens[i]);
	}
	if (pitches.size() > 1) {
		sort(pitches.begin(), pitches.end());
	}
}



//////////////////////////////
//
// Tool_composite::checkForTremoloReduction -- 
//

void Tool_composite::getBeamedNotes(vector<HTp>& notes, HTp starting) {
	notes.clear();
	notes.push_back(starting);
	int Lcount = (int)count(starting->begin(), starting->end(), 'L');
	int Jcount = (int)count(starting->begin(), starting->end(), 'J');
	int beamcounter = Lcount - Jcount;
	if (beamcounter <= 0) {
		notes.clear();
		return;
	}
	HTp current = starting->getNextToken();
	while (current) {
		if (current->isBarline()) {
			break;
		}
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (*current == ".") {
			current = current->getNextToken();
			continue;
		}
		notes.push_back(current);
		Lcount = (int)count(starting->begin(), starting->end(), 'L');
		Jcount = (int)count(starting->begin(), starting->end(), 'J');
		beamcounter += Lcount - Jcount;
		if (beamcounter <= 0) {
			break;
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_composite::addLabels -- add Grouping labels to composite rhythm groups.
//

void Tool_composite::addLabels(HumdrumFile& infile, int amount) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		int hasLabel = 0;
		int hasLabelAbbr = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->compare(0, 3, "*I\"") == 0) {
				hasLabel = i;
			}
			if (token->compare(0, 3, "*I'") == 0) {
				hasLabelAbbr = i;
			}
		}
		if (hasLabel) {
			if (amount == 2) {
				HTp token = infile.token(hasLabel, 0);
				token->setText("*I\"Group A");
				token = infile.token(hasLabel, 1);
				token->setText("*I\"Group B");
			} else if (amount == -2) {
				int fcount = infile[hasLabel].getFieldCount();
				HTp token = infile.token(hasLabel, fcount-1);
				token->setText("*I\"Group B");
				token = infile.token(hasLabel, fcount-2);
				token->setText("*I\"Group A");
			}
		}
		if (hasLabelAbbr) {
			if (amount == 2) {
				HTp token = infile.token(hasLabelAbbr, 0);
				token->setText("*I'Gr.A");
				token = infile.token(hasLabelAbbr, 1);
				token->setText("*I'Gr.B");
			} else if (amount == -2) {
				int fcount = infile[hasLabelAbbr].getFieldCount();
				HTp token = infile.token(hasLabelAbbr, fcount-1);
				token->setText("*I\'Gr.B");
				token = infile.token(hasLabelAbbr, fcount-2);
				token->setText("*I\'Gr.A");
			}
		}
	}

}




// END_MERGE

} // end namespace hum



