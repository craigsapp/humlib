//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun  1 14:52:42 PDT 2024
// Last Modified: Sat Jun  1 21:51:14 PDT 2024
// Filename:      cli/hum2epr.cpp
// URL:           https://github.com/craigsapp/g/blob/master/src/hum2epr.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Converter from Humdrum to Extended Piano Roll data for use with pandas or
//                similar 2-D data structure.
//

#include "humlib.h"

#include <cctype>
#include <ctime>
#include <iostream>
#include <map>

using namespace std;
using namespace hum;

// Function definitions:
void   processFile           (HumdrumFile& infile);
void   printHeader           (HumdrumFile& infile);
void   printHeaderReferences (HumdrumFile& infile);
void   getLabels             (vector<string>& labels, HumdrumFile& infile);
string getTrackLabel         (vector<HTp>& kstarts, int index);
void   printData             (HumdrumFile& infile);
void   printChordNotes       (HTp token);
void   printFooter           (HumdrumFile& infile);
void   printFooterReferences (HumdrumFile& infile);
bool   isInterestingReferenceRecord(const string& key);
void   printMeasure          (HumdrumFile& infile, int index);
void   printExpansion        (HumdrumFile& infile, int index);
void   printLabel            (HumdrumFile& infile, int index);
void   printTimeSignature    (HumdrumFile& infile, int index, int field);
void   printTempo            (HumdrumFile& infile, int index, int field);
void   printKeySignature     (HumdrumFile& infile, int index, int field);
void   printKeyDesignation   (HumdrumFile& infile, int index, int field);
void   printExpansionList    (HumdrumFile& infile, int lineIndex, int fieldIndex);
void   printExpansionLabel   (HumdrumFile& infile, int lineIndex, int fieldIndex);
void   prepareSeconds        (vector<double>& seconds, map<HumNum, double>& timemap, HumdrumFile& infile);
string getDate               (void);
void   applyThru             (HumdrumFile& infile);
vector<string> getExpansionLabels(HumdrumFile& infile);
string getExpansionList(HumdrumFile& infile);
map<string, string> getEmbeddedExpansionLists(HumdrumFile& infile);

// User-interface options:
Options options;
double m_velocity    = 1.0;   // used with --velocity
bool   m_barnumsQ    = true;  // used with -M (no measure numbers)
bool   m_keysQ       = true;  // used with -K (no key info markup)
bool   m_keysigsQ    = true;  // used with -K (no key info markup)
bool   m_labelsQ     = true;  // used with -L (no expansion label markup)
bool   m_expansionsQ = true;  // used with -E (no expansion lists markup)
bool   m_referencesQ = true;  // used with -R (no reference records)
bool   m_temposQ     = true;  // used with -T (no tempo markup)
bool   m_timesigsQ   = true;  // used with -I (no time signature markup)
bool   m_secondsQ    = false; // used with -s (start times and durations in seconds)
string m_variant;     // used with -v (thru label expansion variant)
string m_realization; // used with -r (thru label realization)

// Other global variables:
vector<string>      m_labels;
vector<double>      m_seconds;
map<HumNum, double> m_timemap;


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	options.define("velocity=d:1.0",         "default MIDI note attack velocity");
	options.define("s|seconds=b",            "timeline in seconds rather than quarter notes");
	options.define("v|variant=s:",           "Choose expansion list variant");
	options.define("r|realization=s:",       "Create expansion list");

	// metadata comments:
	options.define("E|no-expansions=b",      "do not export label expansion lists");
	options.define("I|no-time-signatures=b", "do not export time signatures");
	options.define("K|no-key-info=b",        "do not export tempo markings");
	options.define("L|no-labels=b",          "do not export label markings");
	options.define("M|no-measure-numbers=b", "do not add measure numbers comments");
	options.define("R|no-references=b",      "do not export reference records");
	options.define("T|no-tempos=b",          "do not export tempo markings");
	options.define("Z|no-markup=b",          "do not export any metadata markup");

	options.process(argc, argv);

	m_velocity = options.getDouble("velocity");
	m_secondsQ = options.getBoolean("seconds");

	if (options.getBoolean("no-markup")) {
		m_barnumsQ    = false;
		m_labelsQ     = false;
		m_expansionsQ = false;
		m_keysQ       = false;
		m_keysigsQ    = false;
		m_temposQ     = false;
		m_timesigsQ   = false;
	} else {
		m_barnumsQ    = !options.getBoolean("no-measure-numbers");
		m_labelsQ     = !options.getBoolean("no-labels");
		m_expansionsQ = !options.getBoolean("no-expansions");
		if (options.getBoolean("no-key-info")) {
			m_keysQ    = false;
			m_keysigsQ = false;
		}
		m_temposQ     = !options.getBoolean("no-tempos");
		m_timesigsQ   = !options.getBoolean("no-time-signatures");
	}
	m_referencesQ = !options.getBoolean("no-references");

	m_variant     = options.getString("variant");
	m_realization = options.getString("realization");

	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// applyThru -- expand score (if expansion-list is present).
