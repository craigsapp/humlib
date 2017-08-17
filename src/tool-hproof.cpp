//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug  7 20:13:37 EDT 2017
// Last Modified: Wed Aug 16 07:14:55 EDT 2017
// Filename:      tool-hproof.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-hproof.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   **harm proof-reading tool.
//
//

#include "tool-hproof.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_hproof -- Set the recognized options for the tool.
//

Tool_hproof::Tool_hproof(void) {
	// put option definitions here
}



///////////////////////////////
//
// Tool_hproof::run -- Primary interfaces to the tool.
//

bool Tool_hproof::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_hproof::run(HumdrumFile& infile, ostream& out) {
	int status = run(infile);
	out << infile;
	return status;
}


bool Tool_hproof::run(HumdrumFile& infile) {
	markNonChordTones(infile);
	infile.appendLine("!!!RDF**kern: N = marked note, color=chocolate (non-chord tone)");
	infile.appendLine("!!!RDF**kern: Z = marked note, color=black (chord tone)");
	infile.createLinesFromTokens();
	return true;
}



//////////////////////////////
//
// Tool_hproof::markNonChordTones -- Mark
//

void Tool_hproof::markNonChordTones(HumdrumFile& infile) {
	vector<HTp> list;
	infile.getSpineStartList(list);
	vector<HTp> hlist;
	for (auto it : list) {
		if (*it == "**harm") {
			hlist.push_back(it);
		}
		if (*it == "**rhrm") {
			hlist.push_back(it);
		}
	}
	if (hlist.empty()) {
		cerr << "Warning: No **harm or **rhrm spines in data" << endl;
		return;
	}
	
	processHarmSpine(infile, hlist[0]);
}



//////////////////////////////
//
// processHarmSpine --
//

void Tool_hproof::processHarmSpine(HumdrumFile& infile, HTp hstart) {
	string key = "*C:";  // assume C major if no key designation
	HTp token = hstart;
	HTp ntoken = token->getNextNNDT();
	while (token) {
		markNotesInRange(infile, token, ntoken, key);
		if (!ntoken) {
			break;
		}
		if (ntoken && token) {
			getNewKey(token, ntoken, key);
		}
		token = ntoken;
		ntoken = ntoken->getNextNNDT();
	}
}



//////////////////////////////
//
// Tool_hproof::getNewKey --
//

void Tool_hproof::getNewKey(HTp token, HTp ntoken, string& key) {
	token = token->getNextToken();
	while (token && (token != ntoken)) {
		if (token->isKeyDesignation()) {
			key = *token;
		}
		token = token->getNextToken();
	}
}



//////////////////////////////
//
// Tool_hproof::markNotesInRange --
//

void Tool_hproof::markNotesInRange(HumdrumFile& infile, HTp ctoken, HTp ntoken, const string& key) {
	if (!ctoken) {
		return;
	}
	int startline = ctoken->getLineIndex();
	int stopline = infile.getLineCount();
	if (ntoken) {
		stopline = ntoken->getLineIndex();
	}
	vector<int> cts;
	cts = Convert::harmToBase40(ctoken, key);
	for (int i=startline; i<stopline; i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			if (!infile.token(i, j)->isKern()) {
				continue;
			}
			HTp tok = infile.token(i, j);
			if (tok->isNull()) {
				continue;
			}
			if (tok->isRest()) {
				continue;
			}
			markHarmonicTones(tok, cts);
		}
	}

// cerr << "TOK\t" << ctoken << "\tLINES\t" << startline << "\t" << stopline << "\t";
// for (int i=0; i<cts.size(); i++) {
// cerr << " " << Convert::base40ToKern(cts[i]);
// }
// cerr << endl;

}



//////////////////////////////
//
// Tool_hproof::markHarmonicTones --
//

void Tool_hproof::markHarmonicTones(HTp tok, vector<int>& cts) {
	int count = tok->getSubtokenCount();
	vector<int> notes = cts;
	string output;
	for (int i=0; i<count; i++) {
		string subtok = tok->getSubtoken(i);
		int pitch = Convert::kernToBase40(subtok);
		if (i > 0) {
			output += " ";
		}
		bool found = false;
		for (int j=0; j<(int)cts.size(); j++) {
			if (pitch % 40 == cts[j] % 40) {
				output += subtok;
				output += "Z";
				found = true;
				break;
			}
		}
		if (!found) {
			output += subtok;
			output += "N";
		}
	}
	tok->setText(output);
}



// END_MERGE

} // end namespace hum



