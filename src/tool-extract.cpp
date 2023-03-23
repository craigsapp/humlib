//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Mon Oct  7 22:39:02 PDT 2019 Fixed -g option
// Filename:      tool-extract.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-extract.h
// Syntax:        C++11;; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   C++ implementation of the Humdrum Toolkit extract command.
//
// To do:       Allow *x records to be echoed when using -s 0 insertion
//					 Currently spines with *x are unwrapped and the *x is changed
//					 to *.
//

#include "tool-extract.h"
#include "HumRegex.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdlib.h>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_extract::Tool_extract -- Set the recognized options for the tool.
//

Tool_extract::Tool_extract(void) {
	define("P|F|S|x|exclude=s:", "Remove listed spines from output");
	define("i=s:", "Exclusive interpretation list to extract from input");
	define("I=s:", "Exclusive interpretation exclusion list");
	define("f|p|s|field|path|spine=s:",
			 "for extraction of particular spines");
	define("C|count=b", "print a count of the number of spines in file");
	define("c|cointerp=s:**kern", "Exclusive interpretation for cospines");
	define("g|grep=s:", "Extract spines which match a given regex.");
	define("r|reverse=b", "reverse order of spines by **kern group");
	define("R=s:**kern", "reverse order of spine by exinterp group");
	define("t|trace=s:", "use a trace file to extract data");
	define("e|expand=b", "expand spines with subspines");
	define("k|kern=s", "Extract by kern spine group");
	define("K|reverse-kern=s", "Extract by kern spine group top to bottom numbering");
	define("E|expand-interp=s:", "expand subspines limited to exinterp");
	define("m|model|method=s:d", "method for extracting secondary spines");
	define("M|cospine-model=s:d", "method for extracting cospines");
	define("Y|no-editoral-rests=b",
			"do not display yy marks on interpreted rests");
	define("n|name|b|blank=s:**blank", "Name if exinterp added with 0");
	define("no-empty|no-empties=b", "Suppress spines with only null data tokens");
	define("empty|empties=b", "Only keep spines with only null data tokens");
	define("spine-list=b", "Show spine list and then exit");
	define("no-rest|no-rests=b", "remove **kern spines containing only rests (and their co-spines)");

	define("debug=b", "print debugging information");
	define("author=b");              // author of program
	define("version=b");             // compilation info
	define("example=b");             // example usages
	define("h|help=b");              // short description
}



/////////////////////////////////
//
// Tool_extract::run -- Primary interfaces to the tool.
//

bool Tool_extract::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_extract::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_extract::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_extract::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	// Re-load the text for each line from their tokens.
	// infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_extract::processFile --
//

void Tool_extract::processFile(HumdrumFile& infile) {
	if (countQ) {
		m_free_text << infile.getMaxTrack() << endl;
		return;
	}
	if (expandQ) {
		expandSpines(field, subfield, model, infile, expandInterp);
	} else if (interpQ) {
		getInterpretationFields(field, subfield, model, infile, interps,
				interpstate);
	} else if (reverseQ) {
		reverseSpines(field, subfield, model, infile, reverseInterp);
	} else if (removerestQ) {
		fillFieldDataByNoRest(field, subfield, model, grepString, infile,
			interpstate);
	} else if (grepQ) {
		fillFieldDataByGrep(field, subfield, model, grepString, infile,
			interpstate);
	} else if (emptyQ) {
		fillFieldDataByEmpty(field, subfield, model, infile, interpstate);
	} else if (noEmptyQ) {
		fillFieldDataByNoEmpty(field, subfield, model, infile, interpstate);
	} else if (fieldQ || excludeQ) {
		fillFieldData(field, subfield, model, fieldstring, infile);
	}

	if (spineListQ) {
		m_free_text << "-s ";
		for (int i=0; i<(int)field.size(); i++) {
			m_free_text << field[i];
			if (i < (int)field.size() - 1) {
				m_free_text << ",";
			}
		}
		m_free_text << endl;
		return;
	}

	if (debugQ && !traceQ) {
		m_free_text << "!! Field Expansion List:";
		for (int j=0; j<(int)field.size(); j++) {
			m_free_text << " " << field[j];
			if (subfield[j]) {
				m_free_text << (char)subfield[j];
			}
			if (model[j]) {
				m_free_text << (char)model[j];
			}
		}
		m_free_text << endl;
	}

	// preserve SEGMENT filename if present (now printed in main())
	// infile.printNonemptySegmentLabel(m_humdrum_text);

	// analyze the input file according to command-line options
	if (fieldQ || grepQ || removerestQ) {
		extractFields(infile, field, subfield, model);
	} else if (excludeQ) {
		excludeFields(infile, field, subfield, model);
	} else if (traceQ) {
		extractTrace(infile, tracefile);
	} else {
		m_humdrum_text << infile;
	}
}



//////////////////////////////
//
// Tool_extract::getNullDataTracks --
//

vector<int> Tool_extract::getNullDataTracks(HumdrumFile& infile) {
	vector<int> output(infile.getMaxTrack() + 1, 1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			if (!output[track]) {
				continue;
			}
			if (!token->isNull()) {
				output[track] = 0;
			}
		}
		// maybe exit here if all tracks are non-null
	}

	return output;
}



//////////////////////////////
//
// Tool_extract::fillFieldDataByEmpty -- Only keep the spines which contain only
//    null data tokens.
//

void Tool_extract::fillFieldDataByEmpty(vector<int>& field, vector<int>& subfield,
		vector<int>& model, HumdrumFile& infile, int negate) {

	field.reserve(infile.getMaxTrack()+1);
	subfield.reserve(infile.getMaxTrack()+1);
	model.reserve(infile.getMaxTrack()+1);
	field.resize(0);
	subfield.resize(0);
	model.resize(0);
	vector<int> nullTrack = getNullDataTracks(infile);

	int zero = 0;
	for (int i=1; i<(int)nullTrack.size(); i++) {
		if (negate) {
			if (!nullTrack[i]) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		} else {
			if (nullTrack[i]) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		}
	}

}



//////////////////////////////
//
// Tool_extract::fillFieldDataByNoEmpty -- Only keep spines which are not all
//   null data tokens.
//

void Tool_extract::fillFieldDataByNoEmpty(vector<int>& field, vector<int>& subfield,
		vector<int>& model, HumdrumFile& infile, int negate) {

	field.reserve(infile.getMaxTrack()+1);
	subfield.reserve(infile.getMaxTrack()+1);
	model.reserve(infile.getMaxTrack()+1);
	field.resize(0);
	subfield.resize(0);
	model.resize(0);
	vector<int> nullTrack = getNullDataTracks(infile);
	for (int i=1; i<(int)nullTrack.size(); i++) {
		nullTrack[i] = !nullTrack[i];
	}

	int zero = 0;
	for (int i=1; i<(int)nullTrack.size(); i++) {
		if (negate) {
			if (!nullTrack[i]) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		} else {
			if (nullTrack[i]) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		}
	}
}



