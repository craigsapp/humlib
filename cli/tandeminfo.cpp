//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 12 10:58:43 PDT 2024
// Last Modified: Mon Aug 12 10:58:48 PDT 2024
// Filename:      cli/tandeminfo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/tandeminfo.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   List tandem interpretations in input file(s).
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile            (HumdrumFile& infile);
string getMeaning             (HTp token);
string checkForKeySignature   (const string& tok);
string checkForKeyDesignation (const string& tok);
string checkForInstrumentInfo (const string& tok);
string checkForLabelInfo      (const string& tok);
string checkForTimeSignature  (const string& tok);
string checkForMeter          (const string& tok);
string checkForTempoMarking   (const string& tok);
string checkForClef           (const string& tok);
string checkForStaffPartGroup (const string& tok);
string checkForTuplet         (const string& tok);
string checkForHands          (const string& tok);
string checkForPosition       (const string& tok);
string checkForCue            (const string& tok);
string checkForFlip           (const string& tok);
string checkForTremolo        (const string& tok);
string checkForOttava         (const string& tok);
string checkForPedal          (const string& tok);
string checkForLigature       (const string& tok);
string checkForColoration     (const string& tok);
string checkForRscale         (const string& tok);
string checkForTimebase       (const string& tok);

bool exclusiveQ = true;   // used with -X option (don't print exclusive interpretation)
bool unknownQ   = false;  // used with -u option (print only unknown tandem interpretations)
bool filenameQ  = false;  // used with -f option (print only unknown tandem interpretations)
bool meaningQ   = false;  // used with -m option (print meaning of interpretation)

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("f|filename=b",                            "show filename");
	options.define("m|meaning=b",                             "give meaning of tandem interpretation");
	options.define("u|unknown-tandem-interpretations-only=b", "do not show exclusive interpretation context");
	options.define("X|no-exclusive-interpretations=b",        "do not show exclusive interpretation context");
	options.process(argc, argv);
	exclusiveQ = !options.getBoolean("no-exclusive-interpretations");
	unknownQ   = options.getBoolean("unknown-tandem-interpretations-only");
	filenameQ  = options.getBoolean("filename");
	meaningQ   = options.getBoolean("meaning");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isManipulator()) {
			continue;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (*token == "*") {
				continue;
			}
			string meaning;
			if (meaningQ) {
				meaning = getMeaning(token);
				if (unknownQ) {
					if (meaning != "unknown") {
						continue;
					}
				}
			}
			if (filenameQ) {
				cout << infile.getFilename() << "\t";
			}
			if (exclusiveQ) {
				cout << token->getDataType() << "\t";
			}
			cout << token;
			if (meaningQ) {
				cout << "\t" << meaning;
			}
			cout << endl;
		}
	}
}



//////////////////////////////
//
// getMeaning -- Return meaning of the input token; otherwise, return "unknown".
//

