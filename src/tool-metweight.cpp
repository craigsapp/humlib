//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Thu Jul 16 2026
// Filename:      tool-metweight.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-metweight.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Label the metric weight (strong/half-strong/weak) of note
//                and rest attacks in **kern spines, derived algorithmically
//                from the time signature (see getWeightClass).
//

#include "tool-metweight.h"
#include "tool-composite.h"
#include "tool-meter.h"
#include "Convert.h"

#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Tool_metweight::Tool_metweight -- Set the recognized options for the tool.
//

Tool_metweight::Tool_metweight(void) {
	define("f|full=b",         "print full text labels (strong/half-strong/weak/unclassified) instead of abbreviations");
	define("i|integer=b",      "print integer rank labels (1/2/3/4) instead of abbreviations");
	define("x|cdata=b",        "label the spine **cdata-metweight instead of **metweight");
	define("k|kern-tracks=s",  "process only the specified kern spines");
	define("s|spine|spines=s", "process only the specified spines");
	define("n|null=b",         "always use the null token . for unclassified positions");
	define("t|tied=b",         "label secondary tied notes instead of giving them the null token .");
	define("c|composite=b",    "add a **kern-comp spine (composite rhythm of all voices) below the voices and label only that spine");
}



//////////////////////////////
//
// Tool_metweight::run -- Do the main work of the tool.
//

bool Tool_metweight::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_metweight::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_metweight::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_metweight::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_metweight::initialize --
//

void Tool_metweight::initialize(void) {
	m_fullQ      = getBoolean("full");
	m_integerQ   = getBoolean("integer");
	m_cdataQ     = getBoolean("cdata");
	m_nullQ      = getBoolean("null");
	m_tiedQ      = getBoolean("tied");
	m_compositeQ = getBoolean("composite");

	if (getBoolean("spine")) {
		m_spineTracks = getString("spine");
	} else if (getBoolean("kern-tracks")) {
		m_kernTracks = getString("kern-tracks");
	}
}



//////////////////////////////
//
// Tool_metweight::processFile -- Insert a **metweight spine after every
//    processed **kern spine, containing the metric weight of the current
//    note/rest attack.  With -c only a composite spine is added below the
//    voices, and only that spine gets a **metweight spine.
//

void Tool_metweight::processFile(HumdrumFile& infile) {
	if (m_compositeQ) {
		addCompositeSpine(infile);
	}

	vector<HTp> voices;
	getVoices(voices, infile);
	if (voices.empty()) {
		// Nothing to label, but pass the input on unchanged.
		m_humdrum_text << infile;
		return;
	}

	// Delegate time-signature/beat-position analysis (including pickup
	// measures) to the meter tool, which annotates each **kern attack
	// with "auto" parameters read back out below; "-r" also covers rests.
	Tool_meter meterTool;
	meterTool.process("-r");
	meterTool.run(infile);

	vector<vector<string>> results;
	fillVoiceResults(results, infile, voices);

	// Insert each voice's spine right after its own track, processing from
	// last to first so that earlier (not-yet-processed) voices' track
	// numbers are not shifted by a later insertion.
	string exinterp = m_cdataQ ? "**cdata-metweight" : "**metweight";
	for (int i = (int)voices.size() - 1; i >= 0; i--) {
		int track = voices[i]->getTrack();
		infile.insertDataSpineAfter(track, results[i], ".", exinterp);
	}
	infile.createLinesFromTokens();

	// Enable usage in verovio (`!!!filter: metweight`)
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_metweight::addCompositeSpine -- -c option: replace the input with the
//    output of the composite tool, which adds a **kern-comp spine (the merged
//    rhythm of all voices) below the existing voices.  Without "-a" the
//    composite spine is prepended, which places it below the input voices in
//    the rendered score.
//

void Tool_metweight::addCompositeSpine(HumdrumFile& infile) {
	if (infile.getKernSpineStartList().empty()) {
		return;
	}

	stringstream composite;
	Tool_composite compositeTool;
	compositeTool.process("");
	compositeTool.run(infile, composite);
	infile.readString(composite.str());
}



//////////////////////////////
//
// Tool_metweight::getVoices -- The spines to label: with -c only the
//    composite spine, otherwise the **kern spines selected by -k or -s (all
//    of them when neither option is given).  The composite rhythm is always
//    built from all **kern spines, so -k/-s do not apply in that mode.
//

void Tool_metweight::getVoices(vector<HTp>& voices, HumdrumFile& infile) {
	voices.clear();

	if (m_compositeQ) {
		vector<HTp> spinestarts = infile.getKernLikeSpineStartList();
		for (int i = 0; i < (int)spinestarts.size(); i++) {
			if (*spinestarts[i] == "**kern-comp") {
				voices.push_back(spinestarts[i]);
			}
		}
		return;
	}

	int maxTrack = infile.getMaxTrack();

	m_selectedKernSpines.resize(maxTrack + 1); // +1 since track=0 is not used
	fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), true);

	vector<HTp> kernspines = infile.getKernSpineStartList();

	// Calculate which input spines to process based on -k or -s option:
	if (!m_kernTracks.empty()) {
		vector<int> ktracks = Convert::extractIntegerList(m_kernTracks, maxTrack);
		fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), false);
		for (int i = 0; i < (int)ktracks.size(); i++) {
			int index = ktracks[i] - 1;
			if ((index < 0) || (index >= (int)kernspines.size())) {
				continue;
			}
			int track = kernspines.at(index)->getTrack();
			m_selectedKernSpines.at(track) = true;
		}
	} else if (!m_spineTracks.empty()) {
		fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), false);
		infile.makeBooleanTrackList(m_selectedKernSpines, m_spineTracks);
	}

	for (int i = 0; i < (int)kernspines.size(); i++) {
		if (m_selectedKernSpines.at(kernspines[i]->getTrack())) {
			voices.push_back(kernspines[i]);
		}
	}
}



