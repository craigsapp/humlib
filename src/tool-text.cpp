//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Last Modified: Thu Jan 30 22:26:35 PST 2020
// Filename:      tool-text.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-text.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Switch between text encoding and corrected encoding.
//

#include "tool-text.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_text::Tool_text -- Set the recognized options for the tool.
//

Tool_text::Tool_text(void) {
	define("1|first=b",           "Display only first verse for each part");
	define("a|above=b",           "Show above the music");
	define("b|below=b",           "Show below the music");
	define("C|no-count=b",        "Do not count syllables");
	define("y|hyphen=b",          "Do not join syllables into single word");
	define("M|no-merge=b",        "Do not merge syllables");
	define("r|remove-first=b",    "Remove text from music, except first verse");
	define("T|remove-all=b",      "Remove all text from music");
	define("raw=b",               "Show only text, no line or rhyme");
	define("ref|refrain|rline=b", "Show rline only");
	define("verse|ver|plint=b",   "Show pline only");
	define("v|show-verse=b",      "Force display of verse/refrain labels (if output is raw)");
	define("R|norep|repeats|no-repeats=b", "Suppress repeated plines ending in r");
}



/////////////////////////////////
//
// Tool_text::run -- Do the main work of the tool.
//

bool Tool_text::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_text::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_text::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}



bool Tool_text::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_text::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_text::initialize(void) {
	m_onlyQ        =  getBoolean("first");
	m_aboveQ       =  getBoolean("above");
	m_joinQ        = !getBoolean("hyphen");
	m_removeQ      =  getBoolean("remove-first");
	m_removeAllQ   =  getBoolean("remove-all");
	m_rawQ         =  getBoolean("raw");
	m_refrainOnlyQ =  getBoolean("refrain");
cerr << "REFRAIN " << m_refrainOnlyQ << endl;
	m_verseOnlyQ   =  getBoolean("verse");
	m_repeatsQ     =  getBoolean("no-repeats");
	m_countQ       = !getBoolean("no-count");
	m_showVerseQ   = true;
	if (m_rawQ) {
		m_showVerseQ = false;
		m_showVerseQ = getBoolean("show-verse");
	}
}



//////////////////////////////
//
// Tool_text::processFile --
//

void Tool_text::processFile(HumdrumFile& infile) {
	removeText(infile);
	m_humdrum_text << infile;
	m_humdrum_text << "!!@@BEGIN: ";
	if (m_aboveQ) {
		m_humdrum_text << "PREHTML" << endl;
	} else {
		m_humdrum_text << "POSTHTML" << endl;
	}
	m_humdrum_text << "!!@CONTENT:" << endl;
	m_humdrum_text << m_output.str();
	m_humdrum_text << "!!@@END: ";
	if (m_aboveQ) {
		m_humdrum_text << "PREHTML" << endl;
	} else {
		m_humdrum_text << "POSTHTML" << endl;
	}
}



/////////////////////////////
//
// Tool_text::removeText -- move **text above/below music.
//    Groups **text spines by **kern (part) and processes left → right.
//

void Tool_text::removeText(HumdrumFile& infile) {

	// Get all spine starts (in left-to-right order)
	vector<HTp> sspines;
	infile.getSpineStartList(sspines);

	// Build grouped text arrays (one per **kern)
	vector<vector<HTp>> twoarray;
	makeTextArray(twoarray, sspines);

	// Debug (optional)
	// cout << "ARRAY SIZE: " << twoarray.size() << endl;

	// Output HTML wrapper
	string style = makeStyle();
	m_output << style;

	m_output << "!! <table class=\"pline\">" << endl;
	m_output << "!! <tr class=\"header\">" << endl;

	if (!m_rawQ) {
		m_output << "!!    <th class=\"pline\"> Bline </th>" << endl;
	}

	m_output << "!!    <th class=\"pline-text\"> text </th>" << endl;

	if (!m_rawQ) {
		m_output << "!!    <th class=\"rp-rf\"> <span title=\"Rhyme phoneme\">rp</span>";
		m_output << "/<span title=\"Rhyme syllable\">rf</span> </th>" << endl;
	}
	if (m_countQ) {
		m_output << "!!    <th class=\"syl\" title=\"Syllables\"> syl </th>" << endl;
	}
	m_output << "!!    <th title=\"Rhyme Scheme\"> rs </th>" << endl;

	m_output << "!! </tr>" << endl;


	///////////////////////////////////////////
	//
	// Process each part (group of text spines)
	//

	for (int i = 0; i < (int)twoarray.size(); i++) {

		vector<HTp>& part = twoarray[i];

		// Process each verse spine in the part (LEFT → RIGHT)
		for (int j = 0; j < (int)part.size(); j++) {

			HTp spine = part[j];
			if (!spine) {
				continue;
			}

			// Extract text / plines
			removePartText(spine, j, (int)part.size());

			// Handle removal options
			if (m_removeAllQ && j == 0) {
				spine->setText("**Xtext");
			} else if (m_removeQ && j > 0) {
				spine->setText("**Xtext");
			}
		}
	}

	m_output << "\n!! </table>\n";
}


