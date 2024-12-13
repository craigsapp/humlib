//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Sep 12 13:31:48 PDT 2023
// Last Modified: Mon Sep 18 10:45:14 PDT 2023
// Filename:      tool-meter.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-meter.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
// Documentation: https://doc.verovio.humdrum.org/filter/meter
//
// Description:   Meter/beat data extraction tool.
//

#include "tool-meter.h"
#include "HumRegex.h"
#include "Convert.h"

#include <iomanip>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_meter::Tool_meter -- Set the recognized options for the tool.
//

Tool_meter::Tool_meter(void) {
	define("c|comma=b",                       "display decimal points as commas");
	define("d|denominator=b",                 "display denominator spine");
	define("e|eighth=b",                      "metric positions in eighth notes rather than beats");
	define("f|float=b",                       "floating-point beat values instead of rational numbers");
	define("h|half=b",                        "metric positions in half notes rather than beats");
	define("j|join=b",                        "join time signature information and metric positions into a single token");
	define("n|numerator=b",                   "display numerator spine");
	define("q|quarter=b",                     "metric positions in quarter notes rather than beats");
	define("r|rest=b",                        "add meteric positions of rests");
	define("s|sixteenth=b",                   "metric positions in sixteenth notes rather than beats");
	define("t|time-signature|tsig|m|meter=b", "display active time signature for each note");
	define("w|whole=b",                       "metric positions in whole notes rather than beats");
	define("z|zero=b",                        "start of measure is beat 0 rather than beat 1");

	define("B|no-beat=b",                     "Do not display metric positions (beats)");
	define("D|digits=i:0",                    "number of digits after decimal point");
	define("L|no-label=b",                    "do not add labels to analysis spines");
}



/////////////////////////////////
//
// Tool_meter::run -- Do the main work of the tool.
//

bool Tool_meter::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_meter::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_meter::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_meter::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_meter::initialize --
//

void Tool_meter::initialize(void) {

	m_commaQ       = getBoolean("comma");
	m_denominatorQ = getBoolean("denominator");
	m_digits       = getInteger("digits");
	m_floatQ       = getBoolean("float");
	m_halfQ        = getBoolean("half");
	m_joinQ        = getBoolean("join");
	m_nobeatQ      = getBoolean("no-beat");
	m_nolabelQ     = getBoolean("no-label");
	m_numeratorQ   = getBoolean("numerator");
	m_quarterQ     = getBoolean("quarter");
	m_halfQ        = getBoolean("half");
	m_eighthQ      = getBoolean("eighth");
	m_sixteenthQ   = getBoolean("sixteenth");
	m_restQ        = getBoolean("rest");
	m_tsigQ        = getBoolean("meter");
	m_wholeQ       = getBoolean("whole");
	m_zeroQ        = getBoolean("zero");

	if (m_digits < 0) {
		m_digits = 0;
	}
	if (m_digits > 15) {
		m_digits = 15;
	}

	if (m_joinQ && !(m_tsigQ || m_numeratorQ || m_denominatorQ)) {
		m_tsigQ = true;
	}
	if (m_joinQ) {
		m_nobeatQ = false;
	}
	if (m_joinQ && m_numeratorQ && m_denominatorQ) {
		m_tsigQ = true;
	}

	if (m_tsigQ) {
		m_numeratorQ = true;
		m_denominatorQ = true;
	}

	// Only one fix-width metric position allowed, prioritize
	// largest given duration:
	if (m_wholeQ) {
		m_halfQ      = false;
		m_quarterQ   = false;
		m_eighthQ    = false;
		m_sixteenthQ = false;
	} else if (m_halfQ) {
		m_wholeQ     = false;
		m_quarterQ   = false;
		m_eighthQ    = false;
		m_sixteenthQ = false;
	} else if (m_quarterQ) {
		m_wholeQ     = false;
		m_halfQ      = false;
		m_eighthQ    = false;
		m_sixteenthQ = false;
	} else if (m_eighthQ) {
		m_wholeQ     = false;
		m_halfQ      = false;
		m_quarterQ   = false;
		m_sixteenthQ = false;
	} else if (m_sixteenthQ) {
		m_wholeQ     = false;
		m_halfQ      = false;
		m_quarterQ   = false;
		m_eighthQ    = false;
	}

}



//////////////////////////////
//
// Tool_meter::processFile --
//

void Tool_meter::processFile(HumdrumFile& infile) {
	analyzePickupMeasures(infile);
	getMeterData(infile);
	printMeterData(infile);
}


