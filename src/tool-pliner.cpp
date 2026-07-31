//
// Programmer:    Alexander Morgan
// Creation Date: Wed Jul 15 2026
// Filename:      tool-pliner.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Insert *pline poetic-line annotations into a **kern score
//                by aligning each voice's **text declamation against the
//                poem reconstructed by textract from the underlay.
//

#include "tool-pliner.h"
#include "tool-textract.h"
#include "HumRegex.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_pliner::Tool_pliner -- Set the recognized options for the tool.
//

Tool_pliner::Tool_pliner(void) {
	define("s|syllables|syl=s:", "allowed line lengths passed to textract (comma-separated set; e.g. 7,11)");
	define("l|lines=i:0", "expected poem line count passed to textract (0=auto; sonetto→14)");
	define("t|text=s:", "poem text file (skip textract; empty lines discarded, lines trimmed)");
}



/////////////////////////////////
//
// Tool_pliner::run -- Do the main work of the tool.
//

bool Tool_pliner::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_pliner::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pliner::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_pliner::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_pliner::initialize --
//

void Tool_pliner::initialize(void) {
	// nothing to do yet.
}



//////////////////////////////
//
// Tool_pliner::processFile --
//

void Tool_pliner::processFile(HumdrumFile& infile) {
	m_poem.clear();
	m_voices.clear();

	bool havePoem = extractPoem(infile, m_poem);
	if (!havePoem) {
		m_humdrum_text << infile;
		return;
	}

	getVoices(infile, m_voices);

	if (getenv("PLINER_DEBUG")) {
		for (size_t li=0; li<m_poem.size(); li++) {
			cerr << "POEM line=" << li << ":";
			for (auto& pw : m_poem[li]) {
				cerr << " [" << pw.norm << "]";
			}
			cerr << endl;
		}
	}

	map<int, map<int, string>> insertions;

	for (Voice& voice : m_voices) {
		vector<SungWord> words;
		buildSungWords(voice.textStart, words);

		if (getenv("PLINER_DEBUG")) {
			cerr << "VOICE track=" << voice.kernStart->getTrack() << endl;
			for (auto& w : words) {
				cerr << "  word=[" << w.norm << "] cap=" << w.capitalized
				     << " line=" << w.token->getLineIndex() << endl;
			}
		}

		vector<Span> spans;
		alignVoice(words, m_poem, spans);

		int kernTrack = voice.kernStart->getTrack();
		int textTrack = voice.textStart->getTrack();

		for (Span& span : spans) {
			if (!span.startToken) {
				continue;
			}
			if ((span.line < 0) || (span.line >= (int)m_poem.size())) {
				continue;
			}
			string modifier = getModifier(m_poem, span);
			string text = "*pline:" + to_string(span.line + 1) + modifier;
			int li = span.startToken->getLineIndex();
			insertions[li][kernTrack] = text;
			insertions[li][textTrack] = text;
		}
	}

	emitOutput(infile, insertions);
}



//////////////////////////////
//
// Tool_pliner::extractPoem -- Load the poem either from a user-specified
//    text file (-t/--text) or via textract from the underlay, then split
//    it into normalized PoemWord lines for alignment.
//

bool Tool_pliner::extractPoem(HumdrumFile& infile, vector<vector<PoemWord>>& poem) {
	poem.clear();

	string text;
	if (getBoolean("text")) {
		string path = getString("text");
		if (path.empty()) {
			return false;
		}
		ifstream in(path.c_str());
		if (!in) {
			cerr << "Error: cannot open text file: " << path << endl;
			return false;
		}
		ostringstream oss;
		oss << in.rdbuf();
		text = oss.str();
	} else {
		Tool_textract textract;
		vector<string> argv;
		argv.push_back("textract");
		if (getBoolean("syllables")) {
			argv.push_back("-s");
			argv.push_back(getString("syllables"));
		}
		int linesOpt = getInteger("lines");
		if (linesOpt > 0) {
			argv.push_back("-l");
			argv.push_back(to_string(linesOpt));
		}
		textract.process(argv);
		textract.run(infile);
		text = textract.getFreeText();
	}

	if (text.empty()) {
		return false;
	}

	HumRegex hre;
	istringstream stream(text);
	string line;
	int lineNum = 0;
	while (getline(stream, line)) {
		// Trim leading/trailing whitespace; skip empty lines.
		size_t start = line.find_first_not_of(" \t\r\n");
		if (start == string::npos) {
			continue;
		}
		size_t end = line.find_last_not_of(" \t\r\n");
		line = line.substr(start, end - start + 1);

		vector<string> rawwords;
		hre.split(rawwords, line, "\\s+");

		vector<PoemWord> lineWords;
		for (string& w : rawwords) {
			if (w.empty()) {
				continue;
			}
			string norm = normalizeWord(w);
			if (norm.empty()) {
				continue;
			}
			if ((norm[0] == '\'') && !lineWords.empty()) {
				lineWords.back().norm += norm;
				lineWords.back().original += " " + w;
				continue;
			}
			PoemWord pw;
			pw.original = w;
			pw.norm = norm;
			pw.line = lineNum;
			pw.pos = (int)lineWords.size();
			lineWords.push_back(pw);
		}

		if (!lineWords.empty()) {
			poem.push_back(lineWords);
			lineNum++;
		}
	}

	return !poem.empty();
}



