//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 01:02:57 PST 2016
// Last Modified: Sat Apr 30 12:05:40 PDT 2022
// Filename:      tool-filter.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-filter.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Example of extracting a 2D pitch grid from
//                a score for dissonance analysis.
//

#include "tool-filter.h"

// tools which filter can process:
#include "tool-autoaccid.h"
#include "tool-autobeam.h"
#include "tool-autostem.h"
#include "tool-binroll.h"
#include "tool-chantize.h"
#include "tool-chooser.h"
#include "tool-chord.h"
#include "tool-cint.h"
#include "tool-colorgroups.h"
#include "tool-colortriads.h"
#include "tool-composite.h"
#include "tool-deg.h"
#include "tool-dissonant.h"
#include "tool-double.h"
#include "tool-extract.h"
#include "tool-fb.h"
#include "tool-flipper.h"
#include "tool-gasparize.h"
#include "tool-half.h"
#include "tool-homorhythm.h"
#include "tool-homorhythm2.h"
#include "tool-hproof.h"
#include "tool-humdiff.h"
#include "tool-humsheet.h"
#include "tool-humtr.h"
#include "tool-imitation.h"
#include "tool-worex.h"
#include "tool-kern2mens.h"
#include "tool-kernview.h"
#include "tool-mei2hum.h"
#include "tool-melisma.h"
#include "tool-mens2kern.h"
#include "tool-metlev.h"
#include "tool-modori.h"
#include "tool-msearch.h"
#include "tool-myank.h"
#include "tool-cmr.h"
#include "tool-phrase.h"
#include "tool-recip.h"
#include "tool-restfill.h"
#include "tool-rid.h"
#include "tool-satb2gs.h"
#include "tool-scordatura.h"
#include "tool-semitones.h"
#include "tool-shed.h"
#include "tool-sic.h"
#include "tool-simat.h"
#include "tool-slurcheck.h"
#include "tool-spinetrace.h"
#include "tool-strophe.h"
#include "tool-synco.h"
#include "tool-tabber.h"
#include "tool-tassoize.h"
#include "tool-thru.h"
#include "tool-tie.h"
#include "tool-timebase.h"
#include "tool-transpose.h"
#include "tool-tremolo.h"
#include "tool-trillspell.h"

#include "HumRegex.h"

#include <algorithm>
#include <cmath>


using namespace std;

