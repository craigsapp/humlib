//
// Programmer:    Alexander Morgan
// Creation Date: Fri Jul 24 2026
// Filename:      tool-textract.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Reconstruct the poetic text being set in a score by
//                examining each voice's **text underlay.  Musical
//                repetitions are collapsed; lines present in a majority
//                of voices are kept, ordered by each voice's mostly-
//                linear declamation.
//

#include "tool-textract.h"
#include "HumRegex.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <set>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Tool_textract::Tool_textract --
//

Tool_textract::Tool_textract(void) {
	define("s|syllables|syl=s:", "expected line lengths in syllables (comma-separated, cycles; e.g. 11,7)");
}



//////////////////////////////
//
// Tool_textract::run --
//

bool Tool_textract::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_textract::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_textract::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_textract::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_textract::initialize --
//

void Tool_textract::initialize(void) {
	m_sylCounts.clear();

	if (getBoolean("syllables")) {
		HumRegex hre;
		vector<string> pieces;
		hre.split(pieces, getString("syllables"), "\\s*,\\s*");
		for (string& p : pieces) {
			if (p.empty()) {
				continue;
			}
			try {
				int n = stoi(p);
				if (n > 0) {
					m_sylCounts.push_back(n);
				}
			} catch (...) {
				// ignore malformed entries
			}
		}
	}
}



//////////////////////////////
//
// Tool_textract::processFile --
//

void Tool_textract::processFile(HumdrumFile& infile) {
	vector<Voice> voices;
	getVoices(infile, voices);

	for (Voice& voice : voices) {
		buildSungWords(voice.textStart, voice.words);
		collapseRepeats(voice.words);
		segmentLines(voice);
		dedupeVoiceLines(voice);
	}

	reconstructText(voices);
}



//////////////////////////////
//
// Tool_textract::getVoices -- Pair each **kern with its following **text.
//

void Tool_textract::getVoices(HumdrumFile& infile, vector<Voice>& voices) {
	voices.clear();
	vector<HTp> starts;
	infile.getSpineStartList(starts);

	for (int i=0; i<(int)starts.size(); i++) {
		if (!starts[i]->isKern()) {
			continue;
		}
		if ((i + 1 < (int)starts.size()) && starts[i+1]->isDataType("**text")) {
			Voice voice;
			voice.textStart = starts[i+1];
			voices.push_back(voice);
		}
	}
}



//////////////////////////////
//
// Tool_textract::normalizeWord -- Lower-case ASCII, strip punctuation,
//    fold common accented vowels, keep apostrophes.
//

string Tool_textract::normalizeWord(const string& text) {
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
				case 0x80: case 0x81: case 0xA0: case 0xA1: folded = 'a'; break;
				case 0x88: case 0x89: case 0xA8: case 0xA9: folded = 'e'; break;
				case 0x8C: case 0x8D: case 0xAC: case 0xAD: folded = 'i'; break;
				case 0x92: case 0x93: case 0xB2: case 0xB3: folded = 'o'; break;
				case 0x99: case 0x9A: case 0xB9: case 0xBA: folded = 'u'; break;
			}
			if (folded) {
				out += folded;
				i++;
				continue;
			}
		}
		// "e1"/"a1"/... grave encoding used in some underlays
		if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
			char base = (char)tolower(c);
			if ((i + 1 < text.size()) && (text[i+1] == '1') &&
					((base == 'a') || (base == 'e') || (base == 'i') ||
					 (base == 'o') || (base == 'u'))) {
				out += base;
				i++;
				continue;
			}
			out += base;
			continue;
		}
		if (c == '-') {
			continue;
		}
		out += (char)c;
	}
	return out;
}



//////////////////////////////
//
// Tool_textract::cleanOrigPiece -- Strip hyphens/markers from one syllable
//    for display joining.
//