//////////////////////////////
//
// Tool_pliner::getVoices -- Identify each **kern spine paired with the
//    **text spine that immediately follows it (the convention used
//    throughout this corpus).
//

void Tool_pliner::getVoices(HumdrumFile& infile, vector<Voice>& voices) {
	voices.clear();
	vector<HTp> starts;
	infile.getSpineStartList(starts);

	for (int i=0; i<(int)starts.size(); i++) {
		if (!starts[i]->isKern()) {
			continue;
		}
		Voice voice;
		voice.kernStart = starts[i];
		if ((i + 1 < (int)starts.size()) && starts[i+1]->isDataType("**text")) {
			voice.textStart = starts[i+1];
		}
		if (voice.textStart) {
			voices.push_back(voice);
		}
	}
}



//////////////////////////////
//
// Tool_pliner::normalizeWord -- Lower-case (ASCII only, to avoid
//    corrupting multi-byte UTF-8 accented characters) and strip common
//    punctuation/markers so that sung syllables and poem words can be
//    compared for equality.  Apostrophes are preserved since they are
//    meaningful within Italian elisions (e.g. "l'alma").  Accented
//    vowels (a grave/acute, e grave/acute, etc., encoded as two-byte
//    UTF-8 sequences) are folded down to their plain-vowel equivalent,
//    since the sung **text underlay and the reference !!@VERSE: text
//    frequently disagree on whether/which accent to write for the same
//    word (e.g. poem "e`" vs. sung "e"), and such spelling differences
//    should not block an otherwise correct word match.
//

string Tool_pliner::normalizeWord(const string& text) {
	string out;
	for (size_t i=0; i<text.size(); i++) {
		unsigned char c = (unsigned char)text[i];
		if ((c == '<') || (c == '>')) {
			continue;
		}
		if ((c == ',') || (c == '.') || (c == ':') || (c == ';') ||
				(c == '!') || (c == '?') || (c == '"')) {
			continue;
		}
		if ((c == 0xC3) && (i + 1 < text.size())) {
			unsigned char c2 = (unsigned char)text[i+1];
			char folded = 0;
			switch (c2) {
				case 0x80: case 0x81: case 0xA0: case 0xA1: folded = 'a'; break; // A/a grave/acute
				case 0x88: case 0x89: case 0xA8: case 0xA9: folded = 'e'; break; // E/e grave/acute
				case 0x8C: case 0x8D: case 0xAC: case 0xAD: folded = 'i'; break; // I/i grave/acute
				case 0x92: case 0x93: case 0xB2: case 0xB3: folded = 'o'; break; // O/o grave/acute
				case 0x99: case 0x9A: case 0xB9: case 0xBA: folded = 'u'; break; // U/u grave/acute
			}
			if (folded) {
				out += folded;
				i++;
				continue;
			}
		}
		if ((c >= 'A') && (c <= 'Z')) {
			out += (char)(c - 'A' + 'a');
		} else {
			out += (char)c;
		}
	}
	while (!out.empty() && (out.front() == '-')) {
		out.erase(out.begin());
	}
	while (!out.empty() && (out.back() == '-')) {
		out.pop_back();
	}
	return out;
}



//////////////////////////////
//
// Tool_pliner::cleanSyllable -- (unused helper retained for API symmetry
//    with normalizeWord; currently a synonym).
//

