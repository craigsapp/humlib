//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Programmer:    Daniel Shanahan
// Creation Date: Wed Jun 26 13:12:48 CEST 2019
// Last Modified: Wed Jun 26 17:35:34 CEST 2019
// Filename:      cli/npvi.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/npvi.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Calculate nPVI from melodies.  See:
//    Patel, Iversen & Rosenberg. "Comparing the rhythm and melody of"
//    speech and music: The case of British English and French".  
//    JASA 119(5), May 2006, pp. 3034-3047.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile         (HumdrumFile& infile, Options& options);
double calculateNpviRhythm (HumdrumFile& infile, Options& option);
double calculateCvPitch    (HumdrumFile& infile, Options& options);
string getReference        (HumdrumFile& infile, const string& targetKey);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("A|all=b", "extract all features");
	options.define("k|kern=i:1", "kern spine to analyze");
	options.define("f|filename=b", "print file name");
	options.define("n|nationality=b", "print nationality");
	options.define("p|population=b", "use population standard deviation");
	options.define("c|cv=b", "print CV analysis");
	options.define("debug=b", "print debugging info");
	options.define("t|timbre", "get timbre");
	options.define("d|date", "get date");
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	double npvi       = calculateNpviRhythm(infile, options);
	bool cvQ          = false;
	bool filenameQ    = false;
	bool nationalityQ = false;
	bool timbreQ = false;
	bool dateQ = false;
	if (options.getBoolean("all")) {
		cvQ = true;
		filenameQ = true;
		nationalityQ = true;
		timbreQ = true;
		dateQ = true;
	} else {
		cvQ = options.getBoolean("cv");
		filenameQ = options.getBoolean("filename");
		nationalityQ = options.getBoolean("nationality");
		timbreQ = options.getBoolean("timbre");
		dateQ = options.getBoolean("date");
	}
	if (filenameQ) {
		cout << infile.getFilename() << "\t";
	}
	cout << npvi;
	if (cvQ) {
		cout << "\t" << calculateCvPitch(infile, options);
	}
	if (nationalityQ) {
		cout << "\t" << getReference(infile, "CNT");
	}
	if (timbreQ) {
		cout << "\t" << getReference(infile, "timbre");
	}
	if (dateQ) {
		cout << "\t" << getReference(infile, "CDT");
	}
	cout << endl;
}



//////////////////////////////
//
// getReference -- Get the reference record value for a given
//      reference record key.  This is used to extract the
//      nationality of the composer (where the key is "CNT").
//

string getReference(HumdrumFile& infile, const string& targetKey) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReference()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != targetKey) {
			continue;
		}
		return infile[i].getReferenceValue();
	}
	return ".";
}



//////////////////////////////
//
// calculateNpviRhythm -- Extract IOIs of note attacks to use as
//    input to the nPVI calculation.
//

double calculateNpviRhythm(HumdrumFile& infile, Options& options) {
	vector<HTp> spinestarts = infile.getKernSpineStartList();
	int kspine = options.getInteger("kern");
	int kindex = kspine - 1;
	if (kindex < 0) {
		kindex = 0;
	} else if (kindex > (int)spinestarts.size() - 1) {
		kindex = (int)spinestarts.size() - 1;
	}
	HTp token = spinestarts[kindex];
	vector<double> durations;
	durations.reserve(infile.getLineCount());
	double pdur = 0.0;
	bool initialized =  false;
	while (token) {
		if (!token->isData()) {
			token = token->getNextToken();
			continue;
		}
		if (token->isNull()) {
			token = token->getNextToken();
			continue;
		}
		if (token->isGrace()) {
			token = token->getNextToken();
			continue;
		}
		double dur = token->getDuration().getFloat();
		if (token->isNoteAttack()) {
			if (initialized) {
				durations.push_back(pdur);
			}
			initialized = true;
			pdur = dur;
		} else {
			pdur += dur;
		}
		token = token->getNextToken();
	}
	if (pdur > 0) {
		durations.push_back(pdur);
	}
	if (options.getBoolean("debug")) {
		cerr << "\nDURATIONS =";
		for (int i=0; i<(int)durations.size(); i++) {
			cerr << " " << durations[i];
		}
		cerr << endl;
	}
	return Convert::nPvi(durations);
}



//////////////////////////////
//
// calculateCvPitch -- Calculate the coefficient of variation (CV) for 
//    pitch intervals in a melodic sequence.  CV is the standard deviation
//    of a sequence divided by the mean of the sequence.  Pitch is extracted
//    MIDI key nummbers, and then intervals are calculated as differences 
//    in successive pitches. If there are chords, then then the first note
//    in the chord token will be used (use the chord tool to sort the order
//    of the pitches in a specific way in such cases).
//

double calculateCvPitch(HumdrumFile& infile, Options& options) {
	vector<HTp> spinestarts = infile.getKernSpineStartList();
	int kspine = options.getInteger("kern");
	int kindex = kspine - 1;
	if (kindex < 0) {
		kindex = 0;
	} else if (kindex > (int)spinestarts.size() - 1) {
		kindex = (int)spinestarts.size() - 1;
	}
	HTp token = spinestarts[kindex];
	vector<double> intervals;
	intervals.reserve(infile.getLineCount());
	double ppitch = -100.0;
	while (token) {
		if (!token->isData()) {
			token = token->getNextToken();
			continue;
		}
		if (token->isNull()) {
			token = token->getNextToken();
			continue;
		}
		if (token->isGrace()) {
			token = token->getNextToken();
			continue;
		}
		if (!token->isNoteAttack()) {
			token = token->getNextToken();
			continue;
		}
		double pitch = Convert::kernToMidiNoteNumber(token);
		if (ppitch < 0) {
			ppitch = pitch;
			token = token->getNextToken();
			continue;
			
		}
		double interval = fabs((double)pitch - (double)ppitch);
		intervals.push_back(interval);
		token = token->getNextToken();
		ppitch = pitch;
	}

	if (options.getBoolean("debug")) {
		cerr << "INTERVALS =";
		for (int i=0; i<(int)intervals.size(); i++) {
			cerr << " " << intervals[i];
		}
		cerr << endl;
	}

	if (options.getBoolean("population")) {
		return Convert::coefficientOfVariationPopulation(intervals);
	} else {
		return Convert::coefficientOfVariationSample(intervals);
	}
}



