//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 25 19:23:06 PDT 2019
// Last Modified: Wed Sep 25 19:23:08 PDT 2019
// Filename:      musedata2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/musedata2hum.cpp
// Syntax:        C++11; humlib
// vim:           ts=3:noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "tool-musedata2hum.h"
#include "Convert.h"
#include "HumRegex.h"
#include "HumGrid.h"

#include <chrono>
#include <ctime>
#include <sstream>

using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_musedata2hum::Tool_musedata2hum --
//

Tool_musedata2hum::Tool_musedata2hum(void) {
	// Options& options = m_options;
	// options.define("k|kern=b","display corresponding **kern data");

	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");
}



//////////////////////////////
//
// initialize --
//

void Tool_musedata2hum::initialize(void) {
	m_stemsQ = getBoolean("stems");
	m_recipQ = getBoolean("recip");
}



//////////////////////////////
//
// Tool_musedata2hum::setOptions --
//

void Tool_musedata2hum::setOptions(int argc, char** argv) {
	m_options.process(argc, argv);
}


void Tool_musedata2hum::setOptions(const vector<string>& argvlist) {
    m_options.process(argvlist);
}



//////////////////////////////
//
// Tool_musedata2hum::getOptionDefinitions -- Used to avoid
//     duplicating the definitions in the test main() function.
//

Options Tool_musedata2hum::getOptionDefinitions(void) {
	return m_options;
}



//////////////////////////////
//
// Tool_musedata2hum::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool Tool_musedata2hum::convertFile(ostream& out, const string& filename) {
	MuseDataSet mds;
	int result = mds.readFile(filename);
	if (!result) {
		cerr << "\nMuseData file [" << filename << "] has syntax errors\n";
		cerr << "Error description:\t" << mds.getError() << "\n";
		exit(1);
	}
	return convert(out, mds);
}


bool Tool_musedata2hum::convert(ostream& out, istream& input) {
	MuseDataSet mds;
	mds.read(input);
	return convert(out, mds);
}


bool Tool_musedata2hum::convertString(ostream& out, const string& input) {
	MuseDataSet mds;
	int result = mds.readString(input);
	if (!result) {
		cout << "\nXML content has syntax errors\n";
		cout << "Error description:\t" << mds.getError() << "\n";
		exit(1);
	}
	return convert(out, mds);
}


bool Tool_musedata2hum::convert(ostream& out, MuseDataSet& mds) {
	initialize();

	HumGrid outdata;
	int partcount = mds.getPartCount();
	bool status = true;
	for (int i=0; i<partcount; i++) {
		status &= convertPart(outdata, mds, i);
	}

	HumdrumFile outfile;
	outdata.transferTokens(outfile);
	outfile.createLinesFromTokens();

	// Convert comments in header of first part:
	for (int i=0; i< mds[0].getLineCount(); i++) {
		if (mds[0][i].isAnyNote()) {
			break;
		}
		if (mds[0].getLine(i).compare(0, 2, "@@") == 0) {
			string output = mds[0].getLine(i);
			for (int j=0; j<(int)output.size(); j++) {
				if (output[j] == '@') {
					output[j] = '!';
				} else {
					break;
				}
			}
			out << output << endl;
		}
	}

	string composer = mds[0].getComposer();
	if (!composer.empty()) {
		out << "!!!COM: " << composer << endl;
	}

	string cdate = mds[0].getComposerDate();
	if (!cdate.empty()) {
		out << "!!!CDT: " << cdate << endl;
	}

	string worktitle = mds[0].getWorkTitle();
	if (!worktitle.empty()) {
		out << "!!!OTL: " << worktitle << endl;
	}

	string movementtitle = mds[0].getMovementTitle();
	if (!movementtitle.empty()) {
		out << "!!!OMV: " << movementtitle << endl;
	}

	string opus = mds[0].getOpus();
	if (!opus.empty()) {
		out << "!!!OPS: " << opus << endl;
	}

	string number = mds[0].getNumber();
	if (!number.empty()) {
		out << "!!!ONM: " << number << endl;
	}

	if (!m_omd.empty()) {
		out << "!!!OMD: " << m_omd << endl;
	}

	out << outfile;

	string source = mds[0].getSource();
	if (!source.empty()) {
		out << "!!!SMS: " << source << endl;
	}

	string encoder = mds[0].getEncoderName();
	if (!encoder.empty()) {
		out << "!!!ENC: " << encoder << endl;
	}

	string edate = mds[0].getEncoderDate();
	if (!edate.empty()) {
		out << "!!!END: " << edate << endl;
	}

	stringstream ss;
	auto nowtime = std::chrono::system_clock::now();
	time_t currenttime = std::chrono::system_clock::to_time_t(nowtime);
	ss << std::ctime(&currenttime);
	out << "!!!ONB: Converted from MuseData with musedata2hum on " << ss.str();

	string copyright = mds[0].getCopyright();
	if (!copyright.empty()) {
		out << "!!!YEM: " << copyright << endl;
	}

	// Convert comments in footer of last part:
	int lastone = mds.getPartCount() - 1;
	vector<string> outputs;
	for (int i=mds[lastone].getLineCount() - 1; i>=0; i--) {
		if (mds[lastone][i].isAnyNote()) {
			break;
		}
		if (mds[lastone].getLine(i).compare(0, 2, "@@") == 0) {
			string output = mds[lastone].getLine(i);
			for (int j=0; j<(int)output.size(); j++) {
				if (output[j] == '@') {
					output[j] = '!';
				} else {
					break;
				}
			}
			outputs.push_back(output);
		}
	}

	for (int i=(int)outputs.size() - 1; i>=0; i--) {
		out << outputs[i] << endl;
	}

	return status;
}



