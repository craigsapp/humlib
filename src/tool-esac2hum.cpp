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
//   * 1-7 = scale degree (major mode by default)
//   * ^ = treated like a scale degree: indicates tied to previous note.
//   * 0 = rest
//   * 7b = lowered 7th degree (major) such as B- in C tonic or Fn in G tonic.
//   * 4# = raised 4th degree (major) such as F# in C tonic or Bn in F tonic.
//   * -7 = in the 3rd octave rather than 4th
//   * +3 = in the 5rd octave rather than 4th
//   * _ = power of two increase from the minimum rhythmic duration
//   		e..g, if minrhy=16 then 3_ is an 8th note, 3__ is a 4th and 3___ is a 2nd.
//   * . = augmentation dot
//   * () = tuplets inside parentheses.
//   * measures are separated by (2) spaces.
//   * linebreaks = phrase separator
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
// Analytic fields that are extracted automatically from musical data (WebEsAC):
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
// Other metadata fields (MAPPET (Music Analysis Software Package),
// see: https://esf.ccarh.org/ccarh-wiki/22-01_Mappet_v2.pdf#page=38)
// Currently not processed (no data with these to test):
// PB[]       = Publisher
// CNR.       = Cut number of tape, etc.
// REG[]      = Region
// ETH[]      = Ethnic group (also language-family, etc.)
// SRC[]      = Source / way of tradition
// RHY[]      = Rhythm incipit
// NP[]       = Number of phrases (Same as PHR_NO[] from WebEsAC).
// STN[]      = Soundtrack number
// ST[]       = Soundtrack (tape, video, etc.)
// MOD[]      = Scale/Mode
// RANGE[]    = low - high tone
// CAD[]      = cadential tones (WebEsAC PHR_CAD?)
// FOP[]      = FORM pitch
// FOR[]      = FORM rhythm
// FOC[]      = FORM contour
// PHRAS[]    = up/down-beat of phrase
// FCT[]      = function
// CMT[]      = comment (BEM[] in WebEsAC?)
// Interval analyses:
// UN[]       = P1 (Unison)
// 2S[]       = +m2 (s = small)
// 2M[]       = +M2
// 3S[]       = +m3
// 3M[]       = +M3
// 4P[]       = +P4
// 4A[]       = +A4
// 5P[]       = +P5
// 6S[]       = +m6
// 6M[]       = +M6
// -----------------------------------
// 7S[]       = +m7
// 7M[]       = +M7
// 8P[]       = +P8
// 8L[]       = ascending, larger than an octave
// SI[]       = sum of intervals ascending (%)
// ABS[]      = absolute (sum) of (all) intervals
// SA[]       = steps ascending (%)
// LA[]       = leaps ascending (%)
// -----------------------------------
// SB[]       = sum of bars (absolute number)
// 2s[]       = -m2 (s = small)
// 2m[]       = -M2
// 3s[]       = -m3
// 3m[]       = -M3
// 4p[]       = -P4
// 4a[]       = -A4
// 5p[]       = -P5
// 6s[]       = -m6
// 6m[]       = -M6
// -----------------------------------
// 7s[]       = -m7
// 7m[]       = -M7
// 8p[]       = -P8
// 8l[]       = leap down, larger than an octave
// si[]       = sum of descent intervals (%)
// dt[]       = descendence tendency (pos., neg., 0)
// sd[]       = steps descend. (%)
// ld[]       = leaps descend. (%)
// -----------------------------------
// 1D[]       = Duration: smallest unit
// 2D[]       = Duration: double length
// 3D[]       = Duration: triple length
// 4D[]       = Duration: quadruple length
// LL[]       = Longer durations (length)
// DD[]       = double dotted
// TD[]       = triple dotted
// QD[]       = quadruple dotted
// TS[]       = triplets/sextuplets
// SD[]       = sum of different durations (absolute)
// SL[]       = sum of all durations (incl. pauses; absolute)
// -----------------------------------
// LD[]       = degrees lower than -3b
// L3M[]      = lower third degree (minor)
// L3[]       = lower third degree (major)
// L4[]       = lower perfect fourth degrees
// L4A[]      = lower augmented fourth degrees
// L5[]       = lower perfect fifth degrees
// L6M[]      = lower sixth degree (minor)
// L6[]       = lower sixth degree (major)
// L7M[]      = lower seventh degree (minor)
// -----------------------------------
// L7[]       = lower third degree (major)
// M1[]       = middle first degree ("Tonic")
// M2M[]      = middle second degree (minor)
// M2[]       = middle second degree (major)
// M3M[]      = middle third degree (minor)
// M3[]       = middle third degree (major)
// M4[]       = middle fourth degree (perfect)
// M4A[]      = middle fourth degree (augmented)
// M5[]       = middle fifth degree (perfect)
// M6M[]      = middle sixth degree (minor)
// M6[]       = middle sixth degree (major)
// -----------------------------------
// M7M[]      = middle seventh degree (minor)
// M7[]       = middle seventh degree (major)
// U1[]       = upper first degree (perfect)
// U2M[]      = upper second degree (minor)
// U2[]       = upper second degree (major)
// U3M[]      = upper third degree (minor)
// U3[]       = upper third degree (major)
// HD[]       = higher degrees
//
// EsAC files (page 61)
// BALLAD30.SM
// BALLAD80.SM
// DVA0.SM
// ERK10.SM
// TESTD.SM
// BALLAD40.SM
// ALTDEU1.SM
// FINK0.SM
// ERK20.SM
// TESTE.SM
// BALLAD50.SM
// ALTDEU2.SM
// VARIANT0.SM
// ERK30.SM
// TEST1.SM
// BALLAD10.SM
// BALLAD60.SM
// BOEHME10.SM
// ERK5.SM
// BALLAD20.SM
// KINDER0.SM
// BOEHME20.SM
// TEST0.SM
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
// Tool_esac2hum::convert -- Convert an EsAC data into Humdrum data.
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
	m_globalComments.clear();

	HumRegex hre;
	string buffer;

	// First find the next CUT[] line in the input which indcates
	// the start of a song.  There typically is a non-empty line just above CUT[]
	// containing information about the collection.
	if (m_cutline.empty()) {
		while (!infile.eof()) {
			getline(infile, buffer);

			if (hre.search(buffer, "^[!#]{2,}")) {
				hre.search(buffer, "^([!#]{2,})(.*)$");
				string prefix = hre.getMatch(1);
				string postfix = hre.getMatch(2);
				hre.replaceDestructive(prefix, "!", "#", "g");
				string comment = prefix + postfix;
				m_globalComments.push_back(comment);
				continue;
			}

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

	// Now collect lines for the song until another CUT[] is found
	// (do not store previous line above CUT[] which is the source edition label.)
	while (!infile.eof()) {
		getline(infile, buffer);

		if (hre.search(buffer, "^#{2,}")) {
			hre.search(buffer, "^(#{2,})(.*)$");
			string prefix = hre.getMatch(1);
			string postfix = hre.getMatch(2);
			hre.replaceDestructive(prefix, "!", "#", "g");
			string comment = prefix + postfix;
			m_globalComments.push_back(comment);
			continue;
		}

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
			return true;
		}

		if (hre.search(buffer, "^[A-Za-z]+\\s*\\[[^\\]]*\\s*$")) {
			// parameter with opening [
			expectingCloseQ = true;
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

	// Fix UTF-8 double encodings (related to editing with Windows-1252 or ISO-8859-2 programs):

	// Ą: c3 84 c2 84 - c4 84
	hre.replaceDestructive(buffer, "\xc4\x84", "\xc3\x84\xc2\x84", "g");

	// ą: c3 84 c2 85 - c4 85
	hre.replaceDestructive(buffer, "\xc4\x85", "\xc3\x84\xc2\x85", "g");

	// Ć: c3 84 c2 86 -> c4 86
	hre.replaceDestructive(buffer, "\xc4\x86", "\xc3\x84\xc2\x86", "g");

	// ć: c3 84 c2 87 -> c4 87
	hre.replaceDestructive(buffer, "\xc4\x87", "\xc3\x84\xc2\x87", "g");

	// Ę: c3 84 c2 98 -> c4 98
	hre.replaceDestructive(buffer, "\xc4\x98", "\xc3\x84\xc2\x98", "g");

	// ę: c3 84 c2 99 -> c4 99
	hre.replaceDestructive(buffer, "\xc4\x99", "\xc3\x84\xc2\x99", "g");

	// Ł: c4 b9 c2 81 -> c5 81
	hre.replaceDestructive(buffer, "\xc5\x81", "\xc4\xb9\xc2\x81", "g");

	// ł: c4 b9 c2 82 -> c5 82
	hre.replaceDestructive(buffer, "\xc5\x82", "\xc4\xb9\xc2\x82", "g");

	// Ń: c4 b9 c2 83 -> c5 83
	hre.replaceDestructive(buffer, "\xc5\x83", "\xc4\xb9\xc2\x83", "g");

	// ń: c4 b9 c2 84 -> c5 84
	hre.replaceDestructive(buffer, "\xc5\x84", "\xc4\xb9\xc2\x84", "g");

	// Ó: c4 82 c5 93 -> c3 93 (note: not sequential with ó)
	hre.replaceDestructive(buffer, "\xc3\x93", "\xc4\x82\xc5\x93", "g");

	// ó: c4 82 c5 82 -> c3 b3 (note: not sequential with Ó)
	hre.replaceDestructive(buffer, "\xc3\xb3", "\xc4\x82\xc5\x82", "g");

	// Ś: c4 b9 c2 9a -> c5 9a
	hre.replaceDestructive(buffer, "\xc5\x9a", "\xc4\xb9\xc2\x9b", "g");

	// ś: c4 b9 c2 9b -> c5 9b
	hre.replaceDestructive(buffer, "\xc5\x9b", "\xc4\xb9\xc2\x9b", "g");

	// Ź: c4 b9 c5 9a -> c5 b9
	hre.replaceDestructive(buffer, "\xc5\xb9", "\xc4\xb9\xc5\x9a", "g");

	// ź: c4 b9 c5 9f -> c5 ba
	hre.replaceDestructive(buffer, "\xc5\xba", "\xc4\xb9\xc5\x9f", "g");

	// Ż: c4 b9 c5 a5 -> c5 bb
	hre.replaceDestructive(buffer, "\xc5\xbb", "\xc4\xb9\xc5\xa5", "g");

	// ż:  c4 b9 c5 ba -> c5 bc
	hre.replaceDestructive(buffer, "\xc5\xbc", "\xc4\xb9\xc5\xba", "g");


	// Random leftover characters from some character conversion:
	hre.replaceDestructive(buffer, "", "[\x88\x98]", "g");

	// Remove MS-DOS newline character at ends of lines:
	if (!buffer.empty()) {
		if (buffer.back() == 0x0d) {
			// windows newline piece
			buffer.resize(buffer.size() - 1);
		}
	}
	// In VHV, when saving content to the local computer in EsAC mode, the 0x0d character should be added back.
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
				} else if (measure.m_barnum == -1) {
					output << "-"; // "non-controlling" barline.
				} else {
					// visible barline, but not assigned a measure
					// number (probably need more analysis to assign
					// a measure number to this barline).
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
			}
			vector<string>& errors = measure.m_errors;
			if (!errors.empty()) {
				for (int z=0; z<(int)errors.size(); z++) {
					output << "!!" << errors.at(z) << endl;
				}
			}

			// print time signature change
			if (!measure.m_measureTimeSignature.empty()) {
				output << measure.m_measureTimeSignature << endl;
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
	ts = trimSpaces(ts);
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
	} else if (hre.search(ts, "^(\\d+/\\d+(?:\\s+|$)){2,}$")) {
		prepareMultipleTimeSignatures(ts);
	}

	// Complicated case where the time signature changes
	vector<string> timesigs;
	hre.split(timesigs, ts, "\\s+");
	if (timesigs.size() < 2) {
		string error = "ERROR: Cannot find time signature(s) in KEY[] field: ";
		error += m_params["KEY"];
		m_errors.push_back(error);
		return;
	}

/* ggg
	vector<double> bticks(timesigs.size(), 0);
	for (int i=0; i<(int)bticks
*/


}


//////////////////////////////
//
// Tool_esac2hum::Score::prepareMultipleTimeSignatures --
//    N.B.: Will have problems when the duration of time siganture
//    in a list are the same such as "4/4 2/2".
//

void Tool_esac2hum::Score::prepareMultipleTimeSignatures(const string& ts) {
	vector<string> tss;
	HumRegex hre;
	string timesigs = ts;
	hre.split(tss, timesigs, "\\s+");
	if (tss.size() < 2) {
		cerr << "Time sigs: " << ts << " needs to have at least two time signatures" << endl;
	}

	// Calculate tick duration of time signature in list:
	vector<double> tsticks(tss.size(), 0);
	for (int i=0; i<(int)tss.size(); i++) {
		if (!hre.search(tss[i], "^(\\d+)/(\\d+)$")) {
			continue;
		}
		int top = hre.getMatchInt(1);
		int bot = hre.getMatchInt(2);
		double ticks = top * m_minrhy / bot;
		tsticks[i] = ticks;
	}

	//cerr << "\nMultiple time signatures in melody: " << endl;
	//for (int i=0; i<(int)tss.size(); i++) {
	//	cerr << "(" << i+1 << "): " << tss[i] << "\tticks:" << tsticks[i] << endl;
	//}
	//cerr << endl;

	// First assign a time signature to every inner measure in a phrase, which
	// is presumed to be a complete measure:
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		for (int j=1; j<(int)phrase.size()-1; j++) {
			Tool_esac2hum::Measure& measure = phrase.at(j);
			for (int k=0; k<(int)tss.size(); k++) {
				if (tsticks[k] == measure.m_ticks) {
					measure.m_measureTimeSignature = "*M" + tss[k];
					measure.setComplete();
				}
			}
		}
	}

	// Now check if the measure at the end and beginning
	// of the next phrase are both complete.  If not then
	// calculate partial measure pairs.
	for (int i=0; i<(int)size()-1; i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		Tool_esac2hum::Phrase& nextphrase = at(i+1);
		if (phrase.size() < 2) {
			// deal with phrases with a single measure later
			continue;
		}
		if (nextphrase.size() < 2) {
			// deal with phrases with a single measure later
			continue;
		}
		Tool_esac2hum::Measure& measure = phrase.back();
		Tool_esac2hum::Measure& nextmeasure = nextphrase.at(0);

		int mticks  = measure.m_ticks;
		int nmticks = nextmeasure.m_ticks;

		int found1 = -1;
		int found2 = -1;

		for (int j=(int)tss.size() - 1; j>=0; j--) {
			if (tsticks.at(j) == mticks) {
				found1 = j;
			}
			if (tsticks.at(j) == nmticks) {
				found2 = j;
			}
		}
		if ((found1 >= 0) && (found2 >= 0)) {
			// The two measures are complete
			measure.m_measureTimeSignature = "*M" + tss[found1];
			nextmeasure.m_measureTimeSignature = "*M" + tss[found2];
			measure.setComplete();
			nextmeasure.setComplete();
		} else {
			// See if the sum of the two measures match
			// a listed time signature.  if so, then they
			// form two partial measures.
			int ticksum = mticks + nmticks;
			for (int z=0; z<(int)tsticks.size(); z++) {
				if (tsticks.at(z) == ticksum) {
					nextmeasure.m_barnum = -1;
					measure.m_measureTimeSignature = "*M" + tss.at(z);
					nextmeasure.m_measureTimeSignature = "*M" + tss.at(z);
					measure.setPartialBegin();
					nextmeasure.setPartialEnd();
				}
			}
		}
	}

	// Check if the first measure is a complete time signature in duration.
	// If not then mark as pickup measure.  If incomplete and last measure
	// is incomplete, then merge into a single measure (partial start for
	// last measure and partial end for first measure.
	if (empty()) {
		// no data
	} else if ((size() == 1) && (at(0).size() <= 1)) {
		// single measure in melody
	} else {
		Tool_esac2hum::Measure& firstmeasure = at(0).at(0);
		Tool_esac2hum::Measure& lastmeasure  = back().back();

		double firstticks = firstmeasure.m_ticks;
		double lastticks = lastmeasure.m_ticks;

		int foundfirst = -1;
		int foundlast  = -1;

		for (int i=(int)tss.size() - 1; i>=0; i--) {
			if (tsticks.at(i) == firstticks) {
				foundfirst = i;
			}
			if (tsticks.at(i) == lastticks) {
				foundlast = i;
			}
		}

		if ((foundfirst >= 0) && (foundlast >= 0)) {
			// first and last measures are both complete
			firstmeasure.m_measureTimeSignature = "*M" + tss.at(foundfirst);
			lastmeasure.m_measureTimeSignature = "*M" + tss.at(foundlast);
			firstmeasure.setComplete();
			lastmeasure.setComplete();
		} else {
			// if both sum to a time signature than assigned that time signature to both
			double sumticks = firstticks + lastticks;
			int sumfound = -1;
			for (int i=0; i<(int)tsticks.size(); i++) {
				if (tsticks[i] == sumticks) {
					sumfound = i;
					break;
				}
			}
			if (sumfound >= 0) {
				// First and last meatures match a time signture, so
				// use that time signture for both, mark firt measure
				// last pickup (barnum -> 0), and mark last as partial
				// measure start
				firstmeasure.m_measureTimeSignature = "*M" + tss.at(sumfound);
				lastmeasure.m_measureTimeSignature = "*M" + tss.at(sumfound);
				firstmeasure.m_barnum = 0;
				firstmeasure.setPartialEnd();
				lastmeasure.setPartialBegin();
			} else if ((foundfirst >= 0) && (foundlast < 0)) {
				firstmeasure.setComplete();
				lastmeasure.setPartialBegin();
			} else if ((foundfirst < 0) && (foundlast >= 0)) {
				firstmeasure.setPartialEnd();
				lastmeasure.setComplete();
			}
		}
	}


	// Now assign bar numbers
	// First probalby check for pairs of uncategorized measure durations (deal with that later).
	vector<Tool_esac2hum::Measure*> measurelist;
	getMeasureList(measurelist);
	int barnum = 1;
	for (int i=0; i<(int)measurelist.size(); i++) {
		if ((i == 0) && measurelist.at(i)->isPartialEnd()) {
			measurelist.at(i)->m_barnum = 0;
			continue;
		}
		if (measurelist.at(i)->isComplete()) {
			measurelist.at(i)->m_barnum = barnum++;
		} else if (measurelist.at(i)->isPartialBegin()) {
			measurelist.at(i)->m_barnum = barnum++;
		} else if (measurelist.at(i)->isPartialEnd()) {
			measurelist.at(i)->m_barnum = -1;
		} else {
			measurelist.at(i)->m_errors.push_back("UNCATEGORIZED MEASURE");
		}
	}

	// Now remove duplicate time signatures
	string current = "";
	for (int i=0; i<(int)measurelist.size(); i++) {
		if (measurelist.at(i)->m_measureTimeSignature == current) {
			measurelist.at(i)->m_measureTimeSignature = "";
		} else {
			current = measurelist.at(i)->m_measureTimeSignature;
		}
	}

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
			m_keysignature = "*k[f#c#g#]";
		} else if (m_keydesignation == "*c#:") {
			m_keysignature = "*k[f#c#g#d#]";
		} else if (m_keydesignation == "*g#:") {
			m_keysignature = "*k[f#c#g#d#a#]";
		} else if (m_keydesignation == "*d#:") {
			m_keysignature = "*k[f#c#g#d#a#e#]";
		} else if (m_keydesignation == "*a#:") {
			m_keysignature = "*k[f#c#g#d#a#e#b#]";
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
			m_keysignature = "*k[b-e-a-d-g-c-f-]";
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
		string error = "Error: cannot find tonic pitch in KEY[] field: ";
		error += m_params["KEY"];
		m_errors.push_back(error);
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
		} else if (measure[i] == '^') {  // tie placeholder for degree digit
			marker = true;
		} else if (measure[i] == '(') {  // tuplet start
			marker = true;
		} else if (measure[i] == '-') {  // octave lower
			marker = true;
		} else if (measure[i] == '+') {  // octave higher
			marker = true;
		}

		if (marker && !tokens.empty() && !tokens.back().empty()) {
			char checkChar = tokens.back().back();
			if (checkChar == '(') {
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

	string source = m_score.m_params["_source"];
	output << "!!!source:";
	if (!source.empty()) {
		output << " " << source;
	}
	output << endl;

	string id = m_score.m_params["_id"];
	output << "!!!id:";
	if (!id.empty()) {
		output << " " << id;
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
	string source = m_score.m_params["_source"];
	string prefix;
	string sig = m_score.m_params["SIG"];
	string title = m_score.m_params["_title"];
	string id  = m_score.m_params["_id"];
	if (sig.empty()) {
		sig = id;
	}

	HumRegex hre;
	// Should not be spaces, but just in case;
	hre.replaceDestructive(sig, "", "\\s+", "g");
	hre.replaceDestructive(source, "", "\\s+", "g");

	if (!m_filePrefix.empty()) {
		prefix = m_filePrefix;
		source = "";
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
	} else if (!source.empty()) {
		if (hre.search(source, "^DWOK(\\d+)$")) {
			string volume = hre.getMatch(1);
			if (volume.size() == 1) {
				volume = "0" + volume;
			}
			if (!sig.empty()) {
				if (hre.search(sig, "^(\\d\\d)")) {
					string volume2 = hre.getMatch(1);
					if (volume == volume2) {
						source = "DWOK";
						output += source;
					}
				} else {
					output += source + "-";
				}
			} else {
				output += source + "-";
			}
		} else {
			output += source + "-";
		}
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
		} else if (hre.search(infile[i], "^#")) {
			// Do nothing: an external comment, or embedded filter processed
			// when filter loading the file.
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
	if (hre.search(key, "^\\s*([^\\s]+)\\s+(\\d+)\\s+([A-Gacdefg][b#]*)\\s+(.*?)\\s*$")) {
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

	string trd = m_score.m_params["TRD"];
	if (hre.search(trd, "^\\s*(.*)\\ss\\.")) {
		m_score.m_params["_source_trd"] = hre.getMatch(1);
	}
	if (hre.search(trd, "\\bs\\.\\s*(\\d+)\\s*-\\s*(\\d+)?")) {
		m_score.m_params["_page"] = hre.getMatch(1) + "-" + hre.getMatch(2);
	} else if (hre.search(trd, "\\bs\\.\\s*(\\d+)")) {
		m_score.m_params["_page"] = hre.getMatch(1);
	} else {
		cerr << "CANNOT FIND PAGE NUMBER IN " << trd << endl;
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
	HumRegex hre;
	hre.replaceDestructive(bem, " ", "\n", "g");
	output << "!!!ONB: " << bem << endl;
}



//////////////////////////////
//
// Tool_esac2hum::printFooter --
//

void Tool_esac2hum::printFooter(ostream& output, vector<string>& infile) {
	output << "*-" << endl;

	printBemComment(output);
	printPdfLinks(output);
	printPageNumbers(output);
	printConversionDate(output);


	if (m_embedEsacQ) {
		output << "!!@@BEGIN: ESAC" << endl;
		output << "!!@CONTENTS:" << endl;;
		for (int i=0; i<(int)infile.size(); i++) {
			output << "!!" << infile[i] << endl;
		}
		if (m_analysisQ) {
			embedAnalyses(output);
		}
		output << "!!@@END: ESAC" << endl;
	}

	if (!m_globalComments.empty()) {
		for (int i=0; i<(int)m_globalComments.size(); i++) {
			output << m_globalComments.at(i) << endl;
		}
	}
}



///////////////////////////////
//
// Tool_esac2hum::printPageNumbers --
//

void Tool_esac2hum::printPageNumbers(ostream& output) {
	HumRegex hre;
	string trd = m_score.m_params["TRD"];
	if (hre.search(trd, "\\bs\\.\\s*(\\d+)\\s*-\\s*(\\d+)", "im")) {
		output << "!!!page: " << hre.getMatch(1) << "-" << hre.getMatch(2) << endl;
	} else if (hre.search(trd, "\\bs\\.\\s*(\\d+)", "im")) {
		output << "!!!page: " << hre.getMatch(1) << endl;
	}
}



///////////////////////////////
//
// Tool_esac::embedAnalyses --
//

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
	if (m_dwokQ) {
		output << "!!!URL: https://kolberg.ispan.pl/webesac Kolberg WebEsAC" << endl;
	} else {
		output << "!!!URL: http://webesac.pcss.pl WebEsAC" << endl;
	}

	if (!m_dwokQ) {
		return;
	}

	output << "!!!URL: https::kolberg.ispan.pl/dwok/tomy Oskar Kolberg: Complete Works digital edition" << endl;

	printKolbergPdfUrl(output);

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
//     of each (complete) meausre, or partial measure start.
//     the scale degress for each phrase are placed into a word
//     without spaces, and then a space between each phrase.
//
//     Todo: Deal with tied notes at starts of measures.
//

void Tool_esac2hum::Score::analyzeACC(void) {
	string output;
	for (int i=0; i<(int)size(); i++) {
		Tool_esac2hum::Phrase& phrase = at(i);
		for (int j=0; j<(int)phrase.size(); j++) {
			Tool_esac2hum::Measure& measure = phrase.at(j);
			if (measure.isComplete()) {
				output += measure.at(0).getScaleDegree();
			}
		}
		if (i < (int)size() -1) {
			output += " ";
		}
	}
	m_params["ACC"] =  output;
}



//////////////////////////////
//
// Tool_esac2hum::getKolbergInfo --
//

Tool_esac2hum::KolbergInfo Tool_esac2hum::getKolbergInfo(int volume) {

	if (!m_initialized) {
		m_initialized = true;
		// Parameters:          Polish volume title,                                                          English translation,                                    print start page, Equivalent start scan (pdf page), Plate scan page vector
		m_kinfo.emplace( 1,     KolbergInfo("Pieśni ludu polskego",                                          "Polish folk songs",                                                    3,  99, {149, 150, 167, 168, 233, 234, 251, 252, 317, 318, 335, 336, 401, 402, 419, 420, 485, 486, 503, 504}));
		m_kinfo.emplace( 2,     KolbergInfo("Sandomierskie",                                                 "Sandomierz",                                                          23,  34, {}));
		m_kinfo.emplace( 3,     KolbergInfo("Kujawy I",                                                      "Kuyavia I (north central Poland)",                                   209, 221, {}));
		m_kinfo.emplace( 4,     KolbergInfo("Kujawy II",                                                     "Kuyavia II (north central Poland)",                                   69,  83, {}));
		m_kinfo.emplace( 5,     KolbergInfo("Krakowskie I",                                                  "Crakow I",                                                           194, 222, {}));
		m_kinfo.emplace( 6,     KolbergInfo("Krakowskie II",                                                 "Crakow II",                                                            5,  29, {49, 50}));
		//               7:     Krakowskie III/Crakow III: no music
		m_kinfo.emplace( 8,     KolbergInfo("Krakowskie IV",                                                 "Crakow IV",                                                          162, 182, {}));
		m_kinfo.emplace( 9,     KolbergInfo("W. Ks. Poznańskie I",                                           "Grand Duchy of Poznań I",                                            117, 141, {}));
		m_kinfo.emplace(10,     KolbergInfo("W. Ks. Poznańskie II",                                          "Grand Duchy of Poznań II",                                            60,  76, {}));
		m_kinfo.emplace(11,     KolbergInfo("W. Ks. Poznańskie III",                                         "Grand Duchy of Poznań III",                                           39,  57, {}));
		m_kinfo.emplace(12,     KolbergInfo("W. Ks. Poznańskie IV",                                          "Grand Duchy of Poznań IV",                                             3,  19, {}));
		m_kinfo.emplace(13,     KolbergInfo("W. Ks. Poznańskie V",                                           "Grand Duchy of Poznań V",                                              3,  27, {}));
		m_kinfo.emplace(14,     KolbergInfo("W. Ks. Poznańskie VI",                                          "Grand Duchy of Poznań VI",                                           157, 165, {}));
		m_kinfo.emplace(15,     KolbergInfo("W. Ks. Poznańskie VII",                                         "Grand Duchy of Poznań VII",                                          317, 327, {}));
		m_kinfo.emplace(16,     KolbergInfo("Lubelskie I",                                                   "Lublin Voivodeship I",                                               105, 125, {}));
		m_kinfo.emplace(17,     KolbergInfo("Lubelskie II",                                                  "Lublin Voivodeship II",                                                1,  23, {}));
		m_kinfo.emplace(18,     KolbergInfo("Kieleckie I",                                                   "Kielce Voivodeship I",                                                49,  65, {}));
		m_kinfo.emplace(19,     KolbergInfo("Kieleckie II",                                                  "Kielce Voivodeship II",                                                1,  15, {}));
		m_kinfo.emplace(20,     KolbergInfo("Radomskie I",                                                   "Radom Voivodeship I",                                                 75,  95, {}));
		m_kinfo.emplace(21,     KolbergInfo("Radomskie II",                                                  "Radom Voivodeship II",                                                 1,  19, {}));
		m_kinfo.emplace(22,     KolbergInfo("Łęczyckie",                                                     "Łęczyca Voivodeship",                                                 18,  36, {}));
		m_kinfo.emplace(23,     KolbergInfo("Kaliskie",                                                      "Kalisz Region",                                                       54,  68, {}));
		m_kinfo.emplace(24,     KolbergInfo("Mazowsze I",                                                    "Mazovia I",                                                           79, 103, {}));
		m_kinfo.emplace(25,     KolbergInfo("Mazowsze II",                                                   "Mazovia II",                                                           1,  26, {}));
		//              26:     Mazowsze III/Mazovia III: no music
		m_kinfo.emplace(27,     KolbergInfo("Mazowsze IV",                                                   "Mazovia IV",                                                         115, 134, {}));
		m_kinfo.emplace(28,     KolbergInfo("Mazowsze V",                                                    "Mazovia V",                                                           64,  83, {}));
		m_kinfo.emplace(29,     KolbergInfo("Pokucie I",                                                     "Pokuttia I",                                                          90, 122, {}));
		m_kinfo.emplace(30,     KolbergInfo("Pokucie II",                                                    "Pokuttia II",                                                          1,  14, {}));
		m_kinfo.emplace(31,     KolbergInfo("Pokucie III",                                                   "Pokuttia III",                                                        10,  31, {}));
		//              32:     Pokucie IV/Pokuttia IV: no music
		m_kinfo.emplace(33,     KolbergInfo("Chełmskie I",                                                   "Chełm Voivodeship I",                                                114, 150, {163, 164, 175, 176, 177, 178}));
		m_kinfo.emplace(34,     KolbergInfo("Chełmskie II",                                                  "Chełm Voivodeship II",                                                 4,  21, {}));
		m_kinfo.emplace(35,     KolbergInfo("Przemyskie",                                                    "Przemyśl Voivodeship",                                                11,  47, {93, 94, 115, 116, 167, 168}));
		//              36:     Wołyń/Volhynia: complications
		//              37:     Miscellanea I/Miscellanea I: no music
		//              38,     Miscellanea II/Miscellanea II: no music
		m_kinfo.emplace(39,     KolbergInfo("Pomorze",                                                       "Pomerania",                                                           67, 115, {129, 130, 147, 148}));
		m_kinfo.emplace(40,     KolbergInfo("Mazury Pruskie",                                                "Prussian Masuria",                                                    96, 155, {356, 357, 358, 359, 464, 465, 482, 483}));

		m_kinfo.emplace(41,     KolbergInfo("Mazowsze VI",                                                   "Mazovia VI",                                                          20, 95,  {108, 109, 126, 127, 288, 289, 306, 307, 388, 389, 406, 407}));
		m_kinfo.emplace(42,     KolbergInfo("Mazowsze VII",                                                  "Mazovia VII",                                                          6,  15, {}));
		m_kinfo.emplace(43,     KolbergInfo("Śląsk",                                                         "Silesia",                                                             21,  62, {74, 75}));
		m_kinfo.emplace(44,     KolbergInfo("Góry i Podgórze I",                                             "Mountains and Foothills I",                                           64, 110, {111, 112, 129, 130, 195, 196, 213, 214, 343, 344, 361, 362}));
		m_kinfo.emplace(45,     KolbergInfo("Góry i Podgórze II",                                            "Mountains and Foothills II",                                           1,  11, {91, 92, 109, 110, 335, 336, 353, 354, 499, 500}));
		m_kinfo.emplace(46,     KolbergInfo("Kaliskie i Sieradzkie",                                         "Kalisz Region and Sieradz Voivodeship",                                3,  29, {43, 44, 61, 62, 175, 176, 193, 194}));
		m_kinfo.emplace(47,     KolbergInfo("Podole",                                                        "Podolia",                                                             59, 105, {151, 152, 153, 154, 155, 156, 157, 158}));
		m_kinfo.emplace(48,     KolbergInfo("Tarnowskie-Rzeszowskie",                                        "Tarnów-Rzeszów Voivodeship",                                          65, 103, {119, 120, 233, 234, 251, 252}));
		m_kinfo.emplace(49,     KolbergInfo("Sanockie-Krośnieńskie I",                                       "Sanok-Krosno Voivodeship I",                                         109, 185, {189, 190, 239, 240, 257, 258, 387, 388, 405, 406, 455, 456, 473, 474}));
		m_kinfo.emplace(50,     KolbergInfo("Sanockie-Krośnieńskie II",                                      "Sanok-Krosno Voivodeship II",                                          1,  14, {30, 31, 48, 49, 114, 115, 132, 133, 198, 199, 216 ,217, 282, 283, 300, 301, 366, 367, 384, 385}));
		//              51:     Sanockie-Krośnieńskie III/Sanok-Krosno Voivodeship III: no music
		m_kinfo.emplace(52,     KolbergInfo("Białoruś-Polesie",                                              "Belarus-Polesia",                                                    116, 169, {182, 183, 200, 201, 266, 267, 284, 285, 382, 383, 400, 401, 530, 531, 548, 549}));
		m_kinfo.emplace(53,     KolbergInfo("Litwa",                                                         "Lithuania",                                                          142, 176, {195, 196, 325, 326, 359, 360, 441, 442, 459, 460}));
		m_kinfo.emplace(54,     KolbergInfo("Ruś karpacka I",                                                "Carpathian Ruthenia I",                                              267, 365, {371, 372}));
		m_kinfo.emplace(55,     KolbergInfo("Ruś karpacka II",                                               "Carpathian Ruthenia II",                                              22,  37, {48, 49, 146, 147, 164, 165, 214, 215, 232, 233, 426, 427, 444, 445}));
		m_kinfo.emplace(56,     KolbergInfo("Ruś czerwona I",                                                "Red Ruthenia I",                                                      61, 157, {173, 174, 191, 192, 209, 210, 259, 260, 27, 278, 311, 312, 329, 330, 443, 444, 461, 462}));
		//              57:     Ruś czerwona II/Red Ruthenia II, 14, 19, {70, 71, 88, 89}: complications
		//              58:     Materiały do etnografii Słowian wschodnich/Materials for the ethnography of the Eastern Slavs
		//              59/I,   Materiały do etnografii Słowian zachodnich i południowych. Cz. I Łużyce/Materials for the ethnography of western and southern Slavs. Part I Lusatia
		//              59/II,  Materiały do etnografii Słowian zachodnich i południowych. Cz. II Czechy, Słowacja/Materials for the ethnography of western and southern Slavs. Part II Czech Republic, Slovakia
		//              59/III, Materiały do etnografii Słowian zachodnich i południowych. Cz. III Słowiańszczyzna południowa/Materials for the ethnography of western and southern Slavs. Part III Southern Slavs
		//              60:     Przysłowia/Proverbs: no music
		//              61,     Pisma muzyczne, cz. I/Musical writings, part I: no music
		//              62,     Pisma muzyczne, cz. II/"Musical writings, part II, 25, 33: not in EsAC
		//              63,     Studia, rozprawy i artykuły/Studies, dissertations and articles", 55, 113: not in EsAC
		m_kinfo.emplace(64,     KolbergInfo("Korespondencja Oskara Kolberga, cz. I (1837-1876)",             "Correspondence of Oskar Kolberg, Part I (1837-1876)",                216, 261, {}));
		//              65:     Korespondencja Oskara Kolberga, cz. II (1877-1882)/Correspondence of Oskar Kolberg, Part II (1877-1882): no music
		//              66:     Korespondencja Oskara Kolberga, cz. III (1883-1890)/Correspondence of Oskar Kolberg, Part III (1883-1890): no music
		///             67:     Pieśni i melodie ludowe w opracowaniu fortepianowym, cz. I-II/Folk songs and melodies in piano arrangement, part I-II, 3, 22, not in EsAC
		//              68:     Kompozycje wokalno-instrumentalne/Vocal and instrumental compositions, 3, 49, not in EsAC
		//              69:     Kompozycje fortepianowe/Piano compositions: 3, 39, not in EsAC
		//              70:     Pieśni ludu polskiego. Supl. do t.1/Polish folk songs: Supplement to Volume 1: no music
		m_kinfo.emplace(71,     KolbergInfo("Sandomierskie. Suplement do t. 2 Dzieła Wszystkie",             "Sandomierz Voivodeship. Supplement to vol. 2 of The Complete Works",   3,  40, {116, 117, 134, 135}));
		m_kinfo.emplace(72.1,   KolbergInfo("Kujawy. Suplement do t. 3 i 4, cz. I, Kuyavia",                 "Supplement to vol. 3 and 4, part I",                                   3,  30, {292, 293, 310, 311}));
		//              72.2,   Kujawy. Suplement do t. 3 i 4, cz. II, Kuyavia/Supplement to vol. 3 and 4, part II,: missing information about pages?
		m_kinfo.emplace(73,     KolbergInfo("Krakowsike. Suplement do T. 5-8",                               "Cracow: Supplement to Volumes 5-8",                                   39, 163, {297, 298, 407, 408}));
		//              74:     Wielkie Księstwo Poznańskie. Suplement do t. 9-15/The Grand Duchy of Poznan. Supplement to vol. 9-15: no music
		m_kinfo.emplace(75,     KolbergInfo("Lubelskie. Supplement do tomów 16-17",                          "Lublin: Supplement to volumes 16-17",                                  4,  60, {281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324}));
		m_kinfo.emplace(76,     KolbergInfo("Kieleckie. Supplement do T. 18-19",                             "Kielce: Supplement to Volumes 18-19",                                  5,  41, {}));
		//              77:     Radomskie. Suplement do t. 20 i 21. I/Radomskie: Supplement to Volumes 20-21. I: complications
		m_kinfo.emplace(78,     KolbergInfo("Łęczyckie. Suplement do t. 22",                                 "Łęczyca Voivodeship: Supplement to Volume 22",                         3,   1, {}));
		m_kinfo.emplace(79,     KolbergInfo("Kaliskie. Suplement do t. 23",                                  "Kalisz Region. Supplement to vol. 23",                                 3,  38, {}));
		m_kinfo.emplace(80,     KolbergInfo("Mazowsze. Suplement do t. 24-28, cz. I",                        "Mazovia. Supplement to vol. 24-28, part I",                            7,  89, {}));
		m_kinfo.emplace(81,     KolbergInfo("Pokucie. Suplement do tomów 29-32",                             "Corrections: Supplement to Volumes 29-32",                             3,  73, {121, 122, 139, 140}));
		m_kinfo.emplace(82,     KolbergInfo("Chełmskie. Suplement do T. 33 i 34",                            "Chełm supplement to Volumes 33 and 34",                                7, 105, {}));
		m_kinfo.emplace(83,     KolbergInfo("Przemyskie. Suplement do tomu 35 DWOK",                         "Przemyśl Voivodeship: Supplement to Volume 35 DWOK",                   9, 112, {380, 381, 382, 383, 384, 385, 386, 387}));
		m_kinfo.emplace(84,     KolbergInfo("Wołyń. Suplement do t. 36., Volhynia",                          "Supplement to Volume 36",                                             35,  97, {313, 314, 315, 316, 317, 318, 319, 320}));


	}

	auto it = m_kinfo.find(volume);
	if (it != m_kinfo.end()) {
		return it->second;
	} else {
		return KolbergInfo();
	}
}



//////////////////////////////
//
// Tool_esac::getKolbergUrl --
//

string Tool_esac2hum::getKolbergUrl(int volume) {
	if ((volume < 1) || (volume > 84)) {
		// Such a volume does not exist, return empty string.
		return "";
	}

	stringstream ss;
	ss << std::setw(3) << std::setfill('0') << volume;
	// not https://
	string url = "http://www.oskarkolberg.pl/MediaFiles/";
	url += ss.str();
	url += "dwok.pdf";

	KolbergInfo kinfo = getKolbergInfo(volume);
	if (kinfo.titlePL.empty()) {
		// Do not have the page number info for volume, so just give URL for the volume.
		return url;
	}

	string pageinfo = m_score.m_params["_page"];
	int printPage = 0;
	if (!pageinfo.empty()) {
		HumRegex hre;
		if (hre.search(pageinfo, "(\\d+)")) {
			printPage = hre.getMatchInt(1);
		} else {
		     cerr << "XX PRINT PAGE: " << printPage << "\t_page: " << pageinfo << endl;
		}
	} else {
		cerr << "YY PRINT PAGE: " << printPage << "\t_page: IS EMPTY: " << pageinfo << endl;
	}

	// Calculate the scan page that matches with the print page:
	int startPrintPage = kinfo.firstPrintPage;
	int startScanPage  = kinfo.firstScanPage;
	int scanPage = calculateScanPage(printPage, startPrintPage, startScanPage, kinfo.plates);

	url += "#page=" + to_string(scanPage);

	if (!kinfo.titlePL.empty()) {
		url += " @PL{Oskar Kolberg Dzieła Wszystkie " + to_string(volume) + ": " + kinfo.titlePL;
		url += ", s. " + pageinfo;
		url += "}";
	}

	if (!kinfo.titleEN.empty()) {
		url += " @EN{Oskar Kolberg Complete Works " + to_string(volume) + ": " + kinfo.titleEN;
		url += ", p";
		if (pageinfo.find("-") != string::npos) {
			url += "p";
		}
		url += ". " + pageinfo;
		url += "}";
	}

	if (kinfo.titlePL.empty() && kinfo.titleEN.empty()) {
		url += " @PL{Oskar Kolberg Dzieła Wszystike " + to_string(volume);
		url += " @PL{Oskar Kolberg Complete Works " + to_string(volume);
	}

	return url;
}



//////////////////////////////
//
// Tool_esac2hum::printKolbergPdfUrl --
//

void Tool_esac2hum::printKolbergPdfUrl(ostream& output) {
	string source = m_score.m_params["_source"];
	HumRegex hre;
	if (!hre.search(source, "^DWOK(\\d+)")) {
		return;
	}
	int volume = hre.getMatchInt(1);
	string url = getKolbergUrl(volume);
	if (!url.empty()) {
		output << "!!!URL-pdf: " << url << endl;
	}
}



//////////////////////////////
//
// Tool_esac2hum::calculateScanPage --
//

int Tool_esac2hum::calculateScanPage(int inputPrintPage, int printPage, int scanPage, const std::vector<int>& platePages) {
	int currentPrintPage = printPage;
	int currentScanPage = scanPage;
	size_t plateIndex = 0;

	// Iterate until we reach the input print page
	while (currentPrintPage < inputPrintPage) {
		++currentScanPage;  // Increment the scan page

		// Check if the current scan page matches the current plate page in the vector
		if (plateIndex < platePages.size() && currentScanPage == platePages[plateIndex]) {
			// Skip the plate page (increment scanPage but not printPage)
			++plateIndex;
		} else {
			// If not a plate page, increment the print page
			++currentPrintPage;
		}
	}

	return currentScanPage;
}


// END_MERGE

} // end namespace hum



