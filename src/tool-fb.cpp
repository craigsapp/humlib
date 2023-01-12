//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sun Nov 27 2022 00:25:34 CET
// Filename:      tool-fb.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-fb.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add figured bass numbers from **kern spines.
//

#include "tool-fb.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_fb::Tool_fb -- Set the recognized options for the tool.
//

Tool_fb::Tool_fb(void) {
	define("c|compound=b",      "output compound intervals for intervals bigger than 9");
	define("a|accidentals=b",   "display accidentals in figured bass output");
	define("b|base=i:0",        "number of the base voice/spine");
	define("i|intervallsatz=b", "display intervals under their voice and not under the lowest staff");
	define("s|sort=b",          "sort figured bass numbers by interval size and not by voice index");
	define("l|lowest=b",        "use lowest note as base note; -b flag will be ignored");
	define("n|normalize=b",     "remove octave and doubled intervals, use compound interval, sort intervals");
	define("r|abbr=b",          "use abbreviated figures");
	define("t|attack=b",        "hide intervalls with no attack and when base does not change");
	define("f|figuredbass=b",   "shortcut for -c -a -s -l -n -r -3");
	define("3|hide-three=b",    "hide number 3 if it has an accidental (e.g.: #3 => #)");
}



//////////////////////////////
//
// Tool_fb::run -- Do the main work of the tool.
//

