//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Sat Jun  8 17:37:39 PDT 2024 (ported to humlib)
// Filename:      src/tool-prange.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-prange.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate pitch histograms in a score of **kern data.
//
// Reference:     https://en.wikipedia.org/wiki/List_of_emojis
//

#include "tool-prange.h"
#include "HumRegex.h"
#include "Convert.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <numeric>

using namespace std;

namespace hum {

// START_MERGE


#define OBJTAB "\t\t\t\t\t\t"
#define SVGTAG "_99%svg%";

#define SVGTEXT(out, text) \
	if (m_defineQ) { \
		out << "SVG "; \
	} else { \
		out << "t 1 1\n"; \
		out << SVGTAG; \
	} \
	printScoreEncodedText((out), (text)); \
	out << "\n";


//////////////////////////////
//
// _VoiceInfo::_VoiceInfo --
//

_VoiceInfo::_VoiceInfo(void) {
	clear();
}



//////////////////////////////
//
// _VoiceInfo::clear --
//

void _VoiceInfo::clear(void) {
	name = "";
	abbr = "";
	midibins.resize(128);
	fill(midibins.begin(), midibins.end(), 0.0);
	diatonic.resize(7 * 12);
	for (int i=0; i<(int)diatonic.size(); i++) {
		diatonic[i].resize(6);
		fill(diatonic[i].begin(), diatonic[i].end(), 0.0);
	}
	track = -1;
	kernQ = false;
	diafinal.clear();
	accfinal.clear();
	namfinal.clear();
	index = -1;
}


//////////////////////////////
//
// _VoiceInfo::print --
//

ostream& _VoiceInfo::print(ostream& out) {
	out << "==================================" << endl;
	out << "track:  " << track << endl;
	out << " name:  " << name << endl;
	out << " abbr:  " << abbr << endl;
	out << " kern:  " << kernQ << endl;
	out << " final:";
	for (int i=0; i<(int)diafinal.size(); i++) {
		out << " " << diafinal.at(i) << "/" << accfinal.at(i);
	}
	out << endl;
	out << " midi:  ";
	for (int i=0; i<(int)midibins.size(); i++) {
		if (midibins.at(i) > 0.0) {
			out << " " << i << ":" << midibins.at(i);
		}
	}
	out << endl;
	out << " diat:  ";
	for (int i=0; i<(int)diatonic.size(); i++) {
		if (diatonic.at(i).at(0) > 0.0) {
			out << " " << i << ":" << diatonic.at(i).at(0);
		}
	}
	out << endl;
	out << "==================================" << endl;
	return out;
}



/////////////////////////////////
//
// Tool_prange::Tool_prange -- Set the recognized options for the tool.
//

Tool_prange::Tool_prange(void) {

	define("A|acc|color-accidentals=b", "add color to accidentals in histogram");
	define("D|diatonic=b",              "diatonic counts ignore chormatic alteration");
	define("K|no-key=b",                "do not display key signature");
	define("N|norm=b",                  "normalize pitch counts");
	define("S|score=b",                 "convert range info to SCORE");
	define("T|no-title=b",              "do not display a title");
	define("a|all=b",                   "generate all-voice analysis");
	define("c|range|count=s:60-71",     "count notes in a particular MIDI note number range (inclusive)");
	define("debug=b",                   "trace input parsing");
	define("d|duration=b",              "weight pitches by duration");
	define("e|embed=b",                 "embed SCORE data in input Humdrum data");
	define("fill=b",                    "change color of fill only");
	define("finalis|final|last=b",      "include finalis note by voice");
	define("f|fraction=b",              "display histogram fractions");
	define("h|hover=b",                 "include svg hover capabilities");
	define("i|instrument=b",            "categorize multiple inputs by instrument");
	define("j|jrp=b",                   "set options for JRP style");
	define("l|local|local-maximum|local-maxima=b",  "use maximum values by voice rather than all voices");
	define("no-define=b",               "do not use defines in output SCORE data");
	define("pitch=b",                   "display pitch info in **pitch format");
	define("print=b",                   "count printed notes rather than sounding");
	define("p|percentile=d:0.0",        "display the xth percentile pitch");
	define("q|quartile=b",              "display quartile notes");
	define("r|reverse=b",               "reverse list of notes in analysis from high to low");
	define("x|extrema=b",               "highlight extrema notes in each part");
	define("sx|scorexml|score-xml|ScoreXML|scoreXML=b", "output ScoreXML format");
	define("title=s:",                  "title for SCORE display");

}


/////////////////////////////////
//
// Tool_prange::run -- Do the main work of the tool.
//

bool Tool_prange::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_prange::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_prange::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_prange::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_prange::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_prange::initialize(void) {
	m_accQ         = getBoolean("color-accidentals");
	m_addFractionQ = getBoolean("fraction");
	m_allQ         = getBoolean("all");
	m_debugQ       = getBoolean("debug");
	m_defineQ      = false;
	m_diatonicQ    = getBoolean("diatonic");
	m_durationQ    = getBoolean("duration");
	m_fillOnlyQ    = getBoolean("fill");
	m_finalisQ     = getBoolean("finalis");
	m_hoverQ       = getBoolean("hover");
	m_instrumentQ  = getBoolean("instrument");
	m_keyQ         = !getBoolean("no-key");
	m_listQ        = false;
	m_localQ       = getBoolean("local-maximum");
	m_normQ        = getBoolean("norm");
	m_notitleQ     = getBoolean("no-title");
	m_percentile   = getDouble("percentile");
	m_percentileQ  = getBoolean("percentile");
	m_pitchQ       = getBoolean("pitch");
	m_printQ       = getBoolean("print");
	m_quartileQ    = getBoolean("quartile");
	m_rangeQ       = getBoolean("range");
	m_reverseQ     = !getBoolean("reverse");
	m_scoreQ       = getBoolean("score");
	m_title        = getString("title");
	m_titleQ       = getBoolean("title");
	m_embedQ       = getBoolean("embed");
	m_extremaQ     = getBoolean("extrema");

	getRange(m_rangeL, m_rangeH, getString("range"));

	if (getBoolean("jrp")) {
		// default style settings for JRP range displays:
		m_scoreQ   = true;
		m_allQ     = true;
		m_hoverQ   = true;
		m_accQ     = true;
		m_finalisQ = true;
		m_notitleQ = true;
	}

	// The percentile is a fraction from 0.0 to 1.0.
	// if the percentile is above 1.0, then it is assumed
	// to be a percentage, in which case the value will be
	// divided by 100 to get it in the range from 0 to 1.
	if (m_percentile > 1) {
		m_percentile = m_percentile / 100.0;
	}

	#ifdef __EMSCRIPTEN__
		// Default styling for JavaScript version of program:
		m_accQ     = !getBoolean("color-accidentals");
		m_scoreQ   = !getBoolean("score");
		m_embedQ   = !getBoolean("embed");
		m_hoverQ   = !getBoolean("hover");
		m_notitleQ = !getBoolean("no-title");
	#endif
}



//////////////////////////////
//
// Tool_prange::processFile --
//

void Tool_prange::processFile(HumdrumFile& infile) {
	prepareRefmap(infile);
	vector<_VoiceInfo> voiceInfo;
	infile.fillMidiInfo(m_trackMidi);
	getVoiceInfo(voiceInfo, infile);
	fillHistograms(voiceInfo, infile);

	if (m_debugQ) {
		for (int i=0; i<(int)voiceInfo.size(); i++) {
			voiceInfo[i].print(cerr);
		}
	}

	if (m_scoreQ) {
		stringstream scoreout;
		printScoreFile(scoreout, voiceInfo, infile);
		if (m_embedQ) {
			if (m_extremaQ) {
				doExtremaMarkup(infile);
			}
			m_humdrum_text << infile;
			printEmbeddedScore(m_humdrum_text, scoreout, infile);
		} else {
			if (m_extremaQ) {
				doExtremaMarkup(infile);
			}
			m_humdrum_text << scoreout.str();
		}
	} else {
		printAnalysis(m_humdrum_text, voiceInfo[0].midibins);
	}
}



//////////////////////////////
//
// Tool_prange::doExtremaMarkup -- Mark highest and lowest note
//     in each **kern spine.
//
//

void Tool_prange::doExtremaMarkup(HumdrumFile& infile) {
	bool highQ = false;
	bool lowQ = false;
	for (int i=0; i<(int)m_trackMidi.size(); i++) {
		int maxindex = -1;
		int minindex = -1;

		for (int j=(int)m_trackMidi[i].size()-1; j>=0; j--) {
			if (m_trackMidi[i][j].empty()) {
				continue;
			}
			if (maxindex < 0) {
				maxindex = j;
				break;
			}
		}

		for (int j=1; j<(int)m_trackMidi[i].size(); j++) {
			if (m_trackMidi[i][j].empty()) {
				continue;
			}
			if (minindex < 0) {
				minindex = j;
				break;
			}
		}

		if ((maxindex < 0) || (minindex < 0)) {
			continue;
		}
		applyMarkup(m_trackMidi[i][maxindex], m_highMark);
		applyMarkup(m_trackMidi[i][minindex], m_lowMark);
		highQ = true;
		lowQ  = true;
	}
	if (highQ) {
		string highRdf = "!!!RDF**kern: " + m_highMark + " = marked note, color=\"hotpink\", highest note";
		infile.appendLine(highRdf);
	}
	if (lowQ) {
		string lowRdf = "!!!RDF**kern: " + m_lowMark + " = marked note, color=\"limegreen\", lowest note";
		infile.appendLine(lowRdf);
	}
	if (highQ || lowQ) {
		infile.createLinesFromTokens();
	}
}



//////////////////////////////
//
// Tool_prange::applyMarkup --
//

void Tool_prange::applyMarkup(vector<pair<HTp, int>>& notelist, const string& mark) {
	for (int i=0; i<(int)notelist.size(); i++) {
		HTp token = notelist[i].first;
		int subtoken = notelist[i].second;
		int tokenCount = token->getSubtokenCount();
		if (tokenCount == 1) {
			string text = *token;
			text += mark;
			token->setText(text);
		} else {
			string stok = token->getSubtoken(subtoken);
			stok = mark + stok;
			token->replaceSubtoken(subtoken, stok);
		}
	}
}



//////////////////////////////
//
// Tool_prange::printEmbeddedScore --
//

void Tool_prange::printEmbeddedScore(ostream& out, stringstream& scoredata, HumdrumFile& infile) {
	int id = getPrangeId(infile);

	out << "!!@@BEGIN: PREHTML\n";
	out << "!!@CONTENT: <div class=\"score-svg\" ";
	out <<    "style=\"margin-top:50px;text-align:center;\" ";
	out <<    " data-score=\"prange-" << id << "\"></div>\n";
	out << "!!@@END: PREHTML\n";
	out << "!!@@BEGIN: SCORE\n";
	out << "!!@ID: prange-" << id << "\n";
	out << "!!@OUTPUTFORMAT: svg\n";
	out << "!!@CROP: yes\n";
	out << "!!@PADDING: 10\n";
	out << "!!@SCALING: 1.5\n";
	out << "!!@SVGFORMAT: yes\n";
	out << "!!@TRANSPARENT: yes\n";
	out << "!!@ANTIALIAS: no\n";
	out << "!!@EMBEDPMX: yes\n";
	out << "!!@ANNOTATE: no\n";
	out << "!!@CONTENTS:\n";
	string line;
	while(getline(scoredata, line)) {
		out << "!!" << line << endl;
	}
	out << "!!@@END: SCORE\n";
}



//////////////////////////////
//
// Tool_prange::getPrangeId -- Find a line in this form
//          ^!!@ID: prange-(\d+)$
//      and return $1+1.  Searching backwards since the HTML section
//      will likely be at the bottom.  Assuming that the prange
//      SVG images are stored in sequence, with the highest ID last
//      in the file if there are more than one.
//

int Tool_prange::getPrangeId(HumdrumFile& infile) {
	string search = "!!@ID: prange-";
	int length = (int)search.length();
	for (int i=infile.getLineCount() - 1; i>=0; i--) {
		HTp token = infile.token(i, 0);
		if (token->compare(0, length, search) == 0) {
			HumRegex hre;
			if (hre.search(token, "prange-(\\d+)")) {
				return hre.getMatchInt(1) + 1;
			}
		}
	}
	return 1;
}



//////////////////////////////
//
// Tool_prange::mergeAllVoiceInfo --
//

void Tool_prange::mergeAllVoiceInfo(vector<_VoiceInfo>& voiceInfo) {
	voiceInfo.at(0).diafinal.clear();
	voiceInfo.at(0).accfinal.clear();

	for (int i=1; i<(int)voiceInfo.size(); i++) {
		if (!voiceInfo[i].kernQ) {
			continue;
		}
		for (int j=0; j<(int)voiceInfo.at(i).diafinal.size(); j++) {
			voiceInfo.at(0).diafinal.push_back(voiceInfo.at(i).diafinal.at(j));
			voiceInfo.at(0).accfinal.push_back(voiceInfo.at(i).accfinal.at(j));
			voiceInfo.at(0).namfinal.push_back(voiceInfo.at(i).name);
		}

		for (int j=0; j<(int)voiceInfo[i].midibins.size(); j++) {
			voiceInfo[0].midibins[j] += voiceInfo[i].midibins[j];
		}

		for (int j=0; j<(int)voiceInfo.at(i).diatonic.size(); j++) {
			for (int k=0; k<(int)voiceInfo.at(i).diatonic.at(k).size(); k++) {
				voiceInfo[0].diatonic.at(j).at(k) += voiceInfo.at(i).diatonic.at(j).at(k);
			}
		}
	}
}



//////////////////////////////
//
// Tool_prange::getVoiceInfo -- get names and track info for **kern spines.
//

void Tool_prange::getVoiceInfo(vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile) {
	voiceInfo.clear();
	voiceInfo.resize(infile.getMaxTracks() + 1);
	for (int i=0; i<(int)voiceInfo.size(); i++) {
		voiceInfo.at(i).index = i;
	}

	vector<HTp> kstarts = infile.getKernSpineStartList();

	if (kstarts.size() == 2) {
		voiceInfo[0].name  = "both";
		voiceInfo[0].abbr  = "both";
		voiceInfo[0].track = 0;
	} else {
		voiceInfo[0].name  = "all";
		voiceInfo[0].abbr  = "all";
		voiceInfo[0].track = 0;
	}


	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			int track = token->getTrack();
			voiceInfo[track].track = track;
			if (token->isKern()) {
				voiceInfo[track].kernQ = true;
			}
			if (!voiceInfo[track].kernQ) {
				continue;
			}
			if (token->isInstrumentName()) {
				voiceInfo[track].name = token->getInstrumentName();
			}
			if (token->isInstrumentAbbreviation()) {
				voiceInfo[track].abbr = token->getInstrumentAbbreviation();
			}
		}
	}


	// Check for piano/Grand Staff parts with LH/RH encoding.
	if (kstarts.size() == 2) {
		string bottomStaff = getHand(kstarts[0]);
		string topStaff    = getHand(kstarts[1]);
		if (!bottomStaff.empty() && !topStaff.empty()) {
			int track = kstarts[0]->getTrack();
			voiceInfo[track].name = "left hand";
			track = kstarts[1]->getTrack();
			voiceInfo[track].name = "right hand";
		}
	}
}



