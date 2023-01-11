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

	compoundQ      = getBoolean("compound");
	accidentalsQ   = getBoolean("accidentals");
	baseQ          = getInteger("base");
	intervallsatzQ = getBoolean("intervallsatz");
	sortQ          = getBoolean("sort");
	lowestQ        = getBoolean("lowest");
	normalizeQ     = getBoolean("normalize");
	abbrQ          = getBoolean("abbr");
	attackQ        = getBoolean("attack");

	if(abbrQ) {
		normalizeQ = true;
	}

	if(normalizeQ) {
		compoundQ = true;
		sortQ = true;
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
		int usedBaseVoiceIndex = baseQ;

		// Overwrite usedBaseVoiceIndex with the lowest voice index of the lowest pitched note
		// TODO: check if this still works for chords
		if(lowestQ) {
			int lowestNotePitch = 99999;
			for (int k=0; k<(int)grid.getVoiceCount(); k++) {
				NoteCell* checkCell = grid.cell(k, i);
				int checkCellPitch = abs(checkCell->getSgnDiatonicPitch());
				if(checkCellPitch > 0 && checkCellPitch < lowestNotePitch) {
					lowestNotePitch = checkCellPitch;
					usedBaseVoiceIndex = k;
				}
			}
		}

		NoteCell* baseCell = grid.cell(usedBaseVoiceIndex, i);
		string keySignature = getKeySignature(infile, baseCell->getLineIndex());

		// Interate through each voice
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if(j == usedBaseVoiceIndex) {
				// Ignore base voice
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);
			// Create FiguredBassNumber
			FiguredBassNumber* number = createFiguredBassNumber(baseCell, targetCell, keySignature);
			if(lastNumbers[j] != 0) {
				// Set currAttackNumberDidChange
				number->currAttackNumberDidChange = targetCell->isSustained() && lastNumbers[j] != number->number;
			}
			currentNumbers[j] = number->number;
			numbers.push_back(number);
		}
		
		// Set current numbers as the new last numbers
		lastNumbers = currentNumbers;
	}

	if(intervallsatzQ) {
		// Create **fb spine for each voice
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount());
			if(voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
			} else {
				infile.appendDataSpine(trackData, ".", "**fb");
			}
		}
	} else {
		// Create **fb spine and bind it to the base voice
		vector<string> trackData = getTrackData(numbers, infile.getLineCount());
		if(baseQ + 1 < grid.getVoiceCount()) {
			int trackIndex = kernspines[baseQ + 1]->getTrack();
			infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
		} else {
			infile.appendDataSpine(trackData, ".", "**fb");
		}
	}

	return true;
};



//////////////////////////////
//
// Tool_fb::getTrackData -- Create **fb spine data with formatted numbers for all voices
//

vector<string> Tool_fb::getTrackData(const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLine(numbers, i);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
};



//////////////////////////////
//
// Tool_fb::getTrackDataForVoice -- Create **fb spine data with formatted numbers for passed voiceIndex
//

vector<string> Tool_fb::getTrackDataForVoice(int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLineAndVoice(numbers, i, voiceIndex);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
};



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
	int num         = (basePitch == 0 || targetPitch == 0) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;

	regex accidRegex("^\\(?(\\w)+([^\\w\\)]*)\\)?$");

	// Parse accidental from getSgnKernPitch()
	string accid = regex_replace(target->getSgnKernPitch(), accidRegex, "$2");

	// Make sure accidWithPitch and keySignature are both lowercase for better comparison
	string accidWithPitch = regex_replace(target->getSgnKernPitch(), accidRegex, "$1$2");
	transform(accidWithPitch.begin(), accidWithPitch.end(), accidWithPitch.begin(), [](unsigned char c) {
		return tolower(c);
	});
	transform(keySignature.begin(), keySignature.end(), keySignature.begin(), [](unsigned char c) {
		return tolower(c);
	});

	// Only show accidentals when they are not included in the key signature
	bool showAccid = false;
	if(accid.length() && keySignature.find(accidWithPitch) == std::string::npos) {
		showAccid = true;
	}

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, showAccid, target->getVoiceIndex(), target->getLineIndex(), target->isAttack());

	return number;
};



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLine -- Find all FiguredBassNumber objects for a slice (line index) of the music.
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex;
	});

	// sort by voiceIndex
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLineAndVoice --
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex and passed voiceIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex, voiceIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex && num->voiceIndex == voiceIndex;
	});

	// sort by voiceIndex (probably not needed here)
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};



//////////////////////////////
//
// Tool_fb::formatFiguredBassNumbers -- Create a **fb data record string out of the passed FiguredBassNumber objects
//

string Tool_fb::formatFiguredBassNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> formattedNumbers;

	// Normalize numbers (remove 8 and 1, sort by size, remove duplicate numbers)
	if(normalizeQ) {
		bool aQ = accidentalsQ;
		// remove 8 and 1 but keep them if they have an accidental
		copy_if(numbers.begin(), numbers.end(), back_inserter(formattedNumbers), [aQ](FiguredBassNumber* num) {
			return (num->getNumberB7() != 8 && num->getNumberB7() != 1) || (aQ && num->showAccidentals);
		});
		// sort by size
		sort(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return a->getNumberB7() < b->getNumberB7();
		});
		// remove duplicate numbers
		formattedNumbers.erase(unique(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) {
			return a->getNumberB7() == b->getNumberB7();
		}), formattedNumbers.end());
	} else {
		formattedNumbers = numbers;
	}

	// Hide numbers if they have no attack
	if(intervallsatzQ && attackQ) {
		vector<FiguredBassNumber*> attackNumbers;
		copy_if(formattedNumbers.begin(), formattedNumbers.end(), back_inserter(attackNumbers), [](FiguredBassNumber* num) {
			return num->isAttack || num->currAttackNumberDidChange;
		});
		formattedNumbers = attackNumbers;
	}

	// Sort numbers by size
	if(sortQ) {
		bool cQ = compoundQ;
		sort(formattedNumbers.begin(), formattedNumbers.end(), [cQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			// sort by getNumberB7 if compoundQ is true otherwise sort by number
			return (cQ) ? a->getNumberB7() > b->getNumberB7() : a->number > b->number;
		});
	}

	if(abbrQ) {
		// Overwrite formattedNumbers with abbreviated numbers
		formattedNumbers = getAbbrNumbers(formattedNumbers);
	}

	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: formattedNumbers) {
		string num = number->toString(compoundQ, accidentalsQ);
		if(num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += num;
		}
	}
	return str;
};