//////////////////////////////
//
// Tool_meter::analyzePickupMeasures --
//

void Tool_meter::analyzePickupMeasures(HumdrumFile& infile) {
	vector<HTp> sstarts;
	infile.getKernLikeSpineStartList(sstarts);
	for (int i=0; i<(int)sstarts.size(); i++) {
		analyzePickupMeasures(sstarts[i]);
	}
}


void Tool_meter::analyzePickupMeasures(HTp sstart) {
	// First dimension are visible barlines.
	// Second dimension are time signature(s) within the barlines.
	vector<vector<HTp>> barandtime;
	barandtime.reserve(1000);
	barandtime.resize(1);
	barandtime[0].push_back(sstart);
	HTp current = sstart->getNextToken();
	while (current) {
		if (current->isTimeSignature()) {
			barandtime.back().push_back(current);
		} else if (current->isBarline()) {
			if (current->find("-") != std::string::npos) {
				current = current->getNextToken();
				continue;
			}
			barandtime.resize(barandtime.size() + 1);
			barandtime.back().push_back(current);
		} else if (*current == "*-") {
			barandtime.resize(barandtime.size() + 1);
			barandtime.back().push_back(current);
			break;
		}
		current = current->getNextToken();
	}

	// Extract the actual duration of measures:
	vector<HumNum> bardur(barandtime.size(), 0);
	for (int i=0; i<(int)barandtime.size() - 1; i++) {
		HumNum starttime = barandtime[i][0]->getDurationFromStart();
		HumNum endtime = barandtime.at(i+1)[0]->getDurationFromStart();
		HumNum duration = endtime - starttime;
		bardur.at(i) = duration;
	}

	// Extract the expected duration of measures:
	vector<HumNum> tsigdur(barandtime.size(), 0);
	int firstmeasure = -1;
	HumNum active = 0;
	for (int i=0; i<(int)barandtime.size() - 1; i++) {
		if (firstmeasure < 0) {
			if (bardur.at(i) > 0) {
				firstmeasure = i;
			}
		}
		if (barandtime[i].size() < 2) {
			tsigdur.at(i) = active;
			continue;
		}
		active = getTimeSigDuration(barandtime.at(i).at(1));
		tsigdur.at(i) = active;
	}

	vector<bool> pickup(barandtime.size(), false);
	for (int i=0; i<(int)barandtime.size() - 1; i++) {
		if (tsigdur.at(i) == bardur.at(i)) {
			// actual and expected are the same
			continue;
		}
		if (tsigdur.at(i) == tsigdur.at(i+1)) {
			if (bardur.at(i) + bardur.at(i+1) == tsigdur.at(i)) {
				pickup.at(i+1) = true;
				i++;
				continue;
			}
		}
		if (bardur.at(i) < tsigdur.at(i)) {
			HumRegex hre;
			HTp barlineToken = barandtime.at(i).at(0);
			if (barlineToken->isBarline() && hre.search(barlineToken, "\\|\\||:?\\|?!\\|?:?")) {
				pickup.at(i) = true;
				continue;
			}
		}
	}

	// check for first-measure pickup
	if (firstmeasure >= 0) {
		if (bardur.at(firstmeasure) < tsigdur.at(firstmeasure)) {
			pickup.at(firstmeasure) = true;
		}
	}

	if (m_debugQ) {
		cerr << "============================" << endl;
		for (int i=0; i<(int)barandtime.size(); i++) {
			cerr << pickup.at(i);
			cerr << "\t";
			cerr << bardur.at(i);
			cerr << "\t";
			cerr << tsigdur.at(i);
			cerr << "\t";
			for (int j=0; j<(int)barandtime[i].size(); j++) {
				cerr << barandtime.at(i).at(j) << "\t";
			}
			cerr << endl;
		}
		cerr << endl;
	}

	// Markup pickup measure notes/rests
	for (int i=0; i<(int)pickup.size() - 1; i++) {
		if (!pickup[i]) {
			continue;
		}
		markPickupContent(barandtime.at(i).at(0), barandtime.at(i+1).at(0));
	}

	// Pickup/incomplete measures covering three or more barlines are not considered
	// (these could be used with dashed barlines or similar).

}



//////////////////////////////
//
// Tool_meter::markPickupContent --
//