//////////////////////////////
//
// Tool_prange::getHand --
//

string Tool_prange::getHand(HTp sstart) {
	HTp current = sstart->getNextToken();
	HTp target = NULL;
	while (current) {
		if (current->isData()) {
			break;
		}
		if (*current == "*LH") {
			target = current;
			break;
		}
		if (*current == "*RH") {
			target = current;
			break;
		}
		current = current->getNextToken();
	}

	if (target) {
		if (*current == "*LH") {
			return "LH";
		} else if (*current == "*RH") {
			return "RH";
		} else {
			return "";
		}
	} else {
		return "";
	}
}



//////////////////////////////
//
// Tool_prange::getInstrumentNames --  Find any instrument names which are listed
//      before the first data line.  Instrument names are in the form:
//
//      *I"name
//

void Tool_prange::getInstrumentNames(vector<string>& nameByTrack, vector<int>& kernSpines,
		HumdrumFile& infile) {
	HumRegex hre;

	int track;
	string name;
	// nameByTrack.resize(kernSpines.size());
	nameByTrack.resize(infile.getMaxTrack() + 1);
	fill(nameByTrack.begin(), nameByTrack.end(), "");
	vector<HTp> kspines = infile.getKernSpineStartList();
	if (kspines.size() == 2) {
		nameByTrack.at(0) = "both";
	} else {
		nameByTrack.at(0) = "all";
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (hre.search(token, "^\\*I\"(.*)\\s*")) {
				name = hre.getMatch(1);
				track = token->getTrack();
				for (int k=0; k<(int)kernSpines.size(); k++) {
					if (track == kernSpines[k]) {
						nameByTrack[k] = name;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// Tool_prange::fillHistograms -- Store notes in score by MIDI note number.
//

void Tool_prange::fillHistograms(vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile) {
	// storage for finals info:
	vector<vector<int>> diafinal;
	vector<vector<int>> accfinal;
	diafinal.resize(infile.getMaxTracks() + 1);
	accfinal.resize(infile.getMaxTracks() + 1);

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			int track = token->getTrack();

			diafinal.at(track).clear();
			accfinal.at(track).clear();

			vector<string> tokens = token->getSubtokens();
			for (int k=0; k<(int)tokens.size(); k++) {
				if (tokens[k].find("r") != string::npos) {
					continue;
				}
				if (tokens[k].find("R") != string::npos) {
					// non-pitched note
					continue;
				}
				bool hasPitch = false;
				for (int m=0; m<(int)tokens[k].size(); m++) {
					char test = tokens[k].at(m);
					if (!isalpha(test)) {
						continue;
					}
					test = tolower(test);
					if ((test >= 'a') && (test <= 'g')) {
						hasPitch = true;
						break;
					}
				}
				if (!hasPitch) {
					continue;
				}
				int octave = Convert::kernToOctaveNumber(tokens[k]) + 3;
				if (octave < 0) {
					cerr << "Note too low: " << tokens[k] << endl;
					continue;
				}
				if (octave >= 12) {
					cerr << "Note too high: " << tokens[k] << endl;
					continue;
				}
				int dpc    = Convert::kernToDiatonicPC(tokens[k]);
				int acc    = Convert::kernToAccidentalCount(tokens[k]);
				if (acc < -2) {
					cerr << "Accidental too flat: " << tokens[k] << endl;
					continue;
				}
				if (acc > +2) {
					cerr << "Accidental too sharp: " << tokens[k] << endl;
					continue;
				}
				int diatonic = dpc + 7 * octave;
				int realdiatonic = dpc + 7 * (octave-3);

				diafinal.at(track).push_back(realdiatonic);
				accfinal.at(track).push_back(acc);

				acc += 3;
				int midi = Convert::kernToMidiNoteNumber(tokens[k]);
				if (midi < 0) {
					cerr << "MIDI pitch too low: " << tokens[k] << endl;
				}
				if (midi > 127) {
					cerr << "MIDI pitch too high: " << tokens[k] << endl;
				}
				if (m_durationQ) {
					double duration = Convert::kernToDuration(tokens[k]).getFloat();
					voiceInfo[track].diatonic.at(diatonic).at(0) += duration;
					voiceInfo[track].diatonic.at(diatonic).at(acc) += duration;
					voiceInfo[track].midibins.at(midi) += duration;
				} else {
					if (tokens[k].find("]") != string::npos) {
						continue;
					}
					if (tokens[k].find("_") != string::npos) {
						continue;
					}
					voiceInfo[track].diatonic.at(diatonic).at(0)++;
					voiceInfo[track].diatonic.at(diatonic).at(acc)++;
					voiceInfo[track].midibins.at(midi)++;
				}
			}
		}
	}

	mergeFinals(voiceInfo, diafinal, accfinal);

	// Sum all voices into midibins and diatonic arrays of vector position 0:
	mergeAllVoiceInfo(voiceInfo);
}



//////////////////////////////
//
// Tool_prange::mergeFinals --
//

void Tool_prange::mergeFinals(vector<_VoiceInfo>& voiceInfo, vector<vector<int>>& diafinal,
		vector<vector<int>>& accfinal) {
	for (int i=0; i<(int)voiceInfo.size(); i++) {
		voiceInfo.at(i).diafinal = diafinal.at(i);
		voiceInfo.at(i).accfinal = accfinal.at(i);
	}
}



//////////////////////////////
//
// Tool_prange::printFilenameBase --
//

void Tool_prange::printFilenameBase(ostream& out, const string& filename) {
	HumRegex hre;
	if (hre.search(filename, "([^/]+)\\.([^.]*)", "")) {
		if (hre.getMatch(1).size() <= 8) {
			printXmlEncodedText(out, hre.getMatch(1));
		} else {
			// problem with too long a name (MS-DOS will have problems).
			// optimize to chop off everything after the dash in the
			// name (for Josquin catalog numbers).
			string shortname = hre.getMatch(1);
			if (hre.search(shortname, "-.*")) {
			   hre.replaceDestructive(shortname, "", "-.*");
				printXmlEncodedText(out, shortname);
			} else {
				printXmlEncodedText(out, shortname);
			}
		}
	}
}



//////////////////////////////
//
// Tool_prange::printReferenceRecords --
//

void Tool_prange::printReferenceRecords(ostream& out, HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		out <<  "\t\t\t\t\t\t<?Humdrum key=\"";
		printXmlEncodedText(out, infile[i].getReferenceKey());
		out << "\" value=\"";
		printXmlEncodedText(out, infile[i].getReferenceValue());
		out << "\"?>\n";
	}
}



//////////////////////////////
//
// Tool_prange::printScoreEncodedText -- print SCORE text string
//    See SCORE 3.1 manual additions (page 19) for more.
//

void Tool_prange::printScoreEncodedText(ostream& out, const string& strang) {
	string newstring = strang;
	HumRegex hre;

	hre.replaceDestructive(newstring, "<<$1", "&([aeiou])acute;", "gi");
	hre.replaceDestructive(newstring, "<<$1", "([áéíóú])", "gi");

	hre.replaceDestructive(newstring, ">>$1", "&([aeiou])grave;", "gi");
	hre.replaceDestructive(newstring, ">>$1", "([àèìòù])", "gi");

	hre.replaceDestructive(newstring, "%%$1", "&([aeiou])uml;", "gi");
	hre.replaceDestructive(newstring, "%%$1", "([äëïöü])", "gi");

	hre.replaceDestructive(newstring, "^^$1", "&([aeiou])circ;", "gi");
	hre.replaceDestructive(newstring, "^^$1", "([âêîôû])", "gi");

	hre.replaceDestructive(newstring, "##c", "&ccedil;",  "g");
	hre.replaceDestructive(newstring, "##C", "&Ccedil;",  "g");
	hre.replaceDestructive(newstring, "?\\|", "\\|",      "g");
	hre.replaceDestructive(newstring, "?\\",  "\\\\",     "g");
	hre.replaceDestructive(newstring, "?m",   "---",      "g");
	hre.replaceDestructive(newstring, "?n",   "--",       "g");
	hre.replaceDestructive(newstring, "?2",   "-sharp",   "g");
	hre.replaceDestructive(newstring, "?1",   "-flat",    "g");
	hre.replaceDestructive(newstring, "?3",   "-natural", "g");
	hre.replaceDestructive(newstring, "\\",   "/",        "g");
	hre.replaceDestructive(newstring, "?[",   "\\[",      "g");
	hre.replaceDestructive(newstring, "?]",   "\\]",      "g");

	out << newstring;
}



//////////////////////////////
//
// Tool_prange::printXmlEncodedText -- convert
//    & to &amp;
//    " to &quot;
//    ' to &spos;
//    < to &lt;
//    > to &gt;
//

void Tool_prange::printXmlEncodedText(ostream& out, const string& strang) {
	HumRegex hre;
	string astring = strang;

	hre.replaceDestructive(astring, "&",  "&amp;",  "g");
	hre.replaceDestructive(astring, "'",  "&apos;", "g");
	hre.replaceDestructive(astring, "\"", "&quot;", "g");
	hre.replaceDestructive(astring, "<",  "&lt;",   "g");
	hre.replaceDestructive(astring, ">",  "&gt;",   "g");

	out << astring;
}



//////////////////////////////
//
// Tool_prange::printScoreFile --
//

void Tool_prange::printScoreFile(ostream& out, vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile) {
	string titlestring = getTitle();

	if (m_defineQ) {
		out << "#define SVG t 1 1 \\n_99%svg%\n";
	}

	string acctext = "g.bar.doubleflat path&#123;color:darkorange;stroke:darkorange;&#125;g.bar.flat path&#123;color:brown;stroke:brown;&#125;g.bar.sharp path&#123;color:royalblue;stroke:royalblue;&#125;g.bar.doublesharp path&#123;color:aquamarine;stroke:aquamarine;&#125;";
	string hovertext = ".bar:hover path&#123;fill:red;color:red;stroke:red &#33;important&#125;";
	string hoverfilltext = hovertext;

	string text1 = "<style>";
	text1 += hoverfilltext;
	if (m_accQ) {
		text1 += acctext;
	}
	text1 += "g.labeltext&#123;color:gray;&#125;";
	text1 += "g.lastnote&#123;color:gray;&#125;";
	if (m_extremaQ) {
		text1 += "g.highest-pitch&#123;color:hotpink;&#125;";
		text1 += "g.lowest-pitch&#123;color:limegreen;&#125;";
	}
	text1 += "</style>";
	string text2 = text1;


	// print CSS style information if requested
	if (m_hoverQ) {
		SVGTEXT(out, text1);
	}

	int maxStaffPosition = getMaxStaffPosition(voiceInfo);

	if (!titlestring.empty()) {
		// print title
		int vpos = 54;
		if (maxStaffPosition > 12) {
			vpos = maxStaffPosition + 3;
		}
		out << "t 2 10 ";
		out << vpos;
		out << " 1 1 0 0 0 0 -1.35\n";
		// out << "_03";
		printScoreEncodedText(out, titlestring);
		out << "\n";
	}

	// print duration label if duration weighting is being used
	SVGTEXT(out, "<g class=\"labeltext\">");
	if (m_durationQ) {
		out << "t 2 185.075 14 1 0.738 0 0 0 0 0\n";
		out << "_00(durations)\n";
	} else {
		out << "t 2 185.075 14 1 0.738 0 0 0 0 0\n";
		out << "_00(attacks)\n";
	}
	SVGTEXT(out, "</g>");

	// print staff lines
	out << "8 1 0 0 0 200\n";   // staff 1
	out << "8 2 0 -6 0 200\n";   // staff 2

	int keysig = getKeySignature(infile);
	// print key signature
	if (keysig) {
		out << "17 1 10 0 " << keysig << " 101.0";
		printKeySigCompression(out, keysig, 0);
		out << endl;
		out << "17 2 10 0 " << keysig;
		printKeySigCompression(out, keysig, 1);
		out << endl;
	}

	// print barlines
	out << "14 1 0 2\n";         // starting barline
	out << "14 1 200 2\n";       // ending barline
	out << "14 1 0 2 8\n";       // curly brace at start

	// print clefs
	out << "3 2 2\n";            // treble clef
	out << "3 1 2 0 1\n";        // bass clef

	assignHorizontalPosition(voiceInfo, 25.0, 170.0);

	double maxvalue = 0.0;
	for (int i=1; i<(int)voiceInfo.size(); i++) {
		double tempvalue = getMaxValue(voiceInfo.at(i).diatonic);
		if (tempvalue > maxvalue) {
			maxvalue = tempvalue;
		}
	}
	for (int i=(int)voiceInfo.size()-1; i>0; i--) {
		if (voiceInfo.at(i).kernQ) {
			printScoreVoice(out, voiceInfo.at(i), maxvalue);
		}
	}
	if (m_allQ) {
		printScoreVoice(out, voiceInfo.at(0), maxvalue);
	}
}


//////////////////////////////
//
// Tool_prange::getMaxStaffPosition(vector<_VoiceInfo>& voiceinfo) {
//

int Tool_prange::getMaxStaffPosition(vector<_VoiceInfo>& voiceInfo) {
	int maxi = getMaxDiatonicIndex(voiceInfo[0].diatonic);
	int maxdiatonic = maxi - 3 * 7;
	int staffline = maxdiatonic - 27;
	return staffline;
}




//////////////////////////////
//
// Tool_prange::printKeySigCompression --
//

void Tool_prange::printKeySigCompression(ostream& out, int keysig, int extra) {
	double compression = 0.0;
	switch (abs(keysig)) {
		case 0: compression = 0.0; break;
		case 1: compression = 0.0; break;
		case 2: compression = 0.0; break;
		case 3: compression = 0.0; break;
		case 4: compression = 0.9; break;
		case 5: compression = 0.8; break;
		case 6: compression = 0.7; break;
		case 7: compression = 0.6; break;
	}
	if (compression <= 0.0) {
		return;
	}
	for (int i=0; i<extra; i++) {
		out << " 0";
	}
	out << " " << compression;
}



//////////////////////////////
//
// Tool_prange::assignHorizontalPosition --
//

void Tool_prange::assignHorizontalPosition(vector<_VoiceInfo>& voiceInfo, int minval, int maxval) {
	int count = 0;
	for (int i=1; i<(int)voiceInfo.size(); i++) {
		if (voiceInfo[i].kernQ) {
			count++;
		}
	}
	if (m_allQ) {
		count++;
	}

	vector<double> hpos(count, 0);
	if (count >= 2) {
		hpos[0] = maxval;
		hpos.back() = minval;
	}

	if (hpos.size() > 2) {
		for (int i=1; i<(int)hpos.size()-1; i++) {
			int ii = hpos.size() - i - 1;
			hpos[i] = (double)ii / (hpos.size()-1) * (maxval - minval) + minval;
		}
	}

	int position = 0;
	if (m_allQ) {
		position = 1;
		voiceInfo[0].hpos = hpos[0];
	}
	for (int i=0; i<(int)voiceInfo.size(); i++) {
		if (voiceInfo.at(i).kernQ) {
			voiceInfo.at(i).hpos = hpos.at(position++);
		}
	}
}



//////////////////////////////
//
// Tool_prange::getKeySignature -- find first key signature in file.
//

int Tool_prange::getKeySignature(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			if (infile[i].isData()) {
				break;
			}
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->isKeySignature()) {
				return Convert::kernKeyToNumber(*token);
			}
		}
	}

	return 0; // C major key signature
}