//////////////////////////////
//
// Tool_metweight::fillVoiceResults -- Calculate the **metweight tokens for
//    each selected **kern spine, one value per line, for
//    appendDataSpine()/insertDataSpineBefore() to consume.
//

void Tool_metweight::fillVoiceResults(vector<vector<string>>& results, HumdrumFile& infile,
		const vector<HTp>& voices) {

	int lineCount = infile.getLineCount();
	results.clear();
	results.resize(voices.size());
	for (int v = 0; v < (int)voices.size(); v++) {
		results[v].resize(lineCount, ".");
	}

	for (int i = 0; i < lineCount; i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int v = 0; v < (int)voices.size(); v++) {
			results[v][i] = getWeightToken(infile, i, voices[v]->getTrack());
		}
	}
}



//////////////////////////////
//
// Tool_metweight::getWeightToken -- Metric weight token for the track's
//    **kern token on this line, using the position Tool_meter annotated.
//    Null tokens return "."; non-attacks/unrecognized meters return
//    WEIGHT_UNCLASSIFIED.
//

string Tool_metweight::getWeightToken(HumdrumFile& infile, int line, int track) {
	HTp token = NULL;
	for (int j = 0; j < infile[line].getFieldCount(); j++) {
		HTp tok = infile.token(line, j);
		if (tok->getTrack() == track) {
			token = tok;
			break;
		}
	}
	if (!token) {
		return ".";
	}

	if (token->isNull()) {
		return ".";
	}

	// The continuation of a tie is not a new attack, so it has no metric
	// weight of its own (-t restores labeling it anyway).  HumdrumToken::
	// isSecondaryTiedNote() only accepts plain **kern, so test the token
	// text directly to cover **kern-comp spines as well.
	if (!m_tiedQ && Convert::isKernSecondaryTiedNote(*token)) {
		return ".";
	}

	HTp position = token;
	if (m_compositeQ && !position->getValueBool("auto", "hasData")) {
		// The meter tool only analyzes **kern spines, so a **kern-comp
		// token is never annotated itself.  The metric position is a
		// property of the line rather than of a particular voice, so read
		// it from a **kern attack on the same line instead.
		position = getMeterToken(infile, line);
	}
	if (!position || !position->getValueBool("auto", "hasData")) {
		return formatWeightClass(WEIGHT_UNCLASSIFIED);
	}

	int top = position->getValueInt("auto", "numerator");
	int bot = position->getValueInt("auto", "denominator");
	HumNum beat(position->getValue("auto", "zeroBeat"));

	return formatWeightClass(getWeightClass(top, bot, beat));
}



//////////////////////////////
//
// Tool_metweight::getMeterToken -- First token on the line that the meter
//    tool annotated with a metric position, or NULL if there is none (for
//    example when every voice is sustaining a note across this line).
//

HTp Tool_metweight::getMeterToken(HumdrumFile& infile, int line) {
	for (int j = 0; j < infile[line].getFieldCount(); j++) {
		HTp token = infile.token(line, j);
		if (token->getValueBool("auto", "hasData")) {
			return token;
		}
	}
	return NULL;
}



//////////////////////////////
//
// Tool_metweight::getWeightClass -- Derive the metric weight
//    (strong/half-strong/weak/unclassified) of a beat position from the
//    time signature.  "beat" is Tool_meter's "zeroBeat": a 0-indexed
//    count of main beats (quarter note in 4/4, dotted quarter in 6/8)
//    from the start of the measure; a fractional beat (e.g. 1/3) is a
//    subdivision within a beat.
//
//    Algorithm: split "beat" into a whole main-beat index and a leftover
//    fraction.  Fraction 0 is a main beat: index 0 is strong, the beat
//    starting the second half of the measure (only if mainBeatCount is
//    even) is half-strong, every other main beat is weak.  A nonzero
//    fraction is only classified (weak) for a compound meter with
//    exactly 2 main beats (6/8), at its second/third eighth note
//    (fraction 1/3 or 2/3); everything else is left unclassified.
//