string Tool_pliner::cleanSyllable(const string& text) {
	return normalizeWord(text);
}



//////////////////////////////
//
// Tool_pliner::buildSungWords -- Walk a **text spine and reconstruct
//    complete words from (possibly hyphenated) syllable tokens, recording
//    the token where each word starts and whether the raw syllable began
//    with a capital letter.
//

void Tool_pliner::buildSungWords(HTp textStart, vector<SungWord>& words) {
	words.clear();
	if (!textStart) {
		return;
	}

	auto endsWithContinuationDash = [](const string& s) {
		string tmp = s;
		while (!tmp.empty()) {
			char c = tmp.back();
			if ((c == ',') || (c == '.') || (c == ':') || (c == ';') ||
					(c == '!') || (c == '?') || (c == '"') || (c == '>')) {
				tmp.pop_back();
				continue;
			}
			break;
		}
		return !tmp.empty() && (tmp.back() == '-');
	};

	bool wordOpen = false;
	string accum;
	HTp startToken = NULL;
	bool capitalized = false;

	auto finalize = [&]() {
		if (wordOpen && !accum.empty()) {
			SungWord sw;
			sw.token = startToken;
			sw.norm = accum;
			sw.capitalized = capitalized;
			words.push_back(sw);
		}
		wordOpen = false;
		accum.clear();
		startToken = NULL;
		capitalized = false;
	};

	HTp cur = textStart;
	while (cur) {
		if (!cur->isData() || cur->isNull()) {
			cur = cur->getNextToken();
			continue;
		}

		string raw = *cur;

		// A single note token can contain two syllables separated by a
		// literal space: the tail end of the previous word followed by
		// the start of the next word (e.g. "-ci as-").
		vector<string> parts;
		size_t pos = 0;
		while (pos <= raw.size()) {
			size_t sp = raw.find(' ', pos);
			if (sp == string::npos) {
				parts.push_back(raw.substr(pos));
				break;
			}
			parts.push_back(raw.substr(pos, sp - pos));
			pos = sp + 1;
		}

		for (int idx=0; idx<(int)parts.size(); idx++) {
			string part = parts[idx];
			string core = part;
			if (!core.empty() && (core[0] == '<')) {
				core = core.substr(1);
			}
			bool leadingDash = !core.empty() && (core[0] == '-');
			bool trailingDash = endsWithContinuationDash(core);

			if ((idx == 0) && wordOpen && leadingDash) {
				// continuation of the currently open word.
				accum += normalizeWord(part);
				if (!trailingDash) {
					finalize();
				}
			} else if (wordOpen && !leadingDash) {
				// Trailing '-' on the previous syllable already marked the
				// word open; some files omit the leading '-' on the next
				// syllable (e.g. "Quan-" / "d'ec-").  Continue unless this
				// part clearly starts a new word (capital letter).
				bool newCap = false;
				for (char c : core) {
					if (isalpha((unsigned char)c)) {
						newCap = isupper((unsigned char)c) != 0;
						break;
					}
				}
				if (!newCap) {
					accum += normalizeWord(part);
					if (!trailingDash) {
						finalize();
					}
				} else {
					finalize();
					startToken = cur;
					accum = normalizeWord(part);
					capitalized = true;
					wordOpen = true;
					if (!trailingDash) {
						finalize();
					}
				}
			} else {
				// starts a new word (closing any dangling previous word).
				if (wordOpen) {
					finalize();
				}
				startToken = cur;
				accum = normalizeWord(part);
				capitalized = false;
				for (char c : core) {
					if (isalpha((unsigned char)c)) {
						capitalized = isupper((unsigned char)c) != 0;
						break;
					}
				}
				if (trailingDash) {
					wordOpen = true;
				} else {
					wordOpen = true; // set so finalize() will push it
					finalize();
				}
			}
		}

		cur = cur->getNextToken();
	}

	finalize();
}