//////////////////////////////
//
// Tool_musedata2hum::convertPart --
//

bool Tool_musedata2hum::convertPart(HumGrid& outdata, MuseDataSet& mds, int index) {
	MuseData& part = mds[index];
	m_lastfigure = NULL;
	m_lastnote = NULL;
	m_lastbarnum = -1;
	m_tpq = part.getInitialTpq();
	m_part = index;
	m_maxstaff = (int)mds.getPartCount();
	
	bool status = true;
	int i = 0;
	while (i < part.getLineCount()) {
		i = convertMeasure(outdata, part, index, i);
	}

	storePartName(outdata, part, index);

	return status;
}



///////////////////////////////
//
// Tool_musedata2hum::storePartName --
//

void Tool_musedata2hum::storePartName(HumGrid& outdata, MuseData& part, int index) {
	string name = part.getPartName();
	if (!name.empty()) {
		outdata.setPartName(index, name);
	}
}



//////////////////////////////
//
// Tool_musedata2hum::convertMeasure --
//

int Tool_musedata2hum::convertMeasure(HumGrid& outdata, MuseData& part, int partindex, int startindex) {
	if (part.getLineCount() == 0) {
		return 1;
	}
	HumNum starttime = part[startindex].getAbsBeat();
	HumNum filedur = part.getFileDuration();
	HumNum diff = filedur - starttime;
	if (diff == 0) {
		// last barline in score, so ignore
		return startindex + 1;;
	}

	GridMeasure* gm = getMeasure(outdata, starttime);
	setMeasureNumber(outdata[(int)outdata.size() - 1], part[startindex]);
	if (partindex == 0) {
		gm->setBarStyle(MeasureStyle::Plain);
	}
	int i = startindex;
	for (i=startindex; i<part.getLineCount(); i++) {
		if ((i != startindex) && part[i].isBarline()) {
			break;
		}
		convertLine(gm, part[i]);
	}
	HumNum endtime = starttime;
	if (i >= part.getLineCount()) {
		endtime = part[i-1].getAbsBeat();
	} else {
		endtime = part[i].getAbsBeat();
	}

	// set duration of measures (so it will be printed in conversion to Humdrum):
	gm->setDuration(endtime - starttime);
	gm->setTimestamp(starttime);
	gm->setTimeSigDur(m_timesigdur);

	if ((i < part.getLineCount()) && part[i].isBarline()) {
		if (partindex == 0) {
			// For now setting the barline style from the 
			// lowest staff.  This is mostly because
			// MEI/verovio can handle only one style
			// on a system barline.  But also because
			// GridMeasure objects only has a setting
			// for a single barline style.
			setMeasureStyle(outdata.back(), part[i]);
		}
	}

	return i;
}

