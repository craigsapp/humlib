//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Nov 30 01:02:57 PST 2016
// Last Modified: Sat Jun 17 23:27:51 CEST 2017
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
#include "tool-autobeam.h"
#include "tool-autostem.h"
#include "tool-binroll.h"
#include "tool-chooser.h"
#include "tool-chord.h"
#include "tool-cint.h"
#include "tool-composite.h"
#include "tool-dissonant.h"
#include "tool-extract.h"
#include "tool-homophonic.h"
#include "tool-hproof.h"
#include "tool-humdiff.h"
#include "tool-imitation.h"
#include "tool-kern2mens.h"
#include "tool-metlev.h"
#include "tool-msearch.h"
#include "tool-myank.h"
#include "tool-phrase.h"
#include "tool-recip.h"
#include "tool-restfill.h"
#include "tool-satb2gs.h"
#include "tool-simat.h"
#include "tool-slurcheck.h"
#include "tool-spinetrace.h"
#include "tool-tabber.h"
#include "tool-tassoize.h"
#include "tool-trillspell.h"
#include "tool-transpose.h"

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



////////////////////////////////
//
// Tool_filter::Tool_filter -- Set the recognized options for the tool.
//

Tool_filter::Tool_filter(void) {
	define("debug=b", "print debug statement");
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
		if (commands[i].first == "autobeam") {
			RUNTOOL(autobeam, infile, commands[i].second, status);
		} else if (commands[i].first == "autostem") {
			RUNTOOL(autostem, infile, commands[i].second, status);
		} else if (commands[i].first == "chord") {
			RUNTOOL(chord, infile, commands[i].second, status);
		} else if (commands[i].first == "cint") {
			RUNTOOL(cint, infile, commands[i].second, status);
		} else if (commands[i].first == "composite") {
			RUNTOOL(composite, infile, commands[i].second, status);
		} else if (commands[i].first == "dissonant") {
			RUNTOOL(dissonant, infile, commands[i].second, status);
		} else if (commands[i].first == "homophonic") {
			RUNTOOL(homophonic, infile, commands[i].second, status);
		} else if (commands[i].first == "hproof") {
			RUNTOOL(hproof, infile, commands[i].second, status);
		} else if (commands[i].first == "imitation") {
			RUNTOOL(imitation, infile, commands[i].second, status);
		} else if (commands[i].first == "extract") {
			RUNTOOL(extract, infile, commands[i].second, status);
		} else if (commands[i].first == "metlev") {
			RUNTOOL(metlev, infile, commands[i].second, status);
		} else if (commands[i].first == "msearch") {
			RUNTOOL(msearch, infile, commands[i].second, status);
		} else if (commands[i].first == "phrase") {
			RUNTOOL(phrase, infile, commands[i].second, status);
		} else if (commands[i].first == "restfill") {
			RUNTOOL(restfill, infile, commands[i].second, status);
		} else if (commands[i].first == "satb2gs") {
			RUNTOOL(satb2gs, infile, commands[i].second, status);
		} else if (commands[i].first == "simat") {
			RUNTOOL2(simat, infile, infile, commands[i].second, status);
		} else if (commands[i].first == "kern2mens") {
			RUNTOOL(kern2mens, infile, commands[i].second, status);
		} else if (commands[i].first == "recip") {
			RUNTOOL(recip, infile, commands[i].second, status);
		} else if (commands[i].first == "slurcheck") {
			RUNTOOL(slurcheck, infile, commands[i].second, status);
		} else if (commands[i].first == "slur") {
			RUNTOOL(slurcheck, infile, commands[i].second, status);
		} else if (commands[i].first == "spinetrace") {
			RUNTOOL(spinetrace, infile, commands[i].second, status);
		} else if (commands[i].first == "tabber") {
			RUNTOOL(tabber, infile, commands[i].second, status);
		} else if (commands[i].first == "tassoize") {
			RUNTOOL(tassoize, infile, commands[i].second, status);
		} else if (commands[i].first == "tasso") {
			RUNTOOL(tassoize, infile, commands[i].second, status);
		} else if (commands[i].first == "transpose") {
			RUNTOOL(transpose, infile, commands[i].second, status);
		} else if (commands[i].first == "trillspell") {
			RUNTOOL(trillspell, infile, commands[i].second, status);
		} else if (commands[i].first == "binroll") {
			RUNTOOL(binroll, infile, commands[i].second, status);
		} else if (commands[i].first == "myank") {
			RUNTOOL(myank, infile, commands[i].second, status);
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
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		if (infile.token(i, 0)->compare(0, 10, "!!!filter:") == 0) {
			text = infile.token(i, 0)->getText();
			hre.replaceDestructive(text, "!!!Xfilter:", "^!!!filter:");
			infile.token(i, 0)->setText(text);
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
	for (int i=0; i<infiles.getCount(); i++) {
		HumdrumFile& infile = infiles[i];
		for (int j=0; j<infile.getLineCount(); j++) {
			if (!infile[i].isUniversalReference()) {
				continue;
			}
			HTp token = infile.token(j, 0);
			if (token->compare(0, 11, "!!!!filter:") == 0) {
				text = token->getText();
				hre.replaceDestructive(text, "!!!!Xfilter:", "^!!!!filter:");
				token->setText(text);
				infile[j].createLineFromTokens();
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

	vector<HumdrumLine*> refs = infile.getReferenceRecords();
	pair<string, string> entry;
	string tag = "filter";
	vector<string> clist;
	HumRegex hre;
   if (m_variant.size() > 0) {
		tag += "-";
		tag += m_variant;
	}
	for (int i=0; i<(int)refs.size(); i++) {
		string refkey = refs[i]->getGlobalReferenceKey();
		if (refkey != tag) {
			continue;
		}
		string command = refs[i]->getGlobalReferenceValue();
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
// Tool_filter::getUniversalCommandList --
//

void Tool_filter::getUniversalCommandList(vector<pair<string, string> >& commands,
		HumdrumFileSet& infiles) {

	vector<HumdrumLine*> refs = infiles.getUniversalReferenceRecords();
	pair<string, string> entry;
	string tag = "filter";
	vector<string> clist;
	HumRegex hre;
   if (m_variant.size() > 0) {
		tag += "-";
		tag += m_variant;
	}
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
}



// END_MERGE

} // end namespace hum