//////////////////////////////
//
// Tool_prange::printScoreVoice -- print the range information for a particular voice (in SCORE format).
//

void Tool_prange::printScoreVoice(ostream& out, _VoiceInfo& voiceInfo, double maxvalue) {
	int mini = getMinDiatonicIndex(voiceInfo.diatonic);
	int maxi = getMaxDiatonicIndex(voiceInfo.diatonic);

	if ((mini < 0) || (maxi < 0)) {
		// no data for voice so skip
		return;
	}

	// int minacci = getMinDiatonicAcc(voiceInfo.diatonic, mini);
	// int maxacci = getMaxDiatonicAcc(voiceInfo.diatonic, maxi);
	int mindiatonic = mini - 3 * 7;
	int maxdiatonic = maxi - 3 * 7;
	// int minacc = minacci - 3;
	// int maxacc = maxacci - 3;

	int    staff;
	double vpos;

	int voicevpos = -3;
	staff = getStaffBase7(mindiatonic);
	int lowestvpos = getVpos(mindiatonic);
	if ((staff == 1) && (lowestvpos <= 0)) {
		voicevpos += lowestvpos - 2;
	}

	if (m_localQ || (voiceInfo.index == 0)) {
		double localmaxvalue = getMaxValue(voiceInfo.diatonic);
		maxvalue = localmaxvalue;
	}
	double width;
	double hoffset = 2.3333;
	double maxhist = 17.6;
	int i;
	int base7;

	// print histogram bars
	for (i=mini; i<=maxi; i++) {
		if (voiceInfo.diatonic.at(i).at(0) <= 0.0) {
			continue;
		}
		base7 = i - 3 * 7;
		staff = getStaffBase7(base7);
		vpos  = getVpos(base7);

		// staring positions of accidentals:
		vector<double> starthpos(6, 0.0);
		for (int j=1; j<(int)starthpos.size(); j++) {
			double width = maxhist * voiceInfo.diatonic.at(i).at(j)/maxvalue;
			starthpos[j] = starthpos[j-1] + width;
		}
		for (int j=(int)starthpos.size() - 1; j>0; j--) {
			starthpos[j] = starthpos[j-1];
		}

		// print chromatic alterations
		for (int j=(int)voiceInfo.diatonic.at(i).size()-1; j>0; j--) {
			if (voiceInfo.diatonic.at(i).at(j) <= 0.0) {
				continue;
			}
			int acc = 0;
			switch (j) {
				case 1: acc = -2; break;
				case 2: acc = -1; break;
				case 3: acc =  0; break;
				case 4: acc = +1; break;
				case 5: acc = +2; break;
			}

			width = maxhist * voiceInfo.diatonic.at(i).at(j)/maxvalue + hoffset;
			if (m_hoverQ) {
				string title = getNoteTitle((int)voiceInfo.diatonic.at(i).at(j), base7, acc);
				SVGTEXT(out, title);
			}
			out << "1 " << staff << " " << (voiceInfo.hpos + starthpos.at(j) + hoffset) << " " << vpos;
			out << " 0 -1 4 0 0 0 99 0 0 ";
			out << width << "\n";
			if (m_hoverQ) {
				SVGTEXT(out, "</g>");
			}
		}
	}

	string voicestring = voiceInfo.name;
	if (voicestring.empty()) {
		voicestring = voiceInfo.abbr;
	}
	if (!voicestring.empty()) {
		HumRegex hre;
		hre.replaceDestructive(voicestring, "", "(\\\\n)+$");
		vector<string> pieces;
		hre.split(pieces, voicestring, "\\\\n");

		if (pieces.size() > 1) {
			voicestring = "";
			for (int i=0; i<(int)pieces.size(); i++) {
				voicestring += pieces[i];
				if (i < (int)pieces.size() - 1) {
					voicestring += "/";
				}
			}
		}

		double increment = 4.0;
		for (int i=0; i<(int)pieces.size(); i++) {
			// print voice name
			double tvoffset = -4.0;
			out << "t 1 " << voiceInfo.hpos << " "
				<< (voicevpos - increment * i)
			  	<< " 1 1 0 0 0 0 " << tvoffset;
			out << "\n";

			if (pieces[i] == "all") {
				out << "_02";
			} else if (pieces[i] == "both") {
				out << "_02";
			} else {
				out << "_00";
			}
			printScoreEncodedText(out, pieces[i]);
			out << "\n";
		}
	}

	// print the lowest pitch in range
	staff = getStaffBase7(mindiatonic);
	vpos = getVpos(mindiatonic);
	if (m_hoverQ) {
		string content = "<g class=\"lowest-pitch\"><title>";
		content += getDiatonicPitchName(mindiatonic, 0);
		content += ": lowest note";
		if (!voicestring.empty()) {
			content += " of ";
			content += voicestring;
			content += "'s range";
		}
		content += "</title>";
		SVGTEXT(out, content);
	}
	out << "1 " << staff << " " << voiceInfo.hpos << " " << vpos
		  << " 0 0 4 0 0 -2\n";
	if (m_hoverQ) {
		SVGTEXT(out, "</g>");
	}

	// print the highest pitch in range
	staff = getStaffBase7(maxdiatonic);
	vpos = getVpos(maxdiatonic);
	if (m_hoverQ) {
		string content = "<g class=\"highest-pitch\"><title>";
		content += getDiatonicPitchName(maxdiatonic, 0);
		content += ": highest note";
		if (!voicestring.empty()) {
			content += " of ";
			content += voicestring;
			content += "'s range";
		}
		content += "</title>";
		SVGTEXT(out, content);
	}
	out << "1 " << staff << " " << voiceInfo.hpos << " " << vpos
		  << " 0 0 4 0 0 -2\n";
	if (m_hoverQ) {
		SVGTEXT(out, "</g>");
	}

	double goffset  = -1.66;
	double toffset  = 1.5;
	double median12 = getMedian12(voiceInfo.midibins);
	double median40 = Convert::base12ToBase40(median12);
	double median7  = Convert::base40ToDiatonic(median40);
	// int    acc      = Convert::base40ToAccidental(median40);

	staff = getStaffBase7(median7);
	vpos = getVpos(median7);

	// these offsets are useful when the quartile pitches are not shown...
	int vvpos = maxdiatonic - median7 + 1;
	int vvpos2 = median7 - mindiatonic + 1;
	double offset = goffset;
	if (vvpos <= 2) {
		offset += toffset;
	} else if (vvpos2 <= 2) {
		offset -= toffset;
	}

	if (m_hoverQ) {
		string content = "<g><title>";
		content += getDiatonicPitchName(median7, 0);
		content += ": median note";
		if (!voicestring.empty()) {
			content += " of ";
			content += voicestring;
			content += "'s range";
		}
		content += "</title>";
		SVGTEXT(out, content);
	}
	out << "1 " << staff << " " << voiceInfo.hpos << " ";
	if (vpos > 0) {
		out << vpos + 100;
	} else {
		out << vpos - 100;
	}
	out << " 0 1 4 0 0 " << offset << "\n";
	if (m_hoverQ) {
		SVGTEXT(out, "</g>");
	}

	if (m_finalisQ) {
		for (int f=0; f<(int)voiceInfo.diafinal.size(); f++) {
			int diafinalis = voiceInfo.diafinal.at(f);
			int accfinalis = voiceInfo.accfinal.at(f);
			int staff = getStaffBase7(diafinalis);
			int vpos = getVpos(diafinalis);
			double goffset = -1.66;
			double toffset = 3.5;

			// these offsets are useful when the quartile pitches are not shown...
			double offset = goffset;
			offset += toffset;

			if (m_hoverQ) {
				string content = "<g class=\"lastnote\"><title>";
				content += getDiatonicPitchName(diafinalis, accfinalis);
				content += ": last note";
				if (!voicestring.empty()) {
					content += " of ";
					if (voiceInfo.index == 0) {
						content += voiceInfo.namfinal.at(f);
					} else {
						content += voicestring;
					}
				}
				content += "</title>";
				SVGTEXT(out, content);
			}
			out << "1 " << staff << " " << voiceInfo.hpos << " ";
			if (vpos > 0) {
				out << vpos + 100;
			} else {
				out << vpos - 100;
			}
			out << " 0 0 4 0 0 " << offset << "\n";
			if (m_hoverQ) {
				SVGTEXT(out, "</g>");
			}
		}
	}

	/* Needs fixing
	int topquartile;
	if (m_quartileQ) {
		// print top quartile
		topquartile = getTopQuartile(voiceInfo.midibins);
		if (m_diatonicQ) {
			topquartile = Convert::base7ToBase12(topquartile);
		}
		staff = getStaffBase7(topquartile);
		vpos = getVpos(topquartile);
		vvpos = median7 - topquartile + 1;
		if (vvpos <= 2) {
			offset = goffset + toffset;
		} else {
			offset = goffset;
		}
		vvpos = maxdiatonic - topquartile + 1;
		if (vvpos <= 2) {
			offset = goffset + toffset;
		}

		if (m_hoverQ) {
			if (m_defineQ) {
				out << "SVG ";
			} else {
				out << "t 1 1\n";
				out << SVGTAG;
			}
			printScoreEncodedText(out, "<g><title>");
			printDiatonicPitchName(out, topquartile, 0);
			out << ": top quartile note";
			if (voicestring.size() > 0) {
				out <<  " of " << voicestring << "\'s range";
			}
			printScoreEncodedText(out, "</title>\n");
		}
		out << "1 " << staff << " " << voiceInfo.hpos << " ";
		if (vpos > 0) {
			out << vpos + 100;
		} else {
			out << vpos - 100;
		}
		out << " 0 0 4 0 0 " << offset << "\n";
		if (m_hoverQ) {
			SVGTEXT(out, "</g>");
		}
	}

	// print bottom quartile
	if (m_quartileQ) {
		int bottomquartile = getBottomQuartile(voiceInfo.midibins);
		if (m_diatonicQ) {
			bottomquartile = Convert::base7ToBase12(bottomquartile);
		}
		staff = getStaffBase7(bottomquartile);
		vpos = getVpos(bottomquartile);
		vvpos = median7 - bottomquartile + 1;
		if (vvpos <= 2) {
			offset = goffset + toffset;
		} else {
			offset = goffset;
		}
		vvpos = bottomquartile - mindiatonic + 1;
		if (vvpos <= 2) {
			offset = goffset - toffset;
		}
		if (m_hoverQ) {
			if (m_defineQ) {
				out << "SVG ";
			} else {
				out << "t 1 1\n";
				out << SVGTAG;
			}
			printScoreEncodedText(out, "<g><title>");
			printDiatonicPitchName(out, bottomquartile, 0);
			out << ": bottom quartile note";
			if (voicestring.size() > 0) {
				out <<  " of " << voicestring << "\'s range";
			}
			printScoreEncodedText(out, "</title>\n");
		}
		out << "1.0 " << staff << ".0 " << voiceInfo.hpos << " ";
		if (vpos > 0) {
			out << vpos + 100;
		} else {
			out << vpos - 100;
		}
		out << " 0 0 4 0 0 " << offset << "\n";
		if (m_hoverQ) {
			SVGTEXT(out, "</g>");
		}
	}
	*/

}