void Tool_meter::markPickupContent(HTp stok, HTp etok) {
	int endline = etok->getLineIndex();
	HTp current = stok;
	while (current) {
		int line = current->getLineIndex();
		if (line > endline) {
			break;
		}
		if (current->isData()) {
			HTp field = current;
			int track = field->getTrack();
			while (field) {
				int ttrack = field->getTrack();
				if (ttrack != track) {
					break;
				}
				if (field->isNull()) {
					field = field->getNextFieldToken();
					continue;
				}
				field->setValue("auto", "pickup", 1);
				HumNum nbt = etok->getDurationFromStart() - field->getDurationFromStart();
				stringstream ntime;
				ntime.str("");
				ntime << nbt.getNumerator() << "/" << nbt.getDenominator();
				field->setValue("auto", "nextBarTime", ntime.str());
				field = field->getNextFieldToken();
			}
		}
		if (current == etok) {
			break;
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_meter::getTimeSigDuration --
//

HumNum Tool_meter::getTimeSigDuration(HTp tsig) {
	HumNum output = 0;
	HumRegex hre;
	if (hre.search(tsig, "^\\*M(\\d+)/(\\d+%?\\d*)")) {
		int top = hre.getMatchInt(1);
		string bot = hre.getMatch(2);
		HumNum botdur = Convert::recipToDuration(bot);
		output = botdur * top;
	}
	return output;
}



//////////////////////////////
//
// Tool_meter::printMeterData --
//

void Tool_meter::printMeterData(HumdrumFile& infile) {
	bool foundLabel = false;
	bool foundData  = false;

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
			continue;
		}

		if ((!foundData) && (!foundLabel) && (!m_nolabelQ) && infile[i].isData()) {
			printLabelLine(infile[i]);
			foundData = true;
		}

		bool hasLabel = false;
		if ((!m_nolabelQ) && (!foundLabel) && (!foundData)) {
			if (searchForLabels(infile[i])) {
				hasLabel = true;
				foundLabel = true;
			}
		}
		printHumdrumLine(infile[i], hasLabel);
	}
}



//////////////////////////////
//
// printLabelLine --
//

void Tool_meter::printLabelLine(HumdrumLine& line) {
	bool forceInterpretation = true;
	bool printLabels = true;
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (token->isKernLike()) {
			i = printKernAndAnalysisSpine(line, i, printLabels, forceInterpretation);
		} else {
			m_humdrum_text << "*";
		}
		if (i < line.getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_meter::printHumdrumLine --
//

void Tool_meter::printHumdrumLine(HumdrumLine& line, bool printLabels) {

	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (token->isKernLike()) {
			i = printKernAndAnalysisSpine(line, i, printLabels);
		} else {
			m_humdrum_text << token;
		}
		if (i < line.getFieldCount() - 1) {
			m_humdrum_text << "\t";
		}
	}
	m_humdrum_text << "\n";
}



//////////////////////////////
//
// Tool_meter::searchForLabels --
//

bool Tool_meter::searchForLabels(HumdrumLine& line) {
	if (!line.isInterpretation()) {
		return false;
	}
	HumRegex hre;
	for (int i=0; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		if (hre.search(token, "^\\*v[ibB]*:")) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// Tool_meter::getHumNum --
//

HumNum Tool_meter::getHumNum(HTp token, const string& parameter) {
	HumRegex hre;
	HumNum output;
	string value = token->getValue("auto", parameter);
	if (hre.search(value, "(\\d+)/(\\d+)")) {
		output = hre.getMatchInt(1);
		output /=  hre.getMatchInt(2);
	} else if (hre.search(value, "(\\d+)")) {
		output = hre.getMatchInt(1);
	}
	return output;
}



//////////////////////////////
//
// Tool_meter::getHumNumString --
//

string Tool_meter::getHumNumString(HumNum input) {
	stringstream tem;
	input.printTwoPart(tem);
	return tem.str();
}



//////////////////////////////
//
// Tool_meter::printKernAndAnalysisSpine --
//

int Tool_meter::printKernAndAnalysisSpine(HumdrumLine& line, int index, bool printLabels, bool forceInterpretation) {
	HTp starttok = line.token(index);
	int track = starttok->getTrack();
	int counter = 0;

	string analysis = ".";
	string numerator = ".";
	string denominator = ".";
	string meter = ".";
	bool hasNote = false;
	bool hasRest = false;

	for (int i=index; i<line.getFieldCount(); i++) {
		HTp token = line.token(i);
		int ttrack = token->getTrack();
		if (ttrack != track) {
			break;
		}
		if (counter > 0) {
			m_humdrum_text << "\t";
		}
		counter++;
		if (forceInterpretation) {
			m_humdrum_text << "*";
		} else {
			m_humdrum_text << token;
		}

		if (line.isData() && !forceInterpretation) {
			if (token->isNull()) {
				// analysis = ".";
			} else if (token->isRest() && !m_restQ) {
				// analysis = ".";
			} else if ((!token->isNoteAttack()) && !(m_restQ && token->isRest())) {
				// analysis = ".";
			} else if ((analysis == ".") && (token->getValueBool("auto", "hasData"))) {
				string data = token->getValue("auto", "zeroBeat");
				if (m_restQ) {
					if (token->isRest()) {
						hasRest = true;
					} else {
						hasNote = true;
					}
				}
				HumNum value;
				HumNum nvalue;
				HumNum dvalue;
				if (!data.empty()) {
					value = getHumNum(token, "zeroBeat");
					if (m_numeratorQ) {
						nvalue = getHumNum(token, "numerator");
						numerator = getHumNumString(nvalue);
					}
					if (m_denominatorQ) {
						dvalue = getHumNum(token, "denominator");
						denominator = getHumNumString(dvalue);
					}
					if (m_tsigQ) {
						meter = numerator;
						meter += "/";
						meter += denominator;
					}
				}
				if (!m_zeroQ) {
					value += 1;
				}
				if (m_floatQ) {
					stringstream tem;
					if (m_digits) {
						tem << std::setprecision(m_digits + 1) << value.getFloat();
					} else {
						tem << value.getFloat();
					}
					analysis = tem.str();
					if (m_commaQ) {
						HumRegex hre;
						hre.replaceDestructive(analysis, ",", "\\.");
					}
				} else {
					analysis = getHumNumString(value);
				}
			}
		} else if (line.isInterpretation() || forceInterpretation) {
			if (token->compare(0, 2, "**") == 0) {
				analysis = "**cdata-beat";
				if (m_tsigQ) {
					meter = "**cdata-tsig";
				}
				if (m_numeratorQ) {
					numerator = "**cdata-num";
				}
				if (m_denominatorQ) {
					denominator = "**cdata-den";
				}
			} else if (*token == "*-") {
				analysis = "*-";
				numerator = "*-";
				denominator = "*-";
				meter = "*-";
			} else if (token->isTimeSignature()) {
				analysis = *token;
				numerator = *token;
				denominator = *token;
				meter = *token;
			} else {
				analysis = "*";
				numerator = "*";
				denominator = "*";
				meter = "*";
				if (printLabels) {
					if (m_quarterQ) {
						analysis = "*vi:4ths:";
					} else if (m_eighthQ) {
						analysis = "*vi:8ths:";
					} else if (m_halfQ) {
						analysis = "*vi:half:";
					} else if (m_wholeQ) {
						analysis = "*vi:whole:";
					} else if (m_sixteenthQ) {
						analysis = "*vi:16ths:";
					} else {
						analysis = "*vi:beat:";
					}
					numerator = "*vi:top:";
					denominator = "*vi:bot:";
					meter = "*vi:tsig:";
					if (m_joinQ) {
						numerator = "";
						denominator = "";
						meter = "";
					}
				}
			}
		} else if (line.isBarline()) {
			analysis = *token;
			numerator = *token;
			denominator = *token;
			meter = *token;
		} else if (line.isCommentLocal()) {
			analysis = "!";
			numerator = "!";
			denominator = "!";
			meter = "!";
		} else {
			cerr << "STRANGE LINE: " << line << endl;
		}
	}

	if (m_joinQ) {
		if (line.isData() && !forceInterpretation) {
			if (m_tsigQ) {
					m_humdrum_text << "\t" << meter;
			} else {
				if (m_numeratorQ) {
					m_humdrum_text << "\t" << numerator;
				}
				if (m_denominatorQ) {
					m_humdrum_text << "\t" << denominator;
				}
			}
		}
		if (!m_nobeatQ) {
			if (line.isData() && !forceInterpretation) {
				m_humdrum_text << ":";
			} else {
				m_humdrum_text << "\t";
			}
			m_humdrum_text << analysis;
			if (line.isData() && hasRest && !hasNote) {
				m_humdrum_text << "r";
			}
		}
	} else {
		if (!m_nobeatQ) {
			m_humdrum_text << "\t" << analysis;
			if (line.isData() && hasRest && !hasNote) {
				m_humdrum_text << "r";
			}
		}
		if (m_tsigQ) {
				m_humdrum_text << "\t" << meter;
		} else {
			if (m_numeratorQ) {
				m_humdrum_text << "\t" << numerator;
			}
			if (m_denominatorQ) {
				m_humdrum_text << "\t" << denominator;
			}
		}

	}

	return index + counter - 1;
}



//////////////////////////////
//
// Tool_meter::getMeterData --
//

void Tool_meter::getMeterData(HumdrumFile& infile) {

	int maxtrack = infile.getMaxTrack();
	vector<HumNum> curNum(maxtrack + 1, 0);
	vector<HumNum> curDen(maxtrack + 1, 0);
	vector<HumNum> curBeat(maxtrack + 1, 0);
	vector<HumNum> curBarTime(maxtrack + 1, 0);

	for (int i=0; i<infile.getLineCount(); i++) {
		processLine(infile[i], curNum, curDen, curBeat, curBarTime);
	}
}



//////////////////////////////
//
// Tool_meter::processLine --
//

void Tool_meter::processLine(HumdrumLine& line, vector<HumNum>& curNum,
		vector<HumNum>& curDen, vector<HumNum>& curBeat,
		vector<HumNum>& curBarTime) {

	int fieldCount = line.getFieldCount();

	if (!line.hasSpines()) {
		return;
	}

	HumRegex hre;
	if (line.isBarline()) {
		for (int i=0; i<fieldCount; i++) {
			HTp token = line.token(i);
			if (!token->isKernLike()) {
				continue;
			}
			if (hre.search(token, "-")) {
				// invisible barline: ignore
				continue;
			}
			int track = token->getTrack();
			HumNum curTime = token->getDurationFromStart();
			curBarTime.at(track) = curTime;
		}
		return;
	}

	if (line.isInterpretation()) {
		// check for time signatures
		for (int i=0; i<fieldCount; i++) {
			HTp token = line.token(i);
			if (!token->isKernLike()) {
				continue;
			}
			if (hre.search(token, "^\\*M(\\d+)/(\\d+)")) {
				int top = hre.getMatchInt(1);
				int bot = hre.getMatchInt(2);
				int track = token->getTrack();
				curNum.at(track) = top;
				curDen.at(track) = bot;
				curBeat.at(track) = 0;
			} else if (hre.search(token, "^\\*beat:\\s*([\\d.%]+)\\s*$")) {
				int track = token->getTrack();
				string recip = hre.getMatch(1);
				curBeat.at(track) = Convert::recipToDuration(recip);
			}
		}
		return;
	}

	if (line.isData()) {
		// check for time signatures
		for (int i=0; i<fieldCount; i++) {
			HTp token = line.token(i);
			if (!token->isKernLike()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if ((!m_restQ) && token->isRest()) {
				continue;
			}
			if (!token->isNoteAttack() && !(m_restQ && token->isRest())) {
				continue;
			}
			int pickup = token->getValueInt("auto", "pickup");
			int track = token->getTrack();
			stringstream value;
			value.str("");
			value << curNum.at(track);
			token->setValue("auto", "numerator", value.str());
			value.str("");
			value << curDen.at(track);
			token->setValue("auto", "denominator", value.str());
			HumNum curTime = token->getDurationFromStart();
			HumNum q;
			if (pickup) {
				HumNum meterDur = curNum.at(track);
				meterDur /= curDen.at(track);
				meterDur *= 4;
				HumNum nbt = getHumNum(token, "nextBarTime");
				q = meterDur - nbt;
			} else {
				q = curTime - curBarTime.at(track);
			}
			value.str("");
			value << q;
			token->setValue("auto", "q", value.str());
			bool compound = false;
			int multiple = curNum.at(track).getNumerator() / 3;
			int remainder = curNum.at(track).getNumerator() % 3;
			int bottom = curDen.at(track).getNumerator();
			if ((curBeat.at(track) == 0) && (bottom >= 8) && (multiple > 1) && (remainder == 0)) {
				compound = true;
			}

			HumNum qq = q;
			if (m_quarterQ) {
				// do nothing (prior calculations are done in quarter notes)
			} else if (m_halfQ) {
				qq /= 2;
			} else if (m_wholeQ) {
				qq /= 4;
			} else if (m_eighthQ) {
				qq *= 2;
			} else if (m_sixteenthQ) {
				qq *= 4;
			} else {
				// convert quarter note metric positions into beat positions
				if (compound) {
					qq *= curDen.at(track);
					qq /= 4;
					qq /= 3;
				} else if (curBeat.at(track) > 0) {
					qq /= curBeat.at(track);
				} else {
					qq *= curDen.at(track);
					qq /= 4;
				}
			}

			value.str("");
			value << qq;
			token->setValue("auto", "zeroBeat", value.str());
			token->setValue("auto", "hasData", 1);

		}
		return;
	}
}


// END_MERGE

} // end namespace hum