//////////////////////////////
//
// Tool_musedata2hum::setMeasureNumber --
//

void Tool_musedata2hum::setMeasureNumber(GridMeasure* gm, MuseRecord& mr) {
	int pos = -1;
	string line = mr.getLine();
	bool space = false;
	for (int i=0; i<(int)line.size(); i++) {
		if (isspace(line[i])) {
			space = true;
			continue;
		}
		if (!space) {
			continue;
		}
		if (isdigit(line[i])) {
			pos = i;
			break;
		}
	}
	if (pos < 0) {
		return;
	}
	int num = stoi(line.substr(pos));
	if (m_lastbarnum >= 0) {
		int temp = num;
		num = m_lastbarnum;
		m_lastbarnum = temp;
	}
	gm->setMeasureNumber(num);
}



//////////////////////////////
//
// Tool_musedata2hum::setMeasureStyle --
//

void Tool_musedata2hum::setMeasureStyle(GridMeasure* gm, MuseRecord& mr) {
	// Add bar numbers as well.
	string line = mr.getLine();
	string barstyle = mr.getMeasureFlagsString();
	if (line.compare(0, 7, "mheavy2") == 0) {
		if (barstyle.find(":|") != string::npos) {
			gm->setStyle(MeasureStyle::RepeatBackward);
		} else {
			gm->setStyle(MeasureStyle::Final);
		}
	} else if (line.compare(0, 7, "mheavy3") == 0) {
		if (barstyle.find("|:") != string::npos) {
			gm->setStyle(MeasureStyle::RepeatForward);
		}
	} else if (line.compare(0, 7, "mheavy4") == 0) {
		if (barstyle.find(":|:") != string::npos) {
			gm->setStyle(MeasureStyle::RepeatBoth);
		}
	} else if (line.compare(0, 7, "mdouble") == 0) {
		gm->setStyle(MeasureStyle::Double);
	}
}


//////////////////////////////
//
// Tool_musedata2hum::convertLine --
//

