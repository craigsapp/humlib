//
// Programmer:    Katherine Wong
// Programmer:    Craig Stuart Sapp
// Creation Date: Mon Mar 13 23:45:00 PDT 2023
// Last Modified: Mon Jun 12 20:22:01 PDT 2023
// Filename:      tool-tspos.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-tspos.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color the positions of notes in triads according to
//                their thirds position:
//                   red   = root
//                   green = third
//                   blue  = fifth
//
// Todo:
//
// implement -D = when -d is used to highlight only the doubled pitch
//                classes in a chord, the -D means to color the other
//                notes in the chord gray.  Place a marker (|) on the
//                non-doubled sonority notes (when it is a triad).
//

#include "tool-tspos.h"
#include "HumRegex.h"

#include <iomanip>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_tspos::Tool_tspos -- Set the recognized options for the tool.
//

Tool_tspos::Tool_tspos(void) {
	define("d|double=b",        "highlight only doubled notes in triads");
	define("3|no-thirds=b",     "do not color thirds");
	define("5|no-fifths=b",     "do not color fifths");
	define("T|no-triads=b",     "do not color full triads");
	define("m|minor-triads=b",  "only analyze major triad");
	define("M|major-triads=b",  "only analyze minor triads");
	define("x|attacks=b",       "only process sonorities with three unique triadic pitch classes attacking at once (sustains in additional voices are allowed)");
	define("v|voice-count=i:0", "only analyze sonorities with given voice count");
	define("c|compressed=b",    "compress music to see more on each system");
	define("top=b",             "mark top voice in analysis output");
	define("t|table=b",         "add analysis table above score");
	define("V|all-voices=b",    "require all voices in score to be sounding");
	define("Q|no-question=b",   "do not show question mark in table header");
}



/////////////////////////////////
//
// Tool_tspos::run -- Do the main work of the tool.
//

bool Tool_tspos::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_tspos::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tspos::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_tspos::run(HumdrumFile& infile) {
	initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_tspos::initialize -- Setup to do before processing a file.
//

void Tool_tspos::initialize(HumdrumFile& infile) {
	m_colorThirds = !getBoolean("no-thirds");
	m_colorFifths = !getBoolean("no-fifths");
	m_colorTriads = !getBoolean("no-triads");
	m_doubleQ = getBoolean("double");
	m_topQ = getBoolean("top");
	m_tableQ = getBoolean("table");
	m_compressedQ = getBoolean("compressed");
	m_voice = getInteger("voice-count");
	if (getBoolean("all-voices")) {
		vector<HTp> kernSpines =  infile.getKernSpineStartList();
		m_voice = (int)kernSpines.size();
	}
	m_triadAttack = getBoolean("attacks");
	m_majorQ = true;
	m_minorQ = true;
	if (getBoolean("major-triads")) {
		m_majorQ = true;
		m_minorQ = false;
	}
	if (getBoolean("minor-triads")) {
		m_majorQ = false;
		m_minorQ = true;
	}
	m_questionQ = !getBoolean("no-question");
}



//////////////////////////////
//
// Tool_tspos::processFile -- Analyze an input file.
//

void Tool_tspos::processFile(HumdrumFile& infile) {
	// Algorithm go line by line in the infile, extracting the notes that are active
	// check to see if the list of notes form a triad
	// label the root third and fifth notes of the triad

	m_toolCount = getToolCounter(infile);

	m_used_markers.resize(7);
	fill(m_used_markers.begin(), m_used_markers.end(), 0);

	avoidRdfCollisions(infile);
	analyzeVoiceCount(infile);

	m_partTriadPositions.resize(infile.getMaxTrack() + 1);
	for (int i = 0; i < (int)infile.getMaxTrack() + 1; i++) {
		m_partTriadPositions.at(i).resize(m_positionCount);
		fill(m_partTriadPositions.at(i).begin(), m_partTriadPositions.at(i).end(), 0);
	}

	m_triadState.clear();
	m_triadState.resize(infile.getLineCount());

	vector<HTp> kernNotes;
	vector<int> midiNotes;
	vector<int> chordPositions;
	vector<int> thirdPositions;
	vector<int> fifthPositions;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) { // if no notes in the line
			continue;
		}
		if ((m_voice > 0)&& (m_voiceCount.at(i) != m_voice)) {
			// Ignore sonorities that do not have the required number of
			// voices.  m_voices==0 means consider all voice counts.
			continue;
		}
		if (m_triadAttack) {
			if (!hasFullTriadAttack(infile[i])) {
				continue;
			}
		}

		// iterate along the line looking at each field, and creating a
		//     list of tokens that are notes.
		kernNotes.clear();
		midiNotes.clear();
		chordPositions.clear();
		thirdPositions.clear();
		fifthPositions.clear();
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j); // get ptr to cell
			// if not a **kern token, then skip:
			if (!token->isKern()) {
				continue;
			}

			if (token->isRest()) {
				continue;
			}

			HTp resolvedToken = NULL;
			if (token->isNull()) {
				resolvedToken = token->resolveNull();
				if (!resolvedToken || resolvedToken->isRest()) {
					continue;
				}
			}
			if (resolvedToken) {
				kernNotes.push_back(resolvedToken);
			} else {
				kernNotes.push_back(token);
			}
		}
		midiNotes = getMidiNotes(kernNotes);

		if (m_colorThirds) { // thirds
			thirdPositions = getThirds(midiNotes);
			checkForTriadicSonority(thirdPositions, i);

			if (m_doubleQ) { // label only doubles if prompted
				keepOnlyDoubles(thirdPositions);
			}

			labelThirds(kernNotes, thirdPositions);
		}

		if (m_colorFifths) { // fifths
			fifthPositions = getFifths(midiNotes);
			checkForTriadicSonority(fifthPositions, i);

			if (m_doubleQ) { // label only doubles if prompted
				keepOnlyDoubles(fifthPositions);
			}

			labelFifths(kernNotes, fifthPositions);
		}

		if (m_colorTriads) { // triads
		  	chordPositions = getChordPositions(midiNotes);
			checkForTriadicSonority(chordPositions, i);
			if (m_doubleQ) { // label only doubles if prompted
				keepOnlyDoubles(chordPositions);
			}
	 		labelChordPositions(kernNotes, chordPositions);
		}
	}

	infile.createLinesFromTokens();

	m_humdrum_text << infile;

	printUsedMarkers();

	string statistics = generateStatistics(infile);
	m_humdrum_text << statistics;
	if (m_compressedQ) {
		m_humdrum_text << "!!!verovio: evenNoteSpacing\n";
	}
}