//////////////////////////////
//
// Tool_extract::fillFieldDataByNoRest --  Find the spines which
//    contain only rests and remove them.  Also remove cospines (non-kern spines
//    to the right of the kern spine containing only rests).  If there are
//    *part# interpretations in the data, then any spine which is all rests
//    will not be removed if there is another **kern spine with the same
//    part number if it is also not all rests.
//

void Tool_extract::fillFieldDataByNoRest(vector<int>& field, vector<int>& subfield,
		vector<int>& model, const string& searchstring, HumdrumFile& infile,
		int state) {

	field.clear();
	subfield.clear();
	model.clear();


	// Check every **kern spine for any notes.  If there is a note
	// then the tracks variable for that spine will be marked
	// as non-zero.
	vector<int> tracks(infile.getMaxTrack() + 1, 0);
	int track;
	int partline = 0;
	bool dataQ = false;
	for (int i=0; i<infile.getLineCount(); i++) {
		if ((!partline) && (!dataQ) && infile[i].hasSpines()) {

		}
		if (!infile[i].isData()) {
			continue;
		}
		dataQ = true;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			track = token->getTrack();
			tracks[track] = 1;
		}
	}

	// Go back and mark any empty spines as non-empty if they
   // are in a part that contains multiple staves. I.e., only
	// delete a staff if all staves for the part are empty.
	// There should be a single *part# line at the start of the
	// score.
	if (partline > 0) {
		vector<HTp> kerns;
		for (int i=0; i<infile[partline].getFieldCount(); i++) {
			HTp token = infile.token(partline, i);
			if (!token->isKern()) {
				continue;
			}
			kerns.push_back(token);
		}
		for (int i=0; i<(int)kerns.size(); i++) {
			for (int j=i+1; j<(int)kerns.size(); j++) {
				if (*kerns[i] != *kerns[j]) {
					continue;
				}
				if (kerns[i]->find("*part") == string::npos) {
					continue;
				}
				int track1 = kerns[i]->getTrack();
				int track2 = kerns[j]->getTrack();
				int state1 = tracks[track1];
				int state2 = tracks[track2];
				if ((state1 && !state2) || (state2 && !state1)) {
					// Prevent empty staff from being removed
					// from a multi-staff part:
					tracks[track1] = 1;
					tracks[track2] = 1;
				}
			}
		}
	}


	// deal with co-spines
	vector<HTp> sstarts;
	infile.getSpineStartList(sstarts);
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (!sstarts[i]->isKern()) {
			track = sstarts[i]->getTrack();
			tracks[track] = 1;
		}
	}

	// remove co-spines attached to removed kern spines
	for (int i=0; i<(int)sstarts.size(); i++) {
		if (!sstarts[i]->isKern()) {
			continue;
		}
		if (tracks[sstarts[i]->getTrack()] != 0) {
			continue;
		}
		for (int j=i+1; j<(int)sstarts.size(); j++) {
			if (sstarts[j]->isKern()) {
				break;
			}
			track = sstarts[j]->getTrack();
			tracks[track] = 0;
		}
	}

	int zero = 0;
	for (int i=1; i<(int)tracks.size(); i++) {
		if (state != 0) {
			tracks[i] = !tracks[i];
		}
		if (tracks[i]) {
			field.push_back(i);
			subfield.push_back(zero);
			model.push_back(zero);
		}
	}

}



//////////////////////////////
//
// Tool_extract::fillFieldDataByGrep --
//

void Tool_extract::fillFieldDataByGrep(vector<int>& field, vector<int>& subfield,
		vector<int>& model, const string& searchstring, HumdrumFile& infile,
		int state) {

	field.reserve(infile.getMaxTrack()+1);
	subfield.reserve(infile.getMaxTrack()+1);
	model.reserve(infile.getMaxTrack()+1);
	field.resize(0);
	subfield.resize(0);
	model.resize(0);

	vector<int> tracks;
	tracks.resize(infile.getMaxTrack()+1);
	fill(tracks.begin(), tracks.end(), 0);
	HumRegex hre;
	int track;

	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (hre.search(infile.token(i, j), searchstring, "")) {
				track = infile[i].token(j)->getTrack();
				tracks[track] = 1;
			}
		}
	}

	int zero = 0;
	for (i=1; i<(int)tracks.size(); i++) {
		if (state != 0) {
			tracks[i] = !tracks[i];
		}
		if (tracks[i]) {
			field.push_back(i);
			subfield.push_back(zero);
			model.push_back(zero);
		}
	}
}



//////////////////////////////
//
// Tool_extract::getInterpretationFields --
//

void Tool_extract::getInterpretationFields(vector<int>& field, vector<int>& subfield,
		vector<int>& model, HumdrumFile& infile, string& interps, int state) {
	vector<string> sstrings; // search strings
	sstrings.reserve(100);
	sstrings.resize(0);

	int i, j, k;
	string buffer;
	buffer = interps;

	HumRegex hre;
	hre.replaceDestructive(buffer, "", "\\s+", "g");

	int start = 0;
	while (hre.search(buffer, start, "^([^,]+)")) {
		sstrings.push_back(hre.getMatch(1));
		start = hre.getMatchEndIndex(1);
	}

	if (debugQ) {
		m_humdrum_text << "!! Interpretation strings to search for: " << endl;
		for (i=0; i<(int)sstrings.size(); i++) {
			m_humdrum_text << "!!\t" << sstrings[i] << endl;
		}
	}

	vector<int> tracks;
	tracks.resize(infile.getMaxTrack()+1);
	fill(tracks.begin(), tracks.end(), 0);

	// Algorithm below could be made more efficient by
	// not searching the entire file...
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			for (k=0; k<(int)sstrings.size(); k++) {
				if (sstrings[k] == *infile.token(i, j)) {
					tracks[infile[i].token(j)->getTrack()] = 1;
				}
			}
		}
	}

	field.reserve(tracks.size());
	subfield.reserve(tracks.size());
	model.reserve(tracks.size());

	field.resize(0);
	subfield.resize(0);
	model.resize(0);

	int zero = 0;
	for (i=1; i<(int)tracks.size(); i++) {
		if (state == 0) {
			tracks[i] = !tracks[i];
		}
		if (tracks[i]) {
			field.push_back(i);
			subfield.push_back(zero);
			model.push_back(zero);
		}
	}

}



//////////////////////////////
//
// Tool_extract::expandSpines --
//