//

void applyThru(HumdrumFile& infile) {
	Tool_thru thru;
	vector<string> argv;

	argv.push_back("thru"); // name of program (placeholder)
	if (!m_variant.empty()) {
		string option = "-v " + m_variant;
		argv.push_back(option);
	} else if (!m_realization.empty()) {
		string option = "-r " + m_realization;
		argv.push_back(option);
	}
	thru.process(argv);
	thru.run(infile);
	if (thru.hasError()) {
		cerr << "Error processing data: " << thru.getError() << endl;
		exit(1);
	}
	string results = thru.getHumdrumText();
	infile.readString(results);
}



//////////////////////////////
//
// printData --
//

void printData(HumdrumFile& infile) {
	getLabels(m_labels, infile);
	if (m_secondsQ) {
		prepareSeconds(m_seconds, m_timemap, infile);
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (m_barnumsQ && infile[i].isBarline()) {
			printMeasure(infile, i);
		}
		if (m_keysigsQ) {
			int field = infile[i].isKeySignature();
			if (field > 0) {
				printKeySignature(infile, i, field-1);
			}
		}
		if (m_keysQ) {
			int field = infile[i].isKeyDesignation();
			if (field > 0) {
				printKeyDesignation(infile, i, field-1);
			}
		}
		if (m_timesigsQ) {
			int field = infile[i].isTimeSignature();
			if (field > 0) {
				printTimeSignature(infile, i, field-1);
			}
		}
		if (m_temposQ) {
			int field = infile[i].isTempo();
			if (field > 0) {
				printTempo(infile, i, field-1);
			}
		}
		if (m_labelsQ) {
			int field = infile[i].isExpansionLabel();
			if (field > 0) {
				printExpansionLabel(infile, i, field-1);
			}
		}
		if (m_expansionsQ) {
			int field = infile[i].isExpansionList();
			if (field > 0) {
				printExpansionList(infile, i, field-1);
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=infile[i].getFieldCount() - 1; j >= 0; j--) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			printChordNotes(token);
		}
	}
}



//////////////////////////////
//
// prepareSeconds -- Create score-line to seconds mapping as well
//     as absolute quarter note to seconds lookup table.
//

void prepareSeconds(vector<double>& seconds, map<HumNum, double>& timemap, HumdrumFile& infile) {
	timemap.clear();
	timemap[0] = 0.0;
	double currentTime = 0.0;
	double currentTempo = 120.0;
	seconds.resize(infile.getLineCount());
	fill(seconds.begin(), seconds.end(), 0.0);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			seconds[i] = currentTime;
			continue;
		}
		if (infile[i].isInterpretation()) {
			int field = infile[i].isTempo();
			if (field) {
				HTp token = infile.token(i, field-1);
				HumRegex hre;
				if (hre.search(token, "^\\*MM(\\d+\\.?\\d*)$")) {
					currentTempo = hre.getMatchDouble(1);
					seconds[i] = currentTime;
					continue;
				}
			}
			seconds[i] = currentTime;
			continue;
		}
		if (!infile[i].isData()) {
			seconds[i] = currentTime;
			continue;
		}
		HumNum lineDur = infile[i].getDuration();
		double deltaTime = lineDur.getFloat() * 60.0 / currentTempo;
		seconds[i] = currentTime;
		timemap[infile[i].getDurationFromStart()] = currentTime;
		currentTime += deltaTime;
	}
	timemap[infile[infile.getLineCount() - 1].getDurationFromStart()] = currentTime;

	// for (int i=0; i<infile.getLineCount(); i++) {
	//	cout << "#\t" << infile[i].getDurationFromStart() << "\t" << m_seconds[i] << "\t" << infile[i] << endl;
	// }
	// cout << "#\n# TIMEMAP\n#\n";
	// for (const auto& pair : timemap) {
	//	cout << "#\t" << pair.first << ":\t" << pair.second << std::endl;
	// }
}