void Tool_musedata2hum::convertLine(GridMeasure* gm, MuseRecord& mr) {
	int tpq          = m_tpq;
	int part         = m_part;
	int staff        = 0;
	int maxstaff     = m_maxstaff;
	int layer        = mr.getLayer();
	if (layer > 0) {
		// convert to an index:
		layer = layer - 1;
	}
	
	HumNum timestamp = mr.getAbsBeat();
	string tok;
	GridSlice* slice = NULL;

	if (mr.isBarline()) {
		tok = mr.getKernMeasureStyle();
	} else if (mr.isAttributes()) {
		map<string, string> attributes;
		mr.getAttributeMap(attributes);

		string mtempo = trimSpaces(attributes["D"]);
		if (!mtempo.empty()) {
			if (timestamp != 0) {
				string value = "!!!OMD: " + mtempo;
				gm->addGlobalComment(value, timestamp);
			} else {
				setInitialOmd(mtempo);
			}
		}

		string mclef = attributes["C"];
		if (!mclef.empty()) {
			string kclef = Convert::museClefToKernClef(mclef);
			if (!kclef.empty()) {
				gm->addClefToken(kclef, timestamp, part, staff, layer, maxstaff);
			}
		}

		string mkeysig = attributes["K"];
		if (!mkeysig.empty()) {
			string kkeysig = Convert::museKeySigToKernKeySig(mkeysig);
			gm->addKeySigToken(kkeysig, timestamp, part, staff, layer, maxstaff);
		}

		string mtimesig = attributes["T"];
		if (!mtimesig.empty()) {
			string ktimesig = Convert::museTimeSigToKernTimeSig(mtimesig);
			slice = gm->addTimeSigToken(ktimesig, timestamp, part, staff, layer, maxstaff);
			setTimeSigDurInfo(ktimesig);
			string kmeter = Convert::museMeterSigToKernMeterSig(mtimesig);
			if (!kmeter.empty()) {
				slice = gm->addMeterSigToken(kmeter, timestamp, part, staff, layer, maxstaff);
			}
		}
	} else if (mr.isRegularNote()) {
		tok = mr.getKernNoteStyle(1, 1);
		slice = gm->addDataToken(tok, timestamp, part, staff, layer, maxstaff);
		if (slice) {
			mr.setVoice(slice->at(part)->at(staff)->at(layer));
			string gr = mr.getLayoutVis();
			if (gr.size() > 0) {
				cerr << "GRAPHIC VERSION OF NOTEA " << gr << endl;
			}
		}
		m_lastnote = slice->at(part)->at(staff)->at(layer)->getToken();
		addNoteDynamics(slice, part, mr);
		addLyrics(slice, part, staff, mr);
	} else if (mr.isFiguredHarmony()) {
		addFiguredHarmony(mr, gm, timestamp, part, maxstaff);
	} else if (mr.isChordNote()) {
		tok = mr.getKernNoteStyle(1, 1);
		if (m_lastnote) {
			string text = m_lastnote->getText();
			text += " ";
			text += tok;
			m_lastnote->setText(text);
		} else {
			cerr << "Warning: found chord note with no regular note to attach to" << endl;
		}
	} else if (mr.isCueNote()) {
		cerr << "PROCESS CUE NOTE HERE: " << mr << endl;
	} else if (mr.isGraceNote()) {
		cerr << "PROCESS GRACE NOTE HERE: " << mr << endl;
	} else if (mr.isChordGraceNote()) {
		cerr << "PROCESS GRACE CHORD NOTE HERE: " << mr << endl;
	} else if (mr.isAnyRest()) {
		tok  = mr.getKernRestStyle(tpq);
		slice = gm->addDataToken(tok, timestamp, part, staff, layer, maxstaff);
		if (slice) {
			mr.setVoice(slice->at(part)->at(staff)->at(layer));
			string gr = mr.getLayoutVis();
			if (gr.size() > 0) {
				cerr << "GRAPHIC VERSION OF NOTEB " << gr << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_musedata2hum::addFiguredHarmony --
//

void Tool_musedata2hum::addFiguredHarmony(MuseRecord& mr, GridMeasure* gm,
		HumNum timestamp, int part, int maxstaff) {
	string fh = mr.getFigureString();
	fh = Convert::museFiguredBassToKernFiguredBass(fh);
	if (fh.find(":") == string::npos) {
		HTp fhtok = new HumdrumToken(fh);
		m_lastfigure = fhtok;
		gm->addFiguredBass(fhtok, timestamp, part, maxstaff);
		return;
	}

	if (!m_lastfigure) {
		HTp fhtok = new HumdrumToken(fh);
		m_lastfigure = fhtok;
		gm->addFiguredBass(fhtok, timestamp, part, maxstaff);
		return;
	}

	// For now assuming only one line extension needs to be transferred.

	// Has a line extension that should be moved to the previous token:
	int position = 0;
	int colpos = -1;
	if (fh[0] == ':') {
		colpos = 0;
	} else {
		for (int i=1; i<(int)fh.size(); i++) {
			if (isspace(fh[i]) && !isspace(fh[i-1])) {
				position++;
			}
			if (fh[i] == ':') {
				colpos = i;
				break;
			}
		}
	}

	string lastfh = m_lastfigure->getText();
	vector<string> pieces;
	int state = 0;
	for (int i=0; i<(int)lastfh.size(); i++) {
		if (state) {
			if (isspace(lastfh[i])) {
				state = 0;
			} else {
				pieces.back() += lastfh[i];
			}
		} else {
			if (isspace(lastfh[i])) {
				// do nothing
			} else {
				pieces.resize(pieces.size()+1);
				pieces.back() += lastfh[i];
				state = 1;
			}
		}
	}

	if (pieces.empty() || (position >= (int)pieces.size())) {
		HTp fhtok = new HumdrumToken(fh);
		m_lastfigure = fhtok;
		gm->addFiguredBass(fhtok, timestamp, part, maxstaff);
		return;
	}

	pieces[position] += ':';
	string oldtok;
	for (int i=0; i<(int)pieces.size(); i++) {
		oldtok += pieces[i];
		if (i<(int)pieces.size() - 1) {
			oldtok += ' ';
		}
	}

	m_lastfigure->setText(oldtok);

	fh.erase(colpos, 1);
	HTp newtok = new HumdrumToken(fh);
	m_lastfigure = newtok;
	gm->addFiguredBass(newtok, timestamp, part, maxstaff);
}



//////////////////////////////
//
// Tool_musedata2hum::addLyrics --
//

void Tool_musedata2hum::addLyrics(GridSlice* slice, int part, int staff, MuseRecord& mr) {
	int versecount = mr.getVerseCount();
	if (versecount == 0) {
		return;
	}
	for (int i=0; i<versecount; i++) {
		string verse = mr.getVerseUtf8(i);
		slice->at(part)->at(staff)->setVerse(i, verse);
	}
	slice->reportVerseCount(part, staff, versecount);
}



//////////////////////////////
//
// Tool_musedata2hum::addNoteDynamics --
//

void Tool_musedata2hum::addNoteDynamics(GridSlice* slice, int part, 
		MuseRecord& mr) {
	string notations = mr.getAdditionalNotationsField();
	vector<string> dynamics(1);
	int state = 0;
	for (int i=0; i<(int)notations.size(); i++) {
		if (state) {
			switch (notations[i]) {
				case 'p':
				case 'm':
				case 'f':
					dynamics.back() += notations[i];
					break;
				default:
					state = 0;
					dynamics.resize(dynamics.size() + 1);
			}
		} else {
			switch (notations[i]) {
				case 'p':
				case 'm':
				case 'f':
					state = 1;
					dynamics.back() = notations[i];
					break;
			}
		}
	}

	bool setdynamics = false;
	for (int i=0; i<(int)dynamics.size(); i++) {
		if (dynamics[i].empty()) {
			continue;
		}
		slice->at(part)->setDynamics(dynamics[i]);
		setdynamics = true;
		break;  // only one dynamic allowed (at least for now)
	}

	if (setdynamics) {
		HumGrid* grid = slice->getOwner();
		if (grid) {
			grid->setDynamicsPresent(part);
		}
	}
}



//////////////////////////////
//
// Tool_musedata2hum::setTimeSigDurInfo --
//

void Tool_musedata2hum::setTimeSigDurInfo(const string& ktimesig) {
	HumRegex hre;
	if (hre.search(ktimesig, "(\\d+)/(\\d+)")) {
		int top = hre.getMatchInt(1);
		int bot = hre.getMatchInt(2);
		HumNum value = 1;
		value /= bot;
		value *= top;
		value.invert();
		value *= 4;  // convert from whole notes to quarter notes
		m_timesigdur = value;
	}
}



//////////////////////////////
//
// Tool_musedata2hum::getMeasure --  Could be imporoved by NlogN search.
//

GridMeasure* Tool_musedata2hum::getMeasure(HumGrid& outdata, HumNum starttime) {
	for (int i=0; i<(int)outdata.size(); i++) {
		if (outdata[i]->getTimestamp() == starttime) {
			return outdata[i];
		}
	}
	// Did not find measure in data, so append to end of list.
	// Assuming that unknown measures are at a later timestamp
	// than those in current list, but should fix this later perhaps.
	GridMeasure* gm = new GridMeasure(&outdata);
	outdata.push_back(gm);
	return gm;
}



//////////////////////////////
//
// Tool_musedata2hum::setInitialOmd --
//

void Tool_musedata2hum::setInitialOmd(const string& omd) {
	m_omd = omd;
}



//////////////////////////////
//
// Tool_musedata2hum::trimSpaces --
//

string Tool_musedata2hum::trimSpaces(string input) {
	string output;
	int status = 0;
	for (int i=0; i<(int)input.size(); i++) {
		if (!status) {
			if (isspace(input[i])) {
				continue;
			}
			status = 1;
		}
		output += input[i];
	}
	for (int i=(int)output.size()-1; i>=0; i--) {
		if (isspace(output[i])) {
			output.resize((int)output.size() - 1);
		} else {
			break;
		}
	}
	return output;
}



// END_MERGE

} // end namespace hum