////////////////////////////
//
// Tool_text::removePartText --
//

void Tool_text::removePartText(HTp startspine, int vth, int vsize) {
	if (hasParam(startspine, "*pline:")) {
		processPlineSpine(startspine, vth, vsize);
	} else if (hasParam(startspine, "*rline:")) {
		processPlineSpine(startspine, vth, vsize);
	} else {
		processTextSpine(startspine, vth, vsize);
	}
}



//////////////////////////////
//
// Tool_text::makeTextArray -- Reverse order of **text for each **kern.  The output
//   is a list of **text spines, firt indexed by **kern (left to right and then indexed
//   by
//

void Tool_text::makeTextArray(vector<vector<HTp>>& texts, vector<HTp> spines) {

	texts.clear();

	vector<HTp> currentGroup;

	for (int i = 0; i < (int)spines.size(); i++) {

		HTp s = spines[i];
		if (!s) continue;

		// When we hit a **kern, start a new group
		if (s->isDataType("**kern")) {

			if (!currentGroup.empty()) {
				texts.push_back(currentGroup);
				currentGroup.clear();
			}

			continue;
		}

		// Collect **text spines into current group
		if (s->isDataType("**text")) {
			currentGroup.push_back(s);
		}
	}

	// push last group
	if (!currentGroup.empty()) {
		texts.push_back(currentGroup);
	}
}


//////////////////////////////
//
// Tool_text::hasParam -- True if *pline exists in spine
//

bool Tool_text::hasParam(HTp tspine, const string& target) {
	HTp current = tspine;
	int len = (int)target.size();
	while (current) {
		if (!current->isInterpretation()) {
			current = current->getNextToken();
			continue;
		} else if (current->compare(0, len, target) == 0) {
			return true;
		}
		current = current->getNextToken();
	}

	return false;
}



/////////////////////////////
//
// Text_tool::getParamList -- Find the parameter anywhere in the list of tokens. 
//     Example if the target is "*rf:" and the first token found is *rf:e, the 
//     string returned will be "e".
//

string Tool_text::getParamListOne(vector<HTp>& tspine, const string& target) {
	int len = (int)target.size();
	for (int i=0; i<(int)tspine.size(); i++) {
		if (!tspine[i]->isInterpretation()) {
			continue;
		} else {
			if (tspine[i]->compare(0, len, target) == 0) {
				return tspine[i]->substr(len);
			}
		}
	}
	string value = "";
	return value;
}

string Tool_text::getParamListTwo(vector<vector<HTp>>& tspine, const string& target) {
	string value = "";
	int len = (int)target.size();

	for (int i = 0; i < (int)tspine.size(); i++) {
		for (int j = 0; j < (int)tspine[i].size(); j++) {
			if (!tspine[i][j]->isInterpretation()) {
				continue;
			}
			if (tspine[i][j]->compare(0, len, target) == 0) {
				return tspine[i][j]->substr(len);
			}
		}
	}

	return value;
}

//
// Similar but search only at the same timestamp for the first data.
//

string Tool_text::getParmTimestamp(HTp token, const string& target) {
	HTp current = token;
	string value = "";
	int len = (int)target.size();

	while (current) {
		if (current->isInterpretation()) {
			if (current->compare(0, len, target) == 0) {
				return current->substr(len);
			}
		}
		current = current->getNextToken();
	}

	return value;
}



//////////////////////////////
//
// Tool_text::processPlineSpine -- Extract a verse/spine of text
//

