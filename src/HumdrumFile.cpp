//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
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

// START_MERGE


//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::HumdrumFile(void) { }



//////////////////////////////
//
// HumdrumFile::~HumdrumFile --
//

HumdrumFile::~HumdrumFile() { }



//////////////////////////////
//
// HumdrumFile::operator[] --
//

HumdrumLine& HumdrumFile::operator[](int index) {
	if ((index < 0) || (index >= (int)lines.size())) {
		cerr << "Error: invalid index " << index << " in HumdrumFile" << endl;
		exit(1);
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
		lines.push_back(s);
	}
	createTokensFromLines();
	if (!analyzeLines())  { return false; }
	if (!analyzeSpines()) { return false; }
	if (!analyzeLinks())  { return false; }
	if (!analyzeTracks()) { return false; }
	return analyzeRhythm();
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
// HumdrumFile::createTokensFromLines -- Generate token array from
//    current state of lines.  If either state is changed, then the
//    other state becomes invalid.  See createLinesFromTokens for
//		regeneration of lines from tokens.
//

void HumdrumFile::createTokensFromLines(void) {
	int i;
	for (i=0; i<lines.size(); i++) {
		lines[i]->createTokensFromLine();
	}
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
// HumdrumFile::size -- Return the number of lines.
//

int HumdrumFile::size(void) const {
	return lines.size();
}



//////////////////////////////
//
// HumdrumFile::getMaxTrack -- returns the number of primary spines in the data.
//

int HumdrumFile::getMaxTrack(void) const {
	return maxtrack;
}



//////////////////////////////
//
// HumdrumFile::printSpineInfo --
//

ostream& HumdrumFile::printSpineInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printSpineInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDataTypeInfo --
//

ostream& HumdrumFile::printDataTypeInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printDataTypeInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printTrackInfo --
//

ostream& HumdrumFile::printTrackInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printTrackInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDurationInfo --
//

ostream& HumdrumFile::printDurationInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printDurationInfo(out) << '\n';
	}
	return out;
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
		} else if (previous.token(i).isTerminateInterpretation()) {
			// No link should be made.  There may be a problem if a
			// new segment is given (this should be handled by a
			// HumdrumSet class, not HumdrumFile.
		} else if (previous.token(i).isAddInterpretation()) {
			// A new data stream is being added, the next linked token
			// should be an exclusive interpretation.
			if (!next.token(i).isExclusiveInterpretation()) {
				cerr << "Error: expecting exclusive interpretation on line "
				     << next.getLineNumber() << " but got "
				     << next.token(i) << endl;
				return false;
			}
			previous.token(i).makeForwardLink(next.token(ii++));
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

	bool init = false;
	int i, j;
	for (i=0; i<size(); i++) {
		if (!lines[i]->hasSpines()) {
			lines[i]->token(0).setLineAddress(i, 0);
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
				sinfo[j]    = to_string(i+1);
				lines[i]->token(j).setDataType(datatype[j]);
				lines[i]->token(j).setSpineInfo(sinfo[j]);
				lines[i]->token(j).setLineAddress(i, j);
				lastspine[j].push_back(&(lines[i]->token(j)));
			}
			maxtrack = datatype.size();
			continue;
		}
		if (datatype.size() != lines[i]->getTokenCount()) {
			cerr << "Error on line " << (i+1) << ':' << endl;
			cerr << "   Expected " << datatype.size() << " fields,"
			     << " but found " << lines[i]->getTokenCount() << endl;
			return false;
		}
		for (j=0; j<lines[i]->getTokenCount(); j++) {
			lines[i]->token(j).setDataType(datatype[j]);
			lines[i]->token(j).setSpineInfo(sinfo[j]);
			lines[i]->token(j).setLineAddress(i, j);
		}
		if (!lines[i]->isManipulator()) {
			continue;
		}
		maxtrack = adjustSpines(*lines[i], datatype, sinfo, maxtrack);
	}
   return true;
}




//////////////////////////////
//
// HumdrumFile::adjustSpines -- adjust datatype and spineinfo based
//   on manipulators.
//

