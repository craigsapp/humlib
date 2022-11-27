#include "tool-figuredbass.h"
#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE

Tool_figuredbass::Tool_figuredbass(void) {
	define("C|non-compound=b", "output compound intervals as non-compound intervals");
	define("A|no-accidentals=b", "ignore accidentals in figured bass output");
	define("b|base=i:0", "number of the base voice/spine");
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

	nonCompoundIntervalsQ = getBoolean("non-compound");
	noAccidentalsQ = getBoolean("no-accidentals");
	baseQ = getInteger("base");

	NoteGrid grid(infile);

	vector<FiguredBassTrack*> data;

	vector<HTp> kernspines = infile.getKernSpineStartList();

	int baseVoiceIndex = baseQ;
	FiguredBassTrack* track = new FiguredBassTrack(baseVoiceIndex);
	data.push_back(track);

	for (int i=0; i<(int)grid.getSliceCount(); i++) {
		NoteCell* baseCell = grid.cell(track->voiceIndex, i);
		FiguredBassSlice* slice = new FiguredBassSlice(baseCell->getLineIndex());
		track->add(slice);
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if(j == baseVoiceIndex) {
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);


			FiguredBassNumber* number = createFiguredBassNumber(baseCell, targetCell);
			slice->add(number);
		}
	}

 	for (FiguredBassTrack* track: data) {
		if(baseVoiceIndex < grid.getVoiceCount()) {
			int trackIndex = kernspines[baseVoiceIndex + 1]->getTrack();
			infile.insertDataSpineBefore(trackIndex, getTrackData(track, infile.getLineCount(), nonCompoundIntervalsQ, noAccidentalsQ), ".", "**fb");
		} else {
			// TODO segmentation fault
			infile.appendDataSpine(getTrackData(track, infile.getLineCount(), nonCompoundIntervalsQ, noAccidentalsQ), ".", "**fb");
		}
	}

	return true;
};

vector<string> Tool_figuredbass::getTrackData(FiguredBassTrack* track, int lineCount, bool nonCompoundIntervalsQ, bool noAccidentalsQ) {
	vector<string> trackData;
	trackData.resize(lineCount);

 	for (FiguredBassSlice* slice: track->slices) {
		trackData[slice->lineIndex] = slice->toString(nonCompoundIntervalsQ, noAccidentalsQ);
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

	FiguredBassNumber* number = new FiguredBassNumber(num, accid);

	return number;
};

FiguredBassTrack::FiguredBassTrack(int index) {
	voiceIndex = index;
};

void FiguredBassTrack::add(FiguredBassSlice* slice) {
	slices.push_back(slice);
};

FiguredBassSlice::FiguredBassSlice(int index) {
	lineIndex = index;
};

void FiguredBassSlice::add(FiguredBassNumber* number) {
	numbers.insert(numbers.begin(), number);
	// numbers.push_back(number);
};

string FiguredBassSlice::toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ) {
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: numbers) {
		string num = number->toString(nonCompoundIntervalsQ, noAccidentalsQ);
		if(num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += number->toString(nonCompoundIntervalsQ, noAccidentalsQ);
		}
	}
	return str;
};

FiguredBassNumber::FiguredBassNumber(int num, string accid) {
	number = num;
	accidentals = accid;
};

string FiguredBassNumber::toString(bool nonCompoundIntervalsQ, bool noAccidentalsQ) {
	int num = (number > 9 && !nonCompoundIntervalsQ) ? number % 7 : number;
	string accid = (!noAccidentalsQ) ? accidentals : "";
	return num > 0 ? to_string(num) + accid : "";
};



// END_MERGE

} // end namespace hum
