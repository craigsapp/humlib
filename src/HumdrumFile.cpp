//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Fri Aug 14 21:57:09 PDT 2015
// Filename:      HumdrumFile.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumdrumFile.h
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Used to store Humdrum text lines from input stream
//                for further parsing.
//

#include "HumdrumFile.h"

#include <sstream>
#include <fstream>

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::HumdrumFile(void) {
	addToTrackStarts(NULL);
}



//////////////////////////////
//
// HumdrumFile::~HumdrumFile --
//

HumdrumFile::~HumdrumFile() { }



//////////////////////////////
//
// HumdrumFile::operator[] -- Negative values will be in reference to the
//    end of the list of lines.
//

HumdrumLine& HumdrumFile::operator[](int index) {
	if (index < 0) {
		index = lines.size() - index;
	}
	if ((index < 0) || (index >= lines.size())) {
		cerr << "Error: invalid index: " << index << endl;
		index = lines.size()-1;
	}
	return *lines[index];
}



//////////////////////////////
//
// HumdrumFile::read -- Load file contents from input stream.
//

bool HumdrumFile::read(istream& infile) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (infile.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine(buffer);
		s->setOwner(this);
		lines.push_back(s);
	}
	if (!analyzeTokens()           ) { return false; }
	if (!analyzeLines()            ) { return false; }
	if (!analyzeSpines()           ) { return false; }
	if (!analyzeLinks()            ) { return false; }
	if (!analyzeTracks()           ) { return false; }
	if (!analyzeGlobalParameters() ) { return false; }
	if (!analyzeLocalParameters()  ) { return false; }
	if (!analyzeTokenDurations()   ) { return false; }
	if (!analyzeRhythm()           ) { return false; }
	if (!analyzeDurationsOfNonRhythmicSpines()) { return false; }
	return true;
}


bool HumdrumFile::read(const char* filename) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return read(cin);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return false;
		}
	}
	int status = read(infile);
	infile.close();
	return status;
}


bool HumdrumFile::read(const string& filename) {
	return read(filename.c_str());
}



//////////////////////////////
//
// HumdrumFile::readNoRhythm -- Load file contents from input stream without
//    parsing the rhythmic information in the file.
//

bool HumdrumFile::readNoRhythm(const string& filename) {
	return readNoRhythm(filename.c_str());
}


bool HumdrumFile::readNoRhythm(const char* filename) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return readNoRhythm(cin);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return false;
		}
	}
	int status = readNoRhythm(infile);
	infile.close();
	return status;
}


bool HumdrumFile::readNoRhythm(istream& infile) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (infile.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine(buffer);
		s->setOwner(this);
		lines.push_back(s);
	}
	if (!analyzeTokens()         ) { return false; }
	if (!analyzeLines()          ) { return false; }
	if (!analyzeSpines()         ) { return false; }
	if (!analyzeLinks()          ) { return false; }
	if (!analyzeTracks()         ) { return false; }
	return true;
}



//////////////////////////////
//
// HumdrumFile::ReadString -- Read contents from a string rather than
//    an istream or filename.
//

bool HumdrumFile::readString(const string& contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}


bool HumdrumFile::readString(const char* contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}



//////////////////////////////
//
// HumdrumFile::ReadStringNoRhythm -- Read contents from a string rather than
//    an istream or filename, but do not parse rhythm.
//

bool HumdrumFile::readStringNoRhythm(const string& contents) {
	stringstream infile;
	infile << contents;
	return readNoRhythm(infile);
}


bool HumdrumFile::readStringNoRhythm(const char* contents) {
	stringstream infile;
	infile << contents;
	return readNoRhythm(infile);
}



//////////////////////////////
//
// HumdrumFile::analyzeTokens -- Generate token array from
//    current state of lines.  If either state is changed, then the
//    other state becomes invalid.  See createLinesFromTokens for
//		regeneration of lines from tokens.
//

bool HumdrumFile::analyzeTokens(void) {
	int i;
	for (i=0; i<lines.size(); i++) {
		lines[i]->createTokensFromLine();
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::createLinesFromTokens -- Generate Humdrum lines from
//   the list of tokens.
//

void HumdrumFile::createLinesFromTokens(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->createLineFromTokens();
	}
}



//////////////////////////////
//
// HumdrumFile::getScoreDuration -- Return the total duration of the score
//		in quarter note units.
//

HumNum HumdrumFile::getScoreDuration(void) const {
	if (lines.size() == 0) {
		return 0;
	}
	return lines.back()->getDurationFromStart();
}



//////////////////////////////
//
// HumdrumFile::getBarline -- Return the given barline.  Negative index
//   accesses from the end of the list.
//

HumdrumLine* HumdrumFile::getBarline(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return NULL;
	}
	if (index >= barlines.size()) {
		return NULL;
	}
	return barlines[index];
}