//////////////////////////////
//
// Tool_tspos::printUsedMarkers --
//

void Tool_tspos::printUsedMarkers(void) {
	for (int i=0; i<(int)m_used_markers.size(); i++) {
		if (!m_used_markers[i]) {
			continue;
		}
		switch (i) {

			case 0:
				m_humdrum_text << "!!!RDF**kern: " << m_root_marker
		 		<< " = marked note, "
				<< "type=\"root position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_root_color << "\"" << endl;
				break;

			case 1:
				m_humdrum_text << "!!!RDF**kern: " << m_third_marker
		 		<< " = marked note, "
				<< "type=\"third position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_third_color << "\"" << endl;
				break;

			case 2:
				m_humdrum_text << "!!!RDF**kern: " << m_fifth_marker
		 		<< " = marked note, "
				<< "type=\"fifth position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_fifth_color << "\"" << endl;
				break;

			case 3:
				m_humdrum_text << "!!!RDF**kern: " << m_3rd_root_marker
		 		<< " = marked note, "
				<< "type=\"open third root position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_3rd_root_color << "\"" << endl;
				break;

			case 4:
				m_humdrum_text << "!!!RDF**kern: " << m_3rd_third_marker
		 		<< " = marked note, "
				<< "type=\"open third, third position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_3rd_third_color << "\"" << endl;
				break;

			case 5:
				m_humdrum_text << "!!!RDF**kern: " << m_5th_root_marker
		 		<< " = marked note, "
				<< "type=\"open fifth, root position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_5th_root_color << "\"" << endl;
				break;

			case 6:
				m_humdrum_text << "!!!RDF**kern: " << m_5th_fifth_marker
		 		<< " = marked note, "
				<< "type=\"open fifth, fifth position\" "
				<< "count=\"" << m_used_markers[i] << " "
				<< "color=\"" << m_5th_fifth_color << "\"" << endl;
				break;
		}
	}
}



//////////////////////////////
//
// Tool_tspos::hasFullTriadAttack --
//

bool Tool_tspos::hasFullTriadAttack(HumdrumLine& line) {
	vector<int> midiPitches = line.getMidiPitchesSortLH();
	vector<int> positive(24, 0);
	int count = 0;
	for (int i=0; i<(int)midiPitches.size(); i++) {
		if (midiPitches[i] > 0) {
			if (!positive[midiPitches[i] % 12]) {
				positive[midiPitches[i] % 12]++;
				positive[midiPitches[i] % 12 + 12]++;
				count++;
			}
		}
	}

	if (count != 3) {
		return false;
	}

	for (int i=0; i<12; i++) {
		if (!positive[i]) {
			continue;
		}
		if (positive[i+3]) {
			if (positive[i+3+3]) { // diminished triad pitch-classes present
				return true;
			} else if (positive[i+3+4]) { // minor
				return true;
			}
		} else if (positive[i+4]) {
			if (positive[i+4+3]) { // major
				return true;
			} else if (positive[i+4+4]) { // augmented
				return true;
			}
		}
	}

	return false;
}




//////////////////////////////
//
// Tool_tspos:analyzeVoiceCount -- Chords count as a single voice.
//

void Tool_tspos::analyzeVoiceCount(HumdrumFile& infile) {
	vector<int>& voices = m_voiceCount;
	voices.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			voices[i] = 0;
			continue;
		}
		voices[i] = countVoicesOnLine(infile, i);
	}
}