//////////////////////////////
//
// Tool_prange::printDiatonicPitchName --
//

void Tool_prange::printDiatonicPitchName(ostream& out, int base7, int acc) {
	out << getDiatonicPitchName(base7, acc);
}



//////////////////////////////
//
// Tool_prange::getDiatonicPitchName --
//

string Tool_prange::getDiatonicPitchName(int base7, int acc) {
	string output;
	int dpc = base7 % 7;
	char letter = (dpc + 2) % 7 + 'A';
	output += letter;
	switch (acc) {
		case -1: output += "&#9837;"; break;
		case +1: output += "&#9839;"; break;
		case -2: output += "&#119083;"; break;
		case +2: output += "&#119082;"; break;
	}
	int octave = base7 / 7;
	output += to_string(octave);
	return output;
}



//////////////////////////////
//
// Tool_prange::printHtmlStringEncodeSimple --
//

void Tool_prange::printHtmlStringEncodeSimple(ostream& out, const string& strang) {
	string newstring = strang;
	HumRegex hre;
	hre.replaceDestructive(newstring, "&", "&amp;", "g");
	hre.replaceDestructive(newstring, "<", "&lt;", "g");
	hre.replaceDestructive(newstring, ">", "&lt;", "g");
	out << newstring;
}



//////////////////////////////
//
// Tool_prange::getNoteTitle -- return the title of the histogram bar.
//    value = duration or count of notes
//    diatonic = base7 value for note
//    acc = accidental for diatonic note.
//

