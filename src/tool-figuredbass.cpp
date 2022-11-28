#include "tool-figuredbass.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

Tool_figuredbass::Tool_figuredbass(void) {
	define("c|compound=b", "output compound intervals for intervals bigger than 9");
	define("A|no-accidentals=b", "ignore accidentals in figured bass output");
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
	noAccidentalsQ = getBoolean("no-accidentals");
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
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if(j == usedBaseVoiceIndex) {
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);
			FiguredBassNumber* number = createFiguredBassNumber(baseCell, targetCell);
			numbers.push_back(number);
		}
	}

	if(intervallsatzQ) {
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount(), compoundQ, noAccidentalsQ, sortQ);
			if(voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
			} else {
				int trackIndex = kernspines[voiceIndex]->getTrack();
				infile.appendDataSpine(trackData, ".", "**fb");
			}
		}
	} else {
		vector<string> trackData = getTrackData(numbers, infile.getLineCount(), compoundQ, noAccidentalsQ, sortQ);
		if(baseQ + 1 < grid.getVoiceCount()) {
			int trackIndex = kernspines[baseQ + 1]->getTrack();
			infile.insertDataSpineBefore(trackIndex, trackData, ".", "**fb");
		} else {
			infile.appendDataSpine(trackData, ".", "**fb");
		}
	}

	return true;
};

vector<string> Tool_figuredbass::getTrackData(vector<FiguredBassNumber*> numbers, int lineCount, bool compoundQ, bool noAccidentalsQ, bool sortQ) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLine(numbers, i);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers, compoundQ, noAccidentalsQ, sortQ);
		}
	}

	return trackData;
};

vector<string> Tool_figuredbass::getTrackDataForVoice(int voiceIndex, vector<FiguredBassNumber*> numbers, int lineCount, bool compoundQ, bool noAccidentalsQ, bool sortQ) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLineAndVoice(numbers, i, voiceIndex);
		if(sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers, compoundQ, noAccidentalsQ, sortQ);
		}
	}

	return trackData;
};

FiguredBassNumber* Tool_figuredbass::createFiguredBassNumber(NoteCell* base, NoteCell* target) {

	int basePitch = base->getSgnDiatonicPitch();
	int targetPitch = target->getSgnDiatonicPitch();
	int num = (basePitch == 0 || targetPitch == 0) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;
	string accid = "";
	if(target->getSgnKernPitch().find("-") != string::npos) {
		accid = '-';
	} else if(target->getSgnKernPitch().find("#") != string::npos) {
		accid = '#';
	}

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, target->getVoiceIndex(), target->getLineIndex());

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

string Tool_figuredbass::formatFiguredBassNumbers(vector<FiguredBassNumber*> numbers, bool compoundQ, bool noAccidentalsQ, bool sortQ) {
	if(sortQ) {
		sort(numbers.begin(), numbers.end(), [compoundQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return (compoundQ) ? a->getNumberB7() > b->getNumberB7() : a->number > b->number;

		});
	}
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: numbers) {
		string num = number->toString(compoundQ, noAccidentalsQ);
		if(num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += num;
		}
	}
	return str;
};

FiguredBassNumber::FiguredBassNumber(int num, string accid, int voiceIdx, int lineIdx) {
	number = num;
	accidentals = accid;
	voiceIndex = voiceIdx;
	lineIndex = lineIdx;
};

string FiguredBassNumber::toString(bool compoundQ, bool noAccidentalsQ) {
	int num = (compoundQ) ? getNumberB7() : number;
	string accid = (!noAccidentalsQ) ? accidentals : "";
	return num > 0 ? to_string(num) + accid : "";
};

int FiguredBassNumber::getNumberB7() {
	int num = (number > 9) ? number % 7 : number;
	return (number > 8 && num == 1) ? 8 : num;
};


// END_MERGE

} // end namespace hum