string Tool_textract::cleanOrigPiece(const string& text) {
	string out;
	for (size_t i=0; i<text.size(); i++) {
		unsigned char c = (unsigned char)text[i];
		if ((c == '<') || (c == '>') || (c == '-')) {
			continue;
		}
		if ((c == ',') || (c == '.') || (c == ':') || (c == ';') ||
				(c == '!') || (c == '?') || (c == '"')) {
			continue;
		}
		out += (char)c;
	}
	return out;
}



//////////////////////////////
//
// Tool_textract::buildSungWords -- Reconstruct words from hyphenated
//    syllables; track capitalization, syllable counts, and <bis> spans.
//

void Tool_textract::buildSungWords(HTp textStart, vector<SungWord>& words) {
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
	string accumNorm;
	string accumOrig;
	int sylCount = 0;
	bool capitalized = false;
	bool bis = false;
	bool inBis = false;

	auto finalize = [&]() {
		if (wordOpen && !accumNorm.empty()) {
			SungWord sw;
			sw.original = accumOrig;
			sw.norm = accumNorm;
			sw.syllables = max(sylCount, 1);
			sw.capitalized = capitalized;
			sw.bis = bis;
			words.push_back(sw);
		}
		wordOpen = false;
		accumNorm.clear();
		accumOrig.clear();
		sylCount = 0;
		capitalized = false;
		bis = false;
	};

	HTp cur = textStart;
	while (cur) {
		if (!cur->isData() || cur->isNull()) {
			cur = cur->getNextToken();
			continue;
		}

		string raw = *cur;
		if (raw.find('<') != string::npos) {
			inBis = true;
		}

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
			if (part.empty()) {
				continue;
			}
			string core = part;
			if (!core.empty() && (core[0] == '<')) {
				core = core.substr(1);
			}
			bool leadingDash = !core.empty() && (core[0] == '-');
			bool trailingDash = endsWithContinuationDash(core);
			string n = normalizeWord(part);
			string o = cleanOrigPiece(part);
			if (n.empty() && o.empty()) {
				continue;
			}

			if ((idx == 0) && wordOpen && leadingDash) {
				accumNorm += n;
				accumOrig += o;
				sylCount++;
				if (!trailingDash) {
					finalize();
				}
			} else {
				if (wordOpen) {
					finalize();
				}
				capitalized = false;
				for (char c : core) {
					if (isalpha((unsigned char)c)) {
						capitalized = isupper((unsigned char)c) != 0;
						break;
					}
				}
				bis = inBis;
				accumNorm = n;
				accumOrig = o;
				sylCount = 1;
				wordOpen = true;
				if (!trailingDash) {
					finalize();
				}
			}
		}

		if (raw.find('>') != string::npos) {
			inBis = false;
		}
		cur = cur->getNextToken();
	}
	finalize();
}



//////////////////////////////
//
// Tool_textract::collapseRepeats -- Remove immediate repeated blocks
//    (musical re-iterations of the same words).
//

void Tool_textract::collapseRepeats(vector<SungWord>& words) {
	// Drop editorial <bis> content entirely: it restates already-sung text.
	{
		vector<SungWord> filtered;
		for (SungWord& w : words) {
			if (!w.bis) {
				filtered.push_back(w);
			}
		}
		words.swap(filtered);
	}

	bool changed = true;
	while (changed) {
		changed = false;
		int n = (int)words.size();
		for (int L = n / 2; L >= 1; L--) {
			for (int i = 0; i + 2 * L <= (int)words.size(); i++) {
				bool match = true;
				for (int k = 0; k < L; k++) {
					if (words[i + k].norm != words[i + L + k].norm) {
						match = false;
						break;
					}
				}
				if (match) {
					words.erase(words.begin() + i + L, words.begin() + i + 2 * L);
					changed = true;
					break;
				}
			}
			if (changed) {
				break;
			}
		}
	}

	// Drop a word that restates the previous two concatenated ("che"+"volgi"→"chevolgi").
	for (int i = 2; i < (int)words.size(); ) {
		if (words[i].norm == words[i-2].norm + words[i-1].norm) {
			words.erase(words.begin() + i);
			continue;
		}
		i++;
	}
}