void Tool_extract::expandSpines(vector<int>& field, vector<int>& subfield, vector<int>& model,
		HumdrumFile& infile, string& interp) {

	vector<int> splits;
	splits.resize(infile.getMaxTrack()+1);
	fill(splits.begin(), splits.end(), 0);

	int i, j;
	for (i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isManipulator()) {
			continue;
		}

		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (strchr(infile.token(i, j)->getSpineInfo().c_str(), '(') != NULL) {
				splits[infile[i].token(j)->getTrack()] = 1;
			}
		}
	}

	field.reserve(infile.getMaxTrack()*2);
	field.resize(0);

	subfield.reserve(infile.getMaxTrack()*2);
	subfield.resize(0);

	model.reserve(infile.getMaxTrack()*2);
	model.resize(0);

	int allQ = 0;
	if (interp.size() == 0) {
		allQ = 1;
	}

	// ggg
	vector<int> dummyfield;
	vector<int> dummysubfield;
	vector<int> dummymodel;
	getInterpretationFields(dummyfield, dummysubfield, model, infile, interp, 1);

	vector<int> interptracks;

	interptracks.resize(infile.getMaxTrack()+1);
	fill(interptracks.begin(), interptracks.end(), 0);

	for (i=0; i<(int)dummyfield.size(); i++) {
		interptracks[dummyfield[i]] = 1;
	}

	int aval = 'a';
	int bval = 'b';
	int zero = 0;
	for (i=1; i<(int)splits.size(); i++) {
		if (splits[i] && (allQ || interptracks[i])) {
			field.push_back(i);
			subfield.push_back(aval);
			model.push_back(zero);
			field.push_back(i);
			subfield.push_back(bval);
			model.push_back(zero);
		} else {
			field.push_back(i);
			subfield.push_back(zero);
			model.push_back(zero);
		}
	}

	if (debugQ) {
		m_humdrum_text << "!!expand: ";
		for (i=0; i<(int)field.size(); i++) {
			m_humdrum_text << field[i];
			if (subfield[i]) {
				m_humdrum_text << (char)subfield[i];
			}
			if (i < (int)field.size()-1) {
				m_humdrum_text << ",";
			}
		}
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_extract::reverseSpines -- reverse the order of spines, grouped by the
//   given exclusive interpretation.
//

void Tool_extract::reverseSpines(vector<int>& field, vector<int>& subfield,
		vector<int>& model, HumdrumFile& infile, const string& exinterp) {

	vector<int> target;
	target.resize(infile.getMaxTrack()+1);
	fill(target.begin(), target.end(), 0);

	vector<HTp> trackstarts;
	infile.getSpineStartList(trackstarts);

	for (int t=0; t<(int)trackstarts.size(); t++) {
		if (trackstarts[t]->isDataType(exinterp)) {
			target.at(t + 1) = 1;
		}
	}

	field.reserve(infile.getMaxTrack()*2);
	field.resize(0);

	int lasti = (int)target.size();
	for (int i=(int)target.size()-1; i>0; i--) {
		if (target[i]) {
			lasti = i;
			field.push_back(i);
			for (int j=i+1; j<(int)target.size(); j++) {
				if (!target.at(j)) {
					field.push_back(j);
				} else {
					break;
				}
			}
		}
	}

	// if the grouping spine is not first, then preserve the
	// locations of the pre-spines.
	int extras = 0;
	if (lasti != 1) {
		extras = lasti - 1;
		field.resize(field.size()+extras);
		for (int i=0; i<(int)field.size()-extras; i++) {
			field[(int)field.size()-1-i] = field[(int)field.size()-1-extras-i];
		}
		for (int i=0; i<extras; i++) {
			field[i] = i+1;
		}
	}

	if (debugQ) {
		m_humdrum_text << "!!reverse: ";
		for (int i=0; i<(int)field.size(); i++) {
			m_humdrum_text << field[i] << " ";
		}
		m_humdrum_text << endl;
	}

	subfield.resize(field.size());
	fill(subfield.begin(), subfield.end(), 0);

	model.resize(field.size());
	fill(model.begin(), model.end(), 0);
}



//////////////////////////////
//
// Tool_extract::fillFieldData --
//

void Tool_extract::fillFieldData(vector<int>& field, vector<int>& subfield,
		vector<int>& model, string& fieldstring, HumdrumFile& infile) {

	int maxtrack = infile.getMaxTrack();

	field.reserve(maxtrack);
	field.resize(0);

	subfield.reserve(maxtrack);
	subfield.resize(0);

	model.reserve(maxtrack);
	model.resize(0);

	HumRegex hre;
	string buffer = fieldstring;
	hre.replaceDestructive(buffer, "", "\\s", "gs");
	int start = 0;
	string tempstr;
	vector<int> tempfield;
	vector<int> tempsubfield;
	vector<int> tempmodel;
	while (hre.search(buffer,  start, "^([^,]+,?)")) {
		tempfield.clear();
		tempsubfield.clear();
		tempmodel.clear();
		processFieldEntry(tempfield, tempsubfield, tempmodel, hre.getMatch(1), infile);
		start += hre.getMatchEndIndex(1);
		field.insert(field.end(), tempfield.begin(), tempfield.end());
		subfield.insert(subfield.end(), tempsubfield.begin(), tempsubfield.end());
		model.insert(model.end(), tempmodel.begin(), tempmodel.end());
	}
}



//////////////////////////////
//
// Tool_extract::processFieldEntry --
//   3-6 expands to 3 4 5 6
//   $   expands to maximum spine track
//   $-1 expands to maximum spine track minus 1, etc.
//

void Tool_extract::processFieldEntry(vector<int>& field,
		vector<int>& subfield, vector<int>& model, const string& astring,
		HumdrumFile& infile) {

	int finitsize = (int)field.size();
	int maxtrack = infile.getMaxTrack();

	vector<HTp> ktracks;
	infile.getKernSpineStartList(ktracks);
	int maxkerntrack = (int)ktracks.size();

	int modletter;
	int subletter;

	HumRegex hre;
	string buffer = astring;

	// remove any comma left at end of input astring (or anywhere else)
	hre.replaceDestructive(buffer, "", ",", "g");

	// first remove $ symbols and replace with the correct values
	if (kernQ) {
		removeDollarsFromString(buffer, maxkerntrack);
	} else {
		removeDollarsFromString(buffer, maxtrack);
	}

	int zero = 0;
	if (hre.search(buffer, "^(\\d+)-(\\d+)$")) {
		int firstone = hre.getMatchInt(1);
		int lastone  = hre.getMatchInt(2);

		if ((firstone < 1) && (firstone != 0)) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at start: " << firstone << endl;
			m_error_text << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if ((lastone < 1) && (lastone != 0)) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at end: " << lastone << endl;
			m_error_text << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if (firstone > maxtrack) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains number too large at start: " << firstone << endl;
			m_error_text << "Maximum number allowed is " << maxtrack << endl;
			return;
		}
		if (lastone > maxtrack) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains number too large at end: " << lastone << endl;
			m_error_text << "Maximum number allowed is " << maxtrack << endl;
			return;
		}

		if (firstone > lastone) {
			for (int i=firstone; i>=lastone; i--) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		} else {
			for (int i=firstone; i<=lastone; i++) {
				field.push_back(i);
				subfield.push_back(zero);
				model.push_back(zero);
			}
		}
	} else if (hre.search(buffer, "^(\\d+)([a-z]*)")) {
		int value = hre.getMatchInt(1);
		modletter = 0;
		subletter = 0;
		if (hre.getMatch(2) ==  "a") {
			subletter = 'a';
		}
		if (hre.getMatch(2) ==  "b") {
			subletter = 'b';
		}
		if (hre.getMatch(2) ==  "c") {
			subletter = 'c';
		}
		if (hre.getMatch(2) ==  "d") {
			modletter = 'd';
		}
		if (hre.getMatch(2) ==  "n") {
			modletter = 'n';
		}
		if (hre.getMatch(2) ==  "r") {
			modletter = 'r';
		}

		if ((value < 1) && (value != 0)) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains too small a number at end: " << value << endl;
			m_error_text << "Minimum number allowed is " << 1 << endl;
			return;
		}
		if (value > maxtrack) {
			m_error_text << "Error: range token: \"" << astring << "\""
				  << " contains number too large at start: " << value << endl;
			m_error_text << "Maximum number allowed is " << maxtrack << endl;
			return;
		}
		field.push_back(value);
		if (value == 0) {
			subfield.push_back(zero);
			model.push_back(zero);
		} else {
			subfield.push_back(subletter);
			model.push_back(modletter);
		}
	}

	if (!kernQ) {
		return;
	}

	// Insert fields to next **kern spine.
	vector<int> newfield;
	vector<int> newsubfield;
	vector<int> newmodel;

	vector<HTp> trackstarts;
	infile.getTrackStartList(trackstarts);
	int i, j;
	int spine;

	// convert kern tracks into spine tracks:
	for (i=finitsize; i<(int)field.size(); i++) {
		if (field[i] > 0) {
			spine = ktracks[field[i]-1]->getTrack();
		   field[i] = spine;
		}
	}

	int startspineindex, stopspineindex;
	for (i=0; i<(int)field.size(); i++) {
		newfield.push_back(field[i]); // copy **kern spine index into new list
		newsubfield.push_back(subfield[i]);
		newmodel.push_back(model[i]);

		// search for non **kern spines after specified **kern spine:
		startspineindex = field[i] + 1 - 1;
		stopspineindex = maxtrack;
		for (j=startspineindex; j<stopspineindex; j++) {
			if (trackstarts[j]->isKern()) {
				break;
			}
			newfield.push_back(j+1);
			newsubfield.push_back(zero);
			newmodel.push_back(zero);
		}
	}

	field    = newfield;
	subfield = newsubfield;
	model    = newmodel;
}



