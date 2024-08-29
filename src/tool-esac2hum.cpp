//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 12 10:58:43 PDT 2024
// Last Modified: Fri Aug 23 08:14:43 PDT 2024
// Filename:      src/tool-esac2hum.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/esac2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Convert newer ESaC files into Humdrum.
//
// EsAC parameters:
//
// https://kolberg.ispan.pl/dwok/kod-esac
//
// CUT[] = The title or text incipit (beginning of lyrics) of the encoded melody.
// There can be both title and incipit in the CUT[] field.  In that case there
// will be two lines of text: the first for the title and the second for the
// incipit.  Example:
//     CUT[Koenig und Marquise
//         Hourah les fill' a quat' deniers,]
// SIG[] = The signature for the melody (new EsAC) which has this format:
//       Volume number | Melody number | Variant | Part | Version
//
// example: 18256a0010
//          18         | 256            | a00     | 1    | 0
// volume number: 18 = 18th volume of Oskar Kolberg: Complete Works
// Melody number : 256 = 256th melody in volume 18
// Variant: a00 = A secondary enumeration of melodies, such as 256a and 256b.
// Part: 1 = EsAC data cannot have key changes, so a song may be split
//           into multiple files (see SWOK13-001)
// Version: 0 = updated version of the file?
//
// KEY[] = contains 4 subfields:
//    1. Abbreviated signature
//    2. Minimal rhythmic unit
//    3. Tonic
//    4. Key signature(s), or "FREI" for unmeasured.
// example:  KEY[18265a 16 A 3/8]
// 	18256a = volume 18 of DWOK, song 256a
// 	    * This is the identifer for the melody (ID).  Newer files
// 	      may have SIG[] fields which are an expanded ID that also encodes
// 	      the volume number and variants/versions.
// 	16th note is the minimal time unit for parsing MEL[] data.
// 	A = tonic note for scale degrees in MEL[] data.
// 	    * This letter should be capitalized, but occasional encoding
// 	      errors will have it as lower case.  "b" and "#" may follow
// 	      for flat and sharp of tonic note, such as Eb for E-flat and F#
// 	      for F-sharp.
//    3/8 = time signature of MEL[] data.
//        * There can be multiple time signatures listed, which indicates
//          that the time signature changes within the MEL[] data (assigment
//          is implicit, so 6/8 + 3/4 cannot be distinguished.
//
// MEL[] = musical content of the melody.
// * 1-7 = scale degree (major mode by default)
// * 0 = rest
// * 7b = lowered 7th degree (major) such as B- in C tonic or Fn in G tonic.
// * 4# = raised 4th degree (major) such as F# in C tonic or Bn in F tonic.
// * -7 = in the 3rd octave rather than 4th
// * +3 = in the 5rd octave rather than 4th
// * _ = power of two increase from the minimum rhythmic duration
// 		e..g, if minrhy=16 then 3_ is an 8th note, 3__ is a 4th and 3___ is a 2nd.
// * . = augmentation dot
// * measures are separated by (2) spaces.
// * linebreaks = phrase separator
//
// Automatic analyses (these can be generated during the conversion
// process by using the -a option to add to the embedded input EsAC data):
//
// TRD[] = source information
//   This is typically one line but may be two.   Text in this field
//   is free-form, but if there is page number information in the source
//   print, it will be prefixed with "s." or "S." and then a page number
//   or two page numbers for the range separated by "-" or " - ". 
//   Here is an example of a two line TRD[]:
//   TRD[2, S. 66
//       1814 aufgezeichnet (vor)]
// REG[] = geographical region if not supplied in TRD[]
//      Example: REG[Mitteleuropa, Frankreich]
//      This field may not be filled in if TRD[] has region information.
// FKT[] = function of the piece (genre): e.g.: lullaby, ballad
// BEM[] = additional notes.
//
// Analytic fields that are extracted automatically from musical data:
//
// MEL_SEM[]  = semitone intervals of melody.
// MEL_RAW[]  = MEL[] without rhythms.
// NO_REP[]   = MEL_RAW[] without note repetitions.
// RTM[]      = rhythm with "x" instead of pitch information.
// SCL_DEG[]  = scale degrees used in MEL[] from low to high
// SCL_SEM[]  = intervals between successive SCL_DEG[] degrees in semitones.
// PHR_NO[]   = number of phrases in melody (lines in MEL[]).
// PHR_BARS[] = number of measures in individual phrases.
// PHR_CAD[]  = cadential notes of phrases (last scale degree of phrase).
// ACC[]      = accented notes in melody (down beat of each full measure).
//

#include "tool-esac2hum.h"
#include "Convert.h"
#include "HumRegex.h"

#include <ctime>
#include <iomanip>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_esac2hum::Tool_esac2hum -- Set the recognized options for the tool.
//

Tool_esac2hum::Tool_esac2hum(void) {
	define("debug=b", "Print debugging statements");
	define("v|verbose=s", "Print verbose messages");
	define("e|embed-esac=b", "Eembed EsAC data in output");
	define("a|analyses|analysis=b", "Generate EsAC analysis fields");
}



//////////////////////////////
//
// Tool_esac2hum::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool Tool_esac2hum::convertFile(ostream& out, const string& filename) {
	initialize();
   ifstream file(filename);
   if (file) {
      return convert(out, file);
   }
   return false;
}


bool Tool_esac2hum::convert(ostream& out, istream& input) {
	convertEsacToHumdrum(out, input);
	return true;
}


bool Tool_esac2hum::convert(ostream& out, const string& input) {
	stringstream ss;
	ss << input;
	convertEsacToHumdrum(out, ss);
	return true;
}


//////////////////////////////
//
// Tool_esac2hum::initialize --
//

void Tool_esac2hum::initialize(void) {
	m_debugQ     = getBoolean("debug");      // print debugging information
	m_verboseQ   = getBoolean("verbose");    // print input EsAC MEL[] data when true
	m_verbose    = getString("verbose");     // p = phrase, m=measure, n=note
	m_embedEsacQ = getBoolean("embed-esac"); // don't print input EsAC data
	m_analysisQ  = getBoolean("analyses");   // embed analysis in EsAC data
	if (m_analysisQ) {
		m_embedEsacQ = true;
	}
}



//////////////////////////////
//
// Tool_esac2hum::convertEsacToHumdrum --
//

void Tool_esac2hum::convertEsacToHumdrum(ostream& output, istream& infile) {
	m_inputline = 0;
	m_prevline = "";

	vector<string> song;  // contents of one EsAC song, extracted from input stream
	song.reserve(1000);

	while (!infile.eof()) {
		if (m_debugQ) {
			cerr << "Getting a song..." << endl;
		}
		bool status = getSong(song, infile);
		if (!status) {
			cerr << "Error getting a song" << endl;
			continue;
		}
		if (m_debugQ) {
			cerr << "Got a song ..." << endl;
		}
		if (song.empty()) {
			cerr << "Song is empty" << endl;
			continue;
		}
		if (song.size() < 4) {
			cerr << "Song is too short" << endl;
			continue;
		}
		convertSong(output, song);
	}
}



//////////////////////////////
//
// Tool_esac2hum::getSong -- get a song from a multiple-song EsAC file.
//     Search for a CUT[] line which indicates the first line of the data.
//     There will/can be some text above the CUT[] line.  The CUT[] field
//     may contain newlnes, so searching only for CUT[ to also handle these
//     cases.
//