//////////////////////////////
//
// Tool_tspos::checkForTriadicSonority -- Mark the given line in the file as a triadic sonority
//     of all MIDI note numbers are positive.
//

void Tool_tspos::checkForTriadicSonority(vector<int>& positions, int line) {
	for (int i=0; i<(int)positions.size(); i++) {
		if (positions[i] > 0) {
			m_triadState.at(line) = true;
			break;
		}
	}
}



//////////////////////////////
//
// Tool_tspos::countVoicesOnLine --
//

int Tool_tspos::countVoicesOnLine(HumdrumFile& infile, int line) {
	int output = 0;
	for (int j=0; j<infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			token = token->resolveNull();
			if (!token) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
		}
		if (token->isRest()) {
			continue;
		}
		output++;
	}
	return output;
}



//////////////////////////////
//
// Tool_tspos::generateStatistics --
//

string Tool_tspos::generateStatistics(HumdrumFile& infile) {
	int sonorityCount = 0;  // total number of sonorities
	int triadCount = 0;     // sonorities that are triads
	HumNum triadDuration = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (m_triadState.at(i)) {
			triadDuration += infile[i].getDuration();
		}
		int attacks = infile[i].getKernNoteAttacks();
		if (attacks == 0) {
			continue;
		}
		sonorityCount++;
		if (m_triadState.at(i)) {
			triadCount++;
		}
	}

	string toolPrefix = "!!!TOOL-";
	if (m_toolCount > 0) {
		toolPrefix += to_string(m_toolCount) + "-";
	}
	toolPrefix += m_toolName + "-";

	stringstream out;
	out << toolPrefix << "sonority-count: " << sonorityCount << endl;
	out << toolPrefix << "sonority-duration: " << infile.getScoreDuration().getFloat() << endl;
	out << toolPrefix << "triadic-count: " << triadCount << endl;
	out << toolPrefix << "triadic-duration: " << triadDuration.getFloat() << endl;

	double percentage = 100.0 * (double)triadCount / (double)sonorityCount;
	percentage = int(percentage * 100.0 + 0.5) / 100.0;
	out << toolPrefix << "count-ratio: " << percentage << "%" << endl;

	percentage = 100.0 * triadDuration.getFloat() / infile.getScoreDuration().getFloat();
	percentage = int(percentage * 100.0 + 0.5) / 100.0;
	out << toolPrefix << "duration-ratio: " << percentage << "%" << endl;

	// Report triads positions by voice:
	vector<string> names = getTrackNames(infile);

	for (int i=1; i<(int)names.size(); i++) {
		out << toolPrefix << m_toolName << "track-name-" << to_string(i) << ": " << names[i] << endl;
	}
	vector<HTp> kernstarts;
	infile.getKernSpineStartList(kernstarts);
	if (!kernstarts.empty()) {
		out << toolPrefix << "first-kern-track: " << kernstarts[0]->getTrack() << endl;
		out << toolPrefix << "last-kern-track: " << kernstarts.back()->getTrack() << endl;
	}
	out << toolPrefix << "kern-count: " << kernstarts.size() << endl;
	out << toolPrefix << "kern-tracks: ";
	for (int i=0; i<(int)kernstarts.size(); i++) {
		out << kernstarts[i]->getTrack();
		if (i < (int)kernstarts.size() - 1) {
			out << " ";
		}
	}
	out << endl;
	int topTrack = kernstarts.back()->getTrack();

	for (int i=1; i<(int)m_partTriadPositions.size(); i++) {
		vector<int>& entry = m_partTriadPositions[i];
		int sum = getVectorSum(entry);
		if (sum == 0) {
			continue;
		}
		string name = names[i];
		int rootCount = 0;
		int thirdCount = 0;
		int fifthCount = 0;
		rootCount += entry[0] + entry[3] + entry[5];
		thirdCount += entry[1] + entry[4];
		fifthCount += entry[2] + entry[6];
		double rootPercent = int(rootCount * 1000.0 / sum + 0.5) / 10.0;
		double thirdPercent = int(thirdCount * 1000.0 / sum + 0.5) / 10.0;
		double fifthPercent = int(fifthCount * 1000.0 / sum + 0.5) / 10.0;
		if (m_topQ && (topTrack == i)) {
			name = "top-" + name;
		}
		out << toolPrefix << "count-sum-" << i << "-" << name << ": " << sum << endl;
		out << toolPrefix << "root-count-" << i << "-" << name << ": " << rootCount << " (" << rootPercent << "%)" << endl;
		out << toolPrefix << "third-count-" << i << "-" << name << ": " << thirdCount << " (" << thirdPercent << "%)" << endl;
		out << toolPrefix << "fifth-count-" << i << "-" << name << ": " << fifthCount << " (" << fifthPercent << "%)" << endl;
	}

	if (m_tableQ) {
		out << generateTable(infile, names);
	}

	return out.str();
}



//////////////////////////////
//
// Tool_tspos::generateTable -- Create an HTML table that the statistics
//     will be filled into.
//

