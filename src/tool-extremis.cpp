//
// Programmer:    Alexander Morgan <alexanderpmorgan@gmail.com>
// Creation Date: Sat May 16 18:00:00 CEST 2026
// Last Modified: Sat May 16 19:00:00 CEST 2026
// Filename:      tool-extremis.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-extremis.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Extract a synthetic spine of the lowest and/or highest
//                sounding pitches in a score.  Consecutive moments that share
//                the same extreme pitch are condensed into a single longer
//                note (using ties only when the duration is not expressible
//                as a single **kern duration).  A new attack is emitted only
//                when the extreme pitch changes or when the source voice
//                rearticulates the extreme pitch.
//

#include "tool-extremis.h"
#include "Convert.h"
#include "HumRegex.h"

#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_extremis::Tool_extremis -- Set the recognized options for the tool.
//

Tool_extremis::Tool_extremis(void) {
	define("l|low|lowest=b", "extract lowest sounding pitch (default)");
	define("h|high|highest=b", "extract highest sounding pitch");
	define("b|both=b", "extract both lowest and highest sounding pitches");
}



/////////////////////////////////
//
// Tool_extremis::run -- Do the main work of the tool.
//

bool Tool_extremis::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_extremis::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_extremis::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_extremis::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_extremis::initialize -- Resolve which synthetic spines should be
//    emitted, based on the command-line flags.
//

void Tool_extremis::initialize(void) {
	m_bothQ = getBoolean("both");
	m_highQ = getBoolean("high");
	m_lowQ  = getBoolean("low");

	// -b is equivalent to passing both -l and -h: emit both synthetic spines.
	if (m_bothQ || (m_lowQ && m_highQ)) {
		m_bothQ = true;
		m_lowQ  = true;
		m_highQ = true;
	} else if (m_highQ) {
		// -h alone: emit highest-pitch spine only.
		m_lowQ  = false;
		m_bothQ = false;
	} else {
		// Default behavior (and -l): emit lowest-pitch spine only.
		m_lowQ  = true;
		m_highQ = false;
		m_bothQ = false;
	}
}



/////////////////////////////
//
// Tool_extremis::processFile --
//