bool Tool_esac2hum::getSong(vector<string>& song, istream& infile) {
	song.resize(0);

	HumRegex hre;
	string buffer;

	// First find the next CUT[] line in the input which indcates
	// the start of a song.  There typically is a non-empty line just above CUT[]
	// containing information about the collection.
	if (m_cutline.empty()) {
		while (!infile.eof()) {
			getline(infile, buffer);
			cleanText(buffer);
			m_inputline++;
			if (buffer.compare(0, 4, "CUT[") == 0) {
				m_cutline = buffer;
				break;
			} else {
				m_prevline = buffer;
				continue;
			}
		}
	}

	if (m_cutline.empty()) {
		return false;
	}

	if (infile.eof()) {
		return false;
	}

	if (!hre.search(m_prevline, "^\\s*$")) {
		song.push_back(m_prevline);
	}
	song.push_back(m_cutline);

	m_prevline.clear();
	m_cutline.clear();

	bool expectingCloseQ = false;

	while (!infile.eof()) {
		getline(infile, buffer);
		cleanText(buffer);
		m_inputline++;
		if (m_debugQ) {
			cerr << "READ LINE: " << buffer << endl;
		}
		if (expectingCloseQ) {
			if (buffer.find("[") != string::npos) {
				cerr << "Strange error on line " << m_inputline << ": " << buffer << endl;
				continue;
			} else if (!hre.search(buffer, "[\\[\\]]")) {
				// intermediate parameter line (not starting or ending)
				song.push_back(buffer);
				continue;
			}

			if (hre.search(buffer, "^[^\\]]*\\]\\s*$")) {
				// closing bracket
				expectingCloseQ = 0;
				song.push_back(buffer);
				continue;
			} else {
				cerr << "STRANGE CASE HERE " << buffer << endl;
			}
			continue;
		}

		if (hre.search(buffer, "^\\s*$")) {
			continue;
		}

		if (hre.search(buffer, "^[A-Za-z][^\\[\\]]*$")) {
			// collection line
			m_prevline = buffer;
			continue;
		}

		if (hre.search(buffer, "^[A-Za-z]+\\s*\\[[^\\]]*\\s*$")) {
			// parameter with opening [
			expectingCloseQ = true;
		} else {
		}

		song.push_back(buffer);
	}

	if (expectingCloseQ) {
		cerr << "Strange case: expecting closing of a song parameter around line " << m_inputline++ << endl;
	}

	return true;
}



//////////////////////////////
//
// Tool_esac2hum::cleanText -- remove \x88 and \x98 bytes from string (should not affect UTF-8 encodings)
//     since those bytes do not seem to be involved with any UTF-8 characters.
//

