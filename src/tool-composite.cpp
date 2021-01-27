//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 14 01:03:07 CEST 2019
// Last Modified: Mon Jan 18 11:10:22 PST 2021
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


/////////////////////////////////
//
// Tool_composite::Tool_composite -- Set the recognized options for the tool.
//

Tool_composite::Tool_composite(void) {
	define("pitch=s:e",    "pitch to display for composite rhythm");
	define("debug=b",      "print debugging information");
	define("group=b",      "group into separate composite steams by group labels");
	define("a|append=b",   "append data to end of line");
	define("p|prepend=b",  "prepend data to end of line");
	define("b|beam=b",     "apply automatic beaming");
	define("G|no-grace=b", "do not include grace notes");
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
	m_pitch = getString("pitch");
}



//////////////////////////////
//
// Tool_composite::processFile --
//

void Tool_composite::processFile(HumdrumFile& infile) {
	Tool_extract extract;
	bool groupQ = getBoolean("group");
	bool appendQ = getBoolean("append");
	bool prependQ = getBoolean("prepend");
	bool graceQ = !getBoolean("no-grace");
	bool debugQ = getBoolean("debug");
	vector<vector<string>> grouping;
	if (groupQ) {
		setupGrouping(grouping, infile);
		if (debugQ) {
			printGroupingInfo(grouping);
			return;
		}
	}

	if (!appendQ) {
		if (groupQ) {
			extract.setModified("s", "0,0,1-$");
		} else {
			extract.setModified("s", "0,1-$");
		}
	} else {
		if (groupQ) {
			extract.setModified("s", "1-$,0,0");
		} else {
			extract.setModified("s", "1-$,0");
		}
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

	bool wroteA = false;
	bool wroteB = false;
	stringstream sstream;
	sstream << infile;
	HumdrumFile originalfile;
	originalfile.readString(sstream.str());
	extract.run(infile);
	infile.readString(extract.getAllText());
	HTp token = NULL;
	HTp token2 = NULL;
	for (int i=0; i<infile.getLineCount(); i++) {
		token = NULL;
		token2 = NULL;
		if (infile[i].isInterpretation()) {
			if (prependQ) {
				if (groupQ) {
					token = infile.token(i, 0);
					token2 = infile.token(i, 1);
				} else {
					token = infile.token(i, 0);
					token2 = NULL;
				}
			} else {
				if (groupQ) {
					token = infile.token(i, infile[i].getFieldCount() - 2);
					token2 = infile.token(i, infile[i].getFieldCount() - 1);
				} else {
					token = infile.token(i, infile[i].getFieldCount() - 1);
					token2 = NULL;
				}
			}
			if (groupQ) {
				if (token && token->compare("**blank") == 0) {
					token->setText("**kern");
				}
				if (token2 && token2->compare("**blank") == 0) {
					token2->setText("**kern");
				}
				// continue;
			} else {
				if (token && token->compare("**blank") == 0) {
					token->setText("**kern");
				}
				// continue;
			}
			// copy time signature and tempos
			for (int j=1; j<infile[i].getFieldCount(); j++) {
				HTp stok = infile.token(i, j);
				if (stok->isTempo()) {
					token->setText(*stok);
					if (token2) {
						token->setText(*stok);
					}
				} else if (stok->isTimeSignature()) {
					token->setText(*stok);
					if (token2) {
						token->setText(*stok);
					}
				} else if (stok->isMensurationSymbol()) {
					token->setText(*stok);
					if (token2) {
						token->setText(*stok);
					}
				} else if (stok->isKeySignature()) {
					token->setText(*stok);
					if (token2) {
						token->setText(*stok);
					}
				} else if (stok->isKeyDesignation()) {
					token->setText(*stok);
					if (token2) {
						token->setText(*stok);
					}
				} else if (*stok == "*") {
					if (groupQ) {
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
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			// Grace note data line (most likely)
			if (!graceQ) {
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
					if (groupQ) {
						if (prependQ) {
							targettok = infile.token(i, 0);
							targettok2 = infile.token(i, 1);
						} else {
							targettok = infile.token(i, infile[i].getFieldCount()-2);
							targettok2 = infile.token(i, infile[i].getFieldCount()-1);
						}
					} else {
						if (prependQ) {
							targettok = infile.token(i, 0);
							targettok2 = NULL;
						} else {
							targettok = infile.token(i, infile[i].getFieldCount()-1);
							targettok2 = NULL;
						}
					}

					if (groupQ) {
						string group = grouping.at(i).at(j);
						if (group == "A") {
							targettok->setText(full);
						} else if (group == "B") {
							targettok2->setText(full);
						}
					} else {
						targettok->setText(full);
					}
					break;
				}
			}
			continue;
		}

		HumNum duration = getLineDuration(infile, i, isNull);
		string recip;
		string recip2;
		if (isNull[i]) {
			recip = ".";
			recip2 = ".";
		} else {
			recip = Convert::durationToRecip(duration);
			recip2 = recip;
		}

		if (groupQ) {

			bool grpa = hasGroup(grouping, originalfile, i, "A");
			bool grpb = hasGroup(grouping, originalfile, i, "B");
			// int atype = getGroupNoteType(grouping, originalfile, i, "A");
			// int btype = getGroupNoteType(grouping, originalfile, i, "A");
cerr << "GRPA " << grpa << " GRPB " << grpb << " LINE " << originalfile[i] << endl;

			HTp token2 = NULL;
			if (prependQ) {
				token = infile.token(i, 0);
				token2 = infile.token(i, 1);
			} else {
				token = infile.token(i, infile[i].getFieldCount()-2);
				token2 = infile.token(i, infile[i].getFieldCount()-1);
			}
			if (isRest[i]) {
				if (!isNull[i]) {
					if (grpa) {
						recip += "r";
					}
					if (grpb) {
						recip2 += "r";
					}
				}
			} else {
				if (grpa) {
					recip += pstring;
				} else {
					recip += "r";
				}
				if (grpb) {
					recip2 += pstring;
				} else {
					recip2 += "r";
				}
			}
			token->setText(recip);
			token2->setText(recip2);

		} else {

			if (prependQ) {
				token = infile.token(i, 0);
			} else {
				token = infile.token(i, infile[i].getFieldCount()-1);
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
	}

	if (groupQ) {
		if (!(appendQ || prependQ)) {
			Tool_extract extract2;
			extract2.setModified("s", "1-2");
			extract2.run(infile);
			infile.readString(extract2.getAllText());
		}
	} else {
		if (!(appendQ || prependQ)) {
			Tool_extract extract2;
			extract2.setModified("s", "1");
			extract2.run(infile);
			infile.readString(extract2.getAllText());
		}
	}

	if (getBoolean("beam")) {
		Tool_autobeam autobeam;
		if (groupQ) {
			if (prependQ) {
				autobeam.setModified("t", "1,2");
			} else {
				int trackcount =  infile.getTrackCount();
				string tstring = to_string(trackcount-1);
				tstring += ",";
				tstring += to_string(trackcount);
				autobeam.setModified("t", tstring);
			}
		} else {
			if (prependQ) {
				autobeam.setModified("t", "1");
			} else {
				int trackcount =  infile.getTrackCount();
				string tstring = to_string(trackcount);
				autobeam.setModified("t", tstring);
			}
		}
		// need to analyze structure for some reason:
		infile.analyzeStrands();
		autobeam.run(infile);
	}

}


//////////////////////////////
//
// Tool_composite::getGroupNoteType --
//
// -2 = No group of that type exists
// -1 = Note sustained
//  0 = No group tokens exists
//  1 = Note or rest attack
//

int Tool_composite::getGroupNoteType(vector<vector<string>>& grouping, HumdrumFile& infile,
		int line, const string& group) {
	// bool hasNull = false;
	// bool hasRest = false;
	// bool hasAttack = false;
	// bool hasSustain = false;
	for (int i=0; i<(int)grouping[line].size(); i++) {
		// HTp token = infile.token(line, i);
		string tgroup = grouping[line][i];
		if (tgroup != group) {
			continue;
		}
	}

	// ggg do more work here
	return 0;
}

//////////////////////////////
//
// Tool_composite::hasGroup --
//

bool Tool_composite::hasGroup(vector<vector<string>>& grouping, HumdrumFile& infile, int line,
		const string& group) {
	if (!infile[line].isData()) {
		return false;
	}
	for (int i=0; i<(int)grouping[line].size(); i++) {
		HTp token = infile.token(line, i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		if (!token->isNoteAttack()) {
			continue;
		}
		string testgroup = grouping[line][i];
		if (testgroup == group) {
			return true;
		}
	}
	return false;
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
// Tool_composite::setupGrouping --
//

void Tool_composite::setupGrouping(vector<vector<string>>& grouping, HumdrumFile& infile) {
	grouping.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		grouping[i].resize(infile[i].getFieldCount());
	}

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
			grouping.at(i).at(j) = group;
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
// Tool_composite::printGroupingInfo -- for debugging of group assignments.
//

void Tool_composite::printGroupingInfo(vector<vector<string>>& grouping) {
	for (int i=0; i<(int)grouping.size(); i++) {
		if (!grouping.empty()) {
			cerr << grouping[i][0];
			for (int j=1; j<(int)grouping[i].size(); j++) {
				cerr << "\t" << grouping[i][j];

			}
		}
		cerr << endl;
	}
}


// END_MERGE

} // end namespace hum