//////////////////////////////
//
// printMeasure -- As a comment.
//

void printMeasure(HumdrumFile& infile, int index) {
	HumRegex hre;
	HTp token = infile[index].token(0);
	cout << "# measure";
	int barnum = infile[index].getBarNumber();
	if (barnum >= 0) {
		cout << " " << barnum;
	} else {
		if (hre.search(token, "^==")) {
			cout << " final";
		} else if (hre.search(token, "=:[|!]+$")) {
			cout << " repeat-back";
		} else if (hre.search(token, "=[!|]+:$")) {
			cout << " repeat-forward";
		} else if (hre.search(token, "=:[!|]+:$")) {
			cout << " repeat-both";
		}
	}
	if (hre.search(token, "^(==[^\\d]*)$")) {
		cout << " " << hre.getMatch(1);
	} else if (hre.search(token, "=\\d*([^\\d]+)")) {
		cout << " " << hre.getMatch(1);
	}
	cout << endl;
}



//////////////////////////////
//
// printTimeignature --
//

void printTimeSignature(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "^\\*M(\\d+/.*)")) {
		cout << "#timesig: " << hre.getMatch(1) << endl;
	}
}



//////////////////////////////
//
// printExpansionLabel --
//

void printExpansionLabel(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "^\\*>(.*)")) {
		string label = hre.getMatch(1);
		if (!label.empty()) {
			cout << "#expansion-label: " << hre.getMatch(1) << endl;
		}
	}
}



//////////////////////////////
//
// printExpansionList --
//

void printExpansionList(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "^\\*>(.*)")) {
		string list = hre.getMatch(1);
		if (!list.empty()) {
			cout << "#expansion-list: " << hre.getMatch(1) << endl;
		}
	}
}



//////////////////////////////
//
// printTempo --
//

void printTempo(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "^\\*MM(\\d+\\.?\\d*$)")) {
		cout << "#quarter: " << hre.getMatch(1) << endl;
	}
}



//////////////////////////////
//
// printKeySignature --
//

void printKeySignature(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "\\[(.*?)\\]")) {
		cout << "#keysig: " << hre.getMatch(1) << endl;
	}
}



//////////////////////////////
//
// printKeyDesignation --
//

void printKeyDesignation(HumdrumFile& infile, int lineIndex, int fieldIndex) {
	HTp token = infile.token(lineIndex, fieldIndex);
	HumRegex hre;
	if (hre.search(token, "^\\*([a-gA-G])([#-]*):(.*)$")) {
		string tonic      = hre.getMatch(1);
		string accidental = hre.getMatch(2);
		string mode       = hre.getMatch(3);
		cout << "#key: " << (char)toupper(tonic[0]);
		if (accidental == "#") {
			cout << "-sharp";
		} else if (accidental == "-") {
			cout << "-flat";
		} else if (accidental == "##") {
			cout << "-double-sharp";
		} else if (accidental == "--") {
			cout << "-double-flat";
		}
		if (mode == "") {
			if (isupper(tonic[0])) {
				cout << " Major";
			} else {
				cout << " Minor";
			}
		} else if (mode == "ion") {
			cout << " Ionian";
		} else if (mode == "dor") {
			cout << " Dorian";
		} else if (mode == "phr") {
			cout << " Phrygian";
		} else if (mode == "lyd") {
			cout << " Lydian";
		} else if (mode == "mix") {
			cout << " Mixolydian";
		} else if (mode == "aeo") {
			cout << " Aeolean";
		} else if (mode == "loc") {
			cout << " Locrian";
		}
		cout << endl;
	}
}



//////////////////////////////
//
// getLabels --
//

void getLabels(vector<string>& labels, HumdrumFile& infile) {
	int maxtrack = infile.getMaxTrack();
	labels.clear();
	labels.resize(maxtrack+1);
	vector<HTp> kstarts = infile.getKernSpineStartList();
	for (int i=0; i<(int)kstarts.size(); i++) {
		int track = kstarts[i]->getTrack();
		string label = getTrackLabel(kstarts, i);
		labels.at(track) = label;
	}
}