string Tool_prange::getNoteTitle(double value, int diatonic, int acc) {
	stringstream output;
	output << "<g class=\"bar";
	switch (acc) {
		case -2: output << " doubleflat";  break;
		case -1: output << " flat";        break;
		case  0: output << " natural";     break;
		case +1: output << " sharp";       break;
		case +2: output << " doublesharp"; break;
	}
	output << "\"";
	output << "><title>";
	if (m_durationQ) {
		output << value / 8.0;
		if (value/8.0 == 1.0) {
			output << " long on ";
		} else {
			output << " longs on ";
		}
		output << getDiatonicPitchName(diatonic, acc);
	} else {
		output << value;
		output << " ";
		output << getDiatonicPitchName(diatonic, acc);
		if (value != 1.0) {
			output << "s";
		}
	}
	output << "</title>";
	return output.str();
}



//////////////////////////////
//
// Tool_prange::getDiatonicInterval --
//

int Tool_prange::getDiatonicInterval(int note1, int note2) {
	int vpos1 = getVpos(note1);
	int vpos2 = getVpos(note2);
	return abs(vpos1 - vpos2) + 1;
}



//////////////////////////////
//
// Tool_prange::getTopQuartile --
//

int Tool_prange::getTopQuartile(vector<double>& midibins) {
	double sum = accumulate(midibins.begin(), midibins.end(), 0.0);

	double cumsum = 0.0;
	int i;
	for (i=midibins.size()-1; i>=0; i--) {
		if (midibins[i] <= 0.0) {
			continue;
		}
		cumsum += midibins[i]/sum;
		if (cumsum >= 0.25) {
			return i;
		}
	}

	return -1;
}