//////////////////////////////
//
// HumdrumFile::getBarlineCount --
//

int HumdrumFile::getBarlineCount(void) const {
	return barlines.size();
}



///////////////////////////////
//
// HumdrumFile::getBarlineDuration --
//

HumNum HumdrumFile::getBarlineDuration(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return 0;
	}
	HumNum startdur = barlines[index]->getDurationFromStart();
	HumNum enddur;
	if (index + 1 < barlines.size() - 1) {
		enddur = barlines[index+1]->getDurationFromStart();
	} else {
		enddur = getScoreDuration();
	}
	return enddur - startdur;
}



///////////////////////////////
//
// HumdrumFile::getBarlineDurationFromStart -- Return the duration between the
//    start of the Humdrum file and the barline.
//

HumNum HumdrumFile::getBarlineDurationFromStart(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return getScoreDuration();
	}
	return barlines[index]->getDurationFromStart();
}



///////////////////////////////
//
// HumdrumFile::getBarlineDurationToEnd -- Return the duration between the
//    barline and the end of the HumdrumFile.
//

HumNum HumdrumFile::getBarlineDurationToEnd(int index) const {
	if (index < 0) {
		index += barlines.size();
	}
	if (index < 0) {
		return 0;
	}
	if (index >= barlines.size()) {
		return getScoreDuration();
	}
	return barlines[index]->getDurationToEnd();
}



////////////////////////////
//
// HumdrumFile::append -- Add a line to the file's contents.
//

void HumdrumFile::append(const char* line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}


void HumdrumFile::append(const string& line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}



////////////////////////////
//
// HumdrumFile::getLineCount -- Return the number of lines.
//

int HumdrumFile::getLineCount(void) const {
	return lines.size();
}



//////////////////////////////
//
// HumdrumFile::getMaxTrack -- returns the number of primary spines in the data.
//

int HumdrumFile::getMaxTrack(void) const {
	return trackstarts.size() - 1;
}



//////////////////////////////
//
// HumdrumFile::printSpineInfo --
//

ostream& HumdrumFile::printSpineInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printSpineInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDataTypeInfo --
//

ostream& HumdrumFile::printDataTypeInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printDataTypeInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printTrackInfo --
//

ostream& HumdrumFile::printTrackInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printTrackInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDurationInfo --
//

ostream& HumdrumFile::printDurationInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printDurationInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::getTrackStart --
//

HumdrumToken* HumdrumFile::getTrackStart(int track) const {
	if ((track > 0) && (track < trackstarts.size())) {
		return trackstarts[track];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumFile::getTrackEndCount -- return the number of ending tokens
//    for the given track
//

int HumdrumFile::getTrackEndCount(int track) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return 0;
	}
	if (track >= trackends.size()) {
		return 0;
	}
	return trackends[track].size();
}



//////////////////////////////
//
// HumdrumFile::getTrackEnd -- return the number of ending tokens
//    for the given track
//

HumdrumToken* HumdrumFile::getTrackEnd(int track, int subtrack) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return NULL;
	}
	if (track >= trackends.size()) {
		return NULL;
	}
	if (subtrack < 0) {
		subtrack += trackends[track].size();
	}
	if (subtrack < 0) {
		return NULL;
	}
	if (subtrack >= trackends[track].size()) {
		return NULL;
	}
	return trackends[track][subtrack];
}



//////////////////////////////
//
// HumdrumFile::analyzeLines -- Mark the line's index number in the
//    HumdrumFile.
//