//////////////////////////////
//
// Tool_extract::removeDollarsFromString -- substitute $ sign for maximum track count.
//

void Tool_extract::removeDollarsFromString(string& buffer, int maxtrack) {
	HumRegex hre;
	char buf2[128] = {0};
	int value2;

	if (hre.search(buffer, "\\$$")) {
		snprintf(buf2, 128, "%d", maxtrack);
		hre.replaceDestructive(buffer, buf2, "\\$$");
	}

	if (hre.search(buffer, "\\$(?![\\d-])")) {
		// don't know how this case could happen, however...
		snprintf(buf2, 128, "%d", maxtrack);
		hre.replaceDestructive(buffer, buf2, "\\$(?![\\d-])", "g");
	}

	if (hre.search(buffer, "\\$0")) {
		// replace $0 with maxtrack (used for reverse orderings)
		snprintf(buf2, 128, "%d", maxtrack);
		hre.replaceDestructive(buffer, buf2, "\\$0", "g");
	}

	while (hre.search(buffer, "\\$(-?\\d+)")) {
		value2 = maxtrack - abs(hre.getMatchInt(1));
		snprintf(buf2, 128, "%d", value2);
		hre.replaceDestructive(buffer, buf2, "\\$-?\\d+");
	}
}



//////////////////////////////
//
// Tool_extract::excludeFields -- print all spines except the ones in the list of fields.
//

void Tool_extract::excludeFields(HumdrumFile& infile, vector<int>& field,
		vector<int>& subfield, vector<int>& model) {
	int start = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << '\n';
			continue;
		} else {
			start = 0;
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				if (isInList(infile[i].token(j)->getTrack(), field)) {
					continue;
				}
				if (start != 0) {
					m_humdrum_text << '\t';
				}
				start = 1;
				m_humdrum_text << infile.token(i, j);
			}
			if (start != 0) {
				m_humdrum_text << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_extract::extractFields -- print all spines in the list of fields.
//

void Tool_extract::extractFields(HumdrumFile& infile, vector<int>& field,
		vector<int>& subfield, vector<int>& model) {

	HumRegex hre;
	int start = 0;
	int target;
	int subtarget;
	int modeltarget;
	string spat;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << '\n';
			continue;
		}

		if (infile[i].isManipulator()) {
			dealWithSpineManipulators(infile, i, field, subfield, model);
			continue;
		}

		start = 0;
		for (int t=0; t<(int)field.size(); t++) {
			target = field[t];
			subtarget = subfield[t];
			modeltarget = model[t];
			if (modeltarget == 0) {
				switch (subtarget) {
					 case 'a':
					 case 'b':
					    modeltarget = submodel;
					    break;
					 case 'c':
					    modeltarget = comodel;
				}
			}
			if (target == 0) {
				if (start != 0) {
					 m_humdrum_text << '\t';
				}
				start = 1;
				if (!infile[i].isManipulator()) {
					if (infile[i].isLocalComment()) {
						m_humdrum_text << "!";
					} else if (infile[i].isBarline()) {
						m_humdrum_text << infile[i].token(0);
					} else if (infile[i].isData()) {
				       m_humdrum_text << ".";
					    // interpretations handled in dealWithSpineManipulators()
					    // [obviously not, so adding a blank one here
					} else if (infile[i].isInterpretation()) {
					    m_humdrum_text << "*";
					 }
				}
			} else {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					 if (infile[i].token(j)->getTrack() != target) {
					    continue;
					 }
					 switch (subtarget) {
					 case 'a':
					    getSearchPat(spat, target, "a");
					    if (hre.search(infile.token(i,j)->getSpineInfo(), spat) ||
					          !hre.search(infile.token(i, j)->getSpineInfo(), "\\(")) {
					       if (start != 0) {
					          m_humdrum_text << '\t';
					       }
					       start = 1;
					       m_humdrum_text << infile.token(i, j);
					    }
					    break;
					 case 'b':
					    getSearchPat(spat, target, "b");
					    if (hre.search(infile.token(i, j)->getSpineInfo(), spat)) {
					       if (start != 0) {
					          m_humdrum_text << '\t';
					       }
					       start = 1;
					       m_humdrum_text << infile.token(i, j);
					    } else if (!hre.search(infile.token(i, j)->getSpineInfo(),
					          "\\(")) {
					       if (start != 0) {
					          m_humdrum_text << '\t';
					       }
					       start = 1;
					       dealWithSecondarySubspine(field, subfield, model, t,
					             infile, i, j, modeltarget);
					    }
					    break;
					 case 'c':
					    if (start != 0) {
					       m_humdrum_text << '\t';
					    }
					    start = 1;
					    dealWithCospine(field, subfield, model, t, infile, i, j,
					       modeltarget, modeltarget, cointerp);
					    break;
					 default:
					    if (start != 0) {
					       m_humdrum_text << '\t';
					    }
					    start = 1;
					    m_humdrum_text << infile.token(i, j);
					 }
				}
			}
		}
		if (start != 0) {
			m_humdrum_text << endl;
		}
	}
}