//////////////////////////////
//
// Tool_prange::getBottomQuartile --
//

int Tool_prange::getBottomQuartile(vector<double>& midibins) {
	double sum = accumulate(midibins.begin(), midibins.end(), 0.0);

	double cumsum = 0.0;
	int i;
	for (i=0; i<(int)midibins.size(); i++) {
		if (midibins[i] <= 0.0) {
			continue;
		}
		cumsum += midibins[i]/sum;
		if (cumsum >= 0.25) {
			return i;
		}
	}

	return -1;
}



//////////////////////////////
//
// Tool_prange::getMaxValue --
//

double Tool_prange::getMaxValue(vector<vector<double>>& bins) {
	double maxi = 0;
	for (int i=1; i<(int)bins.size(); i++) {
		if (bins.at(i).at(0) > bins.at(maxi).at(0)) {
			maxi = i;
		}
	}
	return bins.at(maxi).at(0);
}



//////////////////////////////
//
// Tool_prange::getVpos == return the position on the staff given the diatonic pitch.
//     and the staff. 1=bass, 2=treble.
//     3 = bottom line of clef, 0 = space below first ledger line.
//

double Tool_prange::getVpos(double base7) {
	double output = 0;
	if (base7 < 4 * 7) {
		// bass clef
		output = base7 - (1 + 2*7);  // D2
	} else {
		// treble clef
		output = base7 - (6 + 3*7);  // B3
	}
	return output;
}