string Tool_tspos::generateTable(HumdrumFile& infile, vector<string>& names) {

	string toolPrefix = "TOOL-";
	if (m_toolCount > 0) {
		toolPrefix += to_string(m_toolCount) + "-";
	}
	toolPrefix += m_toolName + "-";

	vector<HTp> spinestarts;
	infile.getSpineStartList(spinestarts);
	stringstream out;
	out << "!!@@BEGIN: PREHTML\n";
	out << "!!@CONTENT:\n";
	out << "!!<div class='tool-" << m_toolName << "'>\n";
	out << "!!<h2 class='header-tool-" << m_toolName << "'>";
	if (m_majorQ && !m_minorQ) {
		out << "Major triadic";
	} else if (!m_majorQ && m_minorQ) {
		out << "Minor triadic";
	} else {
		out << "Triadic";
	}
	out << " sonority position analysis";
	if (m_questionQ) {
		out <<  " <a class='info-" << m_toolName
				<< "' target='_blank' href='https://doc.verovio.humdrum.org/filter/"
				<< m_toolName
				<< "'>?</a>";
	}
	out << "</h2>\n";
	out << "!!<table class='analysis'>\n";
	out << "!!<tr><th class='voice'>Voice</th>"
			<< "<th class='root'>Root</th><th class='third'>Third</th>"
			<< "<th class='fifth'>Fifth</th><th class='total'>Total</th></td>\n";

	int rootSum = 0;
	int thirdSum = 0;
	int fifthSum = 0;

	std::vector<vector<int>>& pt =  m_partTriadPositions;
	for (int i=(int)names.size() - 1; i>0; i--) {
		int track = i;
		if (!spinestarts.at(i-1)->isKern()) {
			continue;
		}
		out << "!!<tr>\n";
		int sum = 0;
		for (int j=0; j<(int)pt[i].size(); j++) {
			sum += pt[i][j];
		}

		int rootCount  = pt.at(i)[0] + pt.at(i)[3] + pt.at(i)[5];
		int thirdCount = pt.at(i)[1] + pt.at(i)[4];
		int fifthCount = pt.at(i)[2] + pt.at(i)[6];

		rootSum  += rootCount;
		thirdSum += thirdCount;
		fifthSum += fifthCount;

		// Requires the colors to be hex:
		string oc1 = makeOpacityColor(m_root_color,  rootCount,  sum);
		string oc3 = makeOpacityColor(m_third_color, thirdCount, sum);
		string oc5 = makeOpacityColor(m_fifth_color, fifthCount, sum);

		out << "!!   <td class='voice'>" << m_fullNames[i] << "</td>\n";
		out << "!!   <td class='root'  style='background-color:" << oc1
			<< ";'>@{" << toolPrefix << "root-count-"  << to_string(track)
			<< "-" << names[track] << "}</td>\n";
		out << "!!   <td class='third' style='background-color:" << oc3
			<< ";'>@{" << toolPrefix << "third-count-" << to_string(track)
			<< "-" << names[track] << "}</td>\n";
		out << "!!   <td class='fifth' style='background-color:" << oc5
				<< ";'>@{" << toolPrefix << "fifth-count-" << to_string(track)
				<< "-" << names[track] << "}</td>\n";
		out << "!!   <td class='total'>@{" << toolPrefix << "count-sum-"
				<< to_string(track) << "-" << names[track] << "}</td>\n";
		out << "!!</tr>\n";
	}

	int totalSum = rootSum + thirdSum + fifthSum;

	string p1 = makePercentString(rootSum,  totalSum, 1);
	string p3 = makePercentString(thirdSum, totalSum, 1);
	string p5 = makePercentString(fifthSum, totalSum, 1);

	string oc1 = makeOpacityColor(m_root_color,  rootSum,  totalSum);
	string oc3 = makeOpacityColor(m_third_color, thirdSum, totalSum);
	string oc5 = makeOpacityColor(m_fifth_color, fifthSum, totalSum);

	// print column sum table
	out << "!!<tr>\n";
	out << "!!<td style='text-align:right; font-style:italic;'>\n";
	out << "!!   sum:\n";
	out << "!!</td>\n";

	// root sum:
	out << "!!<td class='root' style='background-color:" << oc1 << ";'>\n";
	out << "!!   " << rootSum << " (" << p1 << "%)" << endl;
	out << "!!</td>\n";

	// third sum:
	out << "!!<td class='third' style='background-color:" << oc3 << ";'>\n";
	out << "!!   " << thirdSum << " (" << p3 << "%)" << endl;
	out << "!!</td>\n";

	// fifth sum:
	out << "!!<td class='fifth' style='background-color:" << oc5 << ";'>\n";
	out << "!!   " << fifthSum << " (" << p5 << "%)" << endl;
	out << "!!</td>\n";

	// total sum:
	out << "!!<td class='total'>\n";
	out << "!!   " << totalSum << endl;
	out << "!!</td>\n";

	out << "!!</tr>\n";

	out << "!!</table>\n";

	stringstream options;
	int licount = 0;
	options << "!!<ul>\n";

	// Print summary of options used:
	if (m_voice > 0) {
		options << "!!<li> Only sonorities with " << m_voice << " sounding voices analyzed</li>\n";
		licount++;
	}

	if (m_triadAttack) {
		options << "!!<li> Only sonorities with at least three unique pitch-class attacks are analyzed</li>\n";
		licount++;
	}

	if (!(m_colorThirds && m_colorFifths && m_colorTriads)) {
		if (m_colorTriads && m_colorThirds && m_colorFifths) {
			options << "!!<li> Sonorities with two or three triadic positions analyzed</li>\n";
			licount++;
		} else if (m_colorTriads && !m_colorThirds && !m_colorFifths) {
			options << "!!<li> Only sonorities with all three triadic positions analyzed</li>\n";
			licount++;
		} else if (!m_colorTriads && m_colorThirds && m_colorFifths) {
			options << "!!<li> Only sonorities with two triadic positions analyzed</li>\n";
			licount++;
		} else if (!m_colorTriads && m_colorThirds && !m_colorFifths) {
			options << "!!<li> Only sonorities with open thirds analyzed</li>\n";
			licount++;
		} else if (!m_colorTriads && !m_colorThirds && m_colorFifths) {
			options << "!!<li> Only sonorities with open fifths analyzed</li>\n";
			licount++;
		} else if (m_colorTriads && m_colorThirds && !m_colorFifths) {
			options << "!!<li> Ignoring open fifths </li>\n";
			licount++;
		} else if (m_colorTriads && !m_colorThirds && m_colorFifths) {
			options << "!!<li> Ignoring open thirds </li>\n";
			licount++;
		}
	} else {
		// print a message about both two- and three-pitch-class sonorities?
	}

	options << "!!</ul>\n";
	if (licount) {
		out << options.str();
	}

	out << "!!</div>\n";

	// Styling of table:
	out << "!!<style>\n";
	out << "!!div.tool-" << m_toolName << " { padding-left: 20px;}\n";
	out << "!!div.tool-" << m_toolName << " table tr:hover { background: orange; }\n";
	out << "!!div.tool-" << m_toolName << " table {\n";
	out << "!!   border-collapse: collapse;\n";
	out << "!!   border: 1px solid orange;\n";
	out << "!!   margin: 10px;\n";
	out << "!!   max-width: 800px;\n";
	out << "!!}\n";
	out << "!!div.tool-" << m_toolName << " table td.total, div.tool-" << m_toolName
			<< " table th.total { text-align: center; }\n";
	out << "!!th { background-color: bisque; }\n";
	out << "!!div.tool-" << m_toolName << "table th, div.tool-" << m_toolName
			<< " table td { padding: 5px; }\n";
	out << "!!a.info-" << m_toolName << " {\n"
			 << "!!   text-decoration:none;\n"
			 << "!!   font-size:70%;\n"
			 << "!!   display:inline-block;\n"
			 << "!!   background:#ffaa0070;\n"
			 << "!!   text-align:center;\n"
			 << "!!   border: 1px solid orange;\n"
			 << "!!   border-radius:12px;\n"
			 << "!!   width:25px;\n"
			 << "!!   height:25px;\n"
			 << "!!}\n";
	out << "!! a.info-" << m_toolName << ":link, a.info-"
			<< m_toolName << ":visited, a.info-"
			<< m_toolName << ":active { background:orange; }\n";
	out << "!! a.info-" << m_toolName
			<< ":hover { color: white; background-color: purple; }\n";
	out << "!!</style>\n";

	out << "!!@@END: PREHTML\n";
	return out.str();
}