//////////////////////////////
//
// Tool_pliner::alignVoice -- Align a voice's sequence of sung words
//    against the poem, tracking a mostly-forward-moving pointer.  Local
//    repeats are sought within the current line and a short lookback of
//    preceding lines (a closing-stanza repeat may jump back more than
//    one line, e.g. from line 14 back to line 12).  Runs of consecutive
//    words assigned to the same (line, repeat-state) are collapsed into
//    "spans"; each span marks one *pline transition.
//
//    A span only counts as a repeat ("r") if it does not, by the time it
//    ends, reach any further into the line than this voice had already
//    reached before that span began.  This means that an opening
//    partial ("a"/"b"/"c") block sung before the line has ever been
//    completed is not a repeat (there is nothing yet to repeat), even if
//    a later block re-covers that same partial ground again (that later
//    block *is* a repeat, since it does not go beyond what was already
//    reached); but the block that eventually pushes on to complete the
//    line for the first time is not a repeat either, even though it
//    starts by re-treading already-sung words, because it ends up
//    covering new ground.
//
void Tool_pliner::alignVoice(vector<SungWord>& words, vector<vector<PoemWord>>& poem,
		vector<Span>& spans) {
	spans.clear();
	if (poem.empty() || words.empty()) {
		return;
	}

	auto lineWordCount = [&](int line) {
		return (int)poem.at(line).size();
	};

	int ptrLine = -1;
	int ptrPos  = -1;

	// tracks, per poem line, the furthest word position this voice has
	// reached in that line so far (across any span, repeat or not).
	vector<int> farPosInLine(poem.size(), -1);

	bool haveSpan = false;
	Span cur;
	int spanFarAtStart = -1;

	// Furthest position reached in a line, including any progress made
	// so far in the still-open current span (which has not yet been
	// committed to farPosInLine via closeSpan()).
	auto farReached = [&](int line) {
		int far = farPosInLine.at(line);
		if (haveSpan && (cur.line == line) && (cur.endPos > far)) {
			far = cur.endPos;
		}
		return far;
	};

	auto lineReachedEnd = [&](int line) {
		return farReached(line) >= lineWordCount(line) - 1;
	};

	// Occasionally the sung setting elides two adjacent poem words into a
	// single unbroken melisma with no textual break between them (e.g.
	// "Tanti n'aggiungi" sung as one continuous "tantin'aggiungi").  This
	// checks whether a normalized sung word (norm) matches the poem word
	// at (line, pos), optionally also swallowing the next poem word too;
	// returns 0 (no match), 1 (matches just poem[line][pos]), or 2
	// (matches the concatenation of poem[line][pos] and poem[line][pos+1]).
	auto matchLen = [&](int line, int pos, const string& norm) -> int {
		if ((line < 0) || (line >= (int)poem.size())) {
			return 0;
		}
		if ((pos < 0) || (pos >= lineWordCount(line))) {
			return 0;
		}
		if (poem[line][pos].norm == norm) {
			return 1;
		}
		if (pos + 1 < lineWordCount(line)) {
			const string& w1 = poem[line][pos].norm;
			const string& w2 = poem[line][pos+1].norm;
			if ((w1 + w2) == norm) {
				return 2;
			}
			// Elision contracted across a word boundary that the poem
			// text itself did not mark with an apostrophe (e.g. "Gli
			// occhi" written as two separate words but sung/contracted
			// as "Gl'occhi", dropping the final vowel of the first
			// word).  If the sung word contains an apostrophe, treat
			// the part before it as a (possibly truncated) prefix of
			// the first poem word and the part after it as the second
			// poem word.
			size_t apos = norm.find('\'');
			if (apos != string::npos) {
				string left  = norm.substr(0, apos);
				string right = norm.substr(apos + 1);
				if (!left.empty() && !right.empty() &&
						(w1.rfind(left, 0) == 0) && (w2 == right)) {
					return 2;
				}
			}
		}
		return 0;
	};

	auto closeSpan = [&]() {
		cur.repeat = (cur.endPos <= spanFarAtStart);
		if (cur.endPos > farPosInLine[cur.line]) {
			farPosInLine[cur.line] = cur.endPos;
		}
		spans.push_back(cur);
	};

	// Scores a candidate placement (line, pos) for the sung word at index
	// wi by counting how many consecutive sung words (starting at wi,
	// skipping empties, tolerating word-eliding/apostrophe-contraction
	// via matchLen) match the poem starting there.  A candidate whose
	// very first word does not match scores 0.  Capped at a handful of
	// words since that is already enough to confidently distinguish a
	// real match from a coincidental one-word overlap (e.g. two
	// different poem lines both starting with the same short word like
	// "E").
	auto scoreRun = [&](size_t wi, int line, int pos) -> int {
		const int maxRun = 4;
		int score = 0;
		size_t k  = wi;
		int p = pos;
		while (score < maxRun) {
			while ((k < words.size()) && words[k].norm.empty()) {
				k++;
			}
			if (k >= words.size()) {
				break;
			}
			int m = matchLen(line, p, words[k].norm);
			if (m <= 0) {
				break;
			}
			score++;
			p += m;
			k++;
		}
		return score;
	};

	// A voice may enter partway through the poem (e.g. a later voice in
	// an imitative texture that skips the opening line(s) entirely).
	// Rather than always anchoring a voice's very first sung word to
	// poem line 0, search a short lookahead window of this voice's
	// opening words for the (line, pos) placement that gives the best
	// run of consecutive matches (elision-tolerant, via matchLen/
	// scoreRun), and anchor there instead.  If nothing matches at all
	// within the lookahead, fall back to (0, 0) as a last resort.
	auto findInitialAnchor = [&](size_t startIdx, int& outLine, int& outPos) -> bool {
		const int maxLookahead = 6;
		vector<size_t> idxs;
		for (size_t k=startIdx; (k<words.size()) && (idxs.size()<(size_t)maxLookahead); k++) {
			if (!words[k].norm.empty()) {
				idxs.push_back(k);
			}
		}
		if (idxs.empty()) {
			return false;
		}

		int bestScore = 0;
		int bestLine  = -1;
		int bestPos   = -1;
		for (size_t ki=0; ki<idxs.size(); ki++) {
			const string& norm = words[idxs[ki]].norm;
			for (int li=0; li<(int)poem.size(); li++) {
				for (int pi=0; pi<lineWordCount(li); pi++) {
					if (matchLen(li, pi, norm) <= 0) {
						continue;
					}
					// Approximate the line's start position by assuming
					// each of the ki preceding lookahead words consumed
					// exactly one poem word; scoreRun (which is elision-
					// aware) below then confirms/refines the actual run
					// length from that assumed start.
					int startPos = pi - (int)ki;
					if (startPos < 0) {
						continue;
					}
					int score = scoreRun(startIdx, li, startPos);
					if (score > bestScore) {
						bestScore = score;
						bestLine  = li;
						bestPos   = startPos;
					}
				}
			}
		}

		if (bestScore <= 0) {
			return false;
		}
		outLine = bestLine;
		outPos  = bestPos;
		return true;
	};

	for (size_t wi=0; wi<words.size(); wi++) {
		SungWord& sw = words[wi];
		if (sw.norm.empty()) {
			continue;
		}

		int candLine = -1;
		int candPos  = -1;
		bool matched = false;
		// Earliest poem position this sung word covers (equal to candPos
		// unless the word turns out to elide two adjacent poem words
		// together, in which case candPos advances past both while
		// candSpanStart records where the coverage actually began).
		int candSpanStart = -1;

		// Gather every plausible candidate placement for this word and
		// score each by how many consecutive words it (and the words
		// that follow) actually confirm in the poem.  A multi-word
		// confirmed match is always preferred over a match based on a
		// single corresponding syllable/word, no matter which kind of
		// candidate (forward continuation, skipped-word, backward
		// repeat, or a jump to a later line) produced it; a single-word
		// match is only used as a fallback when nothing stronger can be
		// found anywhere.  Ties in score are broken by the priority
		// listed below (lower is preferred), reflecting that the text
		// declamation is almost always linear.
		struct Cand { int line=-1; int pos=-1; int score=0; int priority=99; };
		vector<Cand> cands;

		if (ptrLine < 0) {
			// This voice's very first (non-empty) sung word: don't
			// assume it starts at poem line 0 -- search for where this
			// voice's opening words actually match the poem text.
			int line=-1, pos=-1;
			if (findInitialAnchor(wi, line, pos)) {
				cands.push_back({line, pos, scoreRun(wi, line, pos), 0});
			}
		} else {
			// forward candidate: next word in sequence.
			int fLine, fPos;
			if (ptrPos + 1 < lineWordCount(ptrLine)) {
				fLine = ptrLine;
				fPos  = ptrPos + 1;
			} else {
				fLine = ptrLine + 1;
				fPos  = 0;
			}
			if (fLine < (int)poem.size()) {
				int s = scoreRun(wi, fLine, fPos);
				if (s > 0) {
					cands.push_back({fLine, fPos, s, 0});
				}
			}

			// skip-ahead candidates: a poem word may be skipped/elided
			// in this voice's setting (e.g. no note assigned to it).
			if (ptrPos + 2 < lineWordCount(ptrLine)) {
				int maxSkip = 4;
				for (int skip=2; skip<=maxSkip; skip++) {
					int p = ptrPos + skip;
					if (p >= lineWordCount(ptrLine)) {
						break;
					}
					int s = scoreRun(wi, ptrLine, p);
					if (s > 0) {
						cands.push_back({ptrLine, p, s, 1});
					}
				}
			}

			// bounded backward candidates: current line up to the
			// pointer, then a short lookback of preceding lines.
			// Capitalized re-entries (typical line openings of a
			// repeated tercet/quatrain) may jump back several lines;
			// non-capitals stay closer so common words do not snap to
			// distant earlier matches.
			int maxLineLookback = sw.capitalized ? 6 : 2;
			int lowLine = ptrLine - maxLineLookback;
			if (lowLine < 0) {
				lowLine = 0;
			}
			for (int line = ptrLine; line >= lowLine; line--) {
				int hi = (line == ptrLine) ? ptrPos : (lineWordCount(line) - 1);
				for (int p = hi; p >= 0; p--) {
					int s = scoreRun(wi, line, p);
					if (s > 0) {
						int backDist = ptrLine - line;
						// Near repeats (same/previous line) keep priority
						// 2; farther lookbacks are weaker than forward
						// progress so only a clearly better scoreRun wins.
						int pri = (backDist <= 1) ? 2 : 5;
						cands.push_back({line, p, s, pri});
					}
				}
			}

			// A capitalized word may belong to a line that this voice
			// skips ahead to entirely (e.g. resting through, or
			// omitting, one or more intervening lines), and it need not
			// land on that line's very first word either (the voice may
			// enter only partway into the line, e.g. singing just its
			// tail).  Check every position of a short lookahead of
			// upcoming lines for the best-confirmed placement.
			//
			// Non-capitalized words can also land mid-line ahead of the
			// pointer (e.g. singing "stille di gielo" of line 5 before
			// that line's opening "Quand'ecco").  Search them too, but
			// with lower priority so linear progress still wins ties.
			int maxLineLookahead = sw.capitalized ? 6 : 2;
			int lookaheadPriority = sw.capitalized ? 3 : 4;
			for (int line = ptrLine + 1;
					(line <= ptrLine + maxLineLookahead) && (line < (int)poem.size());
					line++) {
				for (int p = 0; p < lineWordCount(line); p++) {
					int s = scoreRun(wi, line, p);
					if (s > 0) {
						cands.push_back({line, p, s, lookaheadPriority});
					}
				}
			}
		}

		if (!cands.empty()) {
			const Cand* best = &cands[0];
			for (const Cand& c : cands) {
				if ((c.score > best->score) ||
						((c.score == best->score) && (c.priority < best->priority))) {
					best = &c;
				}
			}
			candLine = best->line;
			candSpanStart = best->pos;
			int m = matchLen(candLine, candSpanStart, sw.norm);
			candPos = candSpanStart + std::max(m, 1) - 1;
			matched = true;
		}

		if (!matched && sw.capitalized && (ptrLine >= 0)) {
			// Nothing matched literally anywhere nearby: spelling of the
			// sung syllable may not literally match the reference poem
			// text (e.g. an archaic/dialectal variant), so treat it as a
			// line-start: if the current line has never been sung in
			// full yet, assume this is another attempt at that same
			// (still unfinished) line rather than a jump ahead;
			// otherwise move to the next line.
			if (!lineReachedEnd(ptrLine)) {
				candLine = ptrLine;
				candPos  = 0;
				matched  = true;
			} else if (ptrLine + 1 < (int)poem.size()) {
				candLine = ptrLine + 1;
				candPos  = 0;
				matched  = true;
			}
		}

		if (!matched) {
			// best effort: keep the original forward candidate even
			// though the text did not match (tolerate normalization
			// differences without losing linear progress).
			if (ptrLine < 0) {
				candLine = 0;
				candPos  = 0;
			} else if (ptrPos + 1 < lineWordCount(ptrLine)) {
				candLine = ptrLine;
				candPos  = ptrPos + 1;
			} else if (ptrLine + 1 < (int)poem.size()) {
				candLine = ptrLine + 1;
				candPos  = 0;
			} else {
				candLine = ptrLine;
				candPos  = ptrPos;
			}
			matched = true;
		}

		if (candSpanStart < 0) {
			candSpanStart = candPos;
		}

		ptrLine = candLine;
		ptrPos  = candPos;

		// Landing back at or before the position where the current span
		// itself began (within the same line) marks the start of a
		// fresh declamation block (e.g. two separate partial attempts
		// at a line before it has ever been completed in full, or two
		// separate full repeats of a line separated by a rest).  A
		// backward step that stays strictly after the span's own start
		// (e.g. immediately re-singing the tail end of a line, or a
		// word repeated for emphasis) is treated as a decorative
		// extension of the same ongoing statement instead -- unless the
		// current span has already covered the line in full, in which
		// case a backward step all the way back to the line's own
		// start still begins a new (fresh, complete) statement.  A
		// lesser backward step (a tail-only fragment sung again right
		// after a full statement) is split out into its own *pline
		// transition only if that preceding full statement was itself
		// the line's original (non-repeat) statement -- i.e. this tail
		// fragment is the line's first true repeat and deserves its
		// own tag.  If the preceding full statement was already itself
		// a repeat, a further tail-only echo right after it is instead
		// treated as a decorative extension folded into that span.
		bool curIsFull = haveSpan && (cur.startPos == 0) &&
				(cur.endPos == lineWordCount(cur.line) - 1);
		bool curWouldBeRepeat = haveSpan && (cur.endPos <= spanFarAtStart);
		bool nonForward = haveSpan && (cur.line == candLine) &&
				(curIsFull ? !curWouldBeRepeat : (candSpanStart <= cur.startPos));

		if (!haveSpan || (cur.line != candLine) || nonForward) {
			if (haveSpan) {
				closeSpan();
			}
			cur = Span();
			cur.line       = candLine;
			cur.startPos   = candSpanStart;
			cur.endPos     = candPos;
			cur.startToken = sw.token;
			haveSpan = true;
			spanFarAtStart = farPosInLine[candLine];
		} else {
			if (candPos > cur.endPos) {
				cur.endPos = candPos;
			}
			if (candSpanStart < cur.startPos) {
				cur.startPos = candSpanStart;
			}
		}
	}

	if (haveSpan) {
		closeSpan();
	}
}