void Tool_text::processPlineSpine(HTp tspine, int vth, int vsize) {
	vector<vector<HTp>> plines;
	fillPlines(plines, tspine, vth, vsize);

	for (int i=1; i<(int)plines.size(); i++) {
		addSyllables(plines[i]);
	}

	string verse = getParamListTwo(plines, "*v:");
	string label = "";
	static string lastlabel = "";

	if (m_showVerseQ) {
		if (!verse.empty()) {
			if (!m_refrainOnlyQ) {
				label = "VERSE ";
				label += verse;
			}
		}
		string refrain = getParamListTwo(plines, "*rline:");
		if (!refrain.empty()) {
			if (!verse.empty()) {
				label = "REFRAIN";
			}
		} else if (verse.empty()) {
			if (!m_refrainOnlyQ) {
				label = "VERSE [";
				label += verse;
				label += "]";
			}
		}
	}

	if (label != lastlabel) {
		m_output << "\n!!   <td class=\"verse\" colspan=\"4\">" << label << "</td>";
	}

	HumRegex hre;
	for (int i = 1; i < (int)plines.size(); i++) {
		string plinelabel = getPlineLabel(plines[i]);
		// suppress repeated plines such as:
		//    *pline:8r
		//    *rline:12r
		if (m_repeatsQ) {
			if (hre.search(plinelabel, "r$")) {
				continue;
			}
		}
		zprintPlineRow(plines[i]);
	}
	lastlabel = label;
}



/////////////////////////////
//
// Tool_text::addSyllables --
//

void Tool_text::addSyllables(vector<HTp>& syllables) {
	if (syllables.empty()) {
		return;
	}

	HumRegex hre;
	HTp current = syllables[0]->getNextToken();

	while (current) {
		if (current->isInterpretation()) {
			if (hre.search(current, "^\\*[pr]line:")) {
				break;
			} else if (*current == "*-") {
				break;
			}
		}

		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		syllables.push_back(current);
		current = current->getNextToken();
	}
}




/////////////////////////////
//
// Tool_text::makeStyle -- Print CSS styling.
//

string Tool_text::makeStyle(void) {
	stringstream out;
	out << R"(!! <style>
!! table.pline {
!!   table-layout: auto;
!!   width: auto;
!!   margin-left: 20px;
!!   margin-right: 20px;
!!   margin-bottom: 50px;
!! }
!! table.pline th { 
!!   cursor: pointer; position: 
!!   sticky; top: 0;  
!!   background: orange; 
!!   opacity: 0.8
!!}
!! table.pline td {  vertical-align: top; padding-right: 20px;}
!! table.pline tr.pline:hover td { background-color: #eeeeee; }
!! table.pline .verse { padding-top: 10px; }
!! table.pline td.pline { padding-right: 10px; }
!! table.pline .rp { }
!! table.pline .sylcount { }
!! table.pline .rs { }
!! table.pline .rf {color: fuchsia; }
!! table.pline .rp {color: purple; }
!! table.pline .rs {font-weight: bold; }
!! table.pline .rs {font-weight: bold; }
!! </style>
)";
	return out.str();
}



//////////////////////////////
//
// Tool_text::printPlineRow -- print text for a pline
//

void Tool_text::zprintPlineRow(vector<HTp>& pieces) {
	m_output << "\n!! <tr class=\"pline\">";
	string plinelabel = getPlineLabel(pieces);

	if (!m_rawQ) {
		if (!plinelabel.empty()) {
			m_output << "\n!!   <td class=\"pline\">";
			m_output << plinelabel;
			m_output << "</td>";
		}
	}

	m_output << "\n!!   <td class=\"pline-text\">";
	printPlineSyllables(pieces);
	m_output << "</td>";

	string rp = getParamListOne(pieces, "*rp:");
	string rf = getParamListOne(pieces, "*rf:");

	if (!m_rawQ && !(rp.empty() || rf.empty())) {
		m_output << "\n!!   <td class='rp'>";
		m_output << "<span class=\"rp\">" << rp;
		m_output << "</span>";
		m_output << "/";
		m_output << "<span class=\"rf\">" << rf << "</span>";
		m_output << "</td>";
	}

	if (m_countQ) {
		int sylcount = countSyllables(pieces);
		m_output << "\n!!   <td class='syl'>" << sylcount << "</td>";
	}

	string rs = getParamListOne(pieces, "*rs:");
	m_output << "\n!!   <td class='rs'>" << rs << "</td>";

	m_output << "\n!! </tr>";
}



/////////////////////////////
//
// Tool_text::countSyllables --
//

int Tool_text::countSyllables(vector<HTp>& tokens) {
	int count = 0;
	for (int i=0; i<(int)tokens.size(); i++) {
		if (!tokens[i]->isData()) {
			continue;
		}
		count++;
	}
	return count;
}