//////////////////////////////
//
// Tool_extract::dealWithCospine -- extract the required token(s) from a co-spine.
//

void Tool_extract::dealWithCospine(vector<int>& field, vector<int>& subfield, vector<int>& model,
		int targetindex, HumdrumFile& infile, int line, int cospine,
		int comodel, int submodel, const string& cointerp) {

	vector<string> cotokens;
	cotokens.reserve(50);

	string buffer;
	int i, j, k;
	int index;

	if (infile[line].isInterpretation()) {
		m_humdrum_text << infile.token(line, cospine);
		return;
	}

	if (infile[line].isBarline()) {
		m_humdrum_text << infile.token(line, cospine);
		return;
	}

	if (infile[line].isLocalComment()) {
		m_humdrum_text << infile.token(line, cospine);
		return;
	}

	int count = infile[line].token(cospine)->getSubtokenCount();
	for (k=0; k<count; k++) {
		buffer = infile.token(line, cospine)->getSubtoken(k);
		cotokens.resize(cotokens.size()+1);
		index = (int)cotokens.size()-1;
		cotokens[index] = buffer;
	}

	vector<int> spineindex;
	vector<int> subspineindex;

	spineindex.reserve(infile.getMaxTrack()*2);
	spineindex.resize(0);

	subspineindex.reserve(infile.getMaxTrack()*2);
	subspineindex.resize(0);

	for (j=0; j<infile[line].getFieldCount(); j++) {
		if (infile.token(line, j)->isDataType(cointerp)) {
			continue;
		}
		if (*infile.token(line, j) == ".") {
			continue;
		}
		count = infile[line].token(j)->getSubtokenCount();
		for (k=0; k<count; k++) {
			buffer = infile[line].token(j)->getSubtoken(k);
			if (comodel == 'r') {
				if (buffer == "r") {
					continue;
				}
			}
			spineindex.push_back(j);
			subspineindex.push_back(k);
		}
	}

	if (debugQ) {
		m_humdrum_text << "\n!!codata:\n";
		for (i=0; i<(int)cotokens.size(); i++) {
			m_humdrum_text << "!!\t" << i << "\t" << cotokens[i];
			if (i < (int)spineindex.size()) {
				m_humdrum_text << "\tspine=" << spineindex[i];
				m_humdrum_text << "\tsubspine=" << subspineindex[i];
			} else {
				m_humdrum_text << "\tspine=.";
				m_humdrum_text << "\tsubspine=.";
			}
			m_humdrum_text << endl;
		}
	}

	string buff;

	int start = 0;
	for (i=0; i<(int)field.size(); i++) {
		if (infile.token(line, field[i])->isDataType(cointerp)) {
			continue;
		}

		for (j=0; j<infile[line].getFieldCount(); j++) {
			if (infile[line].token(j)->getTrack() != field[i]) {
				continue;
			}
			if (subfield[i] == 'a') {
				getSearchPat(buff, field[i], "a");
				if ((strchr(infile.token(line, j)->getSpineInfo().c_str(), '(') == NULL) ||
					(infile.token(line, j)->getSpineInfo().find(buff) != string::npos)) {
					printCotokenInfo(start, infile, line, j, cotokens, spineindex,
							 subspineindex);
				}
			} else if (subfield[i] == 'b') {
				// this section may need more work...
				getSearchPat(buff, field[i], "b");
				if ((strchr(infile.token(line, j)->getSpineInfo().c_str(), '(') == NULL) ||
					(strstr(infile.token(line, j)->getSpineInfo().c_str(), buff.c_str()) != NULL)) {
					printCotokenInfo(start, infile, line, j, cotokens, spineindex,
							 subspineindex);
				}
			} else {
				printCotokenInfo(start, infile, line, j, cotokens, spineindex,
					subspineindex);
			}
		}
	}
}



//////////////////////////////
//
// Tool_extract::printCotokenInfo --
//

void Tool_extract::printCotokenInfo(int& start, HumdrumFile& infile, int line, int spine,
		vector<string>& cotokens, vector<int>& spineindex,
		vector<int>& subspineindex) {
	int i;
	int found = 0;
	for (i=0; i<(int)spineindex.size(); i++) {
		if (spineindex[i] == spine) {
			if (start == 0) {
				start++;
			} else {
				m_humdrum_text << subtokenseparator;
			}
			if (i<(int)cotokens.size()) {
				m_humdrum_text << cotokens[i];
			} else {
				m_humdrum_text << ".";
			}
		found = 1;
		}
	}
	if (!found) {
		if (start == 0) {
			start++;
		} else {
			m_humdrum_text << subtokenseparator;
		}
		m_humdrum_text << ".";
	}
}



//////////////////////////////
//
// Tool_extract::dealWithSecondarySubspine -- what to print if a secondary spine
//     does not exist on a line.
//

void Tool_extract::dealWithSecondarySubspine(vector<int>& field, vector<int>& subfield,
		vector<int>& model, int targetindex, HumdrumFile& infile, int line,
		int spine, int submodel) {

	int& i = line;
	int& j = spine;

	HumRegex hre;
	string buffer;
	if (infile[line].isLocalComment()) {
		if ((submodel == 'n') || (submodel == 'r')) {
			m_humdrum_text << "!";
		} else {
			m_humdrum_text << infile.token(i, j);
		}
	} else if (infile[line].isBarline()) {
		m_humdrum_text << infile.token(i, j);
	} else if (infile[line].isInterpretation()) {
		if ((submodel == 'n') || (submodel == 'r')) {
			m_humdrum_text << "*";
		} else {
			m_humdrum_text << infile.token(i, j);
		}
	} else if (infile[line].isData()) {
		if (submodel == 'n') {
			m_humdrum_text << ".";
		} else if (submodel == 'r') {
			if (*infile.token(i, j) == ".") {
				m_humdrum_text << ".";
			} else if (infile.token(i, j)->find('q') != string::npos) {
				m_humdrum_text << ".";
			} else if (infile.token(i, j)->find('Q') != string::npos) {
				m_humdrum_text << ".";
			} else {
				buffer = *infile.token(i, j);
				if (hre.search(buffer, "{")) {
					m_humdrum_text << "{";
				}
				// remove secondary chord notes:
				hre.replaceDestructive(buffer, "", " .*");
				// remove unnecessary characters (such as stem direction):
				hre.replaceDestructive(buffer, "",
						"[^}pPqQA-Ga-g0-9.;%#nr-]", "g");
				// change pitch to rest:
				hre.replaceDestructive(buffer, "[A-Ga-g#n-]+", "r");
				// add editorial marking unless -Y option is given:
				if (editorialInterpretation != "") {
					if (hre.search(buffer, "rr")) {
						 hre.replaceDestructive(buffer, editorialInterpretation, "(?<=rr)");
						 hre.replaceDestructive(buffer, "r", "rr");
					} else {
						 hre.replaceDestructive(buffer, editorialInterpretation, "(?<=r)");
					}
				}
				m_humdrum_text << buffer;
			}
		} else {
			m_humdrum_text << infile.token(i, j);
		}
	} else {
		m_error_text << "Should not get to this line of code" << endl;
		return;
	}
}