//////////////////////////////
//
// Tool_prange::getStaffBase7 -- return 1 if less than middle C; otherwise return 2.
//

int Tool_prange::getStaffBase7(int base7) {
	if (base7 < 4 * 7) {
		return 1;
	} else {
		return 2;
	}
}


//////////////////////////////
//
// Tool_prange::getMaxDiatonicIndex -- return the highest non-zero content.
//

int Tool_prange::getMaxDiatonicIndex(vector<vector<double>>& diatonic) {
	for (int i=diatonic.size()-1; i>=0; i--) {
		if (diatonic.at(i).at(0) != 0.0) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// Tool_prange::getMinDiatonicIndex -- return the lowest non-zero content.
//

int Tool_prange::getMinDiatonicIndex(vector<vector<double>>& diatonic) {
	for (int i=0; i<(int)diatonic.size(); i++) {
		if (diatonic.at(i).at(0) != 0.0) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// Tool_prange::getMinDiatonicAcc -- return the lowest accidental.
//

int Tool_prange::getMinDiatonicAcc(vector<vector<double>>& diatonic, int index) {
	for (int i=1; i<(int)diatonic.at(index).size(); i++) {
		if (diatonic.at(index).at(i) != 0.0) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// Tool_prange::getMaxDiatonicAcc -- return the highest accidental.
//

int Tool_prange::getMaxDiatonicAcc(vector<vector<double>>& diatonic, int index) {
	for (int i=(int)diatonic.at(index).size() - 1; i>0; i--) {
		if (diatonic.at(index).at(i) != 0.0) {
			return i;
		}
	}
	return -1;
}



//////////////////////////////
//
// Tool_prange::prepareRefmap --
//

void Tool_prange::prepareRefmap(HumdrumFile& infile) {
	vector<HLp> refrecords = infile.getGlobalReferenceRecords();
	m_refmap.clear();
	HumRegex hre;
	for (int i = (int)refrecords.size()-1; i>=0; i--) {
		string key = refrecords[i]->getReferenceKey();
		string value = refrecords[i]->getReferenceValue();
		m_refmap[key] = value;
		if (key.find("@") != string::npos) {
			// create default value
			hre.replaceDestructive(key, "", "@.*");
			if (m_refmap[key].empty()) {
				m_refmap[key] = value;
			}
		}
	}
	// fill in @{} templates (mostly for !!!title:)
	int counter = 0; // prevent recursions
	for (auto& entry : m_refmap) {

		if (entry.second.find("@") != string::npos) {
			while (hre.search(entry.second, "@\\{(.*?)\\}")) {
				string key = hre.getMatch(1);
				string value = m_refmap[key];
				hre.replaceDestructive(entry.second, value, "@\\{" + key + "\\}", "g");
				counter++;
				if (counter > 1000) {
					break;
				}
			}
		}

	}

	// prepare title
	if (m_refmap["title"].empty()) {
		m_refmap["title"] = m_refmap["OTL"];
	}
}



//////////////////////////////
//
// Tool_prange::getTitle --
//

string Tool_prange::getTitle(void) {
	string titlestring = "_00";
	HumRegex hre;
	if (m_notitleQ) {
		return "";
	} else if (m_titleQ) {
		titlestring = m_title;
		int counter = 0;

		if (titlestring.find("@") != string::npos) {
			while (hre.search(titlestring, "@\\{(.*?)\\}")) {
				string key = hre.getMatch(1);
				string value = m_refmap[key];
				hre.replaceDestructive(titlestring, value, "@\\{" + key + "\\}", "g");
				counter++;
				if (counter > 1000) {
					break;
				}
			}
		}
		if (!titlestring.empty()) {
			titlestring = "_00" + titlestring;
		}
	} else {
		titlestring = m_refmap["title"];
		if (!titlestring.empty()) {
			titlestring = "_00" + titlestring;
		}
	}
	return titlestring;
}



//////////////////////////////
//
// Tool_prange::clearHistograms --
//

void Tool_prange::clearHistograms(vector<vector<double> >& bins, int start) {
	int i;
	for (i=start; i<(int)bins.size(); i++) {
		bins[i].resize(40*11);
		fill(bins[i].begin(), bins[i].end(), 0.0);
		// bins[i].allowGrowth(0);
	}
	for (int i=0; i<(int)bins.size(); i++) {
		if (bins[i].size() == 0) {
			bins[i].resize(40*11);
			fill(bins[i].begin(), bins[i].end(), 0.0);
		}
	}
}




//////////////////////////////
//
// Tool_prange::printAnalysis --
//

void Tool_prange::printAnalysis(ostream& out, vector<double>& midibins) {
	if (m_percentileQ) {
		printPercentile(out, midibins, m_percentile);
		return;
	}  else if (m_rangeQ) {
		double notesinrange = countNotesInRange(midibins, m_rangeL, m_rangeH);
		out << notesinrange << endl;
		return;
	}

	int i;
	double normval = 1.0;

	// print the pitch histogram

	double fracL = 0.0;
	double fracH = 0.0;
	double fracA = 0.0;
	double sum = accumulate(midibins.begin(), midibins.end(), 0.0);
	if (m_normQ) {
		normval = sum;
	}
	double runningtotal = 0.0;


	out << "**keyno\t";
	if (m_pitchQ) {
		out << "**pitch";
	} else {
		out << "**kern";
	}
	out << "\t**count";
	if (m_addFractionQ) {
		out << "\t**fracL";
		out << "\t**fracA";
		out << "\t**fracH";
	}
	out << "\n";


	int base12;

	if (!m_reverseQ) {
		for (i=0; i<(int)midibins.size(); i++) {
			if (midibins[i] <= 0.0) {
				continue;
			}
			if (m_diatonicQ) {
				base12 = Convert::base7ToBase12(i);
			} else {
				base12 = i;
			}
			out << base12 << "\t";
			if (m_pitchQ) {
				out << Convert::base12ToPitch(base12);
			} else {
				out << Convert::base12ToKern(base12);
			}
			out << "\t";
			out << midibins[i] / normval;
			fracL = runningtotal/sum;
			runningtotal += midibins[i];
			fracH = runningtotal/sum;
			fracA = (fracH + fracL)/2.0;
			fracL = (int)(fracL * 10000.0 + 0.5)/10000.0;
			fracH = (int)(fracH * 10000.0 + 0.5)/10000.0;
			fracA = (int)(fracA * 10000.0 + 0.5)/10000.0;
			if (m_addFractionQ) {
				out << "\t" << fracL;
				out << "\t" << fracA;
				out << "\t" << fracH;
			}
			out << "\n";
		}
	} else {
		for (i=(int)midibins.size()-1; i>=0; i--) {
			if (midibins[i] <= 0.0) {
				continue;
			}
			if (m_diatonicQ) {
				base12 = Convert::base7ToBase12(i);
			} else {
				base12 = i;
			}
			out << base12 << "\t";
			if (m_pitchQ) {
				out << Convert::base12ToPitch(base12);
			} else {
				out << Convert::base12ToKern(base12);
			}
			out << "\t";
			out << midibins[i] / normval;
			fracL = runningtotal/sum;
			runningtotal += midibins[i];
			fracH = runningtotal/sum;
			fracA = (fracH + fracL)/2.0;
			fracL = (int)(fracL * 10000.0 + 0.5)/10000.0;
			fracH = (int)(fracH * 10000.0 + 0.5)/10000.0;
			fracA = (int)(fracA * 10000.0 + 0.5)/10000.0;
			if (m_addFractionQ) {
				out << "\t" << fracL;
				out << "\t" << fracA;
				out << "\t" << fracH;
			}
			out << "\n";
		}
	}

	out << "*-\t*-\t*-";
	if (m_addFractionQ) {
		out << "\t*-";
		out << "\t*-";
		out << "\t*-";
	}
	out << "\n";

	out << "!!tessitura:\t" << getTessitura(midibins) << " semitones\n";

	double mean = getMean12(midibins);
	if (m_diatonicQ && (mean > 0)) {
		mean = Convert::base7ToBase12(mean);
	}
	out << "!!mean:\t\t" << mean;
	out << " (";
	if (mean < 0) {
		out << "unpitched";
	} else {
		out << Convert::base12ToKern(int(mean+0.5));
	}
	out << ")" << "\n";

	int median12 = getMedian12(midibins);
	out << "!!median:\t" << median12;
	out << " (";
	if (median12 < 0) {
		out << "unpitched";
	} else {
		out << Convert::base12ToKern(median12);
	}
	out << ")" << "\n";

}



//////////////////////////////
//
// Tool_prange::getMedian12 -- return the pitch on which half of pitches are above
//     and half are below.
//

int Tool_prange::getMedian12(vector<double>& midibins) {
	double sum = accumulate(midibins.begin(), midibins.end(), 0.0);

	double cumsum = 0.0;
	int i;
	for (i=0; i<(int)midibins.size(); i++) {
		if (midibins[i] <= 0.0) {
			continue;
		}
		cumsum += midibins[i]/sum;
		if (cumsum >= 0.50) {
			return i;
		}
	}

	return -1000;
}



//////////////////////////////
//
// Tool_prange::getMean12 -- return the interval between the highest and lowest
//     pitch in terms if semitones.
//

double Tool_prange::getMean12(vector<double>& midibins) {
	double top    = 0.0;
	double bottom = 0.0;

	int i;
	for (i=0; i<(int)midibins.size(); i++) {
		if (midibins[i] <= 0.0) {
			continue;
		}
		top += midibins[i] * i;
		bottom += midibins[i];

	}

	if (bottom == 0) {
		return -1000;
	}
	return top / bottom;
}



//////////////////////////////
//
// Tool_prange::getTessitura -- return the interval between the highest and lowest
//     pitch in terms if semitones.
//

int Tool_prange::getTessitura(vector<double>& midibins) {
	int minn = -1000;
	int maxx = -1000;
	int i;

	for (i=0; i<(int)midibins.size(); i++) {
		if (midibins[i] <= 0.0) {
			continue;
		}
		if (minn < 0) {
			minn = i;
		}
		if (maxx < 0) {
			maxx = i;
		}
		if (minn > i) {
			minn = i;
		}
		if (maxx < i) {
			maxx = i;
		}
	}
	if (m_diatonicQ) {
		maxx = Convert::base7ToBase12(maxx);
		minn = Convert::base7ToBase12(minn);
	}

	return maxx - minn + 1;
}



//////////////////////////////
//
// Tool_prange::countNotesInRange --
//

double Tool_prange::countNotesInRange(vector<double>& midibins, int low, int high) {
	int i;
	double sum = 0;
	for (i=low; i<=high; i++) {
		sum += midibins[i];
	}
	return sum;
}



//////////////////////////////
//
// Tool_prange::printPercentile --
//

void Tool_prange::printPercentile(ostream& out, vector<double>& midibins, double m_percentile) {
	double sum = accumulate(midibins.begin(), midibins.end(), 0.0);
	double runningtotal = 0.0;
	int i;
	for (i=0; i<(int)midibins.size(); i++) {
		if (midibins[i] <= 0) {
			continue;
		}
		runningtotal += midibins[i] / sum;
		if (runningtotal >= m_percentile) {
			out << i << endl;
			return;
		}
	}

	out << "unknown" << endl;
}



//////////////////////////////
//
// Tool_prange::getRange --
//

void Tool_prange::getRange(int& rangeL, int& rangeH, const string& rangestring) {
	rangeL = -1; rangeH = -1;
	if (rangestring.empty()) {
		return;
	}
	int length = (int)rangestring.length();
	char* buffer = new char[length+1];
	strcpy(buffer, rangestring.c_str());
	char* ptr;
	if (std::isdigit(buffer[0])) {
		ptr = strtok(buffer, " \t\n:-");
		sscanf(ptr, "%d", &rangeL);
		ptr = strtok(NULL, " \t\n:-");
		if (ptr != NULL) {
			sscanf(ptr, "%d", &rangeH);
		}
	} else {
		ptr = strtok(buffer, " :");
		if (ptr != NULL) {
			rangeL = Convert::kernToMidiNoteNumber(ptr);
			ptr = strtok(NULL, " :");
			if (ptr != NULL) {
				rangeH = Convert::kernToMidiNoteNumber(ptr);
			}
		}
	}

	if (rangeH < 0) {
		rangeH = rangeL;
	}

	if (rangeL <   0) { rangeL =   0; }
	if (rangeH <   0) { rangeH =   0; }
	if (rangeL > 127) { rangeL = 127; }
	if (rangeH > 127) { rangeH = 127; }
	if (rangeL > rangeH) {
		int temp = rangeL;
		rangeL = rangeH;
		rangeH = temp;
	}

}



// END_MERGE

} // end namespace hum