//////////////////////////////
//
// Tool_tspos::makeOpacityColor --
//

string Tool_tspos::makeOpacityColor(string& color, double value, double total, bool enhance) {
	stringstream output;
	int opacity;
	if (enhance) {
		opacity = logisticColorMap(value, total);
	} else {
		opacity  = int(value / total * 255.49 + 0.5);
	}
	if (opacity < 0) {
		opacity = 0;
	} else if (opacity > 255) {
		opacity = 255;
	}
	output << color << std::hex << std::setw(2) << std::setfill('0') << opacity << std::dec;
	return output.str();
}



//////////////////////////////
//
// Tool_tspos::makePercentString --
//

string Tool_tspos::makePercentString(double value, double total, int digits) {
	double percent = int(value / total * 100.0 * std::pow(10, digits) + 0.5) / std::pow(10, digits);
	stringstream output;
	output << percent;
	if ((percent - int(percent)) < 1.0 / std::pow(10, digits) / 10.0) {
		output << ".0";
	}
	return output.str();
}



//////////////////////////////
//
// Tool_tspos::getVectorSum --
//

int Tool_tspos::getVectorSum(vector<int>& input) {
	int sum = 0;
	for (int i=0; i<(int)input.size(); i++) {
		sum += input[i];
	}
	return sum;
}



//////////////////////////////
//
// Tool_tspos::getTrackNames --
//