//////////////////////////////
//
// Tool_fb::getAbbrNumbers -- Get abbreviated figured bass numbers
//    If no abbreviation is found all numbers will be shown

vector<FiguredBassNumber*> Tool_fb::getAbbrNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> abbrNumbers;

	// Mapping to abbreviate figured bass numbers
	vector<FiguredBassAbbr*> figuredBassAbbrs = {
		new FiguredBassAbbr("3", {}),
		new FiguredBassAbbr("5", {}),
		new FiguredBassAbbr("5 3", {}),
		new FiguredBassAbbr("6 3", {6}),
		new FiguredBassAbbr("5 4", {4}),
		new FiguredBassAbbr("7 5 3", {7}),
		new FiguredBassAbbr("7 3", {7}),
		new FiguredBassAbbr("7 5", {7}),
		new FiguredBassAbbr("6 5 3", {6, 5}),
		new FiguredBassAbbr("6 4 3", {4, 3}),
		new FiguredBassAbbr("6 4 2", {4, 2}),
		new FiguredBassAbbr("9 5 3", {9}),
		new FiguredBassAbbr("9 5", {9}),
		new FiguredBassAbbr("9 3", {9}),
	};

	string numberString = getNumberString(numbers);

	// Check if an abbreviation exists for passed numbers
	auto it = find_if(figuredBassAbbrs.begin(), figuredBassAbbrs.end(), [numberString](FiguredBassAbbr* abbr) {
		return abbr->str == numberString;
	});

	if (it != figuredBassAbbrs.end()) {
		int index = it - figuredBassAbbrs.begin();
		FiguredBassAbbr* abbr = figuredBassAbbrs[index];
		bool aQ = accidentalsQ;
		// Store numbers to display by the abbreviation mapping in abbrNumbers
		copy_if(numbers.begin(), numbers.end(), back_inserter(abbrNumbers), [abbr, aQ](FiguredBassNumber* num) {
			vector<int> nums = abbr->numbers;
			// Show numbers if they are part of the abbreviation mapping or if they have an accidental
			return find(nums.begin(), nums.end(), num->getNumberB7()) != nums.end() || (num->showAccidentals && aQ);
		});

		return abbrNumbers;
	}

	return numbers;
};



//////////////////////////////
//
// Tool_fb::getNumberString -- Get only the numbers (without accidentals) of passed FiguredBassNumbers
//

string Tool_fb::getNumberString(vector<FiguredBassNumber*> numbers) {
	// Sort numbers by size
	sort(numbers.begin(), numbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->getNumberB7() > b->getNumberB7();
	});
	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* nr: numbers) {
		int num = nr->getNumberB7();
		if(num > 0) {
			if (!first) str += " ";
			first = false;
			str += to_string(num);
		}	
	}
	return str;
};



//////////////////////////////
//
// Tool_fb::getKeySignature -- Get the key signature for a line index of the input file
//

string Tool_fb::getKeySignature(HumdrumFile& infile, int lineIndex) {
	string keySignature = "";
	[&] {
		for (int i = 0; i < infile.getLineCount(); i++) {
			if(i > lineIndex) {
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
};



//////////////////////////////
//
// FiguredBassNumber::FiguredBassNumber -- Constructor
//

FiguredBassNumber::FiguredBassNumber(int num, string accid, bool showAccid, int voiceIdx, int lineIdx, bool isAtk) {
	number          = num;
	accidentals     = accid;
	voiceIndex      = voiceIdx;
	lineIndex       = lineIdx;
	showAccidentals = showAccid;
	isAttack        = isAtk;
};



//////////////////////////////
//
// FiguredBassNumber::toString -- Convert FiguredBassNumber to a string (accidental + number)
//

string FiguredBassNumber::toString(bool compoundQ, bool accidentalsQ) {
	int num = (compoundQ) ? getNumberB7() : number;
	string accid = (accidentalsQ && showAccidentals) ? accidentals : "";
	return num > 0 ? to_string(num) + accid : "";
};



//////////////////////////////
//
// FiguredBassNumber::getNumberB7 -- Get figured bass number as non compound interval (base 7)
//

int FiguredBassNumber::getNumberB7() {
	int num = (number > 9) ? number % 7 : number;
	if(number > 9 && number % 7 == 0) {
		num = 7;
	}
	return (number > 8 && num == 1) ? 8 : num;
};



//////////////////////////////
//
// FiguredBassAbbr::FiguredBassAbbr -- Constructor
//    Helper class to store the mappings for abbreviate figured bass numbers
//

FiguredBassAbbr::FiguredBassAbbr(string s, vector<int> n) {
	str = s;
	numbers = n;
};

// END_MERGE

} // end namespace hum
