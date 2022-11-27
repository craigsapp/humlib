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

	vector<string> data;
	data.resize(infile.getLineCount());

	vector<HTp> kernspines = infile.getKernSpineStartList();

	int baseVoiceIndex = baseQ;

	for (int i=0; i<(int)grid.getSliceCount(); i++) {
		NoteCell* baseCell = grid.cell(baseVoiceIndex, i);
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			if(j == baseVoiceIndex) {
				continue;
			}
			NoteCell* targetCell = grid.cell(j, i);
			int basePitch = baseCell->getSgnDiatonicPitch();
			int targetPitch = targetCell->getSgnDiatonicPitch();
			int num = (basePitch == 0 || targetPitch == 0) ? 0 : abs(abs(targetPitch) - abs(basePitch)) + 1;
			if(num > 9 && !nonCompoundIntervalsQ) {
				num = num % 7;
			}
			if(num > 0) {
				string accid = "";
				if (!noAccidentalsQ) {
					// TODO improve for double sharps and double flats
					if(targetCell->getSgnKernPitch().find("-") != string::npos) {
						accid = '-';
					} else if(targetCell->getSgnKernPitch().find("#") != string::npos) {
						accid = '#';
					}
				}
				
				data[baseCell->getLineIndex()] = accid + to_string(num) + " " + data[baseCell->getLineIndex()];
			}
		}
	}

	if(baseVoiceIndex < grid.getVoiceCount()) {
		int trackIndex = kernspines[baseVoiceIndex + 1]->getTrack();
		infile.insertDataSpineBefore(trackIndex, data, ".", "**fb");
	} else {
		int trackIndex = kernspines[baseVoiceIndex]->getTrack();
		// TODO segmentation fault
		infile.appendDataSpine(data, ".", "**fb");
	}


	return true;

};


// END_MERGE

} // end namespace hum