namespace hum {

// START_MERGE


#define RUNTOOL(NAME, INFILE, COMMAND, STATUS)     \
	Tool_##NAME *tool = new Tool_##NAME;            \
	tool->process(COMMAND);                         \
	tool->run(INFILE);                              \
	if (tool->hasError()) {                         \
		status = false;                              \
		tool->getError(cerr);                        \
		delete tool;                                 \
		break;                                       \
	} else if (tool->hasHumdrumText()) {            \
		INFILE.readString(tool->getHumdrumText());   \
	}                                               \
	delete tool;

#define RUNTOOL2(NAME, INFILE1, INFILE2, COMMAND, STATUS) \
	Tool_##NAME *tool = new Tool_##NAME;            \
	tool->process(COMMAND);                         \
	tool->run(INFILE1, INFILE2);                    \
	if (tool->hasError()) {                         \
		status = false;                              \
		tool->getError(cerr);                        \
		delete tool;                                 \
		break;                                       \
	} else if (tool->hasHumdrumText()) {            \
		INFILE1.readString(tool->getHumdrumText());  \
	}                                               \
	delete tool;

#define RUNTOOLSET(NAME, INFILES, COMMAND, STATUS) \
	Tool_##NAME *tool = new Tool_##NAME;            \
	tool->process(COMMAND);                         \
	tool->run(INFILES);                             \
	if (tool->hasError()) {                         \
		status = false;                              \
		tool->getError(cerr);                        \
		delete tool;                                 \
		break;                                       \
	} else if (tool->hasHumdrumText()) {            \
		INFILES.readString(tool->getHumdrumText());  \
	}                                               \
	delete tool;

#define RUNTOOLSTREAM(NAME, INFILES, COMMAND, STATUS) \
	Tool_##NAME *tool = new Tool_##NAME;               \
	tool->process(COMMAND);                            \
	tool->run(INFILES);                                \
	if (tool->hasError()) {                            \
		status = false;                                 \
		tool->getError(cerr);                           \
		delete tool;                                    \
		break;                                          \
	} else if (tool->hasHumdrumText()) {               \
		INFILES.readString(tool->getHumdrumText());     \
	}                                                  \
	delete tool;



////////////////////////////////
//
// Tool_filter::Tool_filter -- Set the recognized options for the tool.
//

Tool_filter::Tool_filter(void) {
	define("debug=b", "print debug statement");
	define("v|variant=s:", "Run filters labeled with the given variant");
}



/////////////////////////////////
//
// Tool_filter::run -- Primary interfaces to the tool.
//

bool Tool_filter::run(const string& indata) {
	HumdrumFileSet infiles(indata);
	bool status = run(infiles);
	return status;
}


bool Tool_filter::run(HumdrumFile& infile) {
	HumdrumFileSet infiles;
	infiles.appendHumdrumPointer(&infile);
	bool status = run(infiles);
	infiles.clearNoFree();
	return status;
}

bool Tool_filter::runUniversal(HumdrumFileSet& infiles) {
	bool status = true;
	vector<pair<string, string> > commands;
	getUniversalCommandList(commands, infiles);

	for (int i=0; i<(int)commands.size(); i++) {
		if (commands[i].first == "humdiff") {
			RUNTOOLSET(humdiff, infiles, commands[i].second, status);
		} else if (commands[i].first == "chooser") {
			RUNTOOLSET(chooser, infiles, commands[i].second, status);
		} else if (commands[i].first == "myank") {
			RUNTOOL(myank, infiles, commands[i].second, status);
		}
	}

	removeUniversalFilterLines(infiles);

	return status;
}


//
// In-place processing of file:
//

bool Tool_filter::run(HumdrumFileSet& infiles) {
	if (infiles.getCount() == 0) {
		return false;
	}

	initialize(infiles[0]);

	HumdrumFile& infile = infiles[0];

	bool status = true;
	vector<pair<string, string> > commands;
	getCommandList(commands, infile);
	for (int i=0; i<(int)commands.size(); i++) {
		if (commands[i].first == "autoaccid") {
			RUNTOOL(autoaccid, infile, commands[i].second, status);
		} else if (commands[i].first == "autobeam") {
			RUNTOOL(autobeam, infile, commands[i].second, status);
		} else if (commands[i].first == "autostem") {
			RUNTOOL(autostem, infile, commands[i].second, status);
		} else if (commands[i].first == "binroll") {
			RUNTOOL(binroll, infile, commands[i].second, status);
		} else if (commands[i].first == "chantize") {
			RUNTOOL(chantize, infile, commands[i].second, status);
		} else if (commands[i].first == "chord") {
			RUNTOOL(chord, infile, commands[i].second, status);
		} else if (commands[i].first == "cint") {
			RUNTOOL(cint, infile, commands[i].second, status);
		} else if (commands[i].first == "cmr") {
			RUNTOOL(cmr, infile, commands[i].second, status);
		} else if (commands[i].first == "composite") {
			RUNTOOL(composite, infile, commands[i].second, status);
		} else if (commands[i].first == "dissonant") {
			RUNTOOL(dissonant, infile, commands[i].second, status);
		} else if (commands[i].first == "double") {
			RUNTOOL(double, infile, commands[i].second, status);
		} else if (commands[i].first == "fb") {
			RUNTOOL(fb, infile, commands[i].second, status);
		} else if (commands[i].first == "flipper") {
			RUNTOOL(flipper, infile, commands[i].second, status);
		} else if (commands[i].first == "filter") {
			RUNTOOL(filter, infile, commands[i].second, status);
		} else if (commands[i].first == "gasparize") {
			RUNTOOL(gasparize, infile, commands[i].second, status);
		} else if (commands[i].first == "half") {
			RUNTOOL(half, infile, commands[i].second, status);
		} else if (commands[i].first == "homorhythm") {
			RUNTOOL(homorhythm, infile, commands[i].second, status);
		} else if (commands[i].first == "homorhythm2") {
			RUNTOOL(homorhythm2, infile, commands[i].second, status);
		} else if (commands[i].first == "hproof") {
			RUNTOOL(hproof, infile, commands[i].second, status);
		} else if (commands[i].first == "humsheet") {
			RUNTOOL(humsheet, infile, commands[i].second, status);
		} else if (commands[i].first == "humtr") {
			RUNTOOL(humtr, infile, commands[i].second, status);
		} else if (commands[i].first == "imitation") {
			RUNTOOL(imitation, infile, commands[i].second, status);
		} else if (commands[i].first == "worex") {
			RUNTOOL(worex, infile, commands[i].second, status);
		} else if (commands[i].first == "kern2mens") {
			RUNTOOL(kern2mens, infile, commands[i].second, status);
		} else if (commands[i].first == "kernview") {
			RUNTOOL(kernview, infile, commands[i].second, status);
		} else if (commands[i].first == "melisma") {
			RUNTOOL(melisma, infile, commands[i].second, status);
		} else if (commands[i].first == "mens2kern") {
			RUNTOOL(mens2kern, infile, commands[i].second, status);
		} else if (commands[i].first == "metlev") {
			RUNTOOL(metlev, infile, commands[i].second, status);
		} else if (commands[i].first == "modori") {
			RUNTOOL(modori, infile, commands[i].second, status);
		} else if (commands[i].first == "msearch") {
			RUNTOOL(msearch, infile, commands[i].second, status);
		} else if (commands[i].first == "phrase") {
			RUNTOOL(phrase, infile, commands[i].second, status);
		} else if (commands[i].first == "recip") {
			RUNTOOL(recip, infile, commands[i].second, status);
		} else if (commands[i].first == "restfill") {
			RUNTOOL(restfill, infile, commands[i].second, status);
		} else if (commands[i].first == "scordatura") {
			RUNTOOL(scordatura, infile, commands[i].second, status);
		} else if (commands[i].first == "semitones") {
			RUNTOOL(semitones, infile, commands[i].second, status);
		} else if (commands[i].first == "shed") {
			RUNTOOL(shed, infile, commands[i].second, status);
		} else if (commands[i].first == "sic") {
			RUNTOOL(sic, infile, commands[i].second, status);
		} else if (commands[i].first == "simat") {
			RUNTOOL2(simat, infile, infile, commands[i].second, status);
		} else if (commands[i].first == "slurcheck") {
			RUNTOOL(slurcheck, infile, commands[i].second, status);
		} else if (commands[i].first == "slur") {
			RUNTOOL(slurcheck, infile, commands[i].second, status);
		} else if (commands[i].first == "spinetrace") {
			RUNTOOL(spinetrace, infile, commands[i].second, status);
		} else if (commands[i].first == "strophe") {
			RUNTOOL(strophe, infile, commands[i].second, status);
		} else if (commands[i].first == "synco") {
			RUNTOOL(synco, infile, commands[i].second, status);
		} else if (commands[i].first == "tabber") {
			RUNTOOL(tabber, infile, commands[i].second, status);
		} else if (commands[i].first == "tassoize") {
			RUNTOOL(tassoize, infile, commands[i].second, status);
		} else if (commands[i].first == "tassoise") {
			RUNTOOL(tassoize, infile, commands[i].second, status);
		} else if (commands[i].first == "tasso") {
			RUNTOOL(tassoize, infile, commands[i].second, status);
		} else if (commands[i].first == "tie") {
			RUNTOOL(tie, infile, commands[i].second, status);
		} else if (commands[i].first == "transpose") {
			RUNTOOL(transpose, infile, commands[i].second, status);
		} else if (commands[i].first == "tremolo") {
			RUNTOOL(tremolo, infile, commands[i].second, status);
		} else if (commands[i].first == "trillspell") {
			RUNTOOL(trillspell, infile, commands[i].second, status);

		// filters with aliases:

		} else if (commands[i].first == "colortriads") {
			RUNTOOL(colortriads, infile, commands[i].second, status);
		} else if (commands[i].first == "colourtriads") {
			// British spelling
			RUNTOOL(colortriads, infile, commands[i].second, status);

		} else if (commands[i].first == "colorgroups") {
			RUNTOOL(colorgroups, infile, commands[i].second, status);
		} else if (commands[i].first == "colourgroups") { // British spelling
			RUNTOOL(colorgroups, infile, commands[i].second, status);

		} else if (commands[i].first == "deg") { // humlib version of Humdrum Toolkit deg tool
			RUNTOOL(deg, infile, commands[i].second, status);
		} else if (commands[i].first == "degx") { // humlib cli name
			RUNTOOL(deg, infile, commands[i].second, status);

		} else if (commands[i].first == "extract") { // humlib version of Humdrum Toolkit extract tool
			RUNTOOL(extract, infile, commands[i].second, status);
		} else if (commands[i].first == "extractx") { // humlib cli name
			RUNTOOL(extract, infile, commands[i].second, status);

		} else if (commands[i].first == "myank") { // humlib version of Humdrum Extras myank tool
			RUNTOOL(myank, infile, commands[i].second, status);
		} else if (commands[i].first == "myankx") { // humlib cli name
			RUNTOOL(myank, infile, commands[i].second, status);

		} else if (commands[i].first == "rid") { // humlib version of Humdrum Toolkit deg tool
			RUNTOOL(rid, infile, commands[i].second, status);
		} else if (commands[i].first == "ridx") { // Humdrum Extra cli name
			RUNTOOL(rid, infile, commands[i].second, status);
		} else if (commands[i].first == "ridxx") { // humlib cli name
			RUNTOOL(rid, infile, commands[i].second, status);

		} else if (commands[i].first == "satb2gs") { // humlib version of Humdrum Extras satg2gs tool
			RUNTOOL(satb2gs, infile, commands[i].second, status);
		} else if (commands[i].first == "satb2gsx") { // humlib cli name
			RUNTOOL(satb2gs, infile, commands[i].second, status);

		} else if (commands[i].first == "thru") {
			RUNTOOL(thru, infile, commands[i].second, status);
		} else if (commands[i].first == "thru") { // humlib version of Humdrum Toolkit thru tool
			RUNTOOL(thru, infile, commands[i].second, status);
		} else if (commands[i].first == "thrux") { // Humdrum Extras cli name
			RUNTOOL(thru, infile, commands[i].second, status);
		} else if (commands[i].first == "thruxx") { // humlib cli name
			RUNTOOL(thru, infile, commands[i].second, status);

		} else if (commands[i].first == "timebase") { // humlib version of Humdrum Toolkit timebase tool
			RUNTOOL(timebase, infile, commands[i].second, status);
		} else if (commands[i].first == "timebasex") { // humlib cli name
			RUNTOOL(timebase, infile, commands[i].second, status);
		}


	}

	removeGlobalFilterLines(infile);

	// Re-load the text for each line from their tokens in case any
	// updates are needed from token changes.
	infile.createLinesFromTokens();
	return status;
}



//////////////////////////////
//
// Tool_filter::removeGlobalFilterLines --
//

void Tool_filter::removeGlobalFilterLines(HumdrumFile& infile) {
	HumRegex hre;
	string text;

	string maintag = "!!!filter:";
	string mainXtag = "!!!Xfilter:";
	string maintagQuery = "^!!!filter:";

	string maintagV;
	string mainXtagV;
	string maintagQueryV;

	if (m_variant.size() > 0) {
		maintagV = "!!!filter-" + m_variant + ":";
		mainXtagV = "!!!Xfilter-" + m_variant + ":";
		maintagQueryV = "^!!!filter-" + m_variant + ":";
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}

		if (m_variant.size() > 0) {
			if (infile.token(i, 0)->compare(0, maintagV.size(), maintagV) == 0) {
				text = infile.token(i, 0)->getText();
				hre.replaceDestructive(text, mainXtagV, maintagQueryV);
				infile.token(i, 0)->setText(text);
			}
		} else {
			if (infile.token(i, 0)->compare(0, maintag.size(), maintag) == 0) {
				text = infile.token(i, 0)->getText();
				hre.replaceDestructive(text, mainXtag, maintagQuery);
				infile.token(i, 0)->setText(text);
			}
		}
	}
}



