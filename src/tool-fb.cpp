#include "tool-fb.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>

using namespace std;

namespace hum {

// START_MERGE

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

	for (int i=0; i<(int)grid.getSliceCount(); i++) {
		currentNumbers.clear();
		currentNumbers.resize((int)grid.getVoiceCount());
		int usedBaseVoiceIndex = baseQ;
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
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if(j == usedBaseVoiceIndex) {
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);
			FiguredBassNumber* number = createFiguredBassNumber(baseCell, targetCell, keySignature);
			if(lastNumbers[j] != 0) {
				number->currAttackNumberDidChange = targetCell->isSustained() && lastNumbers[j] != number->number;
			}
			currentNumbers[j] = number->number;
			numbers.push_back(number);
		}
		lastNumbers = currentNumbers;
	}

	if(intervallsatzQ) {
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount());
			if(voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
			} else {
				int trackIndex = kernspines[voiceIndex]->getTrack();
				infile.appendDataSpine(trackData, ".", "**fb");
			}
		}
	} else {
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

vector<string> Tool_fb::getTrackData(vector<FiguredBassNumber*> numbers, int lineCount) {
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

vector<string> Tool_fb::getTrackDataForVoice(int voiceIndex, vector<FiguredBassNumber*> numbers, int lineCount) {
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

FiguredBassNumber* Tool_fb::createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature) {

	int basePitch   = base->getSgnDiatonicPitch();
	int targetPitch = target->getSgnDiatonicPitch();
	int num         = (basePitch == 0 || targetPitch == 0) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;

	bool showAccid = false;
	regex accidRegex("^\\(?(\\w)+([^\\w\\)]*)\\)?$");
	string accid = regex_replace(target->getSgnKernPitch(), accidRegex, "$2");

	string accidWithPitch = regex_replace(target->getSgnKernPitch(), accidRegex, "$1$2");
	transform(accidWithPitch.begin(), accidWithPitch.end(), accidWithPitch.begin(), [](unsigned char c) {
		return tolower(c);
	});
	transform(keySignature.begin(), keySignature.end(), keySignature.begin(), [](unsigned char c) {
		return tolower(c);
	});
	if(accid.length() && keySignature.find(accidWithPitch) == std::string::npos) {
		showAccid = true;
	}

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, showAccid, target->getVoiceIndex(), target->getLineIndex(), target->isAttack());

	return number;
};

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex;
	});

	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex, voiceIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex && num->voiceIndex == voiceIndex;
	});

	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};

string Tool_fb::formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers) {

	vector<FiguredBassNumber*> formattedNumbers;

	if(normalizeQ) {
		bool aQ = accidentalsQ;
		copy_if(numbers.begin(), numbers.end(), back_inserter(formattedNumbers), [aQ](FiguredBassNumber* num) {
			return (num->getNumberB7() != 8 && num->getNumberB7() != 1) || (aQ && num->showAccidentals);
		});
		sort(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return a->getNumberB7() < b->getNumberB7();
		});
		formattedNumbers.erase(unique(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) {
			return a->getNumberB7() == b->getNumberB7();
		}), formattedNumbers.end());
	} else {
		formattedNumbers = numbers;
	}

	if(intervallsatzQ && attackQ) {
		vector<FiguredBassNumber*> attackNumbers;
		copy_if(formattedNumbers.begin(), formattedNumbers.end(), back_inserter(attackNumbers), [](FiguredBassNumber* num) {
			return num->isAttack || num->currAttackNumberDidChange;
		});
		formattedNumbers = attackNumbers;
	}

	if(sortQ) {
		bool cQ = compoundQ;
		sort(formattedNumbers.begin(), formattedNumbers.end(), [cQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return (cQ) ? a->getNumberB7() > b->getNumberB7() : a->number > b->number;
		});
	}

	if(abbrQ) {
		formattedNumbers = getAbbrNumbers(formattedNumbers);
	}

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

vector<FiguredBassNumber*> Tool_fb::getAbbrNumbers(vector<FiguredBassNumber*> numbers) {

	vector<FiguredBassNumber*> abbrNumbers;

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

	auto it = find_if(figuredBassAbbrs.begin(), figuredBassAbbrs.end(), [numberString](FiguredBassAbbr* abbr) {
		return abbr->str == numberString;
	});

	if (it != figuredBassAbbrs.end()) {
		int index = it - figuredBassAbbrs.begin();
		FiguredBassAbbr* abbr = figuredBassAbbrs[index];
		bool aQ = accidentalsQ;
		copy_if(numbers.begin(), numbers.end(), back_inserter(abbrNumbers), [abbr, aQ](FiguredBassNumber* num) {
			vector<int> nums = abbr->numbers;
			return find(nums.begin(), nums.end(), num->getNumberB7()) != nums.end() || (num->showAccidentals && aQ);
		});

		return abbrNumbers;
	}

	return numbers;
};

string Tool_fb::getNumberString(vector<FiguredBassNumber*> numbers) {
	sort(numbers.begin(), numbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->getNumberB7() > b->getNumberB7();
	});
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

FiguredBassNumber::FiguredBassNumber(int num, string accid, bool showAccid, int voiceIdx, int lineIdx, bool isAtk) {
	number          = num;
	accidentals     = accid;
	voiceIndex      = voiceIdx;
	lineIndex       = lineIdx;
	showAccidentals = showAccid;
	isAttack        = isAtk;
};

string FiguredBassNumber::toString(bool compoundQ, bool accidentalsQ) {
	int num = (compoundQ) ? getNumberB7() : number;
	string accid = (accidentalsQ && showAccidentals) ? accidentals : "";
	return num > 0 ? to_string(num) + accid : "";
};

int FiguredBassNumber::getNumberB7() {
	int num = (number > 9) ? number % 7 : number;
	if(number > 9 && number % 7 == 0) {
		num = 7;
	}
	return (number > 8 && num == 1) ? 8 : num;
};

FiguredBassAbbr::FiguredBassAbbr(string s, vector<int> n) {
	str = s;
	numbers = n;
};

// END_MERGE

} // end namespace hum