//////////////////////////////
//
// Tool_text::getPlineLabel -- print text for a pline
//

string Tool_text::getPlineLabel(vector<HTp>& pieces) {
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i]->isInterpretation()) {
			if (pieces[i]->compare(0, 7, "*pline:") == 0) {
				return pieces[i]->substr(7);
			} else if (pieces[i]->compare(0, 7, "*rline:") == 0) {
				return pieces[i]->substr(7);
			}
		}
	}
	string out = "NONE";
	return out;
}



//////////////////////////////
//
// Tool_text::printPlineSyllables -- print text for a pline
// XXX
//

void Tool_text::printPlineSyllables(vector<HTp>& pieces) {
	stringstream out;
	vector<HTp> np;

	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i]->isNull()) {
			continue;
		}
		else if (pieces[i]->isData()) {
			np.push_back(pieces[i]);
		}
	}

	// strip dashes
	for (int i=0; i<(int)np.size(); i++) {
		string text = *np[i];
		if (text.empty()) {
		} else {
			m_output << getSyllable(text);
		}
	}

	return;
}



//////////////////////////////
//
// Tool_text::fillPlines -- create each pline entry.
//

void Tool_text::fillPlines(vector<vector<HTp>>& plines, HTp tspine,
		int vth, int vsize) {

	HTp current = tspine;

	plines.clear();

	// plines[0] stores global verse/refrain metadata
	plines.resize(1);

	HumRegex hre;

	// current active pline
	int index = -1;

	while (current) {
		if (current->isInterpretation()) {
			// store verse interpretations globally
			if (current->compare(0, 3, "*v:") == 0) {
				plines[0].push_back(current);
			}

			// create a new pline/rline
			else if (hre.search(current, "^\\*pline:")) {
				if (!m_refrainOnlyQ) {
					plines.resize(plines.size() + 1);
					index = (int)plines.size() - 1;
					plines[index].push_back(current);
				} else {
					current = current->getNextToken();
					continue;
				}
			} else if (hre.search(current, "^\\*rline:")) {
				if (!m_verseOnlyQ) {
					plines.resize(plines.size() + 1);
					index = (int)plines.size() - 1;
					plines[index].push_back(current);
				} else {
					current = current->getNextToken();
					continue;
				}
			}

			// attach rhyme metadata to current pline
			else if (hre.search(current, "^\\*rp:")) {
				if (index >= 0) {
					plines[index].push_back(current);
				}
			} else if (hre.search(current, "^\\*rf:")) {
				if (index >= 0) {
					plines[index].push_back(current);
				}
			} else if (hre.search(current, "^\\*rs:")) {
				if (index >= 0) {
					plines[index].push_back(current);
				}
			}
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_text::printPline --
//

void Tool_text::printPline(vector<vector<HTp>>& p, const char* description) {
	return;
	for (int i=0; i<(int)p.size(); i++) {
		for (int j=0; j<(int)p[i].size(); j++) {
			cerr << "===(" << i <<"," << j << ") = "
			     << p.at(i).at(j) << endl;
		}
	}
}



//////////////////////////////
//
// Tool_text::procesTextSpine -- Extract a verse/spine of text
//

void Tool_text::processTextSpine(HTp tspine, int vth, int vsize) {
	HumdrumFileStructure *infile = tspine->getOwner()->getOwner();
	string name = infile->getPartName(tspine);
	if (!name.empty()) {
		m_output << "!!\n!! <p>" << name << endl;
	} else {
		m_output << "!!\n!! <p>" << "empty" << endl;
	}
	m_output << "!! <p>";

	HTp current = tspine;

	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		} else if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		string syllable = getSyllable(*current);
		m_output << syllable;
		current = current->getNextToken();
	}
	m_output << endl;
}



//////////////////////////////
//
// Tool_text::getSyllable --
//

string Tool_text::getSyllable(const string& text) {
	string newtext = text;
	HumRegex hre;
	if (m_joinQ) {
		hre.replaceDestructive(newtext, "", "^-");
		if (!newtext.empty()) {
			if (newtext.back() == '-') {
				newtext.resize((int)newtext.size() - 1);
			} else {
				newtext += " ";
			}
		} else {
			newtext += " ";
		}
	} else {
		newtext += " ";
	}
	return newtext;
}


// END_MERGE

} // end namespace hum