//////////////////////////////
//
// Tool_filter::removeUniversalFilterLines --
//

void Tool_filter::removeUniversalFilterLines(HumdrumFileSet& infiles) {
	HumRegex hre;
	string text;

	string maintag = "!!!!filter:";
	string mainXtag = "!!!!Xfilter:";
	string maintagQuery = "^!!!!filter:";

	string maintagV;
	string mainXtagV;
	string maintagQueryV;

	if (m_variant.size() > 0) {
		maintagV = "!!!!filter-" + m_variant + ":";
		mainXtagV = "!!!!Xfilter-" + m_variant + ":";
		maintagQueryV = "^!!!!filter-" + m_variant + ":";
	}

	for (int i=0; i<infiles.getCount(); i++) {
		HumdrumFile& infile = infiles[i];
		for (int j=0; j<infile.getLineCount(); j++) {
			if (!infile[i].isUniversalReference()) {
				continue;
			}
			HTp token = infile.token(j, 0);
			if (m_variant.size() > 0) {
				if (token->compare(0, maintagV.size(), maintagV) == 0) {
					text = token->getText();
					hre.replaceDestructive(text, mainXtagV, maintagQueryV);
					token->setText(text);
					infile[j].createLineFromTokens();
				}
			} else {
				if (token->compare(0, maintag.size(), maintag) == 0) {
					text = token->getText();
					hre.replaceDestructive(text, mainXtag, maintagQuery);
					token->setText(text);
					infile[j].createLineFromTokens();
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_filter::getCommandList --
//

void Tool_filter::getCommandList(vector<pair<string, string> >& commands,
		HumdrumFile& infile) {

	vector<HLp> refs = infile.getReferenceRecords();
	pair<string, string> entry;
	string tag = "filter";
	if (m_variant.size() > 0) {
		tag += "-";
		tag += m_variant;
	}
	vector<string> clist;
	HumRegex hre;
	for (int i=0; i<(int)refs.size(); i++) {
		string refkey = refs[i]->getGlobalReferenceKey();
		if (refkey != tag) {
			continue;
		}
		string command = refs[i]->getGlobalReferenceValue();
		splitPipeline(clist, command);
		for (int j=0; j<(int)clist.size(); j++) {
			if (hre.search(clist[j], "^\\s*([^\\s]+)")) {
				entry.first  = hre.getMatch(1);
				entry.second = clist[j];
				commands.push_back(entry);
			}
		}
	}
}



//////////////////////////////
//
//  Tool_filter::splitPipeline --
//

void Tool_filter::splitPipeline(vector<string>& clist, const string& command) {
	clist.clear();
	clist.resize(1);
	clist[0] = "";
	int inDoubleQuotes = -1;
	int inSingleQuotes = -1;
	char ch = '\0';
	char lastch;
	for (int i=0; i<(int)command.size(); i++) {
		lastch = ch;
		ch = command[i];

		if (ch == '"') {
			if (lastch == '\\') {
				// escaped double quote, so treat as regular character
				clist.back() += ch;
				continue;
			} else if (inDoubleQuotes >= 0) {
				// turn off previous double quote sequence
				clist.back() += ch;
				inDoubleQuotes = -1;
				continue;
			} else if (inSingleQuotes >= 0) {
				// in an active single quote, so this is not a closing double quote
				clist.back() += ch;
				continue;
			} else {
				// this is the start of a double quote sequence
				clist.back() += ch;
				inDoubleQuotes = i;
				continue;
			}
		}

		if (ch == '\'') {
			if (lastch == '\\') {
				// escaped single quote, so treat as regular character
				clist.back() += ch;
				continue;
			} else if (inSingleQuotes >= 0) {
				// turn off previous single quote sequence
				clist.back() += ch;
				inSingleQuotes = -1;
				continue;
			} else if (inDoubleQuotes >= 0) {
				// in an active double quote, so this is not a closing single quote
				clist.back() += ch;
				continue;
			} else {
				// this is the start of a single quote sequence
				clist.back() += ch;
				inSingleQuotes = i;
				continue;
			}
		}

		if (ch == '|') {
			if ((inSingleQuotes > -1) || (inDoubleQuotes > -1)) {
				// pipe character
				clist.back() += ch;
				continue;
			} else {
				// this is a real pipe
				clist.resize(clist.size() + 1);
				continue;
			}
		}

		if (isspace(ch) && (!(inSingleQuotes > -1)) && (!(inDoubleQuotes > -1))) {
			if (isspace(lastch)) {
				// don't repeat spaces outside of quotes.
				continue;
			}
		}

		// regular character
		clist.back() += ch;
	}

	// remove leading and trailing spaces
	HumRegex hre;
	for (int i=0; i<(int)clist.size(); i++) {
		hre.replaceDestructive(clist[i], "", "^\\s+");
		hre.replaceDestructive(clist[i], "", "\\s+$");
	}

}



//////////////////////////////
//
// Tool_filter::getUniversalCommandList --
//

void Tool_filter::getUniversalCommandList(vector<pair<string, string> >& commands,
		HumdrumFileSet& infiles) {

	vector<HLp> refs = infiles.getUniversalReferenceRecords();
	pair<string, string> entry;
	string tag = "filter";
	if (m_variant.size() > 0) {
		tag += "-";
		tag += m_variant;
	}
	vector<string> clist;
	HumRegex hre;
	for (int i=0; i<(int)refs.size(); i++) {
		if (refs[i]->getUniversalReferenceKey() != tag) {
			continue;
		}
		string command = refs[i]->getUniversalReferenceValue();
		hre.split(clist, command, "\\s*\\|\\s*");
		for (int j=0; j<(int)clist.size(); j++) {
			if (hre.search(clist[j], "^\\s*([^\\s]+)")) {
				entry.first  = hre.getMatch(1);
				entry.second = clist[j];
				commands.push_back(entry);
			}
		}
	}
}



//////////////////////////////
//
// Tool_filter::initialize -- extract time signature lines for
//    each **kern spine in file.
//

void Tool_filter::initialize(HumdrumFile& infile) {
	m_debugQ = getBoolean("debug");
	m_variant.clear();
	if (getBoolean("variant")) {
		m_variant = getString("variant");
	}
}



// END_MERGE

} // end namespace hum