//////////////////////////////
//
// Tool_extract::getSearchPat --
//

void Tool_extract::getSearchPat(string& spat, int target, const string& modifier) {
	if (modifier.size() > 20) {
		m_error_text << "Error in GetSearchPat" << endl;
		return;
	}
	spat.reserve(16);
	spat = "\\(";
	spat += to_string(target);
	spat += "\\)";
	spat += modifier;
}



//////////////////////////////
//
// Tool_extract::dealWithSpineManipulators -- check for proper Humdrum syntax of
//     spine manipulators (**, *-, *x, *v, *^) when creating the output.
//

void Tool_extract::dealWithSpineManipulators(HumdrumFile& infile, int line,
		vector<int>& field, vector<int>& subfield, vector<int>& model) {

	vector<int> vmanip;  // counter for *v records on line
	vmanip.resize(infile[line].getFieldCount());
	fill(vmanip.begin(), vmanip.end(), 0);

	vector<int> xmanip; // counter for *x record on line
	xmanip.resize(infile[line].getFieldCount());
	fill(xmanip.begin(), xmanip.end(), 0);

	int i = 0;
	int j;
	for (j=0; j<(int)vmanip.size(); j++) {
		if (*infile.token(line, j) == "*v") {
			vmanip[j] = 1;
		}
		if (*infile.token(line, j) == "*x") {
			xmanip[j] = 1;
		}
	}

	int counter = 1;
	for (i=1; i<(int)xmanip.size(); i++) {
		if ((xmanip[i] == 1) && (xmanip[i-1] == 1)) {
			xmanip[i] = counter;
			xmanip[i-1] = counter;
			counter++;
		}
	}

	counter = 1;
	i = 0;
	while (i < (int)vmanip.size()) {
		if (vmanip[i] == 1) {
			while ((i < (int)vmanip.size()) && (vmanip[i] == 1)) {
				vmanip[i] = counter;
				i++;
			}
			counter++;
		}
		i++;
	}

	vector<int> fieldoccur;  // nth occurance of an input spine in the output
	fieldoccur.resize(field.size());
	fill(fieldoccur.begin(), fieldoccur.end(), 0);

	vector<int> trackcounter; // counter of input spines occurances in output
	trackcounter.resize(infile.getMaxTrack()+1);
	fill(trackcounter.begin(), trackcounter.end(), 0);

	for (i=0; i<(int)field.size(); i++) {
		if (field[i] != 0) {
			trackcounter[field[i]]++;
			fieldoccur[i] = trackcounter[field[i]];
		}
	}

	vector<string> tempout;
	vector<int> vserial;
	vector<int> xserial;
	vector<int> fpos;     // input column of output spine

	tempout.reserve(1000);
	tempout.resize(0);

	vserial.reserve(1000);
	vserial.resize(0);

	xserial.reserve(1000);
	xserial.resize(0);

	fpos.reserve(1000);
	fpos.resize(0);

	string spat;
	string spinepat;
	HumRegex hre;
	int subtarget;
	int modeltarget;
	int xdebug = 0;
	int vdebug = 0;
	int suppress = 0;
	int target;
	int tval;
	for (int t=0; t<(int)field.size(); t++) {
		target = field[t];
		subtarget = subfield[t];
		modeltarget = model[t];
		if (modeltarget == 0) {
			switch (subtarget) {
				case 'a':
				case 'b':
					modeltarget = submodel;
					break;
				case 'c':
					modeltarget = comodel;
			}
		}
		suppress = 0;
		if (target == 0) {
			if (infile.token(line, 0)->compare(0, 2, "**") == 0) {
				storeToken(tempout, blankName);
				tval = 0;
				vserial.push_back(tval);
				xserial.push_back(tval);
				fpos.push_back(tval);
			} else if (*infile.token(line, 0) == "*-") {
				storeToken(tempout, "*-");
				tval = 0;
				vserial.push_back(tval);
				xserial.push_back(tval);
				fpos.push_back(tval);
			} else {
				storeToken(tempout, "*");
				tval = 0;
				vserial.push_back(tval);
				xserial.push_back(tval);
				fpos.push_back(tval);
			}
		} else {
			for (j=0; j<infile[line].getFieldCount(); j++) {
				if (infile[line].token(j)->getTrack() != target) {
					continue;
				}
		 // filter by subfield
		 if (subtarget == 'a') {
					getSearchPat(spat, target, "b");
			 if (hre.search(infile.token(line, j)->getSpineInfo(), spat)) {
						continue;
			 }
		 } else if (subtarget == 'b') {
					getSearchPat(spat, target, "a");
			 if (hre.search(infile.token(line, j)->getSpineInfo(), spat)) {
						continue;
					}
				}

				switch (subtarget) {
				case 'a':

					if (!hre.search(infile.token(line, j)->getSpineInfo(), "\\(")) {
						if (*infile.token(line, j)  == "*^") {
							 storeToken(tempout, "*");
						} else {
							 storeToken(tempout, *infile.token(line, j));
						}
					} else {
						getSearchPat(spat, target, "a");
						spinepat =  infile.token(line, j)->getSpineInfo();
						hre.replaceDestructive(spinepat, "\\(", "\\(", "g");
						hre.replaceDestructive(spinepat, "\\)", "\\)", "g");

						if ((*infile.token(line, j) == "*v") &&
							    (spinepat == spat)) {
							 storeToken(tempout, "*");
						} else {
							 getSearchPat(spat, target, "b");
							 if ((spinepat == spat) &&
							       (*infile.token(line, j) ==  "*v")) {
							    // do nothing
							    suppress = 1;
							 } else {
							    storeToken(tempout, *infile.token(line, j));
							 }
						}
					}

					break;
				case 'b':

					if (!hre.search(infile.token(line, j)->getSpineInfo(), "\\(")) {
						if (*infile.token(line, j) == "*^") {
							 storeToken(tempout, "*");
						} else {
							 storeToken(tempout, *infile.token(line, j));
						}
					} else {
						getSearchPat(spat, target, "b");
						spinepat = infile.token(line, j)->getSpineInfo();
						hre.replaceDestructive(spinepat, "\\(", "\\(", "g");
						hre.replaceDestructive(spinepat, "\\)", "\\)", "g");

						if ((*infile.token(line, j) ==  "*v") &&
							    (spinepat == spat)) {
							 storeToken(tempout, "*");
						} else {
							 getSearchPat(spat, target, "a");
							 if ((spinepat == spat) &&
							       (*infile.token(line, j) == "*v")) {
							    // do nothing
							    suppress = 1;
							 } else {
							    storeToken(tempout, *infile.token(line, j));
							 }
						}
					}

					break;
				case 'c':
					// work on later
					storeToken(tempout, *infile.token(line, j));
					break;
				default:
					storeToken(tempout, *infile.token(line, j));
				}

				if (suppress) {
					continue;
				}

				if (tempout[(int)tempout.size()-1] == "*x") {
					tval = fieldoccur[t] * 1000 + xmanip[j];
					xserial.push_back(tval);
					xdebug = 1;
				} else {
					tval = 0;
					xserial.push_back(tval);
				}

				if (tempout[(int)tempout.size()-1] == "*v") {
					tval = fieldoccur[t] * 1000 + vmanip[j];
					vserial.push_back(tval);
					vdebug = 1;
				} else {
					tval = 0;
					vserial.push_back(tval);
				}

				fpos.push_back(j);

			}
		}
	}

	if (debugQ && xdebug) {
		m_humdrum_text << "!! *x serials = ";
		for (int ii=0; ii<(int)xserial.size(); ii++) {
			m_humdrum_text << xserial[ii] << " ";
		}
		m_humdrum_text << "\n";
	}

	if (debugQ && vdebug) {
		m_humdrum_text << "!!LINE: " << infile[line] << endl;
		m_humdrum_text << "!! *v serials = ";
		for (int ii=0; ii<(int)vserial.size(); ii++) {
			m_humdrum_text << vserial[ii] << " ";
		}
		m_humdrum_text << "\n";
	}

	// check for proper *x syntax /////////////////////////////////
	for (i=0; i<(int)xserial.size()-1; i++) {
		if (!xserial[i]) {
			continue;
		}
		if (xserial[i] != xserial[i+1]) {
			if (tempout[i] == "*x") {
				xserial[i] = 0;
				tempout[i] = "*";
			}
		} else {
			i++;
		}
	}

	if ((tempout.size() == 1) || (xserial.size() == 1)) {
		// get rid of *x if there is only one spine in output
		if (xserial[0]) {
			xserial[0] = 0;
			tempout[0] = "*";
		}
	} else if ((int)xserial.size() > 1) {
		// check the last item in the list
		int index = (int)xserial.size()-1;
		if (tempout[index] == "*x") {
			if (xserial[index] != xserial[index-1]) {
				xserial[index] = 0;
				tempout[index] = "*";
			}
		}
	}

	// check for proper *v syntax /////////////////////////////////
	vector<int> vsplit;
	vsplit.resize((int)vserial.size());
	fill(vsplit.begin(), vsplit.end(), 0);

	// identify necessary line splits
	for (i=0; i<(int)vserial.size()-1; i++) {
		if (!vserial[i]) {
			continue;
		}
		while ((i<(int)vserial.size()-1) && (vserial[i]==vserial[i+1])) {
			i++;
		}
		if ((i<(int)vserial.size()-1) && vserial[i]) {
			if (vserial.size() > 1) {
				if (vserial[i+1]) {
					vsplit[i+1] = 1;
				}
			}
		}
	}

	// remove single *v spines:

	for (i=0; i<(int)vsplit.size()-1; i++) {
		if (vsplit[i] && vsplit[i+1]) {
			if (tempout[i] == "*v") {
				tempout[i] = "*";
				vsplit[i] = 0;
			}
		}
	}

	if (debugQ) {
		m_humdrum_text << "!!vsplit array: ";
		for (i=0; i<(int)vsplit.size(); i++) {
			m_humdrum_text << " " << vsplit[i];
		}
		m_humdrum_text << endl;
	}

	if (vsplit.size() > 0) {
		if (vsplit[(int)vsplit.size()-1]) {
			if (tempout[(int)tempout.size()-1] == "*v") {
				tempout[(int)tempout.size()-1] = "*";
				vsplit[(int)vsplit.size()-1] = 0;
			}
		}
	}

	int vcount = 0;
	for (i=0; i<(int)vsplit.size(); i++) {
		vcount += vsplit[i];
	}

	if (vcount) {
		printMultiLines(vsplit, vserial, tempout);
	}

	int start = 0;
	for (i=0; i<(int)tempout.size(); i++) {
		if (tempout[i] != "") {
			if (start != 0) {
				m_humdrum_text << "\t";
			}
			m_humdrum_text << tempout[i];
			start++;
		}
	}
	if (start) {
		m_humdrum_text << '\n';
	}
}