//////////////////////////////
//
// getTrackLabel --
//

string getTrackLabel(vector<HTp>& kstarts, int index) {
	HTp kstart = kstarts.at(index);
	HTp current = kstart->getNextToken();

	HTp iCode   = NULL;  // such as *Ivioln (for violin)
	HTp iNumber = NULL;  // such as *I#2    (for second instrument, such as violin 2)
	HTp iName   = NULL;  // text string for name printed on score
	HTp iAbbr   = NULL;  // instrument abbreviation for systems other than the first
	HTp staff   = NULL;  // staff number

	while (current) {
		if (current->isData()) {
			break;
		}
		if (current->isInstrumentCode()) {
			iCode = current;
		} else if (current->isInstrumentNumber()) {
			iNumber = current;
		} else if (current->isInstrumentName()) {
			iName = current;
		} else if (current->isInstrumentAbbreviation()) {
			iAbbr = current;
		}
		current = current->getNextToken();
	}

	HumRegex hre;
	string output;
	if (iName) {
		string name = iName->substr(3);
		hre.replaceDestructive(name, "", "\"", "g");
		output = name;
	} else if (iAbbr) {
		string abbr = iAbbr->substr(3);
		hre.replaceDestructive(abbr, "", "\"", "g");
		output = abbr;
	} else if (iCode) {
		output = iCode->substr(2);
		if (iNumber) {
			output += iNumber->substr(3);
		}
	} else if (staff) {
		output = staff->substr(1);
	}

	if (output.empty()) {
		// Can't find a name so
		output = "track " + to_string(kstart->getTrack());
	}

	return output;
}



//////////////////////////////
//
// printChordNotes --
//

void printChordNotes(HTp token) {
	vector<string> subtokens = token->getSubtokens();
	double startTime = 0.0;
	if (m_secondsQ) {
		startTime = m_seconds.at(token->getLineIndex());
	} else {
		startTime = token->getDurationFromStart().getFloat();
	}
	for (int i=0; i<(int)subtokens.size(); i++) {
		if (subtokens[i].find("_") != string::npos) {
			// tie continuation note
			continue;
		}
		if (subtokens[i].find("]") != string::npos) {
			// tie ending note
			continue;
		}
		double duration;
		if (m_secondsQ) {
			HumNum startQuarter = token->getDurationFromStart();
			HumNum endQuarter   = startQuarter + token->getDuration();
			double startSeconds = m_timemap[startQuarter];
			double endSeconds   = m_timemap[endQuarter];
			duration = endSeconds - startSeconds;
		} else {
			if (subtokens[i].find("[") != string::npos) {
				// tie starting note
				duration = token->getTiedDuration().getFloat();
				// There will be a limitation if separate chord
				// notes have different tie states, i.e. all notes
				// in a chord must have the same start/end tie states
				// currently.
			} else {
				duration = token->getDuration().getFloat();
			}
		}
		int midiPitch = Convert::kernToMidiNoteNumber(subtokens[i]);
		int track = token->getTrack();

		cout << startTime<< ";\t";
		cout << duration << ";\t";
		cout << midiPitch << ";\t";
		cout << m_velocity << ";\t";
		cout << '"' << m_labels.at(track) << '"';
		cout << endl;
	}
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	printHeader(infile);

	applyThru(infile);
	printData(infile);

	printFooter(infile);
}



//////////////////////////////
//
// printHeader --
//

void printHeader(HumdrumFile& infile) {
	if (m_referencesQ) {
		printHeaderReferences(infile);
	}
	cout << "\"Start\";\t";
	cout << "\"Duration\";\t";
	cout << "\"Pitch\";\t";
	cout << "\"Velocity\";\t";
	cout << "\"Label\"\t";
	cout << endl;
}



//////////////////////////////
//
// printHeaderReferences --
//