vector<string> Tool_tspos::getTrackNames(HumdrumFile& infile) {
	int tracks = infile.getTrackCount();
	m_fullNames.clear();
	m_fullNames.resize(tracks+1);
	vector<string> output(tracks+1);
	for (int i=1; i<(int)output.size(); i++) {
		output[i] = "Spine " + to_string(i);
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (token->compare(0, 3, "*I\"") == 0) {
				string value = token->substr(3);
				if (!value.empty()) {
					int track = token->getTrack();
					output.at(track) = value;
				}
			}
		}
	}
	// Remove colons from names and convert spaces into underscores:
	HumRegex hre;
	for (int i=1; i<(int)output.size(); i++) {
		m_fullNames[i] = output[i];
		hre.replaceDestructive(output[i], "", "^\\s+");
		hre.replaceDestructive(output[i], "", "\\s+$");
		hre.replaceDestructive(output[i], "_", "\\s+", "g");
		hre.replaceDestructive(output[i], "", ":", "g");
	}
	return output;
}



//////////////////////////////
//
// Tool_tspos::labelChordPositions -- Mark the scale degree of notes:
//       0: not in triadic sonority so no label.
//       1 == @: root (such as C in C major triad)
//       3 == N third (such as E in C major triad)
//       5 == Z fifth (such as G in C major triad)
//

void Tool_tspos::labelChordPositions(vector<HTp>& kernNotes, vector<int>& chordPositions) {
	for (int i=0; i<(int)kernNotes.size(); i++) {
		int position = chordPositions.at(i);
		if (position == 0) {
			continue;
		}

		int track = kernNotes[i]->getTrack(); // get part
		string label;
		switch (position) {
			case 1:
				label = m_root_marker;
				m_used_markers[0]++;
				m_partTriadPositions.at(track).at(0)++;
				break;
			case 3:
				label = m_third_marker;
				m_used_markers[1]++;
				m_partTriadPositions.at(track).at(1)++;
				break;
			case 5:
				label = m_fifth_marker;
				m_used_markers[2]++;
				m_partTriadPositions.at(track).at(2)++;
				break;
		}
		if (label.empty()) {
			continue;
		}
		string text = *kernNotes.at(i);
		text += label;
		kernNotes.at(i)->setText(text);
	}
}



//////////////////////////////
//
// Tool_tspos::labelThirds -- Mark the scale degree of notes:
//       0: not in interval sonority so no label.
//       1 == @: root (bottom of the interval)
//       3 == N: third (top of the interval)

void Tool_tspos::labelThirds(vector<HTp>& kernNotes, vector<int>& thirdPositions) {
	for (int i = 0; i < (int)kernNotes.size(); i++) {
		int position = thirdPositions.at(i);
		if (position == 0) {
			continue;
		}

		int track = kernNotes.at(i)->getTrack(); // get part
		string label;
		switch (position) {
			case 1:
				label = m_3rd_root_marker;
				m_used_markers[3]++;
				m_partTriadPositions.at(track).at(3)++;
				break;
			case 3:
				label = m_3rd_third_marker;
				m_used_markers[4]++;
				m_partTriadPositions.at(track).at(4)++;
				break;
		}

		if (label.empty()) {
			continue;
		}
		string text = *kernNotes.at(i);
		text += label;
		kernNotes.at(i)->setText(text);
	}
}



//////////////////////////////
//
// Tool_tspos::labelFifths -- Mark the scale degree of notes:
//       0: not in interval sonority so no label.
//       1 == @: root (bottom of the interval)
//       5 == Z: fifth (top of the interval)

void Tool_tspos::labelFifths(vector<HTp>& kernNotes, vector<int>& fifthPositions) {
	for (int i = 0; i < (int)kernNotes.size(); i++) {
		int position = fifthPositions.at(i);
		if (position == 0) {
			continue;
		}

		int track = kernNotes.at(i)->getTrack(); // get part
		string label;
		switch (position) {
			case 1:
				label = m_5th_root_marker;
				m_used_markers[5]++;
				m_partTriadPositions.at(track).at(5)++;
				break;
			case 5:
				label = m_5th_fifth_marker;
				m_used_markers[6]++;
				m_partTriadPositions.at(track).at(6)++;
				break;
		}

		if (label.empty()) {
			continue;
		}
		string text = *kernNotes.at(i);
		text += label;
		kernNotes.at(i)->setText(text);
	}
}



//////////////////////////////
//
// Tool_tspos::getNoteMods -- Identify if the sonority is a triad, and if so, place
//    the position of the note in the chord:
//       0: not in triadic sonority.
//       1: root (such as C in C major triad)
//       3: third (such as E in C major triad)
//       5: fifth (such as G in C major triad)
//

vector<int> Tool_tspos::getNoteMods(vector<int>& midiNotes) {
	// create modulo 12 arr
	vector<int> pitchClasses(12, 0);
	for (int i = 0; i < (int)midiNotes.size(); i++) {
		int index = midiNotes.at(i) % 12;
		pitchClasses.at(index)++; // add one to the pitch
	}

	vector<int> noteMods;

	for (int i = 0; i < (int)pitchClasses.size(); i++) {
		if (pitchClasses.at(i) != 0) { // not zero
			noteMods.push_back(i); // add the index
		}
	}

	return noteMods;
}