void Tool_esac2hum::cleanText(std::string& buffer) {
	HumRegex hre;
	hre.replaceDestructive(buffer, "", "[\x88\x98]", "g");
	if (!buffer.empty()) {
		if (buffer.back() == 0x0d) {
			// windows newline piece
			buffer.resize(buffer.size() - 1);
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::trimSpaces -- remove any trailing or leading spaces.
//

string Tool_esac2hum::trimSpaces(const string& input) {
	string output = input;
	HumRegex hre;
	hre.replaceDestructive(output, "", "^\\s+");
	hre.replaceDestructive(output, "", "\\s+$");
	return output;
}



//////////////////////////////
//
// Tool_esac2hum::convertSong --
//

void Tool_esac2hum::convertSong(ostream& output, vector<string>& infile) {
	getParameters(infile);
	processSong();
	// printParameters();
	printHeader(output);
	printScoreContents(output);
	printFooter(output, infile);
}



//////////////////////////////
//
// Tool_esac2hum::processSong -- parse and preliminary conversion to Humdrum.
//

void Tool_esac2hum::processSong(void) {
	string mel = m_score.m_params["MEL"];
	m_score.parseMel(mel);
}



//////////////////////////////
//
// Tool_esac2hum::printScoreContents --
//

void Tool_esac2hum::printScoreContents(ostream& output) {

	vector<string>& errors = m_score.m_errors;
	if (!errors.empty()) {
		for (int z=0; z<(int)errors.size(); z++) {
			output << "!!" << errors.at(z) << endl;
		}
	}

	if (!m_score.m_clef.empty()) {
		output << m_score.m_clef << endl;
	}
	if (!m_score.m_keysignature.empty()) {
		output << m_score.m_keysignature << endl;
	}
	if (!m_score.m_keydesignation.empty()) {
		output << m_score.m_keydesignation << endl;
	}
	if (!m_score.m_timesig.empty()) {
		output << m_score.m_timesig << endl;
	}

	for (int i=0; i<(int)m_score.size(); i++) {
		Tool_esac2hum::Phrase& phrase = m_score.at(i);
		if (m_verbose.find("p") != string::npos) {
			output << "!!esac-phrase: " << phrase.esac;
			if (m_verbose.find("pi") != string::npos) {
				output << " [";
				output << "ticks:" << phrase.m_ticks;
				output << "]";
			}
			vector<string>& errors = phrase.m_errors;
			if (!errors.empty()) {
				for (int z=0; z<(int)errors.size(); z++) {
					output << "!!" << errors.at(z) << endl;
				}
			}
			output << endl;
		}

		for (int j=0; j<(int)phrase.size(); j++) {

			Tool_esac2hum::Measure& measure = phrase.at(j);
			if ((j == 0) && (i > 0)) {
				output << "!!LO:LB:g=esac" << endl;
			}
			if (measure.m_barnum != 0) { // don't print barline if first is pickup
				output << "=";
				if (measure.m_barnum > 0) {
					output << measure.m_barnum;
				} else {
					output << "-"; // "non-controlling" barline.
				}
				output  << endl;
			}
			if (m_verbose.find("m") != string::npos) {
				output << "!!esac-measure: " << measure.esac;
				if (m_verbose.find("mi") != string::npos) {
					output << " [";
					output << "ticks:" << measure.m_ticks;
					if (measure.isComplete())  {
						output << "; CM";
					}
					if (measure.isPartialBegin())  {
						output << "; PB";
					}
					if (measure.isPartialEnd())  {
						output << "; PE";
					}
					if (measure.isUnassigned())  {
						output << "; UN";
					}
					output << "]";
				}
				output << endl;
				vector<string>& errors = measure.m_errors;
				if (!errors.empty()) {
					for (int z=0; z<(int)errors.size(); z++) {
						output << "!!" << errors.at(z) << endl;
					}
				}
			}

			for (int k=0; k<(int)measure.size(); k++) {

				Tool_esac2hum::Note& note = measure.at(k);
				if (m_verbose.find("n") != string::npos) {
					output << "!!esac-note: " << note.esac;
					if (m_verbose.find("ni") != string::npos) {
						output << " [";
						output << "ticks:" << note.m_ticks;
						output << ", deg:" << note.m_degree;
						output << ", alt:" << note.m_alter;
						output << ", oct:" << note.m_octave;
						output << "]";
					}
					vector<string>& errors = note.m_errors;
					if (!errors.empty()) {
						for (int z=0; z<(int)errors.size(); z++) {
							output << "!!" << errors.at(z) << endl;
						}
					}
					output << endl;
				}
				output << note.m_humdrum << endl;

			}
		}
	}

	if (m_score.hasFinalBarline()) {
		output << "==" << endl;
	} else {
		output << "=" << endl;
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::parseMel --
//

bool Tool_esac2hum::Score::parseMel(const string& mel) {
	clear();
	reserve(100);

	HumRegex hre;
	if (hre.search(mel, "^\\s*$")) {
		// no data;
		cerr << "ERROR: MEL parameter is empty or non-existent" << endl;
		return false;
	}

	vector<string> lines;
	string line;

	stringstream linestream;
	linestream << mel;

	int lineNumber = 0;
	while (std::getline(linestream, line)) {
		lineNumber++;
		if (hre.search(line, "^\\s*$")) {
			// Skip blank lines
			continue;
		}
		string unknown = line;
		hre.replaceDestructive(unknown, "", "[\\^0-9b\\s/._#()+-]+", "g");
		if (!unknown.empty()) {
			cerr << "Unknown characters " << ">>" << unknown << "<< " << " on mel line " << lineNumber << ": " << line << endl;
		}
		line = Tool_esac2hum::trimSpaces(line);
		lines.push_back(line);
	}

	m_finalBarline = false;
	for (int i=0; i<(int)lines.size(); i++) {
		string line = lines[i];
		if (i == (int)lines.size() - 1) {
			if (hre.search(line, "^(.*)\\s*//\\s*$")) {
				m_finalBarline = true;
				lines.back() = hre.getMatch(1);
			}
		}
	}
	// remove the last line if it is only "//":
	if (!lines.empty()) {
		if (hre.search(lines.back(), "^\\s*$")) {
			lines.resize(lines.size() - 1);
		}
	}
	if (lines.empty()) {
		cerr << "ERROR: No notes in MEL data" << endl;
		return false;
	}

	for (int i=0; i<(int)lines.size(); i++) {
		resize(size() + 1);
		back().parsePhrase(lines[i]);
	}

	analyzeTies();
	analyzePhrases();
 	generateHumdrumNotes();
	calculateClef();
	calculateKeyInformation();
	calculateTimeSignatures();

	return true;
}



//////////////////////////////
//
// Tool_esac2hum::Score::assignFreeMeasureNumbers -- The time signature
//    is "FREI", so assign a measure number to eavery barline, not checking
//    for pickup or partial measures.
//

void Tool_esac2hum::Score::assignFreeMeasureNumbers(void) {
	vector<Tool_esac2hum::Measure*> measurelist;
	getMeasureList(measurelist);

	int barnum = 1;
	for (int i=0; i<(int)measurelist.size(); i++) {
		measurelist[i]->m_barnum = barnum++;
		measurelist[i]->m_partialBegin = false;
		measurelist[i]->m_partialEnd = false;
		measurelist[i]->m_complete = true;
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::assignSingleMeasureNumbers -- There is a
//    single time signature for the entire melody, so identify full
//    and unfull measures, marking full that match the time signature
//    duration as complete, and then try to pair measures and look
//    for a pickup measure at the start of the music.
//    The Measure::tsticks is the expected duration of the measure
//    according to the time signature.
//

void Tool_esac2hum::Score::assignSingleMeasureNumbers(void) {
	vector<Tool_esac2hum::Measure*> measurelist;
	getMeasureList(measurelist);

	if (measurelist.empty()) {
		// strange error: no measures;
		return;
	}

	// first identify complete measures:
	for (int i=0; i<(int)measurelist.size(); i++) {
		if (measurelist[i]->m_tsticks == measurelist[i]->m_ticks) {
			measurelist[i]->setComplete();
		}
	}

	// check for pickup measure at beginning of music
	if (measurelist[0]->m_ticks < measurelist[0]->m_tsticks) {
		measurelist[0]->setPartialEnd();
		// check for partial measure at end that matches end measure
		if (measurelist.back()->m_ticks < measurelist.back()->m_tsticks) {
			measurelist.back()->setPartialBegin();
		}
	}

	// search for pairs of partial measures
	for (int i=1; i<(int)measurelist.size(); i++) {
		if (!measurelist[i]->isUnassigned()) {
			continue;
		}
		if (!measurelist[i-1]->isUnassigned()) {
			continue;
		}
		double ticks1 = measurelist[i-1]->m_ticks;
		double ticks2 = measurelist[i]->m_ticks;
		double tsticks1 = measurelist[i-1]->m_tsticks;
		double tsticks2 = measurelist[i]->m_tsticks;
		if (tsticks1 != tsticks2) {
			// strange error;
			continue;
		}
		if (ticks1 + ticks2 == tsticks2) {
			measurelist[i-1]->setPartialBegin();
			measurelist[i]->setPartialEnd();
		}
	}

	// Now assign barlines to measures. that are complete or
	// partial starts.
	int barnum = 1;
	for (int i=0; i<(int)measurelist.size(); i++) {
		if (measurelist[i]->isComplete()) {
			measurelist[i]->m_barnum = barnum++;
		} else if (measurelist[i]->isPartialBegin()) {
			measurelist[i]->m_barnum = barnum++;
		} else if (measurelist[i]->isPartialEnd()) {
			measurelist[i]->m_barnum = -1;
		}
	}
	if (measurelist[0]->isPartialEnd()) {
		measurelist[0]->m_barnum = 0; // pickup: don't add barline on first measure
	}
}



//////////////////////////////
//
// Tool_esac2hum::Measure::isUnassigned --
//

bool Tool_esac2hum::Measure::isUnassigned(void) {
	return !(m_complete || m_partialBegin || m_partialEnd);
}


//////////////////////////////
//
// Tool_esac2hum::Measure::setComplete --
//

void Tool_esac2hum::Measure::setComplete(void) {
	m_complete     = true;
	m_partialBegin = false;
	m_partialEnd   = false;
}



//////////////////////////////
//
// Tool_esac2hum::Measure::isComplete --
//

bool Tool_esac2hum::Measure::isComplete(void) {
	return m_complete;
}



//////////////////////////////
//
// Tool_esac2hum::Measure::setPartialBegin --
//

void Tool_esac2hum::Measure::setPartialBegin(void) {
	m_complete     = false;
	m_partialBegin = true;
	m_partialEnd   = false;
}



//////////////////////////////
//
// Tool_esac2hum::Measure::isPartialBegin --
//

bool Tool_esac2hum::Measure::isPartialBegin(void) {
	return m_partialBegin;
}




//////////////////////////////
//
// Tool_esac2hum::Measure::setPartialEnd --
//

void Tool_esac2hum::Measure::setPartialEnd(void) {
	m_complete     = false;
	m_partialBegin = false;
	m_partialEnd   = true;
}



//////////////////////////////
//
// Tool_esac2hum::Measure::isPartialEnd --
//

bool Tool_esac2hum::Measure::isPartialEnd(void) {
	return m_partialEnd;
}



//////////////////////////////
//
// Tool_esac2hum::Score::calculateTimeSignatures --
//

void Tool_esac2hum::Score::calculateTimeSignatures(void) {
	string ts = m_params["_time"];
	if (ts.find("FREI") != string::npos) {
		m_timesig = "*MX";
		setAllTimesigTicks(0.0);
		assignFreeMeasureNumbers();
		return;
	}

	HumRegex hre;
	if (hre.search(ts, "^(\\d+)/(\\d+)$")) {
		m_timesig = "*M" + ts;
		int top = hre.getMatchInt(1);
		int bot = hre.getMatchInt(2);
		// check if bot is a power of two?
		double tsticks = top * m_minrhy / bot;
		setAllTimesigTicks(tsticks);
		assignSingleMeasureNumbers();
		return;
	}

	// Complicated case where the time signature changes
	vector<string> timesigs;
	hre.split(timesigs, ts, "\\s+");
	if (timesigs.size() < 2) {
		m_errors.push_back("ERROR: strange format for time signatures.");
		return;
	}

/* ggg
	vector<double> bticks(timesigs.size(), 0);
	for (int i=0; i<(int)bticks
*/


}



//////////////////////////////
//
// Tool_esac2hum::Score::setAllTimeSigTicks -- Used for calculating bar numbers;
//

void Tool_esac2hum::Score::setAllTimesigTicks(double ticks) {
	vector<Tool_esac2hum::Measure*> measurelist;
	getMeasureList(measurelist);

	for (int i=0; i<(int)measurelist.size(); i++) {
		measurelist[i]->m_tsticks = ticks;
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::calculateKeyInformation --
//

void Tool_esac2hum::Score::calculateKeyInformation(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	vector<int> b40pcs(40, 0);
	for (int i=0; i<(int)notelist.size(); i++) {
		int pc = notelist[i]->m_b40degree;
		if ((pc >= 0) && (pc < 40)) {
			b40pcs.at(pc)++;
		}
	}

	string tonic = m_params["_tonic"];
	if (tonic.empty()) {
		// no tonic for some strange reason
		// error will be reported when calculating Humdrum pitches.
		return;
	}
	char letter = std::toupper(tonic[0]);

	// Compare counts of third and sixth pitch classes:
	int majorsum = b40pcs.at(12) + b40pcs.at(29);
	int minorsum = b40pcs.at(11) + b40pcs.at(28);
	if (minorsum > majorsum) {
		letter = std::tolower(letter);
	}
	string flats;
	string sharps;
	for (int i=1; i<(int)tonic.size(); i++) {
		if (tonic[i] == 'b') {
			flats += "-";
		} else if (tonic[i] == '#') {
			sharps += "#";
		}
	}

	m_keydesignation = "*";
	m_keydesignation += letter;

	if (!flats.empty() && !sharps.empty()) {
		m_errors.push_back("ERROR: tonic note cannot include both sharps and flats.");
	}
	if (!flats.empty()) {
		m_keydesignation += flats;
	} else {
		m_keydesignation += sharps;
	}
	m_keydesignation += ":";

	if (std::isupper(letter)) {

		// major key signature
		if (m_keydesignation == "*C:") {
			m_keysignature = "*k[]";
		} else if (m_keydesignation == "*G:") {
			m_keysignature = "*k[f#]";
		} else if (m_keydesignation == "*D:") {
			m_keysignature = "*k[f#c#]";
		} else if (m_keydesignation == "*A:") {
			m_keysignature = "*k[f#c#g#]";
		} else if (m_keydesignation == "*E:") {
			m_keysignature = "*k[f#c#g#d#]";
		} else if (m_keydesignation == "*B:") {
			m_keysignature = "*k[f#c#g#d#a#]";
		} else if (m_keydesignation == "*F#:") {
			m_keysignature = "*k[f#c#g#d#a#e#]";
		} else if (m_keydesignation == "*C#:") {
			m_keysignature = "*k[f#c#g#d#a#e#b#]";
		} else if (m_keydesignation == "*F:") {
			m_keysignature = "*k[b-]";
		} else if (m_keydesignation == "*B-:") {
			m_keysignature = "*k[b-e-]";
		} else if (m_keydesignation == "*E-:") {
			m_keysignature = "*k[b-e-a-]";
		} else if (m_keydesignation == "*A-:") {
			m_keysignature = "*k[b-e-a-d-]";
		} else if (m_keydesignation == "*D-:") {
			m_keysignature = "*k[b-e-a-d-g-]";
		} else if (m_keydesignation == "*G-:") {
			m_keysignature = "*k[b-e-a-d-g-c-]";
		} else if (m_keydesignation == "*C-:") {
			m_keysignature = "*k[b-e-a-d-g-f-]";
		} else {
			m_errors.push_back("ERROR: invalid/exotic key signature required.");
		}

	} else  {

		// minor key signature
		if (m_keydesignation == "*a:") {
			m_keysignature = "*k[]";
		} else if (m_keydesignation == "*e:") {
			m_keysignature = "*k[f#]";
		} else if (m_keydesignation == "*b:") {
			m_keysignature = "*k[f#c#]";
		} else if (m_keydesignation == "*f#:") {
			m_keysignature = "*k[f#c#g$]";
		} else if (m_keydesignation == "*c#:") {
			m_keysignature = "*k[f#c#g$d#]";
		} else if (m_keydesignation == "*g#:") {
			m_keysignature = "*k[f#c#g$d#a#]";
		} else if (m_keydesignation == "*d#:") {
			m_keysignature = "*k[f#c#g$d#a#e#]";
		} else if (m_keydesignation == "*a#:") {
			m_keysignature = "*k[f#c#g$d#a#e#b#]";
		} else if (m_keydesignation == "*d:") {
			m_keysignature = "*k[b-]";
		} else if (m_keydesignation == "*g:") {
			m_keysignature = "*k[b-e-]";
		} else if (m_keydesignation == "*c:") {
			m_keysignature = "*k[b-e-a-]";
		} else if (m_keydesignation == "*f:") {
			m_keysignature = "*k[b-e-a-d-]";
		} else if (m_keydesignation == "*b-:") {
			m_keysignature = "*k[b-e-a-d-g-]";
		} else if (m_keydesignation == "*e-:") {
			m_keysignature = "*k[b-e-a-d-g-c-]";
		} else if (m_keydesignation == "*a-:") {
			m_keysignature = "*k[b-e-a-d-g-f-]";
		} else {
			m_errors.push_back("ERROR: invalid/exotic key signature required.");
		}
	}

}



//////////////////////////////
//
// Tool_esac2hum::Score::calculateClef --
//

void Tool_esac2hum::Score::calculateClef(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	double sum = 0;
	double count = 0;
	int min12 = 1000;
	int max12 = -1000;

	for (int i=0; i<(int)notelist.size(); i++) {
		int b12 = notelist[i]->m_b12;
		if (b12 > 0) {
			sum += b12;
			count++;
			if (b12 < min12) {
				min12 = b12;
			}
			if (b12 > max12) {
				max12 = b12;
			}
		}
	}
	double average = sum / count;


	if ((min12 > 54) && (average >= 60.0)) {
		m_clef = "*clefG2";
	} else if ((max12 < 67) && (average < 60.0)) {
		m_clef = "*clefF4";
	} else if ((min12 > 47) && (min12 <= 57) && (max12 < 77) && (max12 >= 65)) {
		m_clef = "*clefGv2";
	} else if (average < 60.0) {
		m_clef = "*clefF2";
	} else {
		m_clef = "*clefG2";
	}
}



//////////////////////////////
//
// Tool_esac2hum::generateHumdrumNotes --
//

void Tool_esac2hum::Score::generateHumdrumNotes(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	string tonic = m_params["_tonic"];
	if (tonic.empty()) {
		m_errors.push_back("Error: cannot find KEY[] tonic pitch");
		return;
	}
	char letter = std::tolower(tonic[0]);
	m_b40tonic = 40 * 4 + 2;  // start with middle C
	switch (letter) {
		case 'd': m_b40tonic +=  6; break;
		case 'e': m_b40tonic += 12; break;
		case 'f': m_b40tonic += 17; break;
		case 'g': m_b40tonic += 23; break;
		case 'a': m_b40tonic += 29; break;
		case 'b': m_b40tonic += 35; break;
	}
	int flats = 0;
	int sharps = 0;
	for (int i=1; i<(int)tonic.size(); i++) {
		if (tonic[i] == 'b') {
			flats++;
		} else if (tonic[i] == '#') {
			sharps++;
		}
	}
	if (flats > 0) {
		m_b40tonic -= flats;
	} else if (sharps > 0) {
		m_b40tonic += sharps;
	}

	string minrhy = m_params["_minrhy"];
	if (minrhy.empty()) {
		m_errors.push_back("Error: cannot find KEY[] minrhy");
		return;
	}

	m_minrhy = std::stoi(minrhy);
	// maybe check of power of two?

	for (int i=0; i<(int)notelist.size(); i++) {
		notelist.at(i)->generateHumdrum(m_minrhy, m_b40tonic);
	}

}



//////////////////////////////
//
// Tool_esac2hum::Note::generateHumdrum -- convert EsAC note to Humdrum note token.
//

void Tool_esac2hum::Note::generateHumdrum(int minrhy, int b40tonic) {
	string pitch;
	if (m_degree != 0) {
		m_b40degree = 0;
		switch (abs(m_degree)) {
			case 2: m_b40degree += 6;  break;
			case 3: m_b40degree += 12; break;
			case 4: m_b40degree += 17; break;
			case 5: m_b40degree += 23; break;
			case 6: m_b40degree += 29; break;
			case 7: m_b40degree += 35; break;
		}
		if ((m_alter >= -2) && (m_alter <= 2)) {
			m_b40degree += m_alter;
		} else {
			m_errors.push_back("Error: chromatic alteration on note too large");
		}
		m_b40 = 40 * m_octave + m_b40degree + b40tonic;
		pitch = Convert::base40ToKern(m_b40);
		// m_b12 is used for calculating clef later on.
		m_b12 = Convert::base40ToMidiNoteNumber(m_b40);
	} else {
		pitch = "r";
		m_b40 = -1000;
		m_b40degree = -1000;
	}

	HumNum duration(1, minrhy);
	int multiplier = (1 << m_underscores);
	duration *= multiplier;
	duration *= 4;  // convert from whole notes to quarter notes
	duration *= m_factor;
	string recip = Convert::durationToRecip(duration);
	for (int i=0; i<m_dots; i++) {
		recip += ".";
	}

	m_humdrum.clear();
	if (m_phraseBegin) {
		m_humdrum += "{";
	}

	if (m_tieBegin && !m_tieEnd) {
		m_humdrum += "[";
	}

	m_humdrum += recip;
	m_humdrum += pitch;

	if (!m_tieBegin && m_tieEnd) {
		m_humdrum += "]";
	} else if (m_tieBegin && m_tieEnd) {
		m_humdrum += "_";
	}

	if (m_phraseEnd) {
		m_humdrum += "}";
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeTies -- Create a list of notes
//     in each phrase and then assign a phrase start to the
//     first non-rest note, and phrase end to the last non-rest note.
//

void Tool_esac2hum::Score::analyzeTies(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	for (int i=1; i<(int)notelist.size(); i++) {
		// negative m_degree indicates a tied note to previous note
		if (notelist.at(i)->m_degree < 0) {
			// Tied note, so link to previous note.
			notelist.at(i)->m_tieEnd = true;
			notelist.at(i-1)->m_tieBegin = true;
			if (notelist.at(i-1)->m_degree >= 0) {
				notelist.at(i)->m_degree = -notelist.at(i-1)->m_degree;
				// Copy chromatic alteration and octave:
				notelist[i]->m_alter = notelist.at(i-1)->m_alter;
				notelist[i]->m_octave = notelist.at(i-1)->m_octave;
			}
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::getNoteList -- Return a list of all notes
//      in the score.
//

void Tool_esac2hum::Score::getNoteList(vector<Tool_esac2hum::Note*>& notelist) {
	notelist.clear();
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		for (int j=0; j<(int)phrase.size(); j++) {
			Tool_esac2hum::Measure& measure = phrase[j];
			for (int k=0; k<(int)measure.size(); k++) {
				notelist.push_back(&measure.at(k));
			}
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::getMeasureList --
//

void Tool_esac2hum::Score::getMeasureList(vector<Tool_esac2hum::Measure*>& measurelist) {
	measurelist.clear();
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		for (int j=0; j<(int)phrase.size(); j++) {
			Tool_esac2hum::Measure& measure = phrase[j];
			measurelist.push_back(&measure);
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzePhrases -- Create a list of notes in the score
//     and then search for ^ (-1 degrees) which mean a tied continuation
//     of the previous note.
//

void Tool_esac2hum::Score::analyzePhrases(void) {
	// first create a list of the notes in the score
	vector<Tool_esac2hum::Note*> notelist;
	for (int i=0; i<(int)size(); i++) {
		getPhraseNoteList(notelist, i);

		if (notelist.empty()) {
			at(i).m_errors.push_back("ERROR: no notes in phrase.");
			return;
		}

		// Find the first non-rest note and mark with phrase start:
		bool foundNote = false;
		for (int j=0; j<(int)notelist.size(); j++) {
			if (notelist.at(j)->m_degree <= 0) {
				continue;
			}
			foundNote = true;
			notelist.at(j)->m_phraseBegin = true;
			break;
		}

		if (!foundNote) {
			at(i).m_errors.push_back("Error: cannot find any notes in phrase.");
			continue;
		}

		// Find the last non-rest note and mark with phrase end:
		for (int j=(int)notelist.size()-1; j>=0; j--) {
			if (notelist.at(j)->m_degree <= 0) {
				continue;
			}
			notelist.at(j)->m_phraseEnd = true;
			break;
		}
	}
}


//////////////////////////////
//
// Tool_esac2hum::Score::getPhraseNoteList -- Return a list of all notes
//      in the 0-indexed phrase
//

void Tool_esac2hum::Score::getPhraseNoteList(vector<Tool_esac2hum::Note*>& notelist, int index) {
	notelist.clear();
	if (index < 0) {
		m_errors.push_back("ERROR: trying to access a negative phrase index");
		return;
	}
	if (index >= (int)size()) {
		m_errors.push_back("ERROR: trying to access a phrase index that is too large");
		return;
	}
	Tool_esac2hum::Phrase& phrase = at(index);

	for (int i=0; i<(int)phrase.size(); i++) {
		Tool_esac2hum::Measure& measure = phrase[i];
		for (int j=0; j<(int)measure.size(); j++) {
			Tool_esac2hum::Note& note = measure.at(j);
			notelist.push_back(&note);
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::Phrase::getNoteList -- Return a list of all notes
//      in the phrase.
//

void Tool_esac2hum::Phrase::getNoteList(vector<Tool_esac2hum::Note*>& notelist) {
	notelist.clear();
	Tool_esac2hum::Phrase& phrase = *this;

	for (int i=0; i<(int)phrase.size(); i++) {
		Tool_esac2hum::Measure& measure = phrase[i];
		for (int j=0; j<(int)measure.size(); j++) {
			Tool_esac2hum::Note& note = measure.at(j);
			notelist.push_back(&note);
		}
	}
}



//////////////////////////////
//
// Tool_esac2hum::Phrase::parsePhrase --
//

bool Tool_esac2hum::Phrase::parsePhrase(const string& phrase) {
	esac = phrase;

	vector<string> bars;

	HumRegex hre;
	string newphrase = phrase;
	newphrase = trimSpaces(newphrase);
	hre.split(bars, newphrase, "\\s+");
	if (bars.empty()) {
		cerr << "Funny error with no measures" << endl;
		return false;
	}
	int length = (int)bars.size();
	for (int i=0; i<length; i++) {
		resize(size() + 1);
		back().parseMeasure(bars[i]);
	}

	// Calculate ticks for phrase:
	m_ticks = 0;
	for (int i=0; i<(int)size(); i++) {
		m_ticks += at(i).m_ticks;
	}

	return true;
}



//////////////////////////////
//
// Tool_esac2hum::Measure::parseMeasure --
//     Also deal with () for ties.
//     Split notes by digit.  Prefix characters attached to digit:
//        ^: equivalent to digit, tied to previous note.
//        -: digit is scale degree in lower octave.
//        (: slur start
//

bool Tool_esac2hum::Measure::parseMeasure(const string& measure) {
	esac = measure;
	vector<string> tokens;
	vector<HumNum> factors;
	HumNum factor = 1;
	int length = (int)measure.size();
	for (int i=0; i<length; i++) {
		if (measure[i] == '(') {
			factor = 2;
			factor /= 3;
		}

		bool marker = false;
		if (std::isdigit(measure[i])) {
			marker = true;
		} else if (measure[i] == '(') {  // tuplet start
			marker = true;
		} else if (measure[i] == '-') {  // flat
			marker = true;
		} else if (measure[i] == '+') {  // sharp
			marker = true;
		} else if (measure[i] == '^') {  // tie placeholder for degree
			marker = true;
		}

		if (marker && !tokens.empty() && !tokens.back().empty()) {
			char checkChar = tokens.back().back();
			if (checkChar == '(') {
				marker = false;
			} else if (checkChar == '^') {
				marker = false;
			} else if (checkChar == '-') {
				marker = false;
			} else if (checkChar == '+') {
				marker = false;
			}
		}

		if (marker) {
			tokens.resize(tokens.size() + 1);
			tokens.back() += measure[i];
			factors.resize(factors.size() + 1);
			factors.back() = factor;
		} else {
			if (!tokens.empty()) {
				tokens.back() += measure[i];
			} else {
				cerr << "!!ERROR: unknown character at start of measure: " << measure << endl;
			}
		}

		if (measure[i] == ')') {
			factor = 1;
		}
	}

	if (tokens.empty()) {
		cerr << "!!ERROR: In measure: " << measure << ": no notes to parts." << endl;
		return false;
	}

	for (int i=0; i<(int)tokens.size(); i++) {
		resize(size() + 1);
		back().parseNote(tokens[i], factors[i]);
	}

	// Calculate ticks for measure:
	m_ticks = 0;
	for (int i=0; i<(int)size(); i++) {
		m_ticks += at(i).m_ticks;
	}

	return true;
}



//////////////////////////////
//
// Tool_esac2hum::Note::parseNote --
//

bool Tool_esac2hum::Note::parseNote(const string& note, HumNum factor) {
	esac = note;

	int minus = 0;
	int plus = 0;
	int b = 0;
	int s = 0;
	m_degree = 0;
	m_dots = 0;

	for (int i=0; i<(int)note.size(); i++) {
		if (note[i] == '.') {        // augmentation dot
			m_dots++;
		} else if (note[i] == '_') { // duration modifier
			m_underscores++;
		} else if (note[i] == '-') { // lower octave
			minus++;
		} else if (note[i] == '+') { // upper octave
			plus++;
		} else if (note[i] == 'b') { // flat
			b++;
		} else if (note[i] == '#') { // sharp
			s++;
		} else if (isdigit(note[i])) {
			m_degree = note[i] - '0';
		} else if (note[i] == '^') { // tied to previous note
			m_degree = -1000;
		}
	}

	m_ticks = 1 << m_underscores;
	if (m_dots > 0) {
		m_ticks = m_ticks * (2.0 - 1.0/(1 << m_dots));
	}

	if (b > 2) {
		cerr << "!! ERROR: more than double flat not parseable, note: " << esac << endl;
	}
	if (s > 2) {
		cerr << "!! ERROR: more than double sharp not parseable, note: " << esac << endl;
	}

	m_alter = s - b;
 	m_octave = plus - minus;

	m_factor = factor;

	return true;
}



//////////////////////////////
//
// Tool_esac2hum::printHeader --
//

void Tool_esac2hum::printHeader(ostream& output) {
	string filename = createFilename();
	output << "!!!!SEGMENT: " << filename << endl;

	string title = m_score.m_params["_title"];
	output << "!!!OTL:";
	if (!title.empty()) {
		output << " " << title;
	}
	output << endl;
	// sometimes CUT[] has two lines, and the sescond is the text incipit:
	string incipit = m_score.m_params["_incipit"];
	if (!incipit.empty()) {
		output << "!!!TIN: " << incipit << endl;
	}

	string id = m_score.m_params["_id"];
	output << "!!!id:";
	if (!id.empty()) {
		output << " " << id;
	}
	output << endl;

	string source = m_score.m_params["_source"];
	output << "!!!source:";
	if (!source.empty()) {
		output << " " << source;
	}
	output << endl;

	string signature = m_score.m_params["SIG"];
	output << "!!!signature:";
	if (!signature.empty()) {
		output << " " << signature;
	}
	output << endl;

	output << "**kern" << endl;
}



//////////////////////////////
//
// Tool_esac2hum::createFilename -- from SIG[] and CUT[], with spaces in CUT[] turned into
//     underscores and accents removed from characters.
//
//     Also need to deal with decomposed accents, if necessary:
//         0x0301: Combining acute accent
//         0x0300: Combining grave accent
//         0x0302: Combining circumflex accent
//         0x0303: Combining tilde
//         0x0308: Combining diaeresis (umlaut)
//         0x0327: Combining cedilla
//         0x0328: Combining ogonek
//         0x0304: Combining macron
//         0x0306: Combining breve
//         0x0307: Combining dot above
//         0x0323: Combining dot below
//         0x030A: Combining ring above
//         0x030B: Combining double acute accent
//         0x030C: Combining caron
//
//
//    std::unordered_map<char, char> m_accent_map = {
//         {'á', 'a'}, {'à', 'a'}, {'ä', 'a'}, {'â', 'a'}, {'ã', 'a'}, {'å', 'a'},
//         {'é', 'e'}, {'è', 'e'}, {'ë', 'e'}, {'ê', 'e'},
//         {'í', 'i'}, {'ì', 'i'}, {'ï', 'i'}, {'î', 'i'},
//         {'ó', 'o'}, {'ò', 'o'}, {'ö', 'o'}, {'ô', 'o'}, {'õ', 'o'}, {'ø', 'o'},
//         {'ú', 'u'}, {'ù', 'u'}, {'ü', 'u'}, {'û', 'u'},
//         {'ý', 'y'}, {'ÿ', 'y'},
//         {'ñ', 'n'}, {'ç', 'c'},
//         {'ą', 'a'}, {'ć', 'c'}, {'ę', 'e'}, {'ł', 'l'}, {'ń', 'n'},
//         {'ś', 's'}, {'ź', 'z'}, {'ż', 'z'}
//    };

string Tool_esac2hum::createFilename(void) {
	string prefix = m_score.m_params["_source"];
	string sig = m_score.m_params["SIG"];
	string title = m_score.m_params["_title"];
	string id  = m_score.m_params["_id"];
	if (sig.empty()) {
		sig = id;
	}

	HumRegex hre;
	// Should not be spaces, but just in case;
	hre.replaceDestructive(sig, "", "\\s+", "g");
	hre.replaceDestructive(prefix, "", "\\s+", "g");

	if (!m_filePrefix.empty()) {
		prefix = m_filePrefix;
	}

	// Convert spaces to underscores:
	hre.replaceDestructive(title, "_", "\\s+", "g");
	// Remove accents:
	hre.replaceDestructive(title, "a", "á", "g");
	hre.replaceDestructive(title, "a", "à", "g");
	hre.replaceDestructive(title, "a", "ä", "g");
	hre.replaceDestructive(title, "a", "â", "g");
	hre.replaceDestructive(title, "a", "ã", "g");
	hre.replaceDestructive(title, "a", "å", "g");
	hre.replaceDestructive(title, "e", "é", "g");
	hre.replaceDestructive(title, "e", "è", "g");
	hre.replaceDestructive(title, "e", "ë", "g");
	hre.replaceDestructive(title, "e", "ê", "g");
	hre.replaceDestructive(title, "i", "í", "g");
	hre.replaceDestructive(title, "i", "ì", "g");
	hre.replaceDestructive(title, "i", "ï", "g");
	hre.replaceDestructive(title, "i", "î", "g");
	hre.replaceDestructive(title, "o", "ó", "g");
	hre.replaceDestructive(title, "o", "ò", "g");
	hre.replaceDestructive(title, "o", "ö", "g");
	hre.replaceDestructive(title, "o", "ô", "g");
	hre.replaceDestructive(title, "o", "õ", "g");
	hre.replaceDestructive(title, "o", "ø", "g");
	hre.replaceDestructive(title, "u", "ú", "g");
	hre.replaceDestructive(title, "u", "ù", "g");
	hre.replaceDestructive(title, "u", "ü", "g");
	hre.replaceDestructive(title, "u", "û", "g");
	hre.replaceDestructive(title, "y", "ý", "g");
	hre.replaceDestructive(title, "y", "ÿ", "g");
	hre.replaceDestructive(title, "n", "ñ", "g");
	hre.replaceDestructive(title, "c", "ç", "g");
	hre.replaceDestructive(title, "a", "ą", "g");
	hre.replaceDestructive(title, "c", "ć", "g");
	hre.replaceDestructive(title, "e", "ę", "g");
	hre.replaceDestructive(title, "l", "ł", "g");
	hre.replaceDestructive(title, "n", "ń", "g");
	hre.replaceDestructive(title, "s", "ś", "g");
	hre.replaceDestructive(title, "z", "ź", "g");
	hre.replaceDestructive(title, "z", "ż", "g");
	hre.replaceDestructive(title, "", "[^a-zA-Z0-9-_.]", "g");

	std::transform(title.begin(), title.end(), title.begin(),
			[](unsigned char c) { return std::tolower(c); });

	string output;
	if (!prefix.empty()) {
		output += prefix + "-";
	}
	output += sig;
	if (!(sig.empty() || title.empty())) {
		output += "-";
	}
	output += title;
	if (output.empty()) {
		output = "file";
	}
	output += m_filePostfix;

	return output;
}



//////////////////////////////
//
// Tool_esac2hum::getParameters --
//

void Tool_esac2hum::getParameters(vector<string>& infile) {
	m_score.m_params.clear();
	HumRegex hre;
	bool expectingCloseQ = false;
	string lastKey = "";
	for (int i=0; i<(int)infile.size(); i++) {
		if (hre.search(infile[i], "^\\s*$")) {
			continue;
		}
		if ((i == 0) && hre.search(infile[i], "^([A-Z_a-z][^\\]\\[]*)\\s*$")) {
			m_score.m_params["_source"] = hre.getMatch(1);
			continue;
		}
		if (expectingCloseQ) {
			if (infile[i].find("[") != string::npos) {
				cerr << "Strange case searching for close: " << infile[i] << endl;
			} else if (infile[i].find("]") == string::npos) {
				// continuing a parameter:
				if (lastKey == "") {
					cerr << "Strange case of no last key when closing parameter: " << infile[i] << endl;
				} else {
					m_score.m_params[lastKey] += "\n" + infile[i];
				}
			} else if (hre.search(infile[i], "^([^\\]]+)\\]\\s*$")) {
				// closing a parameter:
				if (lastKey == "") {
					cerr << "Strange case B of no last key when closing parameter: " << infile[i] << endl;
				} else {
					string value = hre.getMatch(1);
					m_score.m_params[lastKey] += "\n" + value;
					expectingCloseQ = false;
					continue;
				}
			} else {
				cerr << "Problem closing parameter: " << infile[i] << endl;
			}
			continue;
		} else if (hre.search(infile[i], "^\\s*([A-Z_a-z]+)\\s*\\[([^\\]]*)\\]\\s*$")) {
			// single line parameter
			string key   = hre.getMatch(1);
			string value = hre.getMatch(2);

			// Rare cases where the key has lower case letters that should not be there:
			std::transform(key.begin(), key.end(), key.begin(),
					[](unsigned char c) { return std::toupper(c); });

			m_score.m_params[key] = value;
			continue;
		} else if (hre.search(infile[i], "^\\s*([A-Z_a-z]+)\\s*\\[([^\\]]*)\\s*$")) {
			// opening of a parameter
			string key   = hre.getMatch(1);
			string value = hre.getMatch(2);

			// Rare cases where the key has lower case letters that should not be there:
			std::transform(key.begin(), key.end(), key.begin(),
					[](unsigned char c) { return std::toupper(c); });

			m_score.m_params[key] = value;
			lastKey = key;
			expectingCloseQ = true;
			continue;
		} else {
			cerr << "UNKNOWN CASE: " << infile[i] << endl;
		}
	}

	// The CUT[] line can be multiple lines, the first being the title and
	// the second being the text incipit.  Split them into _title and _incipit
	// fields (not checking if more than two lines):
	string cut = m_score.m_params["CUT"];
	if (hre.search(cut, "^\\s*(.*?)\\n(.*?)\\s*$", "s")) {
		m_score.m_params["_title"]   = trimSpaces(hre.getMatch(1));
		m_score.m_params["_incipit"] = trimSpaces(hre.getMatch(2));
	} else {
		// Don't know if CUT[] is title or incipit, but assign to title.
		m_score.m_params["_title"] = trimSpaces(cut);
		m_score.m_params["_incipit"] = "";
	}

	string key = m_score.m_params["KEY"];
	if (hre.search(key, "^\\s*([^\\s]+)\\s+(\\d+)\\s+([A-Gacdefg][bs]*)\\s+(.*?)\\s*$")) {
		string id     = hre.getMatch(1);
		string minrhy = hre.getMatch(2);
		string tonic  = hre.getMatch(3);
		if (tonic.size() >= 1) {
			if (tonic[0] == 'b') {
				cerr << "Error: key signature cannot be 'b'." << endl;
			} else {
				if (std::islower(tonic[0])) {
					cerr << "Warning: Tonic note should be upper case." << endl;
					tonic[0] = std::toupper(tonic[0]);
				}
			}
		}
		string time   = hre.getMatch(4);
		m_score.m_params["_id"]     = id;
		m_score.m_params["_minrhy"] = minrhy;
		m_score.m_params["_tonic"]  = tonic;
		m_score.m_params["_time"]   = time;
		m_minrhy = stoi(minrhy);
	} else {
		cerr << "Problem parsing KEY parameter: " << key << endl;
	}

	string trd;
	if (hre.search(trd, "^\\s*(.*)\\ss\\.")) {
		m_score.m_params["_source_trd"] = hre.getMatch(1);
	}
	if (hre.search(trd, "s\\.\\s*(\\d+-?\\d*)")) {
		// Could be text aftewards about the origin of the song.
		m_score.m_params["_page"] = hre.getMatch(1);
	}

	if (m_debugQ) {
		printParameters();
	}

	if (hre.search(m_score.m_params["_source_trd"], "^\\s*(DWOK\\d+)")) {
		m_dwokQ = true;
	} else if (hre.search(m_score.m_params["_source"], "^\\s*(DWOK\\d+)")) {
		m_dwokQ = true;
	}

}



//////////////////////////////
//
// Tool_esac2hum::printParameters --
//

void Tool_esac2hum::printParameters(void) {
	cerr << endl;
	cerr << "========================================" << endl;
    for (const auto& [key, value] : m_score.m_params) {
        cerr << "Key: " << key << ", Value: " << value << endl;
    }
	cerr << "========================================" << endl;
	cerr << endl;
}



//////////////////////////////
//
// Tool_esac2hum::printBemComment --
//

void Tool_esac2hum::printBemComment(ostream& output) {
	string bem = m_score.m_params["BEM"];
	if (bem.empty()) {
		return;
	}
	string english = m_bem_translation[bem];
	if (english.empty()) {
		output << "!!!ONB: " << bem << endl;
	} else {
		output << "!!!ONB@@PL: " << bem << endl;
		output << "!!!ONB@@EN: " << english << endl;
	}
}



//////////////////////////////
//
// Tool_esac2hum::printFooter --
//

void Tool_esac2hum::printFooter(ostream& output, vector<string>& infile) {
	output << "*-" << endl;

	printBemComment(output);
	printPdfLinks(output);
	printConversionDate(output);

	if (m_embedEsacQ) {
		output << "!!@@BEGIN: ESAC" << endl;
		output << "!!@CONTENTS:" << endl;;
		for (int i=0; i<(int)infile.size(); i++) {
//			cout << "!!" << infile[i] << endl;
		}
		if (m_analysisQ) {
			embedAnalyses(output);
		}
		output << "!!@@END: ESAC" << endl;
	}
}



void Tool_esac2hum::embedAnalyses(ostream& output) {
	m_score.doAnalyses();
	string MEL_SEM  = m_score.m_params["MEL_SEM"];
	string MEL_RAW  = m_score.m_params["MEL_RAW"];
	string NO_REP   = m_score.m_params["NO_REP"];
	string RTM      = m_score.m_params["RTM"];
	string SCL_DEG  = m_score.m_params["SCL_DEG"];
	string SCL_SEM  = m_score.m_params["SCL_SEM"];
	string PHR_NO   = m_score.m_params["PHR_NO"];
	string PHR_BARS = m_score.m_params["PHR_BARS"];
	string PHR_CAD  = m_score.m_params["PHR_CAD"];
	string ACC      = m_score.m_params["ACC"];

	bool allEmptyQ = true;
	if      (!MEL_SEM.empty() ) { allEmptyQ = false; }
	else if (!MEL_RAW.empty() ) { allEmptyQ = false; }
	else if (!NO_REP.empty()  ) { allEmptyQ = false; }
	else if (!RTM.empty()     ) { allEmptyQ = false; }
	else if (!SCL_DEG.empty() ) { allEmptyQ = false; }
	else if (!SCL_SEM.empty() ) { allEmptyQ = false; }
	else if (!PHR_NO.empty()  ) { allEmptyQ = false; }
	else if (!PHR_BARS.empty()) { allEmptyQ = false; }
	else if (!PHR_CAD.empty() ) { allEmptyQ = false; }
	else if (!ACC.empty()     ) { allEmptyQ = false; }

	if (allEmptyQ) {
		// no analyses for some strange reason.
		return;
	}
	output << "!!@ANALYSES:" << endl;
	if (!MEL_SEM.empty() ) { output << "!!MEL_SEM["  << MEL_SEM  << "]" << endl; }
	if (!MEL_RAW.empty() ) { output << "!!MEL_RAW["  << MEL_RAW  << "]" << endl; }
	if (!NO_REP.empty()  ) { output << "!!NO_REP["   << NO_REP   << "]" << endl; }
	if (!RTM.empty()     ) { output << "!!RTM["      << RTM      << "]" << endl; }
	if (!SCL_DEG.empty() ) { output << "!!SCL_DEG["  << SCL_DEG  << "]" << endl; }
	if (!SCL_SEM.empty() ) { output << "!!SCL_SEM["  << SCL_SEM  << "]" << endl; }
	if (!PHR_NO.empty()  ) { output << "!!PHR_NO["   << PHR_NO   << "]" << endl; }
	if (!PHR_BARS.empty()) { output << "!!PHR_BARS[" << PHR_BARS << "]" << endl; }
	if (!PHR_CAD.empty() ) { output << "!!PHR_CAD["  << PHR_CAD  << "]" << endl; }
	if (!ACC.empty()     ) { output << "!!ACC["      << ACC      << "]" << endl; }

}


///////////////////////////////
//
// Tool_esac2hum::printPdfLinks --
//

void Tool_esac2hum::printPdfLinks(ostream& output) {
	output << "!!!URL: http://webesac.pcss.pl WebEsAC" << endl;

	if (!m_dwokQ) {
		return;
	}

	output << "!!!URL: https::kolberg.ispan.pl/dwok/tomy Oskar Kolberg: Complete Works digital edition" << endl;

	string source = m_score.m_params["_source"];
	HumRegex hre;
	if (!hre.search(source, "^DWOK(\\d+)")) {
		return;
	}
	string volume = hre.getMatch(1);
	if (volume.size() == 1) {
		volume = "0" + volume;
	}
	if (volume.size() == 2) {
		volume = "0" + volume;
	}
	if (volume.size() > 3) {
		return;
	}
	string nozero = volume;
	hre.replaceDestructive(nozero, "" , "^0+");
	// need http:// not https:// for the following PDF link:
	output << "!!!URL-pdf: http://oskarkolberg.pl/MediaFiles/" << volume << "dwok.pdf" << " Oskar Kolberg: Complete Works, volume " << nozero << endl;

}



///////////////////////////////
//
// Tool_esac2hum::printCoversionDate --
//

void Tool_esac2hum::printConversionDate(ostream& output) {
	std::time_t t = std::time(nullptr);
	std::tm* now = std::localtime(&t);
	output << "!!!ONB: Converted on ";
	output << std::put_time(now, "%Y/%m/%d");
	output << " with esac2hum" << endl;
}



//////////////////////////////
//
// Tool_esac2hum::Score::doAnalyses --
//

void Tool_esac2hum::Score::doAnalyses(void) {
	analyzeMEL_SEM();
	analyzeMEL_RAW();
	analyzeNO_REP();
	analyzeRTM();
	analyzeSCL_DEG();
	analyzeSCL_SEM();
	analyzePHR_NO();
	analyzePHR_BARS();
	analyzePHR_CAD();
	analyzeACC();
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeMEL_SEM -- Current
//   algorithm: calculate intervals across rest, ignore tied notes.
//   values are differences between m_b12 of notes.
//

void Tool_esac2hum::Score::analyzeMEL_SEM(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	vector<int> b12s;  // list of notes to calculate intervals between

	for (int i=0; i<(int)notelist.size(); i++) {
		if (notelist[i]->isRest()) {
			continue;
		}
		if (notelist[i]->m_tieEnd) {
			continue;
		}
		b12s.push_back(notelist[i]->m_b12);
	}

	string output;
	for (int i=1; i<(int)b12s.size(); i++) {
		int difference = b12s[i] - b12s[i-1];
		output += to_string(difference);
		if (i < (int)b12s.size() - 1) {
			output += " ";
		}
	}

	m_params["MEL_SEM"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeMEL_RAW -- Remove rhythms from MEL[] data.
//    Preserve spaces as in original MEL[];
//    What to do with parentheses? Currently removed.
//    What to do with tied notes?  Currently removed.
//

void Tool_esac2hum::Score::analyzeMEL_RAW(void) {
	string output = m_params["MEL"];
	HumRegex hre;
	hre.replaceDestructive(output, "", "[^\\d+\\sb#-]+", "g");
	hre.replaceDestructive(output, "", "\\s*//\\s*$");
	hre.replaceDestructive(output, "\n!!", "\n", "g");
	m_params["MEL_RAW"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeNO_REP -- Return
//     the non-repeated notes/rests without rhythms
//     in each phrase with a newlines between phrases
//     and no spaces between notes or measures.
//

void Tool_esac2hum::Score::analyzeNO_REP(void) {
	string output;
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		string line = phrase.getNO_REP();
		if (i > 0) {
			output += "\n    ";
		}
		output += line;
	}

	HumRegex hre;
	hre.replaceDestructive(output, "\n!!", "\n", "g");

	m_params["NO_REP"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Phrase::getNO_REP -- Return
//     the non-repeated notes/rests without rhythms
//     with no spaces between notes or measures.
//     What to do if line starts with an ending tied note?
//     Currently ignoring leading tied end notes.
//

string Tool_esac2hum::Phrase::getNO_REP(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);
	string output;
	int foundNonTie = false;
	string lastitem = "";
	for (int i=0; i<(int)notelist.size(); i++) {
		if (!foundNonTie && notelist[i]->m_tieEnd) {
			continue;
		}
		foundNonTie = true;
		string curitem = notelist[i]->getScaleDegree();
		if (curitem != lastitem) {
			output += curitem;
			lastitem = curitem;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeRTM -- Convert pitches/rests to "x".
//      What to do with tied notes?  Leaving ^ in for now.
//      What to do with ()?  Removing for now.
//

void Tool_esac2hum::Score::analyzeRTM(void) {
	string output = m_params["MEL"];
	HumRegex hre;
	hre.replaceDestructive(output, "", "[()]+", "g");
	hre.replaceDestructive(output, "x", "[+-]*(\\d|\\^)[b#]*", "g");
	hre.replaceDestructive(output, "", "\\s*//\\s*$");
	hre.replaceDestructive(output, "\n!!", "\n", "g");
	m_params["RTM"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeSCL_DEG -- List of scale degrees
//     present in melody from lowest to highest with no spaces between
//     the scale degrees.
//

void Tool_esac2hum::Score::analyzeSCL_DEG(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);
	map<int, Tool_esac2hum::Note*> list;
	for (int i=0; i<(int)notelist.size(); i++) {
		if (notelist[i]->isRest()) {
			continue;
		}
		if (notelist[i]->m_tieEnd) {
			continue;
		}
		int b40 = notelist[i]->m_b40;
		list[b40] = notelist[i];
	}

	string output;
	for (const auto& pair : list) {
		output += pair.second->getScaleDegree();
	}
	m_params["SCL_DEG"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeSCL_SEM -- Get the semitone
//    between scale degrees in SCL_DEG analysis.
//

void Tool_esac2hum::Score::analyzeSCL_SEM(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);
	map<int, Tool_esac2hum::Note*> list;
	for (int i=0; i<(int)notelist.size(); i++) {
		if (notelist[i]->isRest()) {
			continue;
		}
		if (notelist[i]->m_tieEnd) {
			continue;
		}
		int b40 = notelist[i]->m_b40;
		list[b40] = notelist[i];
	}

	string output;
	Tool_esac2hum::Note* lastnote = nullptr;
	for (const auto& pair : list) {
		if (lastnote == nullptr) {
			lastnote = pair.second;
			continue;
		}
		int second = pair.second->m_b12;
		int first = lastnote->m_b12;
		int difference = second -first;
		if (!output.empty()) {
			output += " ";
		}
		output += to_string(difference);
		lastnote = pair.second;
	}
	m_params["SCL_SEM"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzePHR_NO --
//

void Tool_esac2hum::Score::analyzePHR_NO(void) {
	int phraseCount = (int)size();
	m_params["PHR_NO"] = to_string(phraseCount);
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzePHR_BARS -- Return the number
//    of measures in each phrase.
//

void Tool_esac2hum::Score::analyzePHR_BARS(void) {
	string output;
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		int barCount = phrase.getFullMeasureCount();
		output += to_string(barCount);
		if (i < (int)size() - 1) {
			output += " ";
		}
	}
	m_params["PHR_BARS"] = output;
}



//////////////////////////////
//
// Tool_esac2hum:::Phrase::getFullMeasureCount -- Return the number
//      of measures, but subtrack one if the first measure is a
//      partialEnd and the last is a partialBegin.
//

int Tool_esac2hum::Phrase::getFullMeasureCount(void) {
	int measureCount = (int)size();
	if (measureCount < 2) {
		return measureCount;
	}
	if (at(0).isPartialEnd() && back().isPartialBegin()) {
		measureCount--;
	}

	// if the fist is partial and the last is not, also -1
	if (at(0).isPartialEnd() && back().isComplete()) {
		measureCount--;
	}

	// if the fist is complete and the last is incomplete, also -1
	if (at(0).isComplete() && back().isPartialBegin()) {
		measureCount--;
	}


	// what to do if first measure is pickup (and maybe last measure)?
	return measureCount;
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzePHR_CAD -- Give a space-delimited
//     list of the last scale degree of each phrase.
//

void Tool_esac2hum::Score::analyzePHR_CAD(void) {
	string output;
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		output += phrase.getLastScaleDegree();
		if (i < (int)size() - 1) {
			output += " ";
		}
	}
	m_params["PHR_CAD"] = output;
}



//////////////////////////////
//
// Tool_esac2hum::Phrase::getLastScaleDegree --
//

string Tool_esac2hum::Phrase::getLastScaleDegree(void) {
	vector<Tool_esac2hum::Note*> notelist;
	getNoteList(notelist);

	for (int i=(int)notelist.size() - 1; i>=0; i--) {
		if (notelist[i]->isPitch()) {
			return notelist[i]->getScaleDegree();
		}
	}

	return "?";
}


//////////////////////////////
//
// Tool_esac2hum::Note::getScaleDegree -- return the scale degree
//     string for the note, such as: 6, -6, +7b, 5#.
//

string Tool_esac2hum::Note::getScaleDegree(void) {
	string output;
	if (m_octave < 0) {
		for (int i=0; i<-m_octave; i++) {
			output += "-";
		}
	} else if (m_octave > 0) {
		for (int i=0; i<m_octave; i++) {
			output += "+";
		}
	}
	output += to_string(m_degree);
	if (m_alter < 0) {
		for (int i=0; i<-m_alter; i++) {
			output += "b";
		}
	} else if (m_alter > 0) {
		for (int i=0; i<m_alter; i++) {
			output += "#";
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_esac2hum::Note::isPitch -- return true if scale degree is not 0.
//

bool Tool_esac2hum::Note::isPitch(void) {
	return (m_degree > 0);
}



//////////////////////////////
//
// Tool_esac2hum::Note::isRest -- return true if scale degree is 0.
//

bool Tool_esac2hum::Note::isRest(void) {
	return (m_degree <= 0);
}



//////////////////////////////
//
// Tool_esac2hum::Score::analyzeACC --  The first scale degree
//     of each (complete) meausre
//

void Tool_esac2hum::Score::analyzeACC(void) { }


// END_MERGE

} // end namespace hum