void printHeaderReferences(HumdrumFile& infile) {
	cout << "####################################################" << endl;
	cout << "##converter: hum2epr" << endl;
	cout << "##conversion-date: " << getDate() << endl;
	vector<string> expansionLabels = getExpansionLabels(infile);
	if (!expansionLabels.empty()) {
		cout << "##expansion-labels: ";
		for (int i=0; i<(int)expansionLabels.size(); i++) {
			cout << expansionLabels[i];
			if (i < (int)expansionLabels.size() - 1) {
				cout << ", ";
			}
		}
		cout << endl;
		string expansionList = getExpansionList(infile);
		if (!expansionList.empty()) {
			cout << "##expansion: " << expansionList << endl;
		} else {
			cout << "##expansion: strange problem" << endl;
		}
	}
	cout << "##timeline: ";
	if (m_secondsQ) {
		cout << "seconds";
	} else {
		cout << "quarters";
	}
	cout << endl;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isReference()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key == "SEGMENT") {
			key = "FILE";
		}
		if (!isInterestingReferenceRecord(key)) {
			continue;
		}
		string value = infile[i].getReferenceValue();
		cout << "##" << key << ": " << value << endl;
	}
	cout << "####################################################" << endl;
}



//////////////////////////////
//
// getExpansionList --
//

string getExpansionList(HumdrumFile& infile) {
	if (!m_realization.empty()) {
		HumRegex hre;
		return hre.replaceCopy(m_realization, ", ", "\\s,\\s", "g");
	}
	map<string, string> lists = getEmbeddedExpansionLists(infile);
	if (lists.empty()) {
		return "";
	}

	if (!m_variant.empty()) {
		return lists[m_variant];
	}

	return lists[""];
}



//////////////////////////////
//
// getEmbeddedExpansionLists --
//

map<string, string> getEmbeddedExpansionLists(HumdrumFile& infile) {
	map<string, string> output;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (!infile[i].isInterpretation()) {
			continue;
		}
		int field = infile[i].isExpansionList();
		if (field > 0) {
			HTp token = infile.token(i, field-1);
			HumRegex hre;
			if (hre.search(token, "^\\*>(.*?)\\[(.*?)\\]$")) {
				string name = hre.getMatch(1);
				string expansion = hre.getMatch(2);
				hre.replaceDestructive(expansion, ", ", "\\s*,\\s*", "g");
				output[name] = expansion;
			}
		}
	}
	return output;
}



//////////////////////////////
//
// getExpansionLabels --
//

vector<string> getExpansionLabels(HumdrumFile& infile) {
	vector<string> output;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isInterpretation()) {
			continue;
		}
		int field = infile[i].isExpansionLabel();
		if (field) {
			HTp token = infile.token(i, field-1);
			string value = token->substr(2);
			if (!value.empty()) {
				output.push_back(value);
			}
		}
	}
	return output;
}



//////////////////////////////
//
// getDate -- Get today's date in YYYY-MM-DD format.
//

string getDate(void) {
	time_t now = std::time(nullptr);
	tm* localtm = std::localtime(&now);

	// Print in YYYY-MM-DD format
	stringstream ss;
	int year  = localtm->tm_year + 1900;
	int month = localtm->tm_mon + 1;
	int day   = localtm->tm_mday;
	ss << year << "-";
	if (month < 10) {
		ss << 0;
	}
	ss << month << "-";
	if (day < 10) {
		ss << 0;
	}
	ss << day;
	return ss.str();
}



//////////////////////////////
//
// isInterestingReferenceRecord -- Returns true if the reference record key
//     is interesting to transfer to the output data.
//

bool isInterestingReferenceRecord(const string& key) {
	if (key.empty()) {
		return false;
	}
	if (key.find("filter") != string::npos) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// printFooter --
//
void printFooter(HumdrumFile& infile) {
	if (m_referencesQ) {
		printFooterReferences(infile);
	}
}



//////////////////////////////
//
// printFooterReferences --
//

void printFooterReferences(HumdrumFile& infile) {
	cout << "####################################################" << endl;
	int counter = 0;
	int index = -1;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (infile[i].isTerminator()) {
			index = i;
			break;
		}
	}
	for (int i=index; i<infile.getLineCount(); i++) {
		string key = infile[i].getReferenceKey();
		if (key == "SEGMENT") {
			key = "FILE";
		}
		if (!isInterestingReferenceRecord(key)) {
			continue;
		}
		counter++;
		string value = infile[i].getReferenceValue();
		cout << "##" << key << ": " << value << endl;
	}
	if (counter > 0) {
		cout << "####################################################" << endl;
	}
}