string getMeaning(HTp token) {
	string tok = token->substr(1);
	string meaning;

	meaning = checkForKeySignature(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForKeyDesignation(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForInstrumentInfo(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForLabelInfo(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTimeSignature(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForMeter(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTempoMarking(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForClef(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForStaffPartGroup(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTuplet(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForHands(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForPosition(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForCue(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForFlip(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTremolo(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForOttava(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForPedal(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForLigature(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForColoration(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForRscale(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTimebase(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTimebase -- Humdrum Toolkit interpretations related
//      to the timebase tool.
//

string checkForTimebase(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^tb(\\d+)$")) {
		string number = hre.getMatch(1);
		string output = "timebase: all data lines (should) have a duration of " + number;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForRscale -- Extended interpretation for adjusting the visual
//     display of note durations when they do not match the logical
//     note durations (such as show a quarter note as if it were a
//     half note, which would be indicated by "*rscale:2". Or a
//     half note as if it were a quarter note with "*rscale:1/2".
//     Also related to the rscale tool from Humdrum Extras and humlib.
//     Used in verovio.
//

string checkForRscale(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^rscale:(\\d+)(/\\d+)?$")) {
		string fraction = hre.getMatch(1) + hre.getMatch(2);
		string output = "visual rhythmic scaling factor " + fraction;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForColoration -- Extended interpretations for displaying
//     coloration lines (brackets) for mensural music.
//

string checkForColoration(const string& tok) {
	if (tok == "col") {
		return "start of coloration line";
	}
	if (tok == "Xcol") {
		return "end of coloration line";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForLigature -- Extended interpretations for displaying
//     ligature lines for mensural music.
//

string checkForLigature(const string& tok) {
	if (tok == "lig") {
		return "start of ligature line";
	}
	if (tok == "Xlig") {
		return "end of ligature line";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForPedal -- Extended interpretations for displaying
//     ottava lines in music notation.
//

string checkForPedal(const string& tok) {
	if (tok == "ped") {
		return "sustain pedal down";
	}
	if (tok == "Xped") {
		return "sustain pedal up";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForOttava -- Extended interpretations for displaying
//     ottava lines in music notation.
//

string checkForOttava(const string& tok) {
	if (tok == "8va") {
		return "start of 8va line";
	}
	if (tok == "X8va") {
		return "end of 8va line";
	}
	if (tok == "8ba") {
		return "start of 8ba (ottava basso) line";
	}
	if (tok == "X8ba") {
		return "end of 8ba (ottava basso) line";
	}
	return "unknown";
}




//////////////////////////////
//
// checkForTremolo -- Extended interpretations for collapsing
//     repeated notes into tremolos in music notation rendering.
//     Used specifically by verovio.
//

string checkForTremolo(const string& tok) {
	if (tok == "tremolo") {
		return "start of tremolo rendering of repeated notes";
	}
	if (tok == "Xtremolo") {
		return "end of tremolo rendering of repeated notes";
	}
	return "unknown";
}


//////////////////////////////
//
// checkForFlip -- Extended interpretations for use with the
//     flipper humlib command.
//

string checkForFlip(const string& tok) {
	if (tok == "flip") {
		return "switch order of subspines, specific to flipper tool";
	}
	if (tok == "Xflip") {
		return "cancel flipping of subspine, specific to flipper tool";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForCue -- Extended interpretations for visual rendering
//     *cue means display as cue-sized notes.  Probably change
//     this so that *cue means following notes are cue notes
//     and add *cuesz for cue-sized notes (that are not cues
//     from other instruments).
//

string checkForCue(const string& tok) {
	if (tok == "cue") {
		return "cue-sized notation follows";
	}
	if (tok == "Xcue") {
		return "cancel cue-sized notation";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForPosition -- Extended interpretations for visual rendering
//     data above/below staff.  Useful in particular for **dynam.
//     Staff number in part (relative to top staff) can be given
//     as a number following a colon after the placement.
//

string checkForPosition(const string& tok) {
	if (tok == "above") {
		return "place items above staff";
	}
	if (tok == "above:1") {
		return "place items above first staff of part";
	}
	if (tok == "above:2") {
		return "place items above second staff of part";
	}
	if (tok == "below") {
		return "place items below staff";
	}
	if (tok == "below:1") {
		return "place items below first staff of part";
	}
	if (tok == "below:2") {
		return "place items below second staff of part";
	}
	if (tok == "center") {
		return "centered items between two staves";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForHands -- Extended interpretations to indicate which
//     hand is playing the notes (for grand-staff keyboard in particular).
//

string checkForHands(const string& tok) {
	if (tok == "LH") {
		return "notes played by left hand";
	}
	if (tok == "RH") {
		return "notes played by right hand";
	}
	return "unknown";
}



//////////////////////////////
//
// checkForTuplet -- Extended interpretations for **kern data to control
//     visual stylings of tuplet numbers and brackets.
//

string checkForTuplet(const string& tok) {
	if (tok == "Xbrackettup") {
		return "suppress brackets for tuplets";
	}
	if (tok == "brackettup") {
		return "do not suppress brackets for tuplets (default)";
	}
	if (tok == "tuplet") {
		return "show tuplet numbers (default)";
	}
	if (tok == "Xtuplet") {
		return "do not show tuplet numbers";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForStaffPartGroup -- Humdrum Toolkit interpretation (*staff), and
//    extensions to *part to group multiple staves into a single part as
//    well as *group for grouping staves/parts into instrument class
//    groups (useful for controlling connecting barlines across multiple
//    staves).
//

string checkForStaffPartGroup (const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^staff(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "staff " + number;
			return output;
		}
		output = "staves " + tok.substr(5);
		return output;
	}

	if (hre.search(tok, "^part(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "part " + number;
			return output;
		}
		output = "parts " + tok.substr(5);
		return output;
	}

	if (hre.search(tok, "^group(\\d+)(/\\d+)*$")) {
		string number = hre.getMatch(1);
		string second = hre.getMatch(2);
		string output;
		if (second.empty()) {
			output = "group " + number;
			return output;
		}
		output = "groups " + tok.substr(5);
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForClef -- Humdrum Toolkit interpretations.  Extension
//     is "*clefX" for percussion clef (checked for below),
//     and *clefG2yy for an invisible clef (not visually rendered).
//

string checkForClef(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^clef([GFCX])(.*?)([12345])?(yy)?$")) {
		string ctype = hre.getMatch(1);
		string octave = hre.getMatch(2);
		string line = hre.getMatch(3);
		string invisible = hre.getMatch(4);
		string output = "clef: ";
		if (ctype == "X") {
			output += "percussion";
			if (!line.empty()) {
				output += ", line: " + line;
			}
			if (!octave.empty()) {
				return "unknown";
			}
		} else {
			output += ctype;
			if (line.empty()) {
				return "unknown";
			}
			output += ", line: " + line;
			if (!octave.empty()) {
				if (hre.search(octave, "^v+$")) {
					output += ", octave displacement -" + to_string(octave.size());
					
				} else if (hre.search(octave, "^\\^+$")) {
					output += ", octave displacement +" + to_string(octave.size());
				}
			}
		}
		if (!invisible.empty()) {
			output += ", invisible (not displayed in music rendering)";
		}
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTimeSignature -- Humdrum Toolkit interpretations.
//      Extended for use with rare non-notatable rhythm bases, such as
//      *M3/3%2 for three triplet whole notes to the measure (this
//      is equivalent in duration to *M2/1 but gives a more refined
//      version of what the beat is.  Maybe also allow "*M2/4." which
//      would be equivalent to an explicit compound *M6/8 time signature.
//      Other extensions could also be done such as *M4/4yy for an invisible
//      time signature.  And another extension could be *M2/8+3/8 for *M5/8
//      split into 2 + 3 beat groupings.
//

string checkForTimeSignature(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^M(\\d+)/(\\d+)(%\\d+)?$")) {
		string top = hre.getMatch(1);
		string bot = hre.getMatch(2) + hre.getMatch(3);
		string output = "time signature: top: " + top + ", bottom: " + bot;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForMeter -- Humdrum Toolkit interpretations. Extended for use
//    with mensural signs.
//

string checkForMeter(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^met\\((.*?)\\)$")) {
		string meter = hre.getMatch(1);
		if (meter == "c") {
			return "meter (common time)";
		}
		if (meter == "c\\|") {
			return "meter (cut time)";
		}
		if (meter == "") {
			return "meter (empty)";
		}
		string output = "mensuration sign: " + meter;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTempoMarking -- Humdrum Toolit interpretations.
//

string checkForTempoMarking(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^MM(\\d+)(\\.\\d*)?")) {
		string tempo = hre.getMatch(1) + hre.getMatch(2);
		string output = "tempo: " + tempo + " quarter notes per minute";
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForLabelInfo -- Humdrum Toolkit interpretations.
//     Used by the thru command.
//

string checkForLabelInfo(const string& tok) {
	HumRegex hre;
	if (!hre.search(tok, "^>")) {
		return "unknown";
	}

	if (hre.search(tok, "^>(\\[.*\\]$)")) {
		string list = hre.getMatch(1);
		string output = "default expansion list: " + list;
		return output;
	}

	if (hre.search(tok, "^>([^[\\[\\]]+)(\\[.*\\]$)")) {
		string expansionName = hre.getMatch(1);
		string list = hre.getMatch(2);
		string output = "alternate expansion list: label: " + expansionName;
		output += ", expansion list: " + list;
		return output;
	}

	if (hre.search(tok, "^>([^\\[\\]]+)$")) {
		string label = hre.getMatch(1);
		string output = "expansion label: " + label;
		return output;
	}

	return "unknown";

}



//////////////////////////////
//
// checkForInstrumentInfo -- Humdrum Toolkit and extended interpretations.
//     Humdrum Tookit:
//         instrument group
//         instrument class
//         instrument code
//     Extended:
//         instrument name
//         instrument abbreviation
//

string checkForInstrumentInfo(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^I\"(.*)")) {
		string name = hre.getMatch(1);
		string output = "printable instrument name: \"";
		output += name;
		output += "\"";
		return output;
	}

	if (hre.search(tok, "^I'(.*)")) {
		string abbr = hre.getMatch(1);
		string output = "printable instrument abbreviation \"";
		output += abbr;
		output += "\"";
		return output;
	}

	if (hre.search(tok, "^IC(.*)")) {
		string iclass = hre.getMatch(1);
		string output = "instrument class (";
		output += iclass;
		output += ")";
		return output;
	}

	if (hre.search(tok, "^IC(.*)")) {
		string group = hre.getMatch(1);
		string output = "instrument group (";
		output += group;
		output += ")";
		return output;
	}

	if (hre.search(tok, "^I([a-z].*)")) {
		string code = hre.getMatch(1);
		string output = "instrument code (";
		output += code;
		HumInstrument inst;
		inst.setHumdrum(code);
		string text = inst.getName();
		if (!text.empty()) {
			output += ": \"" + text + "\"";
		} else {
			output += ": unknown code";
		}
		output += ")";
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForKeySignature -- Standard Humdrum Toolkit interpretations.
//     Extended key signatures are possible (and detected by this function),
//     but typically the standard ones are in circle-of-fifths orderings.
//     This function also allows double sharps/flats in the key signature
//     which are very uncommon in real music.  Standard key signatures:
//    
//     *k[f#c#g#d#a#e#b#]
//     *k[c#g#d#a#e#b#]
//     *k[g#d#a#e#b#]
//     *k[d#a#e#b#]
//     *k[a#e#b#]
//     *k[e#b#]
//     *k[b#]
//     *k[]
//     *k[b-]
//     *k[b-e-]
//     *k[b-e-a-]
//     *k[b-e-a-d-]
//     *k[b-e-a-d-g-]
//     *k[b-e-a-d-g-c-]
//     *k[b-e-a-d-g-c-f-]
//

string checkForKeySignature(const string& tok) {

	// visual styling interpretations for key signatures:
	if (tok == "kcancel") {
		return "show cancellation naturals when changing key signatures";
	}
	if (tok == "Xkcancel") {
		return "do not show cancellation naturals when changing key signatures (default)";
	}

	// analytic key signature:
	if (tok == "k[]") {
		return "key signature, no sharps or flats";
	}
	HumRegex hre;
	if (hre.search(tok, "^k\\[([a-gA-G]+[n#-]{1,2})+\\]$")) {
		string output = "key signature";
		int flats = 0;
		int sharps = 0;
		int naturals = 0;
		int doubleflats = 0;
		int doublesharps = 0;
		for (int i=0; i<hre.getMatchCount(); i++) {
			string note = hre.getMatch(i+1);
			if (note.find("##") != string::npos) {
				doublesharps++;
			} else if (note.find("--") != string::npos) {
				doubleflats++;
			} else if (note.find("#") != string::npos) {
				sharps++;
			} else if (note.find("-") != string::npos) {
				flats++;
			} else if (note.find("n") != string::npos) {
				naturals++;
			}
		}

		if (sharps) {
			output += ", ";
			if (sharps == 1) {
				output += "1 sharp";
			} else {
				output += to_string(sharps) + " sharps";
			}
		}

		if (flats) {
			output += ", ";
			if (flats == 1) {
				output += "1 flat";
			} else {
				output += to_string(flats) + " flats";
			}
		}

		if (naturals) {
			output += ", ";
			if (naturals == 1) {
				output += "1 natural";
			} else {
				output += to_string(naturals) + " naturals";
			}
		}

		if (doublesharps) {
			output += ", ";
			if (doublesharps == 1) {
				output += "1 double sharp";
			} else {
				output += to_string(doublesharps) + " double sharps";
			}
		}

		if (doubleflats) {
			output += ", ";
			if (doubleflats == 1) {
				output += "1 double flat";
			} else {
				output += to_string(doubleflats) + " double flats";
			}
		}

		return output;
	}
	return "unknown";
}



//////////////////////////////
//
// checkForKeyDesignation -- Standard Humdrum Toolkit interpretations, plus
//     modal extensions by Brett Arden.  Typically only used in **kern data.
//

string checkForKeyDesignation(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^([a-gA-G])([-#]*):(ion|dor|phr|lyd|mix|aeo|loc)?$")) {
		string tonic = hre.getMatch(1);
		string accid = hre.getMatch(2);
		string mode  = hre.getMatch(3);
		bool isUpper = isupper(tonic[0]);
		string output = "key designation: ";
		if (mode.empty()) {
			output += toupper(tonic[0]);

			if (accid == "") {
				// do nothing
			} else if (accid == "#") {
				output += "-sharp";
			} else if (accid == "-") {
				output += "-flat";
			} else if (accid == "##") {
				output += "-double-sharp";
			} else if (accid == "--") {
				output += "-double-flat";
			} else if (accid == "###") {
				output += "-triple-sharp";
			} else if (accid == "---") {
				output += "-triple-flat";
			} else {
				return "unknown";
			}

			if (isUpper) {
				output += " major";
			} else {
				output += " minor";
			}
			return output;
		} else {
			// Modal key
			if (isUpper && ((mode == "dor") || (mode == "phr") || (mode == "aeo") || (mode == "lyd"))) {
				// need a lower-case letter for these modes (minor third above tonic)
				return "unknown";
			}
			if ((!isUpper) && ((mode == "ion") || (mode == "lyd") || (mode == "mix"))) {
				// need an upper-case letter for these modes (major third above tonic)
				return "unknown";
			}
			output += toupper(tonic[0]);

			if (accid == "") {
				// do nothing
			} else if (accid == "#") {
				output += "-sharp";
			} else if (accid == "-") {
				output += "-flat";
			} else if (accid == "##") {
				output += "-double-sharp";
			} else if (accid == "--") {
				output += "-double-flat";
			} else if (accid == "###") {
				output += "-triple-sharp";
			} else if (accid == "---") {
				output += "-triple-flat";
			} else {
				return "unknown";
			}

			if (mode == "ion") {
			} else if (mode == "dor") {
				output += " dorian";
			} else if (mode == "phr") {
				output += " phrygian";
			} else if (mode == "lyd") {
				output += " lydian";
			} else if (mode == "mix") {
				output += " mixolydian";
			} else if (mode == "aeo") {
				output += " aeolian";
			} else if (mode == "loc") {
				output += " locrian";
			} else {
				return "unknown";
			}
			return output;
		}
	}

	return "unknown";
}