//////////////////////////////
//
// Tool_tspos::getThirds -- Identify if the sonority is a third interval, and if so,
//    place the position of the note in the output.
//       0: not in the sonority
//       1: root (bottom of the interval)
//       3: third (top of the interval)

vector<int> Tool_tspos::getThirds(vector<int>& midiNotes) {
	vector<int> output(midiNotes.size(), 0);

	if (midiNotes.empty()) {
		return output;
	}

	vector<int> noteMods = getNoteMods(midiNotes); // we know noteMods is sorted
	if (noteMods.size() != 2) {
		return output;
	}

	int interval = noteMods[1] - noteMods[0];
	int rootClass = -1; // currently uninitialized
	int thirdClass = -1;

	if (m_majorQ && m_minorQ) {

		if (interval == 3 || interval == 4) { // third is found
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
		}
		else if (interval == 8 || interval == 9) { // third is found (inversion)
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(0);
		}

	}

	else if (!m_majorQ && m_minorQ) {
		if (interval == 3) { // minor third is found
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
		}
		else if (interval == 9) { // major sixth is found (inversion)
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(0);
		}
	}

	else if (m_majorQ && !m_minorQ) {

		if (interval == 4) { // major third is found
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
		}
		else if (interval == 8) { // minor sixth is found (inversion)
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(0);
		}

	}

	if (rootClass == -1) { // third was not found
		return output;
	}

	// populate output
	for (int i = 0; i < (int)midiNotes.size(); i++) {
		if (midiNotes.at(i) % 12 == rootClass) {
		output.at(i) = 1;
		} else if (midiNotes.at(i) % 12 == thirdClass) {
			output.at(i) = 3;
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_tspos::getFifths -- Identify if the sonority is a fifth interval, and if so,
//    place the position of the note in the output.
//       0: not in the sonority
//       1: root (bottom of the interval)
//       5: fifth (top of the interval)

vector<int> Tool_tspos::getFifths(vector<int>& midiNotes) {
	vector<int> output(midiNotes.size(), 0);

	if (midiNotes.empty()) {
		return output;
	}

	vector<int> noteMods = getNoteMods(midiNotes);
	if (noteMods.size() != 2) {
		return output;
	}

	int interval = noteMods.at(1) - noteMods.at(0);
	int rootClass = -1; // currently uninitialized
	int fifthClass = -1;

	if ((interval == 7) || (interval == 6)) { // fifth found
		rootClass = noteMods.at(0);
		fifthClass = noteMods.at(1);
	}

	if (interval == 5) { // inverted fifth found
		rootClass = noteMods.at(1);
		fifthClass = noteMods.at(0);
	}

	if (rootClass == -1) { // fifth not found
		return output;
	}

	// populate output
	for (int i = 0; i < (int)midiNotes.size(); i++) {
		if (midiNotes.at(i) % 12 == rootClass) {
			output.at(i) = 1;
		}
		else if (midiNotes.at(i) % 12 == fifthClass) {
			output.at(i) = 5;
		}
	}

	return output;
}



///////////////////////////////
//
// Tool_tspos::getChordPositions --
//

vector<int> Tool_tspos::getChordPositions(vector<int>& midiNotes) {
	vector<int> output(midiNotes.size(), 0);
	if (midiNotes.empty()) {
		return output;
	}

	vector<int> noteMods = getNoteMods(midiNotes);

	if (noteMods.size() != 3) { // not a triad
		return output;
	}

	int bint = noteMods.at(1) - noteMods.at(0);
	int tint = noteMods.at(2) - noteMods.at(1);

	int rootClass = -1; // curr uninitialized
	int thirdClass = -1;
	int fifthClass = -1;

	if (m_majorQ && m_minorQ) {

		if ((bint == 3 && tint == 4) || (bint == 4 && tint == 3) || (bint == 3 && tint == 3)) { // root pos
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
			fifthClass = noteMods.at(2);
		} else if ((bint == 4 && tint == 5) || (bint == 3 && tint == 5) || (bint == 3 && tint == 6)) { // first inv
			rootClass = noteMods.at(2);
			thirdClass = noteMods.at(0);
			fifthClass = noteMods.at(1);
		} else if ((bint == 5 && tint == 3) || (bint == 5 && tint == 4) || (bint == 6 && tint == 3)) { // sec inv
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(2);
			fifthClass = noteMods.at(0);
		}
	}

	else if (!m_majorQ && m_minorQ) {
		if (bint == 3 && tint == 4) {// minor root pos
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
			fifthClass = noteMods.at(2);
		} else if (bint == 4 && tint == 5) { // minor first inv
			rootClass = noteMods.at(2);
			thirdClass = noteMods.at(0);
			fifthClass = noteMods.at(1);
		} else if (bint == 5 && tint == 3) { // minor sec inv
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(2);
			fifthClass = noteMods.at(0);
		}
	}

	else if (m_majorQ && !m_minorQ) {
		if (bint == 4 && tint == 3) {// major root pos
			rootClass = noteMods.at(0);
			thirdClass = noteMods.at(1);
			fifthClass = noteMods.at(2);
		} else if (bint == 3 && tint == 5) { // major first inv
			rootClass = noteMods.at(2);
			thirdClass = noteMods.at(0);
			fifthClass = noteMods.at(1);
		} else if (bint == 5 && tint == 4) { // major sec inv
			rootClass = noteMods.at(1);
			thirdClass = noteMods.at(2);
			fifthClass = noteMods.at(0);
		}
	}

	if (rootClass == -1) {
		return output;
	}

	for (int i = 0; i < (int)midiNotes.size(); i++) {
		if (midiNotes.at(i) % 12 == rootClass) {
			output.at(i) = 1;
		}
		else if (midiNotes.at(i) % 12 == thirdClass) {
			output.at(i) = 3;
		}
		else if (midiNotes.at(i) % 12 == fifthClass) {
			output.at(i) = 5;
		}
	}

	if (m_doubleQ) {
		keepOnlyDoubles(output); // call some function that marks only doubles
	}

	return output;
}



//////////////////////////////
//
// Tool_tspos::keepOnlyDoubles --
//

void Tool_tspos::keepOnlyDoubles(vector<int>& output) {
	map<int, int> positionCounts = {{1, 0}, {3, 0}, {5, 0}};

	for (int i = 0; i < (int)output.size(); i++) { // create hashmap of counts
		if (output[i] == 1) {
			positionCounts[1]++;
		}
		else if (output[i] == 3) {
			positionCounts[3]++;
		}
		else if (output[i] == 5) {
			positionCounts[5]++;
		}
	}

	for (auto positionCount : positionCounts) {
		if (positionCount.second == 1) { // if only appears once
		replace(output.begin(), output.end(), positionCount.first, 0); // replace with 0
		}
	}

	return;
}



//////////////////////////////
//
// Tool_tspos::getMidiNotes -- Convert kern notes to MIDI note numbers.
//    If the note is sustaining, then make the MIDI note number negative.
//

vector<int> Tool_tspos::getMidiNotes(vector<HTp>& kernNotes) {
	vector<int> output(kernNotes.size());
	if (kernNotes.empty()) {
		return output;
	}
	for (int i=0; i<(int)kernNotes.size(); i++) {
		int midiNote = kernNotes.at(i)->getMidiPitch();
		if (midiNote < 0) {
			// negative values indicate sustained note.
			midiNote = -midiNote;
		}
		output.at(i) = midiNote;
	}
	return output;
}



//////////////////////////////
//
// Tool_tspos::avoidRdfCollisions -- Adjust markers if they are already
//      defined in the file.
//

void Tool_tspos::avoidRdfCollisions(HumdrumFile& infile) {
	map<string, bool> rdfs;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string value = infile[i].getReferenceValue();
		HumRegex hre;
		if (hre.search(value, "^\\s*([^\\s]+)\\s*=\\s*(.*)\\s*$")) {
			string rdf = hre.getMatch(1);
			rdfs[rdf] = true;
		}
	}

	vector<string> replacement(7);
	replacement[0] = "😀";
	replacement[1] = "😄";
	replacement[2] = "😉";
	replacement[3] = "😍";
	replacement[4] = "😂";
	replacement[5] = "😎";
	replacement[6] = "😗";

	if (rdfs[m_root_marker]) {
		m_root_marker = replacement[0];
	}
	if (rdfs[m_third_marker]) {
		m_third_marker = replacement[1];
	}
	if (rdfs[m_fifth_marker]) {
		m_fifth_marker = replacement[2];
	}
	if (rdfs[m_3rd_root_marker]) {
		m_3rd_root_marker = replacement[3];
	}
	if (rdfs[m_3rd_third_marker]) {
		m_3rd_third_marker = replacement[4];
	}
	if (rdfs[m_5th_root_marker]) {
		m_5th_root_marker = replacement[5];
	}
	if (rdfs[m_5th_fifth_marker]) {
		m_5th_fifth_marker = replacement[6];
	}

}



//////////////////////////////
//
// Tool_tspos::getToolCounter --
//

int Tool_tspos::getToolCounter(HumdrumFile& infile) {
	int counter = 0;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (hre.search(infile[i], "^!!@@BEGIN:\\s*PREHTML\\s*$")) {
			counter++;
		}
	}
	return counter;
}



//////////////////////////////
//
// Tool_tspos::logisticColorMap -- Increase sensitivity of color mapping
//    around 40%.
//

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

int Tool_tspos::logisticColorMap(double input, double max) {
	double center = max * 0.40;
	double k = 0.04;
	int output = max / (1.0 + pow(M_E, -k * (input + center) - (max + center)/2));
	output -= 11.4209;
	output = output * 255.0 / 243.377;
	if (output < 0) {
		output = 0;
	}
	if (output > 255) {
		output = 255;
	}
	return output;
}


// END_MERGE

} // end namespace hum