void Tool_extremis::processFile(HumdrumFile& infile) {
	int lineCount = infile.getLineCount();

	// Build the synthetic events (notes/rests) for whichever spines are
	// requested, then render each spine into a per-line token vector that
	// is consulted when walking through the input lines.
	vector<SynthEvent> lowEvents;
	vector<SynthEvent> highEvents;
	vector<string> lowTokens(lineCount, "");
	vector<string> highTokens(lineCount, "");

	if (m_lowQ) {
		buildEvents(infile, lowEvents, false);
		renderEvents(infile, lowEvents, lowTokens);
	}
	if (m_highQ) {
		buildEvents(infile, highEvents, true);
		renderEvents(infile, highEvents, highTokens);
	}

	bool wantTwoSpines = m_bothQ;
	bool wantHigh = m_highQ;

	// Track whether the analysis spine has been started yet (after the
	// initial **kern exclusive interpretation has been emitted).
	bool started = false;

	for (int i=0; i<lineCount; i++) {
		HumdrumLine& line = infile[i];

		// Global lines (reference records, global comments, universal
		// comments, etc.) are copied through unchanged.
		if (!line.hasSpines()) {
			m_humdrum_text << line << endl;
			continue;
		}

		// Exclusive interpretation line: emit the **kern exclusive
		// interpretation for each output spine, followed by an
		// appropriate clef so the synthetic spine has reasonable
		// notation defaults.
		if (line.isExclusiveInterpretation()) {
			if (wantTwoSpines) {
				m_humdrum_text << "**kern\t**kern" << endl;
			} else {
				m_humdrum_text << "**kern" << endl;
			}
			started = true;

			if (wantTwoSpines) {
				m_humdrum_text << "*clefF4\t*clefG2" << endl;
			} else if (wantHigh) {
				m_humdrum_text << "*clefG2" << endl;
			} else {
				m_humdrum_text << "*clefF4" << endl;
			}
			continue;
		}

		if (!started) {
			// Skip any spined data before the exclusive interpretation
			// (should not occur in well-formed Humdrum input).
			continue;
		}

		// Terminator line: emit *- for each output spine, but only when
		// every token on the input line is itself a terminator (the
		// final termination of the score).  Partial terminations of
		// individual spines are collapsed to null interpretations on
		// the synthetic output.
		if (line.isInterpretation() && line.token(0)->isTerminator()) {
			bool allTerminators = true;
			for (int j=0; j<line.getFieldCount(); j++) {
				if (!line.token(j)->isTerminator()) {
					allTerminators = false;
					break;
				}
			}
			if (allTerminators) {
				if (wantTwoSpines) {
					m_humdrum_text << "*-\t*-" << endl;
				} else {
					m_humdrum_text << "*-" << endl;
				}
				continue;
			}
		}

		// Barline: copy the barline token from the first **kern spine
		// (or use the first token if no **kern spine is present).
		if (line.isBarline()) {
			string bartok = getBarlineToken(line);
			if (wantTwoSpines) {
				m_humdrum_text << bartok << "\t" << bartok << endl;
			} else {
				m_humdrum_text << bartok << endl;
			}
			continue;
		}

		// Interpretation line: pass through key signatures, time
		// signatures, meter symbols, and key designations from the
		// first **kern spine.  Other interpretations (manipulators,
		// instrument labels, etc.) become null interpretations on the
		// synthetic spine.
		if (line.isInterpretation()) {
			string tok = "*";
			string keysig  = getKeySignature(line);
			string timesig = getTimeSignature(line);
			string metsym  = getMeterSymbol(line);
			string keydesg = getKeyDesignation(line);
			if (!keysig.empty()) {
				tok = keysig;
			} else if (!timesig.empty()) {
				tok = timesig;
			} else if (!metsym.empty()) {
				tok = metsym;
			} else if (!keydesg.empty()) {
				tok = keydesg;
			}
			if (wantTwoSpines) {
				m_humdrum_text << tok << "\t" << tok << endl;
			} else {
				m_humdrum_text << tok << endl;
			}
			continue;
		}

		// Local comment line.
		if (line.isCommentLocal()) {
			if (wantTwoSpines) {
				m_humdrum_text << "!\t!" << endl;
			} else {
				m_humdrum_text << "!" << endl;
			}
			continue;
		}

		// Data line: consult the pre-rendered token vectors.
		if (line.isData()) {
			// Grace-note line (zero duration): emit a null token to
			// preserve line alignment without introducing bogus
			// durations on the synthetic spine.
			if (line.getDuration() == 0) {
				if (wantTwoSpines) {
					m_humdrum_text << ".\t." << endl;
				} else {
					m_humdrum_text << "." << endl;
				}
				continue;
			}

			string lowTok  = lowTokens[i].empty()  ? "." : lowTokens[i];
			string highTok = highTokens[i].empty() ? "." : highTokens[i];

			if (wantTwoSpines) {
				m_humdrum_text << lowTok << "\t" << highTok << endl;
			} else if (wantHigh) {
				m_humdrum_text << highTok << endl;
			} else {
				m_humdrum_text << lowTok << endl;
			}
			continue;
		}

		// Anything else (should be rare): emit a null data token.
		if (wantTwoSpines) {
			m_humdrum_text << ".\t." << endl;
		} else {
			m_humdrum_text << "." << endl;
		}
	}
}



//////////////////////////////
//
// Tool_extremis::buildEvents -- Walk through the data lines of the file and
//    group consecutive moments into synthetic events.  A new event starts
//    when the extreme pitch changes or when the source voice rearticulates
//    the extreme pitch (one of the voices producing the extreme pitch
//    attacks on this line).  Barlines do *not* force a new event: if the
//    source voice is sustaining a tied note across the barline, the event
//    is allowed to span the barline so that the synthetic spine can mirror
//    the source tie.  The chunking pass (chunkEvent) is responsible for
//    splitting any such event into separate tied **kern tokens that fit
//    within their respective measures.
//

void Tool_extremis::buildEvents(HumdrumFile& infile,
		vector<SynthEvent>& events, bool wantHigh) {
	events.clear();
	int lineCount = infile.getLineCount();

	SynthEvent current;
	bool inEvent = false;

	for (int i=0; i<lineCount; i++) {
		HumdrumLine& line = infile[i];

		if (!line.isData()) {
			continue;
		}
		if (line.getDuration() == 0) {
			// Grace note line: do not affect event building.
			continue;
		}

		int b40 = 0;
		bool isAttack = false;
		computePitchAndAttack(line, wantHigh, b40, isAttack);

		bool isRestLine = (b40 == 0);
		bool startNew = false;

		if (!inEvent) {
			startNew = true;
		} else if (current.isRest != isRestLine) {
			startNew = true;
		} else if (!isRestLine && (current.b40 != b40)) {
			startNew = true;
		} else if (!isRestLine && isAttack) {
			// Source voice rearticulates the extreme pitch.
			startNew = true;
		}

		if (startNew) {
			if (inEvent) {
				events.push_back(current);
			}
			current.startLine = i;
			current.endLine   = i;
			current.duration  = line.getDuration();
			current.b40       = b40;
			current.isRest    = isRestLine;
			inEvent = true;
		} else {
			current.endLine  = i;
			current.duration = current.duration + line.getDuration();
		}
	}
	if (inEvent) {
		events.push_back(current);
	}
}