//////////////////////////////
//
// Tool_extract::printMultiLines -- print separate *v lines.
//

void Tool_extract::printMultiLines(vector<int>& vsplit, vector<int>& vserial,
		vector<string>& tempout) {
	int i;

	int splitpoint = -1;
	for (i=0; i<(int)vsplit.size(); i++) {
		if (vsplit[i]) {
			splitpoint = i;
			break;
		}
	}

	if (debugQ) {
		m_humdrum_text << "!!tempout: ";
		for (i=0; i<(int)tempout.size(); i++) {
			m_humdrum_text << tempout[i] << " ";
		}
		m_humdrum_text << endl;
	}

	if (splitpoint == -1) {
		return;
	}

	int start = 0;
	int printv = 0;
	for (i=0; i<splitpoint; i++) {
		if (tempout[i] != "") {
			if (start) {
				m_humdrum_text << "\t";
			}
			m_humdrum_text << tempout[i];
			start = 1;
			if (tempout[i] == "*v") {
				if (printv) {
					tempout[i] = "";
				} else {
					tempout[i] = "*";
					printv = 1;
				}
			} else {
				tempout[i] = "*";
			}
		}
	}

	for (i=splitpoint; i<(int)vsplit.size(); i++) {
		if (tempout[i] != "") {
			if (start) {
				m_humdrum_text << "\t";
			}
			m_humdrum_text << "*";
		}
	}

	if (start) {
		m_humdrum_text << "\n";
	}

	vsplit[splitpoint] = 0;

	printMultiLines(vsplit, vserial, tempout);
}



//////////////////////////////
//
// Tool_extract::storeToken --
//

void Tool_extract::storeToken(vector<string>& storage, const string& string) {
	storage.push_back(string);
}

void storeToken(vector<string>& storage, int index, const string& string) {
	storage[index] = string;
}



//////////////////////////////
//
// Tool_extract::isInList -- returns true if first number found in list of numbers.
//     returns the matching index plus one.
//

int Tool_extract::isInList(int number, vector<int>& listofnum) {
	int i;
	for (i=0; i<(int)listofnum.size(); i++) {
		if (listofnum[i] == number) {
			return i+1;
		}
	}
	return 0;

}



//////////////////////////////
//
// Tool_extract::getTraceData --
//

void Tool_extract::getTraceData(vector<int>& startline, vector<vector<int> >& fields,
		const string& tracefile, HumdrumFile& infile) {
	char buffer[1024] = {0};
	HumRegex hre;
	int linenum;
	startline.reserve(10000);
	startline.resize(0);
	fields.reserve(10000);
	fields.resize(0);

	ifstream input;
	input.open(tracefile.c_str());
	if (!input.is_open()) {
		m_error_text << "Error: cannot open file for reading: " << tracefile << endl;
		return;
	}

	string temps;
	vector<int> field;
	vector<int> subfield;
	vector<int> model;

	input.getline(buffer, 1024);
	while (!input.eof()) {
		if (hre.search(buffer, "^\\s*$")) {
			continue;
		}
		if (!hre.search(buffer, "(\\d+)")) {
			continue;
		}
		linenum = hre.getMatchInt(1);
		linenum--;  // adjust so that line 0 is the first line in the file
		temps = buffer;
		hre.replaceDestructive(temps, "", "\\d+");
		hre.replaceDestructive(temps, "", "[^,\\s\\d\\$\\-].*");  // remove any possible comments
		hre.replaceDestructive(temps, "", "\\s", "g");
		if (hre.search(temps, "^\\s*$")) {
			// no field data to process online
			continue;
		}
		startline.push_back(linenum);
		string ttemp = temps;
		fillFieldData(field, subfield, model, ttemp, infile);
		fields.push_back(field);
		input.getline(buffer, 1024);
	}

}