//////////////////////////////
//
// Tool_textract::endsWithVowel -- Last alphabetic char is a vowel sound.
//

bool Tool_textract::endsWithVowel(const string& norm) {
	for (int i=(int)norm.size()-1; i>=0; i--) {
		unsigned char c = (unsigned char)norm[i];
		if (c == '\'') {
			continue;
		}
		if (!isalpha(c)) {
			continue;
		}
		char l = (char)tolower(c);
		return (l == 'a') || (l == 'e') || (l == 'i') || (l == 'o') || (l == 'u');
	}
	return false;
}



//////////////////////////////
//
// Tool_textract::startsWithVowel -- First alphabetic char is a vowel sound.
//

bool Tool_textract::startsWithVowel(const string& norm) {
	for (size_t i=0; i<norm.size(); i++) {
		unsigned char c = (unsigned char)norm[i];
		if (c == '\'') {
			// Leading apostrophe ("'n") is an elision remnant, not a vowel start.
			return false;
		}
		if (!isalpha(c)) {
			continue;
		}
		char l = (char)tolower(c);
		return (l == 'a') || (l == 'e') || (l == 'i') || (l == 'o') || (l == 'u');
	}
	return false;
}



//////////////////////////////
//
// Tool_textract::elidesWith -- Synaloepha / apostrophe elision between words.
//    Mirrors pliner's apostrophe-aware matching: split on ' and check
//    prefix/suffix relationships, plus vowel-vowel synaloepha.
//

bool Tool_textract::elidesWith(const SungWord& left, const SungWord& right) {
	const string& a = left.norm;
	const string& b = right.norm;
	if (a.empty() || b.empty()) {
		return false;
	}

	// Elision remnant attached to the next word ("E" + "'n").
	if (b[0] == '\'') {
		return true;
	}

	// Apostrophe contraction across a split underlay ("gli" + "occhi" with
	// one side written "gl'occhi"-style).  Split on ' and compare pieces
	// with startswith/endswith, as in pliner.
	auto apostropheElision = [](const string& x, const string& y) -> bool {
		size_t apos = x.find('\'');
		if (apos == string::npos) {
			return false;
		}
		string pre = x.substr(0, apos);
		string suf = x.substr(apos + 1);
		if (!pre.empty() && !suf.empty() && !y.empty()) {
			// x = pre'suf; y matches suf (startswith) or pre (endswith of y
			// when y is the left word — handled by swapping args).
			if (y.compare(0, suf.size(), suf) == 0) {
				return true;
			}
			if (y.size() >= pre.size() &&
					(y.compare(y.size() - pre.size(), pre.size(), pre) == 0)) {
				return true;
			}
			// Truncated prefix: "gl" vs "gli".
			if ((pre.size() >= 2) && (y.size() >= 2) &&
					(y.rfind(pre, 0) == 0 || pre.rfind(y, 0) == 0)) {
				return true;
			}
		}
		return false;
	};
	if (apostropheElision(a, b) || apostropheElision(b, a)) {
		return true;
	}

	// Synaloepha: vowel-final + vowel-initial (also across line boundaries).
	return endsWithVowel(a) && startsWithVowel(b);
}



//////////////////////////////
//
// Tool_textract::lineSyllables -- Written syllable sum minus elisions
//    (synaloepha / apostrophe) between adjacent words, including across
//    what may later become a line boundary when lines are merged.
//

int Tool_textract::lineSyllables(const vector<SungWord>& line) {
	if (line.empty()) {
		return 0;
	}
	int n = 0;
	for (const SungWord& w : line) {
		n += w.syllables;
	}
	for (size_t i=0; i+1 < line.size(); i++) {
		if (elidesWith(line[i], line[i+1])) {
			n--;
		}
	}
	return (n > 0) ? n : 0;
}



//////////////////////////////
//
// Tool_textract::expectedSyllables -- Positional target; list cycles.
//

