#include "tool-figuredbass.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>

using namespace std;

namespace hum {

// START_MERGE

Tool_figuredbass::Tool_figuredbass(void) {
	define("c|compound=b", "output compound intervals for intervals bigger than 9");
	define("a|accidentals=b", "display accidentals in figured bass output");
	define("b|base=i:0", "number of the base voice/spine");
	define("i|intervallsatz=b", "display intervals under their voice and not under the lowest staff");
	define("s|sort=b", "sort figured bass numbers by interval size and not by voice index");
	define("l|lowest=b", "use lowest note as base note; -b flag will be ignored");
}

bool Tool_figuredbass::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
	 status &= run(infiles[i]);
	}
	return status;
}

bool Tool_figuredbass::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
	 getAllText(out);
	} else {
	 out << infile;
	}
	return status;
}

bool Tool_figuredbass::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
	 getAllText(out);
	} else {
	 out << infile;
	}
	return status;
}

bool Tool_figuredbass::run(HumdrumFile &infile) {

	compoundQ = getBoolean("compound");
	accidentalsQ = getBoolean("accidentals");
	baseQ = getInteger("base");
	intervallsatzQ = getBoolean("intervallsatz");
	sortQ = getBoolean("sort");
	lowestQ = getBoolean("lowest");

	NoteGrid grid(infile);

	vector<FiguredBassNumber*> numbers;

	vector<HTp> kernspines = infile.getKernSpineStartList();

	for (int i=0; i<(int)grid.getSliceCount(); i++) {
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
			numbers.push_back(number);
		}
	}

	if(intervallsatzQ) {
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount(), compoundQ, accidentalsQ, sortQ);
			if(voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
			} else {
				int trackIndex = kernspines[voiceIndex]->getTrack();
				infile.appendDataSpine(trackData, ".", "**fb");
			}
		}
	} else {
		vector<string> trackData = getTrackData(numbers, infile.getLineCount(), compoundQ, accidentalsQ, sortQ);
		if(baseQ + 1 < grid.getVoiceCount()) {
			int trackIndex = kernspines[baseQ + 1]->getTrack();
			infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
		} else {
			infile.appendDataSpine(trackData, ".", "**fb");
		}
	}

	return true;
};

vector<string> Tool_figuredbass::getTrackData(vector<FiguredBassNumber*> numbers, int lineCount, bool compoundQ, bool accidentalsQ, bool sortQ) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLine(numbers, i);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers, compoundQ, accidentalsQ, sortQ);
		}
	}

	return trackData;
};

vector<string> Tool_figuredbass::getTrackDataForVoice(int voiceIndex, vector<FiguredBassNumber*> numbers, int lineCount, bool compoundQ, bool accidentalsQ, bool sortQ) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLineAndVoice(numbers, i, voiceIndex);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers, compoundQ, accidentalsQ, sortQ);
		}
	}

	return trackData;
};

FiguredBassNumber* Tool_figuredbass::createFiguredBassNumber(NoteCell* base, NoteCell* target, string keySignature) {

	int basePitch = base->getSgnDiatonicPitch();
	int targetPitch = target->getSgnDiatonicPitch();
	int num = (basePitch == 0 || targetPitch == 0) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;

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

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, showAccid, target->getVoiceIndex(), target->getLineIndex());

	return number;
};

vector<FiguredBassNumber*> Tool_figuredbass::filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex;
	});

	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
    	return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};

vector<FiguredBassNumber*> Tool_figuredbass::filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex, voiceIndex](FiguredBassNumber* num) {
		return num->lineIndex == lineIndex && num->voiceIndex == voiceIndex;
	});

	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
    	return a->voiceIndex > b->voiceIndex; 
	});

	return filteredNumbers;
};

string Tool_figuredbass::formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers, bool compoundQ, bool accidentalsQ, bool sortQ) {
	if(sortQ) {
		sort(numbers.begin(), numbers.end(), [compoundQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return (compoundQ) ? a->getNumberB7() > b->getNumberB7() : a->number > b->number;

		});
	}
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: numbers) {
		string num = number->toString(compoundQ, accidentalsQ);
		if(num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += num;
		}
	}
	return str;
};

string Tool_figuredbass::getKeySignature(HumdrumFile& infile, int lineIndex) {
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

FiguredBassNumber::FiguredBassNumber(int num, string accid, bool showAccid, int voiceIdx, int lineIdx) {
	number = num;
	accidentals = accid;
	voiceIndex = voiceIdx;
	lineIndex = lineIdx;
	showAccidentals = showAccid;
};

string FiguredBassNumber::toString(bool compoundQ, bool accidentalsQ) {
	int num = (compoundQ) ? getNumberB7() : number;
	string accid = (accidentalsQ && showAccidentals) ? accidentals : "";
	return num > 0 ? to_string(num) + accid : "";
};

int FiguredBassNumber::getNumberB7() {
	int num = (number > 9) ? number % 7 : number;
	return (number > 8 && num == 1) ? 8 : num;
};


// END_MERGE

} // end namespace hum