int HumdrumFile::adjustSpines(HumdrumLine& line, vector<string>& datatype,
		vector<string>& sinfo, int trackcount) {
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
			newinfo.back() = to_string(++mergecount);
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
					cerr << "ERROR in *x calculation" << endl;
					exit(1);
				}
			} else {
				cerr << "ERROR in *x calculation" << endl;
				exit(1);
			}
		} else if (line.token(i).isTerminateInterpretation()) {
			// do nothing: the spine is terminating;
		} else if (((string)line.token(i)).substr(0, 2) == "**") {
			newtype.resize(newtype.size() + 1);
			newtype.back() = line.getTokenString(i);
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
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

	return trackcount;
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
//   all tokens in a file.
//

bool HumdrumFile::analyzeTokenDurations (void) {
	for (int i=0; i<size(); i++) {
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
	if (!analyzeTokenDurations()) {
		return false;
	}
	vector<HumNum> durstate;
	vector<HumNum> newdurstate;
	vector<HumNum> curdur;
	HumNum linedur;
	HumNum dur;
	HumNum zero(0);

	int i, j, k;
	for (i=0; i<size(); i++) {
		if (lines[i]->isExclusiveInterpretation()) {
			// If an exclusive interpretation line, initialize the durstate.
			if (!getTokenDurations(curdur, i)) { return false; }
			durstate = curdur;
			linedur = 0;
			lines[i]->setDuration(linedur);
			continue;
		} else if (!lines[i]->isManipulator()) {
			if (lines[i]->isData()) {
				if (!getTokenDurations(curdur, i)) { return false; }
				if (curdur.size() != durstate.size()) {
					cerr << "Error on line " << (i+1) 
					     << ": spine problem" << endl;
					cerr << "Line: " << *lines[i] << endl;
					return false;
				}
				linedur = getMinDur(curdur, durstate);
				for (j=0; j<curdur.size(); j++) {
					if (curdur[j].isPositive()) {
						if (durstate[j].isPositive()) {
							cerr << "Error on line " << (i+1) 
					     		<< ": previous rhythm too long in field " << j << endl;
							cerr << "Line: " << *lines[i] << endl;
							return false;
						} else {
							durstate[j] = curdur[j];
						}
					}
				}
				lines[i]->setDuration(linedur);
				if (!decrementDurStates(durstate, linedur, i)) { return false; }
			} else {
				// Not a rhythmic line, so preserve durstate and set the
				// duration of the line to zero.
				lines[i]->setDuration(0);
			}
		} else {
			// Deal with spine manipulators.  The dur state of a spine must
			// be zero when the manipulator is *^, *v, or *-. Exclusive
			// interpretations initialze a new duration state.
			newdurstate.resize(0);
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				if (lines[i]->token(j).isSplitInterpretation()) {
					if (durstate[j].isNonZero()) {
						cerr << "Error on line " << (i+1)
						     << ": notes must end before splitting spine." << endl;
						return false;
					}
					newdurstate.push_back(durstate[j]);
					newdurstate.push_back(durstate[j]);
				} else if (lines[i]->token(j).isMergeInterpretation()) {
					int mergecount = lines[i]->token(j).getNextToken()
							->getPreviousTokenCount();
					if (mergecount <= 1) {
						cerr << "Error on line " << (i+1)
						     << ": merger is incomplete" << endl;
						cerr << "Line: " << lines[i] << endl;
						return false;
					}
					// check that all merger spine running durations
					// are zero; otherwise, there is a rhythmic error in the score.
					for (k=0; k<mergecount; k++) {
						if (durstate[j+k].isNonZero()) {
							cerr << "Error on line " << (i+1)
							     << ": merger is incomplete" << endl;
							cerr << "Line: " << lines[i] << endl;
							return false;
						}
					}
					newdurstate.push_back(durstate[j]);
					j += mergecount - 1;
				} else if (lines[i]->token(j).isExchangeInterpretation()) {
					// switch order of duration states.
					newdurstate.push_back(durstate[j+1]);
					newdurstate.push_back(durstate[j]);
					j++;
				} else if (lines[i]->token(j).isAddInterpretation()) {
					// initialize a new durstate;
					newdurstate.push_back(durstate[j]);
					newdurstate.push_back(zero);
				} else if (lines[i]->token(j).isTerminateInterpretation()) {
					// the durstate should be removed for the current spine.
				} else if (lines[i]->token(j).isExclusiveInterpretation()) {
					// The add interpretation should have added this interpetation,
					// and it should alread by in durstate
					newdurstate.push_back(durstate[j]);
				} else {
					// The manipulator should have a one-to-one mapping with
					// the next token in the spine.
					newdurstate.push_back(durstate[j]);
				}
			}
			// Store the new durstate:
			durstate = newdurstate;
		}
	}

   // Fill in the cumulative duration data:
	HumNum dursum = 0;
	for (i=0; i<lines.size(); i++) {
		lines[i]->setDurationFromStart(dursum);
		dursum += lines[i]->getDuration();
	}

	// Fill in the metrical information
	if (!analyzeMeter()) {
		return false;
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
	int i;
	HumNum sum = 0;
	for (i=0; i<size(); i++) {
		lines[i]->setDurationFromBarline(sum);
		sum += lines[i]->getDuration();
		if (lines[i]->isBarline()) {
			sum = 0;
		}
	}

	sum = 0;
	for (i=size()-1; i>=0; i--) {
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
	for (int i=0; i<(int)durs.size(); i++) {
		if (durs[i].isPositive()) {
			if (mindur.isZero()) {
				mindur = durs[i];
			} else if (mindur > durs[i]) {
				mindur = durs[i];
			}
		}
		if (durstate[i].isPositive()) {
			if (durstate[i].isZero()) {
				mindur = durstate[i];
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
//    notes on a line (not allowed), and convert negative durations
//    to zero (negative durations indicate undefined durations).
//

bool HumdrumFile::cleanDurs(vector<HumNum>& durs, int line) {
	bool zero 		= false;
	bool positive = false;
	for (int i=0; i<(int)durs.size(); i++) {
		if      (durs[i].isPositive()) { positive = true; }
		else if (durs[i].isZero())     { zero     = true; }
		else                           { durs[i]  = 0; }
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
	for (int i=0; i<infile.size(); i++) {
		out << infile[i] << '\n';
	}
	return out;
}

// END_MERGE