int Tool_textract::expectedSyllables(int lineIndex) {
	if (m_sylCounts.empty()) {
		return 0;
	}
	return m_sylCounts[lineIndex % (int)m_sylCounts.size()];
}



//////////////////////////////
//
// Tool_textract::distanceToAllowed -- Min distance to any -s length.
//

int Tool_textract::distanceToAllowed(int syllables) {
	if (m_sylCounts.empty()) {
		return abs(syllables);
	}
	int best = abs(syllables - m_sylCounts[0]);
	for (int t : m_sylCounts) {
		best = min(best, abs(syllables - t));
	}
	return best;
}



//////////////////////////////
//
// Tool_textract::isAllowedLength -- True if syllables matches a -s value.
//

bool Tool_textract::isAllowedLength(int syllables, int tol) {
	if (m_sylCounts.empty()) {
		return false;
	}
	for (int t : m_sylCounts) {
		if (abs(syllables - t) <= tol) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_textract::segmentLines -- Split a voice on capitalized words.
//    Mid-line capitals (not common line-starters) may stay in the current
//    line.  Syllable targets (-s) are applied later in refineLines.
//

void Tool_textract::segmentLines(Voice& voice) {
	voice.lines.clear();
	if (voice.words.empty()) {
		return;
	}

	vector<SungWord> cur;
	auto flush = [&]() {
		if (!cur.empty()) {
			voice.lines.push_back(cur);
			cur.clear();
		}
	};

	for (SungWord& w : voice.words) {
		if (!cur.empty() && w.capitalized) {
			bool doBreak = true;
			if (!likelyLineStart(w.norm)) {
				// Mid-line exception, e.g. "Amore" in "Sciogli pietoso Amore".
				if ((int)cur.size() <= 2) {
					doBreak = false;
				} else if (!m_sylCounts.empty()) {
					int have = lineSyllables(cur);
					if (!isAllowedLength(have, 1)) {
						doBreak = false;
					}
				}
			}
			if (doBreak) {
				flush();
			}
		}
		cur.push_back(w);
	}
	flush();
}



//////////////////////////////
//
// Tool_textract::likelyLineStart -- Common poetic line-initial words;
//    capitals outside this set are more often mid-line exceptions.
//

bool Tool_textract::likelyLineStart(const string& norm) {
	static const set<string> starters = {
		"e", "ed", "a", "ad", "di", "de", "del", "che", "chi",
		"la", "le", "il", "lo", "gli", "ne", "ma", "per", "se",
		"non", "un", "una", "uno", "i", "o", "oh", "ah", "quando",
		"come", "cosi", "poi", "anzi", "hor", "or", "ora", "ore",
		"sol", "solo", "su", "sul", "tra", "fra", "con", "da", "dal"
	};
	if (starters.count(norm)) {
		return true;
	}
	// "E'n", "Né", "Sol' io" folded forms.
	if ((norm == "ne") || (norm.rfind("sol", 0) == 0) ||
			(norm.rfind("e'", 0) == 0) || (norm.rfind("ne'", 0) == 0)) {
		return true;
	}
	return false;
}



//////////////////////////////
//
// Tool_textract::linesSimilar -- Soft equality via token LCS ratio.
//

bool Tool_textract::linesSimilar(const vector<SungWord>& a,
		const vector<SungWord>& b) {
	if (a.empty() || b.empty()) {
		return false;
	}
	if (a.size() == b.size()) {
		bool exact = true;
		for (size_t i=0; i<a.size(); i++) {
			if (a[i].norm != b[i].norm) {
				exact = false;
				break;
			}
		}
		if (exact) {
			return true;
		}
	}
	int maxLen = (int)max(a.size(), b.size());
	int minLen = (int)min(a.size(), b.size());
	if (maxLen - minLen > max(2, minLen / 2)) {
		return false;
	}
	// LCS length
	vector<vector<int>> dp(a.size() + 1, vector<int>(b.size() + 1, 0));
	for (size_t i=1; i<=a.size(); i++) {
		for (size_t j=1; j<=b.size(); j++) {
			if (a[i-1].norm == b[j-1].norm) {
				dp[i][j] = dp[i-1][j-1] + 1;
			} else {
				dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
			}
		}
	}
	double ratio = (double)dp[a.size()][b.size()] / (double)maxLen;
	return ratio >= 0.75;
}



//////////////////////////////
//
// Tool_textract::isSubSequence -- Contiguous sub-sequence test on norms.
//

bool Tool_textract::isSubSequence(const vector<SungWord>& shorter,
		const vector<SungWord>& longer) {
	if (shorter.size() >= longer.size()) {
		return false;
	}
	if (shorter.empty()) {
		return false;
	}
	for (size_t i=0; i + shorter.size() <= longer.size(); i++) {
		bool ok = true;
		for (size_t k=0; k<shorter.size(); k++) {
			if (shorter[k].norm != longer[i+k].norm) {
				ok = false;
				break;
			}
		}
		if (ok) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_textract::dedupeVoiceLines -- Drop musical re-statements of a line;
//    keep the fuller version when one line contains another.
//

void Tool_textract::dedupeVoiceLines(Voice& voice) {
	vector<vector<SungWord>> uniq;
	for (auto& line : voice.lines) {
		if (line.empty()) {
			continue;
		}
		bool absorbed = false;
		for (int i=0; i<(int)uniq.size(); i++) {
			if (linesSimilar(line, uniq[i]) || isSubSequence(line, uniq[i])) {
				absorbed = true;
				break;
			}
			if (isSubSequence(uniq[i], line)) {
				uniq[i] = line;
				absorbed = true;
				break;
			}
		}
		if (!absorbed) {
			uniq.push_back(line);
		}
	}
	voice.lines.swap(uniq);
}



//////////////////////////////
//
// Tool_textract::lineToString --
//

string Tool_textract::lineToString(const vector<SungWord>& line) {
	string out;
	for (size_t i=0; i<line.size(); i++) {
		if (i) {
			out += " ";
		}
		out += line[i].original;
	}
	return out;
}



//////////////////////////////
//
// Tool_textract::consensusLine -- Prefer the most common wording; break
//    ties toward the longest non-repetitious form.
//

vector<Tool_textract::SungWord> Tool_textract::consensusLine(LineCluster& cluster) {
	if (cluster.members.empty()) {
		return {};
	}
	if (cluster.members.size() == 1) {
		return cluster.members[0];
	}

	// Median length: trailing junk / local re-iterations often make outliers long.
	vector<int> lengths;
	for (auto& m : cluster.members) {
		lengths.push_back((int)m.size());
	}
	sort(lengths.begin(), lengths.end());
	int medianLen = lengths[lengths.size() / 2];

	int bestIdx = 0;
	double bestScore = -1e9;
	for (int i=0; i<(int)cluster.members.size(); i++) {
		auto& m = cluster.members[i];
		double score = 0;
		for (int j=0; j<(int)cluster.members.size(); j++) {
			if (linesSimilar(m, cluster.members[j])) {
				score += 1.0;
				// Bonus when lengths agree (same wording, not a padded variant).
				if (m.size() == cluster.members[j].size()) {
					score += 0.5;
				}
			}
		}
		score -= 0.75 * fabs((double)m.size() - (double)medianLen);

		// Penalize an immediate repeated half of the line.
		int n = (int)m.size();
		if ((n >= 4) && (n % 2 == 0)) {
			bool half = true;
			for (int k=0; k<n/2; k++) {
				if (m[k].norm != m[n/2 + k].norm) {
					half = false;
					break;
				}
			}
			if (half) {
				score -= 5.0;
			}
		}
		// Penalize a word that is the concatenation of the previous two
		// (e.g. "che" + "volgi" restated as "chevolgi").
		for (int k=2; k<n; k++) {
			if (m[k].norm == m[k-2].norm + m[k-1].norm) {
				score -= 2.0;
			}
			if ((k >= 3) && (m[k-1].norm + m[k].norm == m[k-3].norm + m[k-2].norm)) {
				score -= 1.0;
			}
		}
		if (score > bestScore) {
			bestScore = score;
			bestIdx = i;
		}
	}

	vector<SungWord> result = cluster.members[bestIdx];
	// Trim trailing words absent from a majority of members (local debris).
	while (result.size() > 1) {
		int idx = (int)result.size() - 1;
		int withWord = 0;
		for (auto& m : cluster.members) {
			if (((int)m.size() > idx) && (m[idx].norm == result[idx].norm)) {
				withWord++;
			}
		}
		if (withWord * 2 < (int)cluster.members.size()) {
			result.pop_back();
		} else {
			break;
		}
	}
	return result;
}



//////////////////////////////
//
// Tool_textract::reconstructText -- Cluster lines across voices, keep
//    majority-supported lines, order by typical singing order, emit text.
//

void Tool_textract::reconstructText(vector<Voice>& voices) {
	if (voices.empty()) {
		return;
	}

	vector<LineCluster> clusters;

	for (int vi=0; vi<(int)voices.size(); vi++) {
		for (int li=0; li<(int)voices[vi].lines.size(); li++) {
			auto& line = voices[vi].lines[li];
			int best = -1;
			for (int ci=0; ci<(int)clusters.size(); ci++) {
				auto rep = consensusLine(clusters[ci]);
				bool similar = linesSimilar(line, rep);
				bool lineInRep = isSubSequence(line, rep);
				bool repInLine = isSubSequence(rep, line);
				if (!similar && !lineInRep && !repInLine) {
					continue;
				}
				// Short fragments only merge if they cover most of the other line.
				if (!similar && lineInRep &&
						((int)line.size() * 2 < (int)rep.size())) {
					continue;
				}
				if (!similar && repInLine &&
						((int)rep.size() * 2 < (int)line.size())) {
					continue;
				}
				// Require shared first word when both are capitalized starts.
				if (!line.empty() && !rep.empty() &&
						line[0].capitalized && rep[0].capitalized &&
						(line[0].norm != rep[0].norm)) {
					continue;
				}
				best = ci;
				break;
			}
			if (best >= 0) {
				clusters[best].members.push_back(line);
				clusters[best].voiceIds.push_back(vi);
				// Prefer storing fuller versions first for later consensus.
				if (line.size() > clusters[best].members[0].size()) {
					swap(clusters[best].members[0],
							clusters[best].members.back());
				}
			} else {
				LineCluster c;
				c.members.push_back(line);
				c.voiceIds.push_back(vi);
				clusters.push_back(c);
			}
		}
	}

	int nVoices = (int)voices.size();
	int majority = (nVoices + 1) / 2;

	// Compute average position among contributing voices.
	for (LineCluster& c : clusters) {
		double sum = 0;
		int count = 0;
		set<int> seen;
		for (int k=0; k<(int)c.voiceIds.size(); k++) {
			int vi = c.voiceIds[k];
			if (seen.count(vi)) {
				continue;
			}
			seen.insert(vi);
			// position = index of this line among that voice's lines
			auto rep = consensusLine(c);
			for (int li=0; li<(int)voices[vi].lines.size(); li++) {
				if (linesSimilar(voices[vi].lines[li], rep) ||
						isSubSequence(voices[vi].lines[li], rep) ||
						isSubSequence(rep, voices[vi].lines[li])) {
					sum += li;
					count++;
					break;
				}
			}
		}
		c.avgPos = (count > 0) ? (sum / count) : 0;
	}

	vector<int> keep;
	for (int ci=0; ci<(int)clusters.size(); ci++) {
		set<int> uniqVoices(clusters[ci].voiceIds.begin(), clusters[ci].voiceIds.end());
		if ((int)uniqVoices.size() >= majority) {
			keep.push_back(ci);
		}
	}

	stable_sort(keep.begin(), keep.end(), [&](int a, int b) {
		if (fabs(clusters[a].avgPos - clusters[b].avgPos) > 1e-6) {
			return clusters[a].avgPos < clusters[b].avgPos;
		}
		return a < b;
	});

	vector<vector<SungWord>> poem;
	for (int ci : keep) {
		auto line = consensusLine(clusters[ci]);
		collapseRepeats(line);
		if (!line.empty()) {
			poem.push_back(line);
		}
	}

	refineLines(poem);

	for (auto& line : poem) {
		if (!line.empty()) {
			m_free_text << lineToString(line) << endl;
		}
	}
}



//////////////////////////////
//
// Tool_textract::refineLines -- Apply -s lengths (list cycles; any listed
//    length is allowed).  Metrical counts subtract synaloepha/apostrophe
//    elision between adjacent words, including across a merge boundary.
//

void Tool_textract::refineLines(vector<vector<SungWord>>& lines) {
	if (lines.empty() || m_sylCounts.empty()) {
		return;
	}

	vector<vector<SungWord>> out;
	int i = 0;
	while (i < (int)lines.size()) {
		int target = expectedSyllables((int)out.size());
		int syl = lineSyllables(lines[i]);

		if (isAllowedLength(syl) || (syl == target)) {
			out.push_back(lines[i]);
			i++;
			continue;
		}

		if (i + 1 < (int)lines.size()) {
			int nextSyl = lineSyllables(lines[i+1]);
			bool nextComplete = isAllowedLength(nextSyl);
			vector<SungWord> combined = lines[i];
			combined.insert(combined.end(), lines[i+1].begin(), lines[i+1].end());
			int combSyl = lineSyllables(combined); // includes cross-line elision
			bool hits = isAllowedLength(combSyl) || (combSyl == target);
			bool improves = distanceToAllowed(combSyl) < distanceToAllowed(syl);

			// Never absorb a complete following line unless the merge exactly hits.
			bool allowMerge = hits || (improves && !nextComplete);
			if (allowMerge && !hits && !lines[i+1].empty() &&
					likelyLineStart(lines[i+1][0].norm) && nextComplete) {
				allowMerge = false;
			}

			if (allowMerge) {
				int j = i + 2;
				while (j < (int)lines.size() && !isAllowedLength(lineSyllables(combined))) {
					if (isAllowedLength(lineSyllables(lines[j]))) {
						break;
					}
					vector<SungWord> trial = combined;
					trial.insert(trial.end(), lines[j].begin(), lines[j].end());
					int t2 = lineSyllables(combined);
					int t3 = lineSyllables(trial);
					if (distanceToAllowed(t3) <= distanceToAllowed(t2)) {
						combined.swap(trial);
						j++;
					} else {
						break;
					}
				}
				out.push_back(combined);
				i = j;
				continue;
			}
		}

		int maxAllowed = m_sylCounts[0];
		for (int t : m_sylCounts) {
			maxAllowed = max(maxAllowed, t);
		}
		if (syl > maxAllowed + 1) {
			int bestBreak = -1;
			int bestDist = 9999;
			for (int k=1; k<(int)lines[i].size(); k++) {
				if (!lines[i][k].capitalized) {
					continue;
				}
				vector<SungWord> left(lines[i].begin(), lines[i].begin() + k);
				int leftSyl = lineSyllables(left);
				int dist = distanceToAllowed(leftSyl);
				if (isAllowedLength(leftSyl, 1) && (dist < bestDist)) {
					bestDist = dist;
					bestBreak = k;
				}
			}
			if (bestBreak > 0) {
				vector<SungWord> left(lines[i].begin(), lines[i].begin() + bestBreak);
				vector<SungWord> right(lines[i].begin() + bestBreak, lines[i].end());
				out.push_back(left);
				lines[i] = right;
				continue;
			}
		}

		out.push_back(lines[i]);
		i++;
	}
	lines.swap(out);
}


// END_MERGE

} // end namespace hum