//////////////////////////////
//
// Tool_extremis::computePitchAndAttack -- For a given data line, determine
//    the extreme (lowest or highest) sounding base-40 pitch across all
//    **kern spines and whether any voice producing that extreme pitch
//    attacks on this line.  Null tokens are resolved to their sustained
//    note.  Returns b40 == 0 if every part is resting on this line.
//

void Tool_extremis::computePitchAndAttack(HumdrumLine& line, bool wantHigh,
		int& outB40, bool& outAttack) {
	outB40 = 0;
	outAttack = false;

	int extremeAbs = 0;
	bool extremeAttacked = false;

	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (!token->isKern()) {
			continue;
		}
		// Distinguish actual attacks (positive base-40) from sustained
		// notes (negative base-40, including resolved null tokens).
		bool tokenIsNull = token->isNull();
		vector<int> pitches;
		token->getBase40PitchesResolveNull(pitches);
		for (int p : pitches) {
			int ap = std::abs(p);
			if (ap == 0) {
				continue;  // rest
			}
			// p > 0 means the note is attacked on this line in this
			// voice; p < 0 means the note is being sustained.  Null
			// tokens always count as sustains (getBase40PitchesResolveNull
			// already enforces this).
			bool isAttack = (p > 0) && !tokenIsNull;

			bool isExtreme = false;
			if (extremeAbs == 0) {
				isExtreme = true;
			} else if (wantHigh) {
				if (ap > extremeAbs) {
					isExtreme = true;
				} else if (ap == extremeAbs && isAttack) {
					// Same pitch, but a fresh attack supersedes a
					// sustain for purposes of attack tracking.
					extremeAttacked = true;
				}
			} else {
				if (ap < extremeAbs) {
					isExtreme = true;
				} else if (ap == extremeAbs && isAttack) {
					extremeAttacked = true;
				}
			}

			if (isExtreme) {
				extremeAbs = ap;
				extremeAttacked = isAttack;
			}
		}
	}

	outB40 = extremeAbs;
	outAttack = extremeAttacked;
}



//////////////////////////////
//
// Tool_extremis::renderEvents -- Convert a list of synthetic events into the
//    per-line **kern tokens that should appear on the synthetic spine.
//    A token is emitted on every data line of the event: the first line of
//    each chunk receives the note attack, while subsequent lines within a
//    chunk receive a null token (".") that represents the continuation of
//    the previous attack.  When the event's total duration cannot be
//    expressed as a single **kern duration, the event is split into the
//    fewest possible chunks (each aligned with input data-line boundaries),
//    and tie markers ("[", "_", "]") connect the chunks.
//

void Tool_extremis::renderEvents(HumdrumFile& infile,
		const vector<SynthEvent>& events, vector<string>& lineTokens) {
	for (const SynthEvent& ev : events) {
		vector<int> chunkStartLines;
		vector<HumNum> chunkDurations;
		chunkEvent(infile, ev, chunkStartLines, chunkDurations);

		int chunkCount = (int)chunkStartLines.size();
		if (chunkCount == 0) {
			continue;
		}

		string defaultPitch = ev.isRest ? string("r")
		                                : Convert::base40ToKern(ev.b40);

		for (int k=0; k<chunkCount; k++) {
			string recip = Convert::durationToRecip(chunkDurations[k]);

			// Try to mirror the source's pitch spelling and any non-rhythm
			// markers (beams, stem direction, explicit accidentals, etc.)
			// from the source sub-token that carries this chunk's pitch.
			string pitch = defaultPitch;
			string markers;
			if (!ev.isRest) {
				markers = getSourceMarkers(infile, chunkStartLines[k],
				                           ev.b40, pitch);
			}

			string tok;
			if (ev.isRest) {
				// Rests are never tied across chunks.
				tok = recip + "r" + markers;
			} else if (chunkCount == 1) {
				tok = recip + pitch + markers;
			} else if (k == 0) {
				tok = "[" + recip + pitch + markers;
			} else if (k == chunkCount - 1) {
				tok = recip + pitch + markers + "]";
			} else {
				tok = recip + pitch + markers + "_";
			}
			lineTokens[chunkStartLines[k]] = tok;
		}

		// Fill the interior data lines of each chunk with null tokens so
		// that the synthetic spine has a token on every data line.
		int nextChunkIdx = 0;
		for (int j=ev.startLine; j<=ev.endLine; j++) {
			if (!infile[j].isData()) {
				continue;
			}
			if (infile[j].getDuration() == 0) {
				continue;
			}
			if (nextChunkIdx < chunkCount &&
					j == chunkStartLines[nextChunkIdx]) {
				nextChunkIdx++;
				continue;
			}
			lineTokens[j] = ".";
		}
	}
}