int Tool_metweight::getWeightClass(int top, int bot, HumNum beat) {
	// Guard against an unset/unrecognized time signature (top or bot not
	// yet known, e.g. before the first *M interpretation in the file) or
	// a negative position (should not normally happen, but would break
	// the arithmetic below if it did).
	if ((top <= 0) || (bot <= 0) || (beat < 0)) {
		return WEIGHT_UNCLASSIFIED;
	}

	// How many main beats does this measure have, and is the meter
	// compound (does each main beat itself split into three)?  "multiple"
	// and "remainder" are just top divided/modulo 3, reused below to
	// test "is top a multiple of 3 that is greater than 3".
	int multiple = top / 3;
	int remainder = top % 3;
	bool compound = (bot >= 8) && (multiple > 1) && (remainder == 0);

	// Compound: 3 eighth notes per main beat, so top/3 main beats
	// (6/8 -> 2, 9/8 -> 3, 12/8 -> 4).  Simple: one main beat per counted
	// unit, so top main beats (4/4 -> 4, 3/4 -> 3, 6/4 -> 6).
	int mainBeatCount = compound ? multiple : top;
	if (mainBeatCount <= 0) {
		// Defensive: cannot happen for a valid time signature (top > 0),
		// but avoids a divide-by-zero-flavored mainBeatCount/2 below.
		return WEIGHT_UNCLASSIFIED;
	}

	// Split "beat" (e.g. 4/3 for the second eighth note of the second
	// compound beat) into its whole main-beat index (1) and the
	// fraction of a beat left over (1/3).
	int mainBeatIndex = beat.getInteger();
	HumNum fraction = beat - HumNum(mainBeatIndex);

	// A main-beat position (no leftover fraction):
	if (fraction == 0) {
		if (mainBeatIndex == 0) {
			// The downbeat: always the strongest position of the measure.
			return WEIGHT_STRONG;
		}
		if (((mainBeatCount % 2) == 0) && (mainBeatIndex == mainBeatCount / 2)) {
			// Exactly the main beat that starts the second half of the
			// measure (only exists when there is an even number of main
			// beats): the secondary/half-strong accent.
			return WEIGHT_HALF_STRONG;
		}
		if (mainBeatIndex < mainBeatCount) {
			// Any other in-range main beat: weak (e.g. beats 2 and 4 of
			// a 4/4 measure).
			return WEIGHT_WEAK;
		}
		// mainBeatIndex >= mainBeatCount would mean the position is at or
		// past the end of the measure; not expected in practice, but
		// left unclassified rather than guessing a weight for it.
		return WEIGHT_UNCLASSIFIED;
	}

	// A position between main beats (nonzero fraction, some subdivision
	// or syncopated offset).  Only give this a weight in the one case
	// where the subdivision is itself a genuinely counted part of the
	// meter: a compound meter with exactly 2 main beats (6/8), at the
	// second or third eighth note of a beat (fraction 1/3 or 2/3).
	if (compound && (mainBeatCount == 2) && ((fraction == HumNum(1, 3)) || (fraction == HumNum(2, 3)))) {
		return WEIGHT_WEAK;
	}

	// Everything else (a simple meter's "and" of the beat, the eighth-note
	// subdivisions of 9/8 or 12/8, sixteenth notes and other deeper
	// subdivisions, syncopated offbeats): not a notated counting position
	// of this meter, so it is left unclassified.
	return WEIGHT_UNCLASSIFIED;
}



//////////////////////////////
//
// Tool_metweight::formatWeightClass -- Convert a weight class into the
//    token representation selected by the -f/-i options (abbreviated
//    strong/half-strong/weak codes by default); -n forces the null
//    token "." for unclassified positions in all three modes.
//

string Tool_metweight::formatWeightClass(int weightClass) {
	if (m_nullQ && (weightClass == WEIGHT_UNCLASSIFIED)) {
		return ".";
	}
	if (m_integerQ) {
		return to_string(weightClass);
	} else if (m_fullQ) {
		switch (weightClass) {
			case WEIGHT_STRONG:      return "strong";
			case WEIGHT_HALF_STRONG: return "half-strong";
			case WEIGHT_WEAK:        return "weak";
			default:                 return "unclassified";
		}
	} else {
		switch (weightClass) {
			case WEIGHT_STRONG:      return "s";
			case WEIGHT_HALF_STRONG: return "hs";
			case WEIGHT_WEAK:        return "w";
			default:                 return "u";
		}
	}
}


// END_MERGE

} // end namespace hum