bool Tool_fb::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_fb::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_fb::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_fb::run(HumdrumFile &infile) {

	m_compoundQ      = getBoolean("compound");
	m_accidentalsQ   = getBoolean("accidentals");
	m_baseQ          = getInteger("base");
	m_intervallsatzQ = getBoolean("intervallsatz");
	m_sortQ          = getBoolean("sort");
	m_lowestQ        = getBoolean("lowest");
	m_normalizeQ     = getBoolean("normalize");
	m_abbrQ          = getBoolean("abbr");
	m_attackQ        = getBoolean("attack");
	m_figuredbassQ   = getBoolean("figuredbass");
	m_hideThreeQ     = getBoolean("hide-three");

	if (m_abbrQ) {
		m_normalizeQ = true;
	}

	if (m_normalizeQ) {
		m_compoundQ = true;
		m_sortQ = true;
	}

	if (m_figuredbassQ) {
		m_compoundQ = true;
		m_accidentalsQ = true;
		m_sortQ = true;
		m_lowestQ = true;
		m_normalizeQ = true;
		m_abbrQ = true;
		m_hideThreeQ = true;
	}

	NoteGrid grid(infile);

	vector<FiguredBassNumber*> numbers;

	vector<HTp> kernspines = infile.getKernSpineStartList();

	vector<int> lastNumbers = {};
	lastNumbers.resize((int)grid.getVoiceCount());
	vector<int> currentNumbers = {};

	// Interate through the NoteGrid and fill the numbers vector with
	// all generated FiguredBassNumbers
	for (int i=0; i<(int)grid.getSliceCount(); i++) {
		currentNumbers.clear();
		currentNumbers.resize((int)grid.getVoiceCount());

		// Reset usedBaseVoiceIndex
		int usedBaseVoiceIndex = m_baseQ;

		// Overwrite usedBaseVoiceIndex with the lowest voice index of the lowest pitched note
		// TODO: check if this still works for chords
		if (m_lowestQ) {
			int lowestNotePitch = 99999;
			for (int k=0; k<(int)grid.getVoiceCount(); k++) {
				NoteCell* checkCell = grid.cell(k, i);
				int checkCellPitch = abs(checkCell->getSgnDiatonicPitch());
				if ((checkCellPitch > 0) && (checkCellPitch < lowestNotePitch)) {
					lowestNotePitch = checkCellPitch;
					usedBaseVoiceIndex = k;
				}
			}
		}

		NoteCell* baseCell = grid.cell(usedBaseVoiceIndex, i);
		string keySignature = getKeySignature(infile, baseCell->getLineIndex());

		// Interate through each voice
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if (j == usedBaseVoiceIndex) {
				// Ignore base voice
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);
			// Create FiguredBassNumber
			FiguredBassNumber* number = createFiguredBassNumber(baseCell, targetCell, keySignature);
			if (lastNumbers[j] != 0) {
				// Set currAttackNumberDidChange
				number->m_currAttackNumberDidChange = (targetCell->isSustained()) && (lastNumbers[j] != number->m_number);
			}
			currentNumbers[j] = number->m_number;
			numbers.push_back(number);
		}
		
		// Set current numbers as the new last numbers
		lastNumbers = currentNumbers;
	}

	if (m_intervallsatzQ) {
		// Create **fb spine for each voice
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount());
			if (voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
			} else {
				infile.appendDataSpine(trackData, ".", "**fb");
			}
		}
	} else {
		// Create **fb spine and bind it to the base voice
		vector<string> trackData = getTrackData(numbers, infile.getLineCount());
		if (m_baseQ + 1 < grid.getVoiceCount()) {
			int trackIndex = kernspines[m_baseQ + 1]->getTrack();
			infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
		} else {
			infile.appendDataSpine(trackData, ".", "**fb");
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_fb::getTrackData -- Create **fb spine data with formatted numbers for all voices
//

vector<string> Tool_fb::getTrackData(const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLine(numbers, i);
		if (sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
}



//////////////////////////////
//
// Tool_fb::getTrackDataForVoice -- Create **fb spine data with formatted numbers for passed voiceIndex
//

vector<string> Tool_fb::getTrackDataForVoice(int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLineAndVoice(numbers, i, voiceIndex);
		if (sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
}



//////////////////////////////
//
// Tool_fb::createFiguredBassNumber -- Create FiguredBassNumber from a NoteCell.
//    The figured bass number (num) is calculated with a base and target NoteCell
//    as well as a passed key signature.
//

FiguredBassNumber* Tool_fb::createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature) {

	// Calculate figured bass number and make value absolue
	// There is currenlty no support for negative numbers (e.g. with intervallsatz)
	int basePitch   = base->getSgnDiatonicPitch();
	int targetPitch = target->getSgnDiatonicPitch();
	int num         = ((basePitch == 0) || (targetPitch == 0)) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;

	// Transform key signature to lower case
	transform(keySignature.begin(), keySignature.end(), keySignature.begin(), [](unsigned char c) {
		return tolower(c);
	});

	char targetPitchClass = Convert::kernToDiatonicLC(target->getSgnKernPitch());
	int targetAccidNr = Convert::base40ToAccidental(target->getSgnBase40Pitch());
	string targetAccid;
	for (int i=0; i<abs(targetAccidNr); i++) {
		targetAccid += (targetAccidNr < 0 ? '-' : '#');
	}

	char basePitchClass = Convert::kernToDiatonicLC(base->getSgnKernPitch());
	int baseAccidNr = Convert::base40ToAccidental(base->getSgnBase40Pitch());
	string baseAccid;
	for (int i=0; i<abs(baseAccidNr); i++) {
		baseAccid += (baseAccidNr < 0 ? '-' : '#');
	}

	string accid = targetAccid;
	bool showAccid = false;

	// Show accidentals when they are not included in the key signature
	if ((targetAccidNr != 0) && (keySignature.find(targetPitchClass + targetAccid) == std::string::npos)) {
		showAccid = true;
	}

	// Show natural accidentals when they are alterations of the key signature
	if ((targetAccidNr == 0) && (keySignature.find(targetPitchClass + targetAccid) != std::string::npos)) {
		accid = 'n';
		showAccid = true;
	}

	// Show accidentlas when pitch class of base and target is equal but alteration is different
	if (basePitchClass == targetPitchClass) {
		if (baseAccidNr == targetAccidNr) {
			showAccid = false;
		} else {
			accid = (targetAccidNr == 0) ? "n" : targetAccid;
			showAccid = true;
		}
	}

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, showAccid, target->getVoiceIndex(), target->getLineIndex(), target->isAttack());

	return number;
}



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLine -- Find all FiguredBassNumber objects for a slice (line index) of the music.
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex](FiguredBassNumber* num) {
		return num->m_lineIndex == lineIndex;
	});

	// sort by voiceIndex
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->m_voiceIndex > b->m_voiceIndex; 
	});

	return filteredNumbers;
}



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLineAndVoice --
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex and passed voiceIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex, voiceIndex](FiguredBassNumber* num) {
		return (num->m_lineIndex == lineIndex) && (num->m_voiceIndex == voiceIndex);
	});

	// sort by voiceIndex (probably not needed here)
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->m_voiceIndex > b->m_voiceIndex; 
	});

	return filteredNumbers;
}