bool HumdrumFile::analyzeLines(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->setLineIndex(i);
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeTracks -- Analyze the track structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeTracks(void) {
	for (int i=0; i<lines.size(); i++) {
		int status = lines[i]->analyzeTracks();
		if (!status) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeLinks -- Generate forward and backwards spine links
//    for each token.
//

bool HumdrumFile::analyzeLinks(void) {
	HumdrumLine* next     = NULL;
	HumdrumLine* previous = NULL;

	for (int i=0; i<lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		previous = next;
		next = lines[i];
		if (previous != NULL) {
			if (!stitchLinesTogether(*previous, *next)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::stitchLinesTogether -- Make forward/backward links for
//    tokens on each line.
//

bool HumdrumFile::stitchLinesTogether(HumdrumLine& previous,
		HumdrumLine& next) {
	int i;


	// first handle simple cases where the spine assignments are one-to-one:
	if (!previous.isInterpretation() && !next.isInterpretation()) {
		if (previous.getTokenCount() != next.getTokenCount()) {
			cerr << "Error lines " << (previous.getLineNumber())
			     << " and " << (next.getLineNumber()) << " not same length\n";
			cerr << "Line " << (previous.getLineNumber()) << ": "
			     << previous << endl;
			cerr << "Line " << (next.getLineNumber()) << ": "
			     << next << endl;
			return false;
		}
		for (i=0; i<previous.getTokenCount(); i++) {
			previous.token(i).makeForwardLink(next.token(i));
		}
		return true;
	}
	int ii = 0;
	for (i=0; i<previous.getTokenCount(); i++) {
		if (!previous.token(i).isManipulator()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isSplitInterpretation()) {
			// connect the previous token to the next two tokens.
			previous.token(i).makeForwardLink(next.token(ii++));
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isMergeInterpretation()) {
			// connect multiple previous tokens which are adjacent *v
			// spine manipulators to the current next token.
			while ((i<previous.getTokenCount()) &&
					previous.token(i).isMergeInterpretation()) {
				previous.token(i).makeForwardLink(next.token(ii));
				i++;
			}
			i--;
			ii++;
		} else if (previous.token(i).isExchangeInterpretation()) {
			// swapping the order of two spines.
			if ((i<previous.getTokenCount()) &&
					previous.token(i+1).isExchangeInterpretation()) {
				previous.token(i+1).makeForwardLink(next.token(ii++));
				previous.token(i).makeForwardLink(next.token(ii++));
			}
			i++;
		} else if (previous.token(i).isTerminateInterpretation()) {
			// No link should be made.  There may be a problem if a
			// new segment is given (this should be handled by a
			// HumdrumSet class, not HumdrumFile.
		} else if (previous.token(i).isAddInterpretation()) {
			// A new data stream is being added, the next linked token
			// should be an exclusive interpretation.
			if (!next.token(ii+1).isExclusiveInterpretation()) {
				cerr << "Error: expecting exclusive interpretation on line "
				     << next.getLineNumber() << " at token " << i << " but got "
				     << next.token(i) << endl;
				return false;
			}
			previous.token(i).makeForwardLink(next.token(ii++));
			ii++;
		} else if (previous.token(i).isExclusiveInterpretation()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else {
			cerr << "Error: should not get here" << endl;
			return false;
		}
	}

	if ((i != previous.getTokenCount()) || (ii != next.getTokenCount())) {
		cerr << "Error: cannot stitch lines together due to alignment problem\n";
		cerr << "Line " << previous.getLineNumber() << ": "
		     << previous << endl;
		cerr << "Line " << next.getLineNumber() << ": "
		     << next << endl;
		cerr << "I = " <<i<< " token count " << previous.getTokenCount() << endl;
		cerr << "II = " <<ii<< " token count " << next.getTokenCount() << endl;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeSpines -- Analyze the spine structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeSpines(void) {
	vector<string> datatype;
	vector<string> sinfo;
	vector<vector<HumdrumToken*> > lastspine;
	trackstarts.resize(0);
	trackends.resize(0);
	addToTrackStarts(NULL);

	bool init = false;
	int i, j;
	for (i=0; i<getLineCount(); i++) {
		if (!lines[i]->hasSpines()) {
			lines[i]->token(0).setFieldIndex(0);
			continue;
		}
		if ((init == false) && !lines[i]->isExclusive()) {
			cerr << "Error on line: " << (i+1) << ':' << endl;
			cerr << "   Data found before exclusive interpretation" << endl;
			cerr << "   LINE: " << *lines[i] << endl;
			return false;
		}
		if ((init == false) && lines[i]->isExclusive()) {
			// first line of data in file.
			init = true;
			datatype.resize(lines[i]->getTokenCount());
			sinfo.resize(lines[i]->getTokenCount());
			lastspine.resize(lines[i]->getTokenCount());
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				datatype[j] = lines[i]->getTokenString(j);
				addToTrackStarts(&lines[i]->token(j));
				sinfo[j]    = to_string(j+1);
				lines[i]->token(j).setSpineInfo(sinfo[j]);
				lines[i]->token(j).setFieldIndex(j);
				lastspine[j].push_back(&(lines[i]->token(j)));
			}
			continue;
		}
		if (datatype.size() != lines[i]->getTokenCount()) {
			cerr << "Error on line " << (i+1) << ':' << endl;
			cerr << "   Expected " << datatype.size() << " fields,"
			     << " but found " << lines[i]->getTokenCount() << endl;
			return false;
		}
		for (j=0; j<lines[i]->getTokenCount(); j++) {
			lines[i]->token(j).setSpineInfo(sinfo[j]);
			lines[i]->token(j).setFieldIndex(j);
		}
		if (!lines[i]->isManipulator()) {
			continue;
		}
		if (!adjustSpines(*lines[i], datatype, sinfo)) { return false; }
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::addToTrackStarts --
//

void HumdrumFile::addToTrackStarts(HumdrumToken* token) {
	if (token == NULL) {
		trackstarts.push_back(NULL);
		trackends.resize(trackends.size()+1);
	} else if ((trackstarts.size() > 1) && (trackstarts.back() == NULL)) {
		trackstarts.back() = token;
	} else {
		trackstarts.push_back(token);
		trackends.resize(trackends.size()+1);
	}
}



//////////////////////////////
//
// HumdrumFile::adjustSpines -- adjust datatype and spineinfo based
//   on manipulators.
//

bool HumdrumFile::adjustSpines(HumdrumLine& line, vector<string>& datatype,
		vector<string>& sinfo) {
	vector<string> newtype;
	vector<string> newinfo;
	int mergecount = 0;
	int i, j;
	for (i=0; i<line.getTokenCount(); i++) {
		if (line.token(i).isSplitInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 2);
			newinfo[newinfo.size()-2] = '(' + sinfo[i] + ")a";
			newinfo[newinfo.size()-1] = '(' + sinfo[i] + ")b";
		} else if (line.token(i).isMergeInterpretation()) {
			mergecount = 0;
			for (j=i+1; j<line.getTokenCount(); j++) {
				if (line.token(j).isMergeInterpretation()) {
					mergecount++;
				} else {
					break;
				}
			}
			newinfo.resize(newtype.size() + 1);
			newinfo.back() = getMergedSpineInfo(sinfo, i, mergecount);
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			i += mergecount;
		} else if (line.token(i).isAddInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = "";
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			newinfo.resize(newinfo.size() + 1);
			addToTrackStarts(NULL);
			newinfo.back() = to_string(getMaxTrack());
		} else if (line.token(i).isExchangeInterpretation()) {
			if (i < line.getTokenCount() - 1) {
				if (line.token(i).isExchangeInterpretation()) {
					// exchange spine information
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i+1];
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i+1];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i];
				} else {
					cerr << "ERROR1 in *x calculation" << endl;
					return false;
				}
				i++;
			} else {
				cerr << "ERROR2 in *x calculation" << endl;
				cerr << "Index " << i << " larger than allowed: "
				     << line.getTokenCount() - 1 << endl;
				return false;
			}
		} else if (line.token(i).isTerminateInterpretation()) {
			// store pointer to terminate token in trackends
			trackends[trackstarts.size()-1].push_back(&(line.token(i)));
		} else if (((string)line.token(i)).substr(0, 2) == "**") {
			newtype.resize(newtype.size() + 1);
			newtype.back() = line.getTokenString(i);
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			if (!((trackstarts.size() > 1) && (trackstarts.back() == NULL))) {
				cerr << "Error: Exclusive interpretation with no preparation "
				     << "on line " << line.getLineIndex()
				     << " spine index " << i << endl;
				cerr << "Line: " << line << endl;
				return false;
			}
			if (trackstarts.back() == NULL) {
cout << "GOING TO ADD " << line.token(i) << endl;
				addToTrackStarts(&line.token(i));
			}
		} else {
			// should only be null interpretation, but doesn't matter
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
		}
	}

	datatype.resize(newtype.size());
	sinfo.resize(newinfo.size());
	for (i=0; i<newtype.size(); i++) {
		datatype[i] = newtype[i];
		sinfo[i]    = newinfo[i];
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::getMergedSpineInfo -- Will only simplify a two-spine
//   merge.  Should be expanded to larger spine mergers.
//

string HumdrumFile::getMergedSpineInfo(vector<string>& info, int starti,
		int extra) {
	string output;
	int len1;
	int len2;
	if (extra == 1) {
		len1 = info[starti].size();
		len2 = info[starti+1].size();
		if (len1 == len2) {
			if (info[starti].substr(0, len1-1) ==
					info[starti+1].substr(0,len2-1)) {
				output = info[starti].substr(1, len1-3);
				return output;
			}
		}
		output = info[starti] + " " + info[starti+1];
		return output;
	}
	output = info[starti];
	for (int i=0; i<extra; i++) {
		output += " " + info[starti+1+extra];
	}
	return output;
}



//////////////////////////////
//
// HumdrumFile::analyzeTokenDurations -- Calculate the duration of
//   all tokens in spines which posses duration in a file.
//

bool HumdrumFile::analyzeTokenDurations (void) {
	for (int i=0; i<getLineCount(); i++) {
		if (!lines[i]->analyzeTokenDurations()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeRhythm -- Analyze the rhythmic structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeRhythm(void) {
	if (getMaxTrack() == 0) {
		return true;
	}
	int startline = getTrackStart(1)->getLineIndex();
	int testline;
	HumNum zero(0);

	int i;
	for (int i=1; i<=getMaxTrack(); i++) {
		if (!getTrackStart(i)->hasRhythm()) {
			// Can't analyze rhythm of spines that do not have rhythm.
			continue;
		}
		testline = getTrackStart(i)->getLineIndex();
		if (testline == startline) {
			if (!assignDurationsToTrack(getTrackStart(i), zero)) {
				return false;
			}
		} else {
			// Spine does not start at beginning of data, so
			// the starting position of the spine has to be
			// determined before continuing.  Search for a token
			// which is on a line with assigned duration, then work
			// outwards from that position.
			continue;
		}
	}

	// Go back and analyze spines which do not start at the beginning
	// of the data stream.
	for (i=1; i<=getMaxTrack(); i++) {
		if (!getTrackStart(i)->hasRhythm()) {
			// Can't analyze rhythm of spines that do not have rhythm.
			continue;
		}
		testline = getTrackStart(i)->getLineIndex();
		if (testline > startline) {
			if (!analyzeRhythmOfFloatingSpine(getTrackStart(i))) { return false; }
		}
	}

	if (!analyzeNullLineRhythms()) { return false; }
	fillInNegativeStartTimes();
	assignLineDurations();
	if (!analyzeMeter()) { return false; }
	if (!analyzeNonNullDataTokens()) { return false; }

	return true;
}



///////////////////////////////
//
// HumdrumFile::analyzeLocalParameters -- only allowing layout
//    parameters at the moment.
//

bool HumdrumFile::analyzeLocalParameters(void) {
	// analyze backward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (!processLocalParametersForTrack(getTrackEnd(i, j),
					getTrackEnd(i, j))) {
				return false;
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::processLocalParametersForTrack --
//

bool HumdrumFile::processLocalParametersForTrack(HumdrumToken* starttok,
		HumdrumToken* current) {

	HumdrumToken* token = starttok;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!processLocalParametersForTrack(
					token->getPreviousToken(i), current)) {
				return false;
			}
		}
		if (!(token->isNull() && token->isManipulator())) {
			if (token->isCommentLocal()) {
				checkForLocalParameters(token, current);
			} else {
				current = token;
			}
		}

		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::checkForLocalParameters -- only allowing layout parameters
//    currently.
//

void HumdrumFile::checkForLocalParameters(HumdrumToken *token,
		HumdrumToken *current) {
	if (token->size() < 1) {
		return;
	}
	if (token->find("!LO:") != 0) {
		return;
	}
	current->setParameters(token);
}



//////////////////////////////
//
// HumdrumFile::analyzeNonNullDataTokens -- For null data tokens, indicate the
//    previous non-null token which the null token refers to.  After a spine
//    merger, there may be multiple previous tokens, so you would have to
//    decide on the actual source token on based on subtrack or subspine
//    information.  The function also gives links to the previous/next
//    non-null tokens, skipping over intervening null data tokens.
//

bool HumdrumFile::analyzeNonNullDataTokens(void) {
	vector<HumdrumToken*> ptokens;

	// analyze forward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		if (!processNonNullDataTokensForTrackForward(getTrackStart(i),
				ptokens)) {
			return false;
		}
	}

	// analyze backward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (!processNonNullDataTokensForTrackBackward(getTrackEnd(i, j),
					ptokens)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeDurationsOfNonRhythmicSpines -- Calculate the duration
//    of non-null data token in non-rhythmic spines.
//

bool HumdrumFile::analyzeDurationsOfNonRhythmicSpines(void) {
	// analyze tokens backwards:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (getTrackEnd(i, j)->hasRhythm()) {
				continue;
			}
			if (!assignDurationsToNonRhythmicTrack(getTrackEnd(i, j), 
					getTrackEnd(i, j))) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::assignDurationsToNonRhythmicTrack --
//

bool HumdrumFile::assignDurationsToNonRhythmicTrack(HumdrumToken* endtoken,
		HumdrumToken* current) {
	HumdrumToken* token = endtoken;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!assignDurationsToNonRhythmicTrack(token->getPreviousToken(i), 
					current)) {
				return false;
			}
		}
		if (token->isData()) {
			if (!token->isNull()) {
				token->setDuration(current->getDurationFromStart() - 
						token->getDurationFromStart());
				current = token;
			}
		}
		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackBackward --
//

bool HumdrumFile::processNonNullDataTokensForTrackBackward(
		HumdrumToken* endtoken, vector<HumdrumToken*> ptokens) {

	HumdrumToken* token = endtoken;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!processNonNullDataTokensForTrackBackward(
					token->getPreviousToken(i), ptokens)) {
				return false;
			}
		}
		if (token->isData()) {
			addUniqueTokens(token->nextNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackForward --
//

bool HumdrumFile::processNonNullDataTokensForTrackForward(
		HumdrumToken* starttoken, vector<HumdrumToken*> ptokens) {
	HumdrumToken* token = starttoken;
	int tcount = token->getNextTokenCount();
	while (tcount > 0) {
		if (!token->isData()) {
			for (int i=1; i<tcount; i++) {
				if (!processNonNullDataTokensForTrackForward(
						token->getNextToken(i), ptokens)) {
					return false;
				}
			}
		} else {
			addUniqueTokens(token->previousNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one next token,
		// so no need to check for more than one next token.
		token = token->getNextToken(0);
		tcount = token->getNextTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::addUniqueTokens --
//

void HumdrumFile::addUniqueTokens(vector<HumdrumToken*>& target,
		vector<HumdrumToken*>& source) {
	int i, j;
	bool found;
	for (i=0; i<source.size(); i++) {
		found = false;
		for (j=0; j<target.size(); j++) {
			if (source[i] == target[i]) {
				found = true;
			}
		}
		if (!found) {
			target.push_back(source[i]);
		}
	}
}



//////////////////////////////
//
// HumdrumFile::assignLineDurations --
//

void HumdrumFile::assignLineDurations(void) {
	HumNum startdur;
	HumNum enddur;
	HumNum dur;
	for (int i=0; i<lines.size()-1; i++) {
		startdur = lines[i]->getDurationFromStart();
		enddur = lines[i+1]->getDurationFromStart();
		dur = enddur - startdur;
		lines[i]->setDuration(dur);
	}
	if (lines.size() > 0) {
		lines.back()->setDuration(0);
	}
}



//////////////////////////////
//
// HumdrumFile::fillInNegativeStartTimes --
//

void HumdrumFile::fillInNegativeStartTimes(void) {
	int i;
	HumNum lastdur = -1;
	HumNum dur;
	for (i=lines.size()-1; i>=0; i--) {
		dur = lines[i]->getDurationFromStart();
		if (dur.isNegative() && lastdur.isNonNegative()) {
			lines[i]->setDurationFromStart(lastdur);
		}
		if (dur.isNonNegative()) {
			lastdur = dur;
			continue;
		}
	}

	// fill in start times for ending comments
	for (i=0; i<lines.size(); i++) {
		dur = lines[i]->getDurationFromStart();
		if (dur.isNonNegative()) {
			lastdur = dur;
		} else {
			lines[i]->setDurationFromStart(lastdur);
		}
	}
}



//////////////////////////////
//
// HumdrumFile::analyzeNullLineRhythms -- When a series of null-token data line
//    occur between two data lines possessing a start duration, then split the
//    duration between those two lines amongst the null-token lines.  For example
//    if a data line starts at time 15, and there is one null-token line before
//    another data line at time 16, then the null-token line will be assigned
//    to the position 15.5 in the score.
//

bool HumdrumFile::analyzeNullLineRhythms(void) {
	vector<HumdrumLine*> nulllines;
	HumdrumLine* previous = NULL;
	HumdrumLine* next = NULL;
	HumNum dur;
	HumNum startdur;
	HumNum enddur;
	int i, j;
	for (i=0; i<(int)lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		if (lines[i]->isAllRhythmicNull()) {
			if (lines[i]->isData()) {
				nulllines.push_back(lines[i]);
			}
			continue;
		}
		dur = lines[i]->getDurationFromStart();
		if (dur.isNegative()) {
			if (lines[i]->isData()) {
				cerr << "Error: found an unexpected negative duration on line "
			     	<< lines[i]->getDurationFromStart()<< endl;
				cerr << "Line: " << *lines[i] << endl;
				return false;
			} else {
				continue;
			}
		}
		next = lines[i];
		if (previous == NULL) {
			previous = next;
			nulllines.resize(0);
			continue;
		}
		startdur = previous->getDurationFromStart();
		enddur   = next ->getDurationFromStart();
		HumNum gapdur = enddur - startdur;
		HumNum nulldur = gapdur / (nulllines.size() + 1);
		for (j=0; j<(int)nulllines.size(); j++) {
			nulllines[j]->setDurationFromStart(startdur + (nulldur * (j+1)));
		}
		previous = next;
		nulllines.resize(0);
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeRhythmOfFloatingSpine --  This analysis function is used
//    to analyze the rhythm of spines which do not start at the beginning of
//    the data.  The function searches for the first line which has an
//    assigned durationFromStart value, and then uses that as the basis
//		for assigning the initial durationFromStart position for the spine.
//

bool HumdrumFile::analyzeRhythmOfFloatingSpine(HumdrumToken* spinestart) {
	HumNum dursum = 0;
	HumNum founddur = 0;
	HumdrumToken* token = spinestart;
	int tcount = token->getNextTokenCount();

	// Find a known durationFromStart for a line in the Humdrum file, then
	// use that to calculate the starting duration of the floating spine.
	if (token->getDurationFromStart().isNonNegative()) {
		founddur = token->getLine()->getDurationFromStart();
	} else {
		tcount = token->getNextTokenCount();
		while (tcount > 0) {
			if (token->getDurationFromStart().isNonNegative()) {
				founddur = token->getLine()->getDurationFromStart();
				break;
			}
			if (token->getDuration().isPositive()) {
				dursum += token->getDuration();
			}
			token = token->getNextToken(0);
		}
	}

	if (founddur.isZero()) {
		cerr << "Error cannot link floating spine to score." << endl;
		return false;
	}

	if (!assignDurationsToTrack(spinestart, founddur - dursum)) {
		return false;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::assignDurationsToTrack --
//

bool HumdrumFile::assignDurationsToTrack(HumdrumToken* starttoken,
		HumNum startdur) {
	if (!starttoken->hasRhythm()) {
		return true;
	}
	int state = starttoken->getState();
	if (!prepareDurations(starttoken, state, startdur)) { return false; }
	return true;
}



//////////////////////////////
//
// HumdrumFile::prepareDurations --
//

bool HumdrumFile::prepareDurations(HumdrumToken* token, int state,
		HumNum startdur) {
	if (state != token->getState()) {
		return true;
	}

	HumdrumToken* initial = token;
	HumNum dursum = startdur;
	token->incrementState();

	if (!setLineDurationFromStart(token, dursum)) { return false; }
	if (token->getDuration().isPositive()) {
		dursum += token->getDuration();
	}
	int tcount = token->getNextTokenCount();

	// Assign line durationFromStarts for primary track first.
	while (tcount > 0) {
		token = token->getNextToken(0);
		if (state != token->getState()) {
			return true;
		}
		token->incrementState();
		if (!setLineDurationFromStart(token, dursum)) { return false; }
		if (token->getDuration().isPositive()) {
			dursum += token->getDuration();
		}
		tcount = token->getNextTokenCount();
	}

	if ((tcount == 0) && (token->isTerminateInterpretation())) {
		if (!setLineDurationFromStart(token, dursum)) { return false; }
	}

	// Process secondary tracks next:
	int newstate = state + 1;

	token = initial;
	dursum = startdur;
	if (token->getDuration().isPositive()) {
		dursum += token->getDuration();
	}
	tcount = token->getNextTokenCount();
	while (tcount > 0) {
		if (tcount > 1) {
			for (int i=1; i<tcount; i++) {
				if (!prepareDurations(token->getNextToken(i), state, dursum)) {
					return false;
				}
			}
		}
		token = token->getNextToken(0);
		if (newstate != token->getState()) {
			return true;
		}
		if (token->getDuration().isPositive()) {
			dursum += token->getDuration();
		}
		tcount = token->getNextTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::setLineDurationFromStart -- Set the duration of
//      a line based on the analysis of tokens in the spine.
//      If the duration
//

bool HumdrumFile::setLineDurationFromStart(HumdrumToken* token,
		HumNum dursum) {
	if ((!token->isTerminateInterpretation()) &&
			token->getDuration().isNegative()) {
		// undefined rhythm, so don't assign line duration information:
		return true;
	}
	HumdrumLine* line = token->getOwner();
	if (line->getDurationFromStart().isNegative()) {
		line->setDurationFromStart(dursum);
	} else if (line->getDurationFromStart() != dursum) {
		cerr << "Error: Inconsistent rhythm analysis occuring near line "
		     << token->getLineNumber() << endl;
		cerr << "Expected durationFromStart to be: " << dursum
		     << " but found it to be " << line->getDurationFromStart() << endl;
		cerr << "Line: " << *line << endl;
		return false;
	}

	return true;
}



///////////////////////////////
//
// HumdrumFile::analyzeGlobalParameters -- only allowing layout
//    parameters at the moment.  Global parameters affect the next
//    line which is either a barline, dataline or an interpretation
//    other than a spine manipulator.  Null lines are also not
//    considered.
//

bool HumdrumFile::analyzeGlobalParameters(void) {
	HumdrumLine* spineline = NULL;
	for (int i=lines.size()-1; i>=0; i--) {
		if (lines[i]->hasSpines()) {
			if (lines[i]->isAllNull())  {
				continue;
			}
			if (lines[i]->isManipulator()) {
				continue;
			}
			if (lines[i]->isCommentLocal()) {
				continue;
			}
			// should be a non-null data, barlines, or interpretation
			spineline = lines[i];
			continue;
		}
		if (spineline == NULL) {
			continue;
		}
		if (!lines[i]->isCommentGlobal()) {
			continue;
		}
		if (lines[i]->find("!!LO:") != 0) {
			continue;
		}
		spineline->setParameters(lines[i]);
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeMeter -- Store the times from the last barline
//     to the current line, as well as the time to the next barline.
//     the sum of these two will be the duration of the barline, except
//     for barlines, where the getDurationToBarline() will store the
//     duration of the measure staring at that barline.  To get the
//     beat, you will have to figure out the current time signature.
//

bool HumdrumFile::analyzeMeter(void) {

	barlines.resize(0);

	int i;
	HumNum sum = 0;
	bool foundbarline = false;
	for (i=0; i<getLineCount(); i++) {
		lines[i]->setDurationFromBarline(sum);
		sum += lines[i]->getDuration();
		if (lines[i]->isBarline()) {
			foundbarline = true;
			barlines.push_back(lines[i]);
			sum = 0;
		}
		if (lines[i]->isData() && !foundbarline) {
			// pickup measure, so set the first measure to the start of the file.
			barlines.push_back(lines[0]);
		}
	}

	sum = 0;
	for (i=getLineCount()-1; i>=0; i--) {
		sum += lines[i]->getDuration();
		lines[i]->setDurationToBarline(sum);
		if (lines[i]->isBarline()) {
			sum = 0;
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::getTokenDurations --
//

bool HumdrumFile::getTokenDurations(vector<HumNum>& durs, int line) {
	durs.resize(0);
	for (int i=0; i<lines[line]->getTokenCount(); i++) {
		HumNum dur = lines[line]->token(i).getDuration();
		durs.push_back(dur);
	}
	if (!cleanDurs(durs, line)) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::decrementDurStates -- Subtract the line duration from
//   the current line of running durations.  If any duration is less
//   than zero, then a rhythm error exists in the data.
//

bool HumdrumFile::decrementDurStates(vector<HumNum>& durs, HumNum linedur,
		int line) {
	if (linedur.isZero()) {
		return true;
	}
	for (int i=0; i<(int)durs.size(); i++) {
		if (!lines[line]->token(i).hasRhythm()) {
			continue;
		}
		durs[i] -= linedur;
		if (durs[i].isNegative()) {
			cerr << "Error: rhythmic error on line " << (line+1)
			     << " field index " << i << endl;
			cerr << "Duration state is: " << durs[i] << endl;
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::getMinDur -- Return the smallest duration on the
//   line.  If all durations are zero, then return zero; otherwise,
//   return the smallest positive duration.
//

HumNum HumdrumFile::getMinDur(vector<HumNum>& durs, vector<HumNum>& durstate) {
	HumNum mindur = 0;
	bool allzero = true;

	for (int i=0; i<(int)durs.size(); i++) {
		if (durs[i].isPositive()) {
			allzero = false;
			if (mindur.isZero()) {
				mindur = durs[i];
			} else if (mindur > durs[i]) {
				mindur = durs[i];
			}
		}
	}
	if (allzero) {
		return mindur;
	}

	for (int i=0; i<(int)durs.size(); i++) {
		if (durstate[i].isPositive() && mindur.isZero()) {
			if (durstate[i].isZero()) {
				// mindur = durstate[i];
			} else if (mindur > durstate[i]) {
				mindur = durstate[i];
			}
		}
	}
	return mindur;
}



//////////////////////////////
//
// HumdrumFile::cleanDurs -- Check if there are grace note and regular
//    notes on a line (not allowed).  Leaves negative durations which
//    indicate undefined durations (needed for keeping track of null
//    tokens in rhythmic spines.
//

bool HumdrumFile::cleanDurs(vector<HumNum>& durs, int line) {
	bool zero 		= false;
	bool positive = false;
	for (int i=0; i<(int)durs.size(); i++) {
		if      (durs[i].isPositive()) { positive = true; }
		else if (durs[i].isZero())     { zero     = true; }
	}
	if (zero && positive) {
		cerr << "Error on line " << (line+1) << " grace note and "
		     << " regular note cannot occur on same line." << endl;
		cerr << "Line: " << *lines[line] << endl;
		return false;
	}
	return true;
}



//////////////////////////////
//
// operator<< -- Print a HumdrumFile.
//

ostream& operator<<(ostream& out, HumdrumFile& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		out << infile[i] << '\n';
	}
	return out;
}

// END_MERGE

} // end namespace std;