//////////////////////////////
//
// Tool_pliner::getModifier -- Compute the *pline suffix for a span:
//    "r" if it is a repeat, plus "a"/"b"/"c" depending on whether the
//    span starts/ends at the line boundaries (no letter if it covers the
//    full line).
//

string Tool_pliner::getModifier(vector<vector<PoemWord>>& poem, Span& span) {
	int lastPos = (int)poem.at(span.line).size() - 1;
	bool startsAtBegin = (span.startPos == 0);
	bool endsAtEnd      = (span.endPos == lastPos);

	string modifier;
	if (span.repeat) {
		modifier += "r";
	}
	if (startsAtBegin && endsAtEnd) {
		// full line: no letter suffix.
	} else if (startsAtBegin && !endsAtEnd) {
		modifier += "a";
	} else if (!startsAtBegin && endsAtEnd) {
		modifier += "b";
	} else {
		modifier += "c";
	}
	return modifier;
}



//////////////////////////////
//
// Tool_pliner::emitOutput -- Stream the input file back out, injecting
//    one synthesized interpretation line immediately before each data
//    line that needs new *pline tokens (merging multiple voices' tokens
//    into a single line when they coincide).
//

void Tool_pliner::emitOutput(HumdrumFile& infile, map<int, map<int, string>>& insertions) {
	for (int i=0; i<infile.getLineCount(); i++) {
		auto it = insertions.find(i);
		if (it != insertions.end()) {
			int fieldCount = infile[i].getFieldCount();
			vector<string> fields(fieldCount, "*");
			for (int j=0; j<fieldCount; j++) {
				int track = infile.token(i, j)->getTrack();
				auto tit = it->second.find(track);
				if (tit != it->second.end()) {
					fields[j] = tit->second;
				}
			}
			for (int j=0; j<fieldCount; j++) {
				m_humdrum_text << fields[j];
				if (j < fieldCount - 1) {
					m_humdrum_text << "\t";
				}
			}
			m_humdrum_text << endl;
		}
		m_humdrum_text << infile[i] << endl;
	}
}


// END_MERGE

} // end namespace hum