//////////////////////////////
//
// Tool_fb::formatFiguredBassNumbers -- Create a **fb data record string out of the passed FiguredBassNumber objects
//

string Tool_fb::formatFiguredBassNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> formattedNumbers;

	// Normalize numbers (remove 8 and 1, sort by size, remove duplicate numbers)
	if (m_normalizeQ) {
		bool aQ = m_accidentalsQ;
		// remove 8 and 1 but keep them if they have an accidental
		copy_if(numbers.begin(), numbers.end(), back_inserter(formattedNumbers), [aQ](FiguredBassNumber* num) {
			return ((num->getNumberWithinOctave() != 8) && (num->getNumberWithinOctave() != 1)) || (aQ && num->m_showAccidentals);
		});
		// sort by size
		sort(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return a->getNumberWithinOctave() < b->getNumberWithinOctave();
		});
		// remove duplicate numbers
		formattedNumbers.erase(unique(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) {
			return a->getNumberWithinOctave() == b->getNumberWithinOctave();
		}), formattedNumbers.end());
	} else {
		formattedNumbers = numbers;
	}

	// Hide numbers if they have no attack
	if (m_intervallsatzQ && m_attackQ) {
		vector<FiguredBassNumber*> attackNumbers;
		copy_if(formattedNumbers.begin(), formattedNumbers.end(), back_inserter(attackNumbers), [](FiguredBassNumber* num) {
			return num->m_isAttack || num->m_currAttackNumberDidChange;
		});
		formattedNumbers = attackNumbers;
	}

	// Sort numbers by size
	if (m_sortQ) {
		bool cQ = m_compoundQ;
		sort(formattedNumbers.begin(), formattedNumbers.end(), [cQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			// sort by getNumberWithinOctave if compoundQ is true otherwise sort by number
			return (cQ) ? a->getNumberWithinOctave() > b->getNumberWithinOctave() : a->m_number > b->m_number;
		});
	}

	if (m_abbrQ) {
		// Overwrite formattedNumbers with abbreviated numbers
		formattedNumbers = getAbbreviatedNumbers(formattedNumbers);
	}

	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: formattedNumbers) {
		string num = number->toString(m_compoundQ, m_accidentalsQ, m_hideThreeQ);
		if (num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += num;
		}
	}
	return str;
}



//////////////////////////////
//
// Tool_fb::getAbbreviatedNumbers -- Get abbreviated figured bass numbers
//    If no abbreviation is found all numbers will be shown

vector<FiguredBassNumber*> Tool_fb::getAbbreviatedNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> abbreviatedNumbers;

	vector<FiguredBassAbbreviationMapping*> mappings = FiguredBassAbbreviationMapping::s_mappings;

	string numberString = getNumberString(numbers);

	// Check if an abbreviation exists for passed numbers
	auto it = find_if(mappings.begin(), mappings.end(), [numberString](FiguredBassAbbreviationMapping* abbr) {
		return abbr->m_str == numberString;
	});

	if (it != mappings.end()) {
		int index = it - mappings.begin();
		FiguredBassAbbreviationMapping* abbr = mappings[index];
		bool aQ = m_accidentalsQ;
		// Store numbers to display by the abbreviation mapping in abbreviatedNumbers
		copy_if(numbers.begin(), numbers.end(), back_inserter(abbreviatedNumbers), [abbr, aQ](FiguredBassNumber* num) {
			vector<int> nums = abbr->m_numbers;
			// Show numbers if they are part of the abbreviation mapping or if they have an accidental
			return (find(nums.begin(), nums.end(), num->getNumberWithinOctave()) != nums.end()) || (num->m_showAccidentals && aQ);
		});

		return abbreviatedNumbers;
	}

	return numbers;
}



//////////////////////////////
//
// Tool_fb::getNumberString -- Get only the numbers (without accidentals) of passed FiguredBassNumbers
//