//////////////////////////////
//
// Tool_extract::extractTrace --
//

void Tool_extract::extractTrace(HumdrumFile& infile, const string& tracefile) {
	vector<int> startline;
	vector<vector<int> > fields;
	getTraceData(startline, fields, tracefile, infile);
	int i, j;

	if (debugQ) {
		for (i=0; i<(int)startline.size(); i++) {
			m_humdrum_text << "!!TRACE " << startline[i]+1 << ":\t";
			for (j=0; j<(int)fields[i].size(); j++) {
				m_humdrum_text << fields[i][j] << " ";
			}
			m_humdrum_text << "\n";
		}
	}


	if (startline.size() == 0) {
		for (i=0; i<infile.getLineCount(); i++) {
			if (!infile[i].hasSpines()) {
				m_humdrum_text << infile[i] << '\n';
			}
		}
		return;
	}

	for (i=0; i<startline[0]; i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << '\n';
		}
	}

	int endline;
	for (j=0; j<(int)startline.size(); j++) {
		if (j == (int)startline.size()-1) {
			endline = infile.getLineCount()-1;
		} else {
			endline = startline[j+1]-1;
		}
		for (i=startline[j]; i<endline; i++) {
			if (!infile[i].hasSpines()) {
				m_humdrum_text << infile[i] << '\n';
			} else {
				printTraceLine(infile, i, fields[j]);
			}
		}
	}
}



//////////////////////////////
//
// Tool_extract::printTraceLine --
//

void Tool_extract::printTraceLine(HumdrumFile& infile, int line, vector<int>& field) {
	int j;
	int t;
	int start = 0;
	int target;

	start = 0;
	for (t=0; t<(int)field.size(); t++) {
		target = field[t];
		for (j=0; j<infile[line].getFieldCount(); j++) {
			if (infile[line].token(j)->getTrack() != target) {
				continue;
			}
			if (start != 0) {
				m_humdrum_text << '\t';
			}
			start = 1;
			m_humdrum_text << infile.token(line, j);
		}
	}
	if (start != 0) {
		m_humdrum_text << endl;
	}
}



//////////////////////////////
//
// Tool_extract::example -- example usage of the sonority program
//

void Tool_extract::example(void) {
	m_free_text <<
	"					                                                          \n"
	<< endl;
}



//////////////////////////////
//
// Tool_extract::usage -- gives the usage statement for the sonority program
//

void Tool_extract::usage(const string& command) {
	m_free_text <<
	"					                                                          \n"
	<< endl;
}



//////////////////////////////
//
// Tool_extract::initialize --
//

void Tool_extract::initialize(HumdrumFile& infile) {
	// handle basic options:
	if (getBoolean("author")) {
		m_free_text << "Written by Craig Stuart Sapp, "
			  << "craig@ccrma.stanford.edu, Feb 2008" << endl;
		return;
	} else if (getBoolean("version")) {
		m_free_text << getArg(0) << ", version: Feb 2008" << endl;
		m_free_text << "compiled: " << __DATE__ << endl;
		return;
	} else if (getBoolean("help")) {
		usage(getCommand().c_str());
		return;
	} else if (getBoolean("example")) {
		example();
		return;
	}

	excludeQ    = getBoolean("x");
	interpQ     = getBoolean("i");
	interps     = getString("i");
	kernQ       = getBoolean("k");
	rkernQ      = getBoolean("K");

	interpstate = 1;
	if (!interpQ) {
		interpQ = getBoolean("I");
		interpstate = 0;
		interps = getString("I");
	}
	if (interps.size() > 0) {
		if (interps[0] != '*') {
			// Automatically add ** if not given on exclusive interpretation
			string tstring = "**";
			interps = tstring + interps;
		}
	}

	removerestQ = getBoolean("no-rest");
	noEmptyQ    = getBoolean("no-empty");
	emptyQ      = getBoolean("empty");
	fieldQ      = getBoolean("f");
	debugQ      = getBoolean("debug");
	countQ      = getBoolean("count");
	traceQ      = getBoolean("trace");
	tracefile   = getString("trace");
	reverseQ    = getBoolean("reverse");
	expandQ     = getBoolean("expand") || getBoolean("E");
	submodel    = getString("model").c_str()[0];
	cointerp    = getString("cointerp");
	comodel     = getString("cospine-model").c_str()[0];

	if (getBoolean("no-editoral-rests")) {
		editorialInterpretation = "";
	}

	if (interpQ) {
		fieldQ = 1;
	}

	if (emptyQ) {
		fieldQ = 1;
	}

	if (noEmptyQ) {
		fieldQ = 1;
	}

	if (expandQ) {
		fieldQ = 1;
		expandInterp = getString("expand-interp");
	}

	if (!reverseQ) {
		reverseQ = getBoolean("R");
		if (reverseQ) {
			reverseInterp = getString("R");
		}
	}

	if (reverseQ) {
		fieldQ = 1;
	}

	if (excludeQ) {
		fieldstring = getString("x");
	} else if (fieldQ) {
		fieldstring = getString("f");
	} else if (kernQ) {
		fieldstring = getString("k");
		fieldQ = 1;
	} else if (rkernQ) {
		fieldstring = getString("K");
		fieldQ = 1;
		fieldstring = reverseFieldString(fieldstring, infile.getMaxTrack());
	}

	spineListQ = getBoolean("spine-list");
	grepQ = getBoolean("grep");
	grepString = getString("grep");

	if (getBoolean("name")) {
		blankName = getString("name");
		if (blankName == "") {
			blankName = "**blank";
		} else if (blankName.compare(0, 2, "**") != 0) {
			if (blankName.compare(0, 1, "*") != 0) {
				blankName = "**" + blankName;
			} else {
				blankName = "*" + blankName;
			}
		}
	}

}


//////////////////////////////
//
// Tool_extract::reverseFieldString --  No dollar expansion for now.
//

string Tool_extract::reverseFieldString(const string& input, int maxval) {
	string output;
	string number;
	for (int i=0; i<(int)input.size(); i++) {
		if (isdigit(input[i])) {
			number += input[i];
			continue;
		} else {
			if (!number.empty()) {
				int value = strtol(number.c_str(), NULL, 10);
				value = maxval - value + 1;
				output += to_string(value);
				output += input[i];
				number.clear();
			}
		}
	}
	if (!number.empty()) {
		int value = strtol(number.c_str(), NULL, 10);
		value = maxval - value + 1;
		output += to_string(value);
	}
	return output;
}


// END_MERGE

} // end namespace hum



