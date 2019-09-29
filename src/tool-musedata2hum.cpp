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
	if (!m_omd.empty()) {
		out << "!!!OMD:\t" << m_omd << endl;
	}
	out << outfile;

	return status;
}



//////////////////////////////
//
// Tool_musedata2hum::convertPart --
//

bool Tool_musedata2hum::convertPart(HumGrid& outdata, MuseDataSet& mds, int index) {
	MuseData& part = mds[index];

	m_tpq = part.getInitialTpq();
	m_part = index;
	m_maxstaff = (int)mds.getPartCount();
	
	bool status = true;
	int i = 0;
	while (i < part.getLineCount()) {
		i = convertMeasure(outdata, part, i);
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

int Tool_musedata2hum::convertMeasure(HumGrid& outdata, MuseData& part, int startindex) {
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
	gm->setBarStyle(MeasureStyle::Plain);
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
		setMeasureStyle(outdata.back(), part[i]);
	}

	return i;
}



//////////////////////////////
//
// Tool_musedata2hum::setMeasureStyle --
//

void Tool_musedata2hum::setMeasureStyle(GridMeasure* gm, MuseRecord& mr) {
	string line = mr.getLine();
	if (line.compare(0, 7, "mheavy2") == 0) {
		gm->setStyle(MeasureStyle::Final);
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
	int voice        = 0;
	HumNum timestamp = mr.getAbsBeat();
	string tok;

	if (mr.isBarline()) {
		tok = mr.getKernMeasureStyle();
	} else if (mr.isAttributes()) {
		map<string, string> attributes;
		mr.getAttributeMap(attributes);

		string mtempo = attributes["D"];
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
			gm->addClefToken(kclef, timestamp, part, staff, voice, maxstaff);
		}

		string mkeysig = attributes["K"];
		if (!mkeysig.empty()) {
			string kkeysig = Convert::museKeySigToKernKeySig(mkeysig);
			gm->addKeySigToken(kkeysig, timestamp, part, staff, voice, maxstaff);
		}

		string mtimesig = attributes["T"];
		if (!mtimesig.empty()) {
			string ktimesig = Convert::museTimeSigToKernTimeSig(mtimesig);
			gm->addTimeSigToken(ktimesig, timestamp, part, staff, voice, maxstaff);
			setTimeSigDurInfo(ktimesig);
		}

	} else if (mr.isNote()) {
		tok = mr.getKernNoteStyle(1, 1);
		GridSlice* slice;
		slice = gm->addDataToken(tok, timestamp, part, staff, voice, maxstaff);
		addNoteDynamics(slice, part, mr);
	} else if (mr.isFiguredHarmony()) {
		string fh = mr.getFigureString();
		fh = Convert::museFiguredBassToKernFiguredBass(fh);
		gm->addFiguredBass(fh, timestamp, part, maxstaff);
	} else if (mr.isChordNote()) {
		cerr << "PROCESS CHORD NOTE HERE: " << mr << endl;
	} else if (mr.isCueNote()) {
		cerr << "PROCESS CUE NOTE HERE: " << mr << endl;
	} else if (mr.isGraceNote()) {
		cerr << "PROCESS GRACE NOTE HERE: " << mr << endl;
	} else if (mr.isChordGraceNote()) {
		cerr << "PROCESS GRACE CHORD NOTE HERE: " << mr << endl;
	} else if (mr.isRest()) {
		tok  = mr.getKernRestStyle(tpq);
		gm->addDataToken(tok, timestamp, part, staff, voice, maxstaff);
	}
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

	for (int i=0; i<(int)dynamics.size(); i++) {
		if (dynamics[i].empty()) {
			continue;
		}
		slice->at(part)->setDynamics(dynamics[i]);
		break;  // only one dynamic allowed (at least for now)
	}

	HumGrid* grid = slice->getOwner();
	if (grid) {
		grid->setDynamicsPresent(part);
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




// END_MERGE

} // end namespace hum