string Tool_fb::getNumberString(vector<FiguredBassNumber*> numbers) {
	// Sort numbers by size
	sort(numbers.begin(), numbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->getNumberWithinOctave() > b->getNumberWithinOctave();
	});
	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* nr: numbers) {
		int num = nr->getNumberWithinOctave();
		if (num > 0) {
			if (!first) str += " ";
			first = false;
			str += to_string(num);
		}	
	}
	return str;
}



//////////////////////////////
//
// Tool_fb::getKeySignature -- Get the key signature for a line index of the input file
//

string Tool_fb::getKeySignature(HumdrumFile& infile, int lineIndex) {
	string keySignature = "";
	[&] {
		for (int i = 0; i < infile.getLineCount(); i++) {
			if (i > lineIndex) {
				return;
			}
			HLp line = infile.getLine(i);
			for (int j = 0; j < line->getFieldCount(); j++) {
				if (line->token(j)->isKeySignature()) {
					keySignature = line->getTokenString(j);
				}
			}
		}
	}();
	return keySignature;
}



//////////////////////////////
//
// FiguredBassNumber::FiguredBassNumber -- Constructor
//

FiguredBassNumber::FiguredBassNumber(int num, string accid, bool showAccid, int voiceIdx, int lineIdx, bool isAtk) {
	m_number          = num;
	m_accidentals     = accid;
	m_voiceIndex      = voiceIdx;
	m_lineIndex       = lineIdx;
	m_showAccidentals = showAccid;
	m_isAttack        = isAtk;
}



//////////////////////////////
//
// FiguredBassNumber::toString -- Convert FiguredBassNumber to a string (accidental + number)
//

string FiguredBassNumber::toString(bool compoundQ, bool accidentalsQ, bool hideThreeQ) {
	int num = (compoundQ) ? getNumberWithinOctave() : m_number;
	string accid = (accidentalsQ && m_showAccidentals) ? m_accidentals : "";
	if ((num == 3) && accidentalsQ && m_showAccidentals && hideThreeQ) {
		return accid;
	}
	return num > 0 ? accid + to_string(num) : "";
}



//////////////////////////////
//
// FiguredBassNumber::getNumberWithinOctave -- Get figured bass number as non compound interval (base 7)
//

int FiguredBassNumber::getNumberWithinOctave(void) {
	int num = (m_number > 9) ? m_number % 7 : m_number;
	if ((m_number > 9) && (m_number % 7 == 0)) {
		num = 7;
	}
	return ((m_number > 8) && (num == 1)) ? 8 : num;
}



//////////////////////////////
//
// FiguredBassAbbreviationMapping::FiguredBassAbbreviationMapping -- Constructor
//    Helper class to store the mappings for abbreviate figured bass numbers
//

FiguredBassAbbreviationMapping::FiguredBassAbbreviationMapping(string s, vector<int> n) {
	m_str = s;
	m_numbers = n;
}



//////////////////////////////
//
// FiguredBassAbbreviationMapping::s_mappings -- Mapping to abbreviate figured bass numbers
//

vector<FiguredBassAbbreviationMapping*> FiguredBassAbbreviationMapping::s_mappings = {
	new FiguredBassAbbreviationMapping("3", {}),
	new FiguredBassAbbreviationMapping("5", {}),
	new FiguredBassAbbreviationMapping("5 3", {}),
	new FiguredBassAbbreviationMapping("6 3", {6}),
	new FiguredBassAbbreviationMapping("5 4", {4}),
	new FiguredBassAbbreviationMapping("7 5 3", {7}),
	new FiguredBassAbbreviationMapping("7 3", {7}),
	new FiguredBassAbbreviationMapping("7 5", {7}),
	new FiguredBassAbbreviationMapping("6 5 3", {6, 5}),
	new FiguredBassAbbreviationMapping("6 4 3", {4, 3}),
	new FiguredBassAbbreviationMapping("6 4 2", {4, 2}),
	new FiguredBassAbbreviationMapping("9 5 3", {9}),
	new FiguredBassAbbreviationMapping("9 5", {9}),
	new FiguredBassAbbreviationMapping("9 3", {9}),
};

// END_MERGE

} // end namespace hum
