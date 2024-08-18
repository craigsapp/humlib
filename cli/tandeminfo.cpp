//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 12 10:58:43 PDT 2024
// Last Modified: Sun Aug 18 00:19:45 PDT 2024
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
string checkForBracket        (const string& tok);
string checkForRscale         (const string& tok);
string checkForTimebase       (const string& tok);
string checkForTransposition  (const string& tok);
string checkForGrp            (const string& tok);
string checkForStria          (const string& tok);
string checkForFont           (const string& tok);
string checkForVerseLabels    (const string& tok);
string checkForLanguage       (const string& tok);
string checkForStemInfo       (const string& tok);
string checkForXywh           (const string& tok);
string checkForCustos         (const string& tok);
string checkForTextInterps    (const string& tok);
string checkForRep            (const string& tok);
string checkForPline          (const string& tok);
string checkForTacet          (const string& tok);
string checkForFb             (const string& tok);
string checkForColor          (const string& tok);
string checkForThru           (const string& tok);

bool exclusiveQ = true;   // used with -X option (don't print exclusive interpretation)
bool unknownQ   = false;  // used with -u option (print only unknown tandem interpretations)
bool filenameQ  = false;  // used with -f option (print only unknown tandem interpretations)
bool meaningQ   = false;  // used with -m option (print meaning of interpretation)
bool locationQ  = false;  // used with -l option (print location of interpretation in file)
bool zeroQ      = false;  // used with -z option (location address by 0-index)

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("f|filename=b",                            "show filename");
	options.define("m|meaning=b",                             "give meaning of tandem interpretation");
	options.define("u|unknown-tandem-interpretations-only=b", "do not show exclusive interpretation context");
	options.define("X|no-exclusive-interpretations=b",        "do not show exclusive interpretation context");
	options.define("l|location=b",                            "show location of interpretation in file (row, column)");
	options.define("z|zero-indexed-locations=b",              "locations are 0-indexed");
	options.process(argc, argv);
	exclusiveQ = !options.getBoolean("no-exclusive-interpretations");
	unknownQ   = options.getBoolean("unknown-tandem-interpretations-only");
	filenameQ  = options.getBoolean("filename");
	meaningQ   = options.getBoolean("meaning");
	locationQ  = options.getBoolean("location");
	zeroQ      = options.getBoolean("zero-indexed-locations");

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
					HumRegex hre;
					if (!hre.search(meaning, "unknown")) {
						continue;
					}
				}
			}
			if (filenameQ) {
				cout << infile.getFilename() << "\t";
			}
			if (locationQ) {
				if (zeroQ) {
					int row = token->getLineIndex();
					int col = token->getFieldIndex();
					cout << "(" << row << ", " << col << ")" << "\t";
				} else {
					int row = token->getLineNumber();
					int col = token->getFieldNumber();
					cout << "(" << row << ", " << col << ")" << "\t";
				}
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

	meaning = checkForBracket(tok);
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

	meaning = checkForTransposition(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForGrp(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForStria(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForFont(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForVerseLabels(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForLanguage(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForStemInfo(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForXywh(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForCustos(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTextInterps(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForRep(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForPline(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForTacet(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForFb(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForColor(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	meaning = checkForThru(tok);
	if (meaning != "unknown") {
		return meaning;
	}

	HumRegex hre;
	if (hre.search(token, "\\s+$")) {
		return "unknown (space at end of interpretation may be the problem)";
	} else {
		return "unknown";
	}
}



//////////////////////////////
//
// checkForThru -- Humdrum Toolkit interpretations related to thru command.
//

string checkForThru(const string& tok) {
	if (tok == "thru") {
		return "data processed by thru command (expansion lists processed)";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForColor -- Extended interprerations for coloring notes in **kern data.
//     Used in verovio.
//

string checkForColor(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^color:(.*)")) {
		string color = hre.getMatch(1);
		string output;
		if (hre.search(tok, "^#[0-9A-Fa-f]{3}$")) {
			output = "3-digit hex ";
		} else if (hre.search(tok, "^#[0-9A-Fa-f]{6}$")) {
			output = "6-digit hex ";
		} else if (hre.search(tok, "^#[0-9A-Fa-f]{8}$")) {
			output = "8-digit hex  (RGB + transparency)";
		} else if (hre.search(tok, "^rgb(\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+)$")) {
			output = "RGB integer";
		} else if (hre.search(tok, "^rgb(\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+\\s*,[\\d.]+)$")) {
			output = "RGB integer with alpha";
		} else if (hre.search(tok, "^hsl(\\d+\\s*,\\s*\\d+%\\s*,\\s*\\d+%)$")) {
			output = "HSL";
		} else if (hre.search(tok, "^hsl(\\d+\\s*,\\s*\\d+%\\s*,\\s*\\d+%,\\s*[\\d.]+)$")) {
			output = "HSL with alpha";
		} else if (hre.search(tok, "^[a-z]+$")) {
			output = "named ";
		}
		output += " color";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForFb -- Extended interprerations especially for **fb (**fa) exclusive
//     interpretations.
//

string checkForFb(const string& tok) {
	if (tok == "reverse") {
		return "reverse order of accidental and number in figured bass";
	}
	if (tok == "Xreverse") {
		return "stop reversing order of accidental and number in figured bass";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTacet -- Extended interprerations for marking parts that are not
//     playing (rests only) in a movement/movement subsection.
//

string checkForTacet(const string& tok) {
	if (tok == "tacet") {
		return "part is tacet in movement/section";
	}
	if (tok == "Xtacet") {
		return "end of part tacet";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForRep -- Extended interprerations for poetic line analysis related to pline tool.
//

string checkForPline(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^pline:(\\d+)([abcr]*)$")) {
		string number = hre.getMatch(1);
		string info = hre.getMatch(2);
		string output = "poetic line markup: " + number + info;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForRep -- Extended interprerations for adding repeat sign shorthand for
//     repeated music.
//

string checkForRep(const string& tok) {
	if (tok == "rep") {
		return "start of repeat sign replacing notes/rests";
	}
	if (tok == "Xrep") {
		return "end of repeat sign replacing notes/rests";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTextInterps -- Extended interprerations for **text and **silbe
//

string checkForTextInterps(const string& tok) {
	if (tok == "ij") {
		return "start of text repeat region";
	}
	if (tok == "Xij") {
		return "end of text repeat region";
	}
	if (tok == "edit") {
		return "start of editorial text region";
	}
	if (tok == "Xedit") {
		return "end of editorial text region";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForCustos -- Extended interprerations for marker
//     at end of system for next note in part.
//

string checkForCustos(const string& tok) {
	HumRegex hre;

	if (tok == "custos") {
		return "custos, pitch unspecified";
	}

	if (tok == "custos:") {
		return "custos, pitch unspecified";
	}

	if (hre.search(tok, "^custos:([A-G]+|[a-g]+)(#+|-+|n)?$")) {
		// also deal with chord custos
		string pitch = hre.getMatch(1);
		string accid = hre.getMatch(2);
		string output = "custos on pitch " + pitch + accid;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForStemInfo -- Extended interprerations
//      for visual display of stems (on left or right side of notes).
//

string checkForXywh(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^xywh-([^:\\s]+):(\\d+),(\\d+),(\\d+),(\\d+)$")) {
		string page = hre.getMatch(1);
		string x = hre.getMatch(2);
		string y = hre.getMatch(3);
		string w = hre.getMatch(4);
		string h = hre.getMatch(5);
		string output = "IIIF bounding box, page=";
		output += page;
		output += ", x=" + x;
		output += ", y=" + y;
		output += ", w=" + w;
		output += ", h=" + h;
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForStemInfo -- Extended interprerations
//      for visual display of stems (on left or right side of notes).
//

string checkForStemInfo(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^(\\d+)/left$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + " notes always have stem up on the left";
		return output;
	}

	if (hre.search(tok, "^(\\d+)\\\\left$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + " notes always have stem down on the left";
		return output;
	}

	if (hre.search(tok, "^(\\d+)/right$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + " notes always have stem up on the right";
		return output;
	}

	if (hre.search(tok, "^(\\d+)\\\\right$")) {
		string rhythm = hre.getMatch(1);
		string output = rhythm + " notes always have stem down on the right";
		return output;
	}

	if (tok == "all/right") {
		string output = "all notes always have stem up on the right";
		return output;
	}

	if (tok == "all\\right") {
		string output = "all notes always have stem down on the right";
		return output;
	}

	if (tok == "all/left") {
		string output = "all notes always have stem up on the left";
		return output;
	}

	if (tok == "all\\left") {
		string output = "all notes always have stem down on the left";
		return output;
	}

	if (tok == "all/center") {
		string output = "all notes always have stem up on notehead center";
		return output;
	}

	if (tok == "all\\center") {
		string output = "all notes always have stem down on notehead center";
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForLanguage -- Humdrum Toolkit and extended interprerations
//      for langauages (for **text and **silbe).
//

string checkForLanguage(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^L([A-Z][^\\s]+)$")) {
		string language = hre.getMatch(1);
		string output = "Language, old style: " + language;
		return output;
	}

	if (hre.search(tok, "^lang:([A-Z]{2,3})$")) {
		string code = hre.getMatch(1);
		string name = Convert::getLanguageName(code);
		if (name.empty()) {
			return "language code " + code +  " (unknown)";
		}
		string output = "language code";
		if (code.size() == 2) {
			output = "ISO 639-3 two-letter language code (";
		} else if (code.size() == 3) {
			output = "ISO 639-3 three-letter language code (";
		}
		output += name;
		output += ")";
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForVerseLabels -- Extended tandem interpretations (used by verovio
//      for visual rendeing of notation).
//

string checkForVerseLabels(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^v:(.*)$")) {
		string output = "verse label \"" + hre.getMatch(1) + "\"";
		return output;
	}
	if (hre.search(tok, "^vv:(.*)$")) {
		string output = "verse label \"" + hre.getMatch(1) + "\", repeated after each system break";
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForFont -- Extended interprtations for styling **text and **silbe.
//

string checkForFont(const string& tok) {
	if (tok == "italic") {
		return "use italic font style";
	}
	if (tok == "Xitalic") {
		return "stop using italic font style";
	}
	if (tok == "bold") {
		return "use bold font style";
	}
	if (tok == "Xbold") {
		return "stop using bold font style";
	}

	return "unknown";
}



//////////////////////////////
//
// checkForStria -- Humdrum Toolkit interpretation.
//

string checkForStria(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^stria(\\d+)$")) {
		string output = "number of staff lines:" + hre.getMatch(1);
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForGrp -- Polyrhythm project interpretations for
//      polyrhythm group assignments.  Related to humlib composite tool.
//

string checkForGrp(const string& tok) {
	HumRegex hre;
	if (hre.search(tok, "^grp:([AB])$")) {
		string output = "composite rhythm grouping label " + hre.getMatch(1);
		return output;
	}

	return "unknown";
}



//////////////////////////////
//
// checkForTransposition -- Humdrum Toolkit interpretations related
//      to pitch transposition.
//

string checkForTransposition(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "ITrd(-?\\d+)c(-?\\d+)$")) {
		string diatonic = hre.getMatch(1);
		string chromatic = hre.getMatch(2);
		string output = "transposition for written part, diatonic: ";
		output += diatonic;
		output += ", chromatic: ";
		output += chromatic;
		return output;
	}

	if (hre.search(tok, "Trd(-?\\d+)c(-?\\d+)$")) {
		string diatonic = hre.getMatch(1);
		string chromatic = hre.getMatch(2);
		string output = "transposed by diatonic: ";
		output += diatonic;
		output += ", chromatic: ";
		output += chromatic;
		return output;
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
// checkForBracket -- Extended interpretations for displaying
//     various bracket lines in visual music notation.
//

string checkForBracket(const string& tok) {
	// Coloration
	if (tok == "col") {
		return "start of coloration bracket";
	}
	if (tok == "Xcol") {
		return "end of coloration bracket";
	}

	// Ligatures
	if (tok == "lig") {
		return "start of ligature bracket";
	}
	if (tok == "Xlig") {
		return "end of ligature bracket";
	}

	// Schoenberg
	if (tok == "haupt") {
		return "start of Hauptstimme bracket";
	}
	if (tok == "Xhaupt") {
		return "end of Hauptstimme bracket";
	}
	if (tok == "neben") {
		return "start of Nebenstimme bracket";
	}
	if (tok == "Xneben") {
		return "end of Nebenstimme bracket";
	}
	if (tok == "rhaupt") {
		return "start of Hauptrhythm bracket";
	}
	if (tok == "Xrhaupt") {
		return "end of Hauptrhythm bracket";
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
	if (tok == "15ma") {
		return "start of 15ma line";
	}
	if (tok == "X15ma") {
		return "end of 15ma line";
	}
	if (tok == "coll8ba") {
		return "coll ottava basso start";
	}
	if (tok == "Xcoll8ba") {
		return "coll ottava basso end";
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
	if (tok == "tupbreak") {
		return "break tuplet at this point";
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
	if (hre.search(tok, "^(m|o)?clef([GFCX])(.*?)([12345])?(yy)?$")) {
		string modori = hre.getMatch(1);
		string ctype = hre.getMatch(2);
		string octave = hre.getMatch(3);
		string line = hre.getMatch(4);
		string invisible = hre.getMatch(5);
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
	if (tok == "MX") {
		return "unmeasured music time signature";
	}
	if (hre.search(tok, "^MX/(\\d+)(%\\d+)?(yy)?")) {
		string output = "unmeasured music with beat " + hre.getMatch(1) + hre.getMatch(2);
		if (hre.getMatch(3) == "yy") {
			output += ", invisible";
			return output;
		}
	}
	if (hre.search(tok, "^M(\\d+)/(\\d+)(%\\d+)?(yy)?$")) {
		string top = hre.getMatch(1);
		string bot = hre.getMatch(2) + hre.getMatch(3);
		string invisible = hre.getMatch(4);
		string output = "time signature: top: " + top + ", bottom: " + bot;
		if (invisible == "yy") {
			output += ", invisible";
		}
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
	if (hre.search(tok, "^(m|o)?met\\((.*?)\\)$")) {
		string modori = hre.getMatch(1);
		string meter = hre.getMatch(2);
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
	if (hre.search(tok, "^MM(\\d+)(\\.\\d*)?$")) {
		string tempo = hre.getMatch(1) + hre.getMatch(2);
		string output = "tempo: " + tempo + " quarter notes per minute";
		return output;
	}

	if (hre.search(tok, "^MM\\[(.*?)\\]$")) {
		string text = hre.getMatch(1);
		string output = "text-based tempo: " + text;
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
//         instrument group	 *IG
//         instrument class	*IC
//         instrument code	*I
//     Extended:
//         instrument name *I"
//         instrument number *I#
//         instrument abbreviation *I'
//
//     modori tool extensions:
//         *mI == modernized
//         *oI == original
//

string checkForInstrumentInfo(const string& tok) {
	HumRegex hre;

	if (hre.search(tok, "^(m|o)?I\"(.*)$")) {
		string modori = hre.getMatch(1);
		string name = hre.getMatch(2);
		string output = "printable instrument name: \"";
		output += name;
		output += "\"";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I'(.*)$")) {
		string modori = hre.getMatch(1);
		string abbr = hre.getMatch(2);
		string output = "printable instrument abbreviation \"";
		output += abbr;
		output += "\"";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?IC([^\\s]*)$")) {
		string modori = hre.getMatch(1);
		string iclass = hre.getMatch(2);
		string output = "instrument class (";
		output += iclass;
		output += ")";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?IG([^\\s]*)$")) {
		string modori = hre.getMatch(1);
		string group = hre.getMatch(2);
		string output = "instrument group (";
		output += group;
		output += ")";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I#(\\d+)$")) {
		string modori = hre.getMatch(1);
		string number = hre.getMatch(2);
		string output = "instrument number (";
		output += number;
		output += ")";
		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}
		return output;
	}

	if (hre.search(tok, "^(m|o)?I([a-z][a-zA-Z0-9_-]+)$")) {
		string modori = hre.getMatch(1);
		string code = hre.getMatch(2);
		bool andy = false;
		bool ory  = false;
		vector<string> codes;
		string tok2 = tok;
		hre.replaceDestructive(tok2, "", "I", "g");
		if (hre.search(tok2, "&")) {
			hre.split(codes, tok2, "&+");
			andy = true;
		} else if (hre.search(tok2, "\\|")) {
			hre.split(codes, tok2, "\\++");
			ory = true;
		} else {
			codes.push_back(tok2);
		}

		string output = "instrument code";
		if (codes.size() != 1) {
			output += "s";
		}
		output += ":";

		for (int i=0; i<(int)codes.size(); i++) {
			output += " (";
			output += codes[i];
			HumInstrument inst;
			inst.setHumdrum(codes[i]);
			string text = inst.getName();
			if (!text.empty()) {
				output += ": \"" + text + "\"";
			} else {
				output += ": unknown code";
			}
			output += ")";
			if (i < (int)codes.size() - 1) {
				if (andy) {
					output += " and ";
				} else if (ory) {
					output += " or ";
				}
			}
		}

		if (modori == "o") {
			output += " (original)";
		} else if (modori == "m") {
			output += " (modern)";
		}

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

	if (tok == "k[]") {
		return "key signature, no sharps or flats";
	}
	if (tok == "ok[]") {
		return "original key signature, no sharps or flats";
	}
	if (tok == "mk[]") {
		return "modern key signature, no sharps or flats";
	}

	HumRegex hre;
	if (hre.search(tok, "^(?:m|o)?k\\[([a-gA-G]+[n#-]{1,2})+\\]$")) {
		string modori;
		string output;
		if (tok[0] == 'o') {
			output = "original ";
		} else if (tok[0] == 'm') {
			output = "modern ";
		}
		output += "key signature";
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
	if (tok == "?:") {
		return "key designation, unknown/unassigned key";
	}
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
			if (isUpper && ((mode == "dor") || (mode == "phr") || (mode == "aeo") || (mode == "loc"))) {
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
				output += " ionian";
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