//////////////////////////////
//
// Tool_extremis::getSourceMarkers -- Locate the source **kern sub-token at
//    `line` whose base-40 pitch matches `b40` and return the non-rhythm
//    non-tie markers it carries (beam markers, stem direction, explicit
//    accidentals, articulations, etc.).  If the matching sub-token is
//    found, `outPitch` is overwritten with the sub-token's pitch spelling
//    (so explicit accidentals like "n" or "X" are preserved).  When the
//    matching sub-token can only be found via null-token resolution
//    (i.e., the source had no explicit kern token at this line for the
//    extreme pitch), beam markers are not copied because they would
//    misrepresent the visual grouping at the chunk's start line.
//

string Tool_extremis::getSourceMarkers(HumdrumFile& infile, int line,
		int b40, string& outPitch) {
	HumdrumLine& dataLine = infile[line];
	bool fromNull = false;
	string subtoken;

	// Pass 1: look for an explicit kern sub-token of the desired pitch on
	// this line (this includes both attacks and tie continuations such as
	// "8C]" or "8C_", which still carry the source's visual markers for
	// that note segment).
	for (int i=0; i<dataLine.getFieldCount(); i++) {
		HTp token = dataLine.token(i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		vector<string> pieces = token->getSubtokens();
		for (const string& sub : pieces) {
			if (sub.find("r") != string::npos) {
				continue;
			}
			int pitchB40 = Convert::kernToBase40(sub);
			if (pitchB40 <= 0) {
				continue;
			}
			if (pitchB40 == b40) {
				subtoken = sub;
				break;
			}
		}
		if (!subtoken.empty()) {
			break;
		}
	}

	// Pass 2: if no direct match, resolve null tokens to find the original
	// kern token that is sustaining this pitch.  Beam markers are not
	// copied in this case because they describe the visual grouping at
	// the original attack location, not at the chunk's start line.
	if (subtoken.empty()) {
		for (int i=0; i<dataLine.getFieldCount(); i++) {
			HTp token = dataLine.token(i);
			if (!token->isKern()) {
				continue;
			}
			if (!token->isNull()) {
				continue;
			}
			HTp resolved = token->resolveNull();
			if (!resolved || resolved->isNull()) {
				continue;
			}
			vector<string> pieces = resolved->getSubtokens();
			for (const string& sub : pieces) {
				if (sub.find("r") != string::npos) {
					continue;
				}
				int pitchB40 = Convert::kernToBase40(sub);
				if (pitchB40 <= 0) {
					continue;
				}
				if (pitchB40 == b40) {
					subtoken = sub;
					fromNull = true;
					break;
				}
			}
			if (!subtoken.empty()) {
				break;
			}
		}
	}

	if (subtoken.empty()) {
		return "";
	}

	// Replace the default pitch spelling with the source's so explicit
	// accidentals such as "n" or "X" are preserved.  Skip rhythm, tie,
	// beam, and stem-direction characters because those are either
	// generated by the chunking pass or appended below as separate
	// markers.
	string sourcePitch;
	for (char c : subtoken) {
		if (isdigit((unsigned char)c) || c == '.' || c == '%') {
			continue;
		}
		if (c == '[' || c == '_' || c == ']') {
			continue;
		}
		if (c == 'L' || c == 'J' || c == 'K' || c == 'k' ||
		    c == '/' || c == '\\') {
			continue;
		}
		sourcePitch += c;
	}
	if (!sourcePitch.empty()) {
		outPitch = sourcePitch;
	}

	// Collect beam markers and stem-direction markers from the source
	// sub-token.  These represent how the source notated this particular
	// note segment.
	string markers;
	if (!fromNull) {
		for (char c : subtoken) {
			if (c == 'L' || c == 'J' || c == 'K' || c == 'k' ||
			    c == '/' || c == '\\') {
				markers += c;
			}
		}
	}
	return markers;
}



//////////////////////////////
//
// Tool_extremis::lineHasMatchingBeamMarker -- Return true if any explicit
//    (non-null) **kern sub-token on `line` has a base-40 pitch equal to
//    `b40` and carries a primary or secondary beam marker (L, J, K, k).
//    This is used by the chunking pass to prevent the synthetic spine
//    from consolidating across source notes that participate in a beam
//    group, so that beam grouping is preserved in the output.
//

bool Tool_extremis::lineHasMatchingBeamMarker(HumdrumFile& infile, int line,
		int b40) {
	if (b40 == 0) {
		return false;
	}
	HumdrumLine& dataLine = infile[line];
	for (int i=0; i<dataLine.getFieldCount(); i++) {
		HTp token = dataLine.token(i);
		if (!token->isKern()) {
			continue;
		}
		if (token->isNull()) {
			continue;
		}
		vector<string> pieces = token->getSubtokens();
		for (const string& sub : pieces) {
			if (sub.find("r") != string::npos) {
				continue;
			}
			int pitchB40 = Convert::kernToBase40(sub);
			if (pitchB40 <= 0) {
				continue;
			}
			if (pitchB40 != b40) {
				continue;
			}
			for (char c : sub) {
				if (c == 'L' || c == 'J' || c == 'K' || c == 'k') {
					return true;
				}
			}
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_extremis::stripRhythmAndTies -- Remove rhythm-related characters
//    (digits, dots, "%") and tie markers ("[", "_", "]") from a **kern
//    sub-token, leaving only pitch spelling and articulation markers.
//

string Tool_extremis::stripRhythmAndTies(const string& subtoken) {
	string result;
	for (char c : subtoken) {
		if (isdigit((unsigned char)c)) {
			continue;
		}
		if (c == '.' || c == '%') {
			continue;
		}
		if (c == '[' || c == '_' || c == ']') {
			continue;
		}
		result += c;
	}
	return result;
}



//////////////////////////////
//
// Tool_extremis::chunkEvent -- Split a synthetic event into the fewest
//    possible chunks where each chunk has a duration that can be expressed
//    as a single **kern recip value, and each chunk boundary coincides with
//    an input data-line boundary within the event.  Barlines that lie
//    inside the event force a chunk boundary so that no chunk crosses a
//    barline (the chunks before and after each barline are still tied
//    together by renderEvents).  Uses a greedy strategy: starting from
//    the first data line of the event, find the largest contiguous group
//    of data lines whose summed duration is "clean" and which does not
//    cross a barline, emit that as one chunk, then continue from the next
//    data line.
//

void Tool_extremis::chunkEvent(HumdrumFile& infile, const SynthEvent& event,
		vector<int>& chunkStartLines, vector<HumNum>& chunkDurations) {
	chunkStartLines.clear();
	chunkDurations.clear();

	// Collect the (line, duration) pairs of every actual data line
	// covered by the event.
	vector<int> dataLines;
	vector<HumNum> dataDurs;
	for (int i=event.startLine; i<=event.endLine; i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].getDuration() == 0) {
			continue;
		}
		dataLines.push_back(i);
		dataDurs.push_back(infile[i].getDuration());
	}

	int n = (int)dataLines.size();
	if (n == 0) {
		return;
	}

	int i = 0;
	while (i < n) {
		int bestJ = i;
		HumNum bestDur = dataDurs[i];
		HumNum runDur  = dataDurs[i];
		if (isCleanKernDuration(runDur)) {
			bestJ = i;
			bestDur = runDur;
		}
		for (int j=i+1; j<n; j++) {
			// Stop extending if a barline appears between the
			// previous data line and this one; the chunk must
			// not cross the barline.
			bool crossesBarline = false;
			for (int k=dataLines[j-1]+1; k<dataLines[j]; k++) {
				if (infile[k].isBarline()) {
					crossesBarline = true;
					break;
				}
			}
			if (crossesBarline) {
				break;
			}
			// Stop extending if the next data line carries source
			// beam markers for the matching pitch: that line should
			// start its own chunk so the source's beam grouping is
			// preserved in the synthetic output.
			if (!event.isRest &&
					lineHasMatchingBeamMarker(infile, dataLines[j],
					                          event.b40)) {
				break;
			}
			runDur = runDur + dataDurs[j];
			if (isCleanKernDuration(runDur)) {
				bestJ = j;
				bestDur = runDur;
			}
		}
		chunkStartLines.push_back(dataLines[i]);
		chunkDurations.push_back(bestDur);
		i = bestJ + 1;
	}
}



//////////////////////////////
//
// Tool_extremis::isCleanKernDuration -- Return true if the given duration
//    (in quarter notes) can be expressed as a single **kern recip value
//    based on a power-of-two division of a whole note (possibly with up
//    to three augmentation dots), or as a breve / long / maxima (with up
//    to one dot).  Triplet-based recip values such as "3...", which
//    Convert::durationToRecip can also produce, are rejected here so
//    that condensed notes stay within ordinary metric notation.
//

bool Tool_extremis::isCleanKernDuration(HumNum dur) {
	if (dur <= 0) {
		return false;
	}
	string recip = Convert::durationToRecip(dur);
	if (recip.find('%') != string::npos) {
		return false;
	}
	// Strip off augmentation dots to obtain the base recip portion.
	size_t dotPos = recip.find('.');
	string base = (dotPos == string::npos) ? recip : recip.substr(0, dotPos);
	if (base.empty()) {
		return false;
	}
	// Breve / long / maxima (and dotted variants) all have a base of
	// one or more '0' characters and are considered clean.
	bool allZero = true;
	for (char c : base) {
		if (c != '0') {
			allZero = false;
			break;
		}
	}
	if (allZero) {
		return true;
	}
	// Otherwise the base must be a positive integer that is a power of two
	// (1, 2, 4, 8, 16, 32, ...).
	int n = 0;
	for (char c : base) {
		if (!isdigit((unsigned char)c)) {
			return false;
		}
		n = n * 10 + (c - '0');
	}
	if (n <= 0) {
		return false;
	}
	return (n & (n - 1)) == 0;
}



//////////////////////////////
//
// Tool_extremis::getBarlineToken -- Return the barline token to use on a
//    barline line.  Uses the first **kern token if available, otherwise
//    falls back to the first token on the line.
//

string Tool_extremis::getBarlineToken(HumdrumLine& line) {
	HTp first = getFirstKernToken(line);
	if (first) {
		return *first;
	}
	if (line.getFieldCount() > 0) {
		return *(line.token(0));
	}
	return "=";
}



//////////////////////////////
//
// Tool_extremis::getKeySignature -- Return the key signature interpretation
//    from the first **kern spine on this line, or an empty string if the
//    line is not a key signature line.
//

string Tool_extremis::getKeySignature(HumdrumLine& line) {
	HTp first = getFirstKernToken(line);
	if (!first) {
		return "";
	}
	if (!first->isKeySignature()) {
		return "";
	}
	return *first;
}



//////////////////////////////
//
// Tool_extremis::getTimeSignature -- Return the time signature interpretation
//    from the first **kern spine on this line, or an empty string if the
//    line is not a time signature line.
//

string Tool_extremis::getTimeSignature(HumdrumLine& line) {
	HTp first = getFirstKernToken(line);
	if (!first) {
		return "";
	}
	if (!first->isTimeSignature()) {
		return "";
	}
	return *first;
}



//////////////////////////////
//
// Tool_extremis::getMeterSymbol -- Return the meter symbol interpretation
//    from the first **kern spine on this line, or an empty string if the
//    line is not a meter symbol line.
//

string Tool_extremis::getMeterSymbol(HumdrumLine& line) {
	HTp first = getFirstKernToken(line);
	if (!first) {
		return "";
	}
	if (!first->isMetricSymbol()) {
		return "";
	}
	return *first;
}



//////////////////////////////
//
// Tool_extremis::getKeyDesignation -- Return the key designation
//    interpretation from the first **kern spine on this line, or an
//    empty string if the line is not a key designation line.
//

string Tool_extremis::getKeyDesignation(HumdrumLine& line) {
	HTp first = getFirstKernToken(line);
	if (!first) {
		return "";
	}
	if (!first->isKeyDesignation()) {
		return "";
	}
	return *first;
}



//////////////////////////////
//
// Tool_extremis::getFirstKernToken -- Return the first **kern token on a
//    line, or NULL if there is none.
//

HTp Tool_extremis::getFirstKernToken(HumdrumLine& line) {
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (token && token->isKern()) {
			return token;
		}
	}
	return NULL;
}



// END_MERGE

} // end namespace hum
