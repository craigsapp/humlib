//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 15 09:57:12 CEST 2018
// Last Modified: Sun Jul 15 09:57:16 CEST 2018
// Filename:      tool-simat.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-simat.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Similarity Matrix tool.
//

#include "tool-simat.h"

#include <algorithm>
#include <sstream>

#include "Convert.h"
#include "HumRegex.h"

#include "pugixml.hpp"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MeasureData::MeasureData --
//

MeasureData::MeasureData(void) {
	m_hist7pc.resize(7);
	std::fill(m_hist7pc.begin(), m_hist7pc.end(), 0.0);
}


MeasureData::MeasureData(HumdrumFile& infile, int startline, int stopline) {
	setStartLine(startline);
	setStopLine(stopline);
	setOwner(infile);
}


MeasureData::MeasureData(HumdrumFile* infile, int startline, int stopline) {
	setStartLine(startline);
	setStopLine(stopline);
	setOwner(infile);
}



//////////////////////////////
//
// MeasureData::~MeasureData --
//

MeasureData::~MeasureData() {
	clear();
}



//////////////////////////////
//
// MeasureData::setOwner --
//

void MeasureData::setOwner(HumdrumFile* infile) {
	m_owner = infile;
}


void MeasureData::setOwner(HumdrumFile& infile) {
	m_owner = &infile;
}



//////////////////////////////
//
// MeasureData::setStartLine --
//

void MeasureData::setStartLine(int startline) {
	m_startline = startline;
}



//////////////////////////////
//
// MeasureData::setStopLine --
//

void MeasureData::setStopLine(int stopline) {
	m_stopline = stopline;
}



//////////////////////////////
//
// MeasureData::getStartLine --
//

int MeasureData::getStartLine(void) {
	return m_startline;
}



//////////////////////////////
//
// MeasureData::getStopLine --
//

int MeasureData::getStopLine(void) {
	return m_stopline;
}



//////////////////////////////
//
// MeasureData::getStartTime -- return the start time in
//     quarter notes
//

double MeasureData::getStartTime(void) {
	if (m_owner == NULL) {
		return 0.0;
	}
	if (getStartLine() < 0) {
		return 0.0;
	}
	return (*m_owner)[getStartLine()].getDurationFromStart().getFloat();
}



//////////////////////////////
//
// MeasureData::getMeasure -- return the measure number of the measure.
//   return -1 if no measure number.
//

int MeasureData::getMeasure(void) {
	if (m_owner == NULL) {
		return -1;
	}
	if (getStartLine() < 0) {
		return -1;
	}
	HumdrumFile& infile = *m_owner;
	if (!infile[getStartLine()].isBarline()) {
		return -1;
	}
	HumRegex hre;
	if (hre.search(infile.token(getStartLine(), 0), "(\\d+)")) {
		return hre.getMatchInt(1);
	} else {
		return -1;
	}
}



//////////////////////////////
//
// MeasureData::getQon -- return the start time class id of the measure.
//

std::string MeasureData::getQon(void) {
	if (m_owner == NULL) {
		return "";
	}
	if (getStartLine() < 0) {
		return "";
	}
	HumdrumFile& infile = *m_owner;
	HumNum ts =  infile[getStartLine()].getDurationFromStart();
	string output = "qon" + to_string(ts.getNumerator());
	if (ts.getDenominator() != 1) {
		output += "-" + to_string(ts.getDenominator());
	}
	return output;
}



//////////////////////////////
//
// MeasureData::getQoff -- return the end time class id of the measure.
//

std::string MeasureData::getQoff(void) {
	if (m_owner == NULL) {
		return "";
	}
	if (getStopLine() < 0) {
		return "";
	}
	HumdrumFile& infile = *m_owner;
	HumNum ts =  infile[getStopLine()].getDurationFromStart();
	string output = "qoff" + to_string(ts.getNumerator());
	if (ts.getDenominator() != 1) {
		output += "-" + to_string(ts.getDenominator());
	}
	return output;
}



//////////////////////////////
//
// MeasureData::getStopTime -- return the stop time in
//     quarter notes
//

double MeasureData::getStopTime(void) {
	if (m_owner == NULL) {
		return 0.0;
	}
	if (getStopLine() < 0) {
		return 0.0;
	}
	return (*m_owner)[getStopLine()].getDurationFromStart().getFloat();
}



//////////////////////////////
//
// MeasureData::getDuration -- return the duration of the measure
//     int quarter notes
//

double MeasureData::getDuration(void) {
	return getStopTime() - getStartTime();
}



//////////////////////////////
//
// MeasureData::getScoreDuration --
//

double MeasureData::getScoreDuration(void) {
	if (m_owner == NULL) {
		return 0.0;
	}
	return m_owner->getScoreDuration().getFloat();
}



//////////////////////////////
//
// MeasureData::clear --
//

void MeasureData::clear(void) {
	m_owner = NULL;
	m_owner       = NULL;
	m_startline   = -1;
	m_startline   = -1;
	m_hist7pc.resize(7);
	std::fill(m_hist7pc.begin(), m_hist7pc.end(), 0.0);
	m_sum7pc      = 0.0;
}



//////////////////////////////
//
// MeasureData::getHistogram7pc --
//

std::vector<double>& MeasureData::getHistogram7pc(void) {
	return m_hist7pc;
}


//////////////////////////////
//
// MeasureData::getSum7pc --
//

double MeasureData::getSum7pc(void) {
	return m_sum7pc;
}



//////////////////////////////
//
// MeasureData::generateNoteHistogram --
//

void MeasureData::generateNoteHistogram(void) {
	m_hist7pc.resize(7);
	std::fill(m_hist7pc.begin(), m_hist7pc.end(), 0.0);
	m_sum7pc = 0;
	if (m_owner == NULL) {
		return;
	}
	if (m_startline < 0) {
		return;
	}
	if (m_stopline < 0) {
		return;
	}

	HumdrumFile& infile = *m_owner;
	for (int i=m_startline; i<m_stopline; i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
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
			double duration = token->getDuration().getFloat();
			int subtokcount = token->getSubtokenCount();
			for (int k=0; k<subtokcount; k++) {
				string subtok = token->getSubtoken(k);
				int pc = Convert::kernToBase7PC(subtok);
				if (pc < 0) {
					continue;
				}
				m_hist7pc.at(pc) += duration;
			}
		}
	}
	m_sum7pc = 0.0;
	for (int i=0; i<(int)m_hist7pc.size(); i++) {
		m_sum7pc += m_hist7pc[i];
	}
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MeasureDataSet::MeasureDataSet --
//

MeasureDataSet::MeasureDataSet(void) {
	m_data.reserve(1000);
}



//////////////////////////////
//
// MeasureDataSet::MeasureDataSet --
//

MeasureDataSet::MeasureDataSet(HumdrumFile& infile) {
	parse(infile);
}



//////////////////////////////
//
// MeasureDataSet::~MeasureDataSet --
//

MeasureDataSet::~MeasureDataSet() {
	clear();
}



//////////////////////////////
//
// MeasureDataSet::clear --
//

void MeasureDataSet::clear(void) {
	for (int i=0; i<(int)m_data.size(); i++) {
		delete m_data[i];
	}
	m_data.clear();
}



//////////////////////////////
//
// MeasureDataSet::parse --
//

int MeasureDataSet::parse(HumdrumFile& infile) {
	int lastbar = 0;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		MeasureData* info = new MeasureData(infile, lastbar, i);
		info->generateNoteHistogram();
		m_data.push_back(info);
		lastbar = i;
	}
	MeasureData* info = new MeasureData(infile, lastbar, infile.getLineCount() - 1);
	m_data.push_back(info);
	return 1;
}



//////////////////////////////
//
// MeasureDataSet::operator[] --
//

MeasureData& MeasureDataSet::operator[](int index) {
	return *m_data[index];
}



//////////////////////////////
//
// MeasureDataSet::getScoreDuration --
//

double MeasureDataSet::getScoreDuration(void) {
	if (m_data.empty()) {
		return 0.0;
	}
	return m_data[0]->getScoreDuration();

}



///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// MeasureComparison::MeasureComparison --
//

MeasureComparison::MeasureComparison() {
	// do nothing
}


MeasureComparison::MeasureComparison(MeasureData& data1, MeasureData& data2) {
	compare(data1, data2);
}


MeasureComparison::MeasureComparison(MeasureData* data1, MeasureData* data2) {
	compare(data1, data2);
}



//////////////////////////////
//
// MeasureComparison::~MeasureComparison --
//

MeasureComparison::~MeasureComparison() {
	clear();
}



//////////////////////////////
//
// MeasureComparison::clear --
//

void MeasureComparison::clear(void) {
	correlation7pc = 0.0;
}



//////////////////////////////
//
// MeasureComparison::compare --
//

void MeasureComparison::compare(MeasureData& data1, MeasureData& data2) {
	compare(&data1, &data2);
}


void MeasureComparison::compare(MeasureData* data1, MeasureData* data2) {
	double sum1 = data1->getSum7pc();
	double sum2 = data2->getSum7pc();
	if ((sum1 == sum2) && (sum1 == 0.0)) {
		correlation7pc = 1.0;
		return;
	}
	if (sum1 == 0.0) {
		correlation7pc = 0.0;
		return;
	}
	if (sum2 == 0.0) {
		correlation7pc = 0.0;
		return;
	}
	correlation7pc = Convert::pearsonCorrelation(data1->getHistogram7pc(), data2->getHistogram7pc());
	if (fabs(correlation7pc - 1.0) < 0.00000001) {
		correlation7pc = 1.0;
	}
}



//////////////////////////////
//
// MeasureComparison::getCorrelation7pc --
//

double MeasureComparison::getCorrelation7pc(void) {
	return correlation7pc;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// MeasureComparisonGrid::MeasureComparisonGrid --
//

MeasureComparisonGrid::MeasureComparisonGrid(void) {
	// do nothing
}


MeasureComparisonGrid::MeasureComparisonGrid(MeasureDataSet& set1, MeasureDataSet& set2) {
	analyze(set1, set2);
}


MeasureComparisonGrid::MeasureComparisonGrid(MeasureDataSet* set1, MeasureDataSet* set2) {
	analyze(set1, set2);
}



//////////////////////////////
//
// MeasureComparisonGrid::~MeasureComparisonGrid --
//

MeasureComparisonGrid::~MeasureComparisonGrid() {
	// do nothing
}



//////////////////////////////
//
// MeasureComparisonGrid::clear --
//

void MeasureComparisonGrid::clear(void) {
	m_grid.clear();
}



//////////////////////////////
//
// MeasureComparisonGrid::analyze --
//

void MeasureComparisonGrid::analyze(MeasureDataSet* set1, MeasureDataSet* set2) {
	analyze(*set1, *set2);
}

void MeasureComparisonGrid::analyze(MeasureDataSet& set1, MeasureDataSet& set2) {
	m_grid.resize(set1.size());
	for (int i=0; i<(int)m_grid.size(); i++) {
		m_grid[i].resize(set2.size());
	}
	for (int i=0; i<(int)m_grid.size(); i++) {
		for (int j=0; j<(int)m_grid[i].size(); j++) {
			m_grid[i][j].compare(set1[i], set2[j]);
		}
	}
	m_set1 = &set1;
	m_set2 = &set2;
}



//////////////////////////////
//
// MeasureComparisonGrid::printCorrelationGrid --
//    default value: out = std::cout
//

ostream& MeasureComparisonGrid::printCorrelationGrid(ostream& out) {
	for (int i=0; i<(int)m_grid.size(); i++) {
		for (int j=0; j<(int)m_grid[i].size(); j++) {
			double correl = m_grid[i][j].getCorrelation7pc();
			if (correl > 0.0) {
				out << int(correl * 100.0 + 0.5)/100.0;
			} else {
				out << -int(-correl * 100.0 + 0.5)/100.0;
			}
			if (j < (int)m_grid[i].size() - 1) {
				out << '\t';
			}
		}
		out << endl;
	}
	return out;
}



//////////////////////////////
//
// MeasureComparisonGrid::printCorrelationDiagonal -- Assuming a square grid for now.
//    default value: out = std::cout
//

ostream& MeasureComparisonGrid::printCorrelationDiagonal(ostream& out) {
	for (int i=0; i<(int)m_grid.size(); i++) {
		for (int j=0; j<(int)m_grid[i].size(); j++) {
			if (i != j) {
				continue;
			}
			double correl = m_grid[i][j].getCorrelation7pc();
			if (correl > 0.0) {
				out << int(correl * 100.0 + 0.5)/100.0;
			} else {
				out << -int(-correl * 100.0 + 0.5)/100.0;
			}
			if (j < (int)m_grid[i].size() - 1) {
				out << '\t';
			}
		}
		out << endl;
	}
	return out;
}



//////////////////////////////
//
// MeasureComparisonGrid::getColorMapping --
//

void MeasureComparisonGrid::getColorMapping(double input, double& hue,
		double& saturation, double& lightness) {
	double maxhue = 0.75 * 360.0;
	hue = input;
	if (hue < 0.0) {
		hue = 0.0;
	}
	hue = hue * hue;
	if (hue != 1.0) {
		hue *= 0.95;
	}

	hue = (1.0 - hue) * 360.0;
	if (hue == 0.0) {
		// avoid -0.0;
		hue = 0.0;
	}

	if (hue > maxhue) {
		hue = maxhue;
	}
	if (hue < 0.0) {
		hue = maxhue;
	}

	saturation = 100.0;
	lightness = 50.0;

	if (hue > 60) {
		lightness = lightness - (hue-60) / (maxhue-60) * lightness / 1.5;
	}
}



//////////////////////////////
//
// MeasureComparisonGrid::getQoff1 -- return the end time class ID of the
//     current grid cell (for the first piece being compared).
//

std::string MeasureComparisonGrid::getQoff1(int index) {
	if (m_set1 == NULL) {
		return "";
	}
	return (*m_set1)[index].getQoff();
}



//////////////////////////////
//
// MeasureComparisonGrid::getQoff2 -- return the end time class ID of the
//     current grid cell (for the first piece being compared).
//

std::string MeasureComparisonGrid::getQoff2(int index) {
	if (m_set2 == NULL) {
		return "";
	}
	return (*m_set2)[index].getQoff();
}



//////////////////////////////
//
// MeasureComparisonGrid::getQon1 -- return the start time class ID of the
//     current grid cell (for the first piece being compared).
//

string MeasureComparisonGrid::getQon1(int index) {
	if (m_set1 == NULL) {
		return "";
	}
	return (*m_set1)[index].getQon();
}



//////////////////////////////
//
// MeasureComparisonGrid::getQon2 -- return the start time class ID of the
//     current grid cell (for the second piece being compared).
//

string MeasureComparisonGrid::getQon2(int index) {
	if (m_set2 == NULL) {
		return "";
	}
	return (*m_set2)[index].getQon();
}



//////////////////////////////
//
// MeasureComparisonGrid::getMeasure1 -- return the measure of the
//     current grid cell (for the first piece being compared).
//

int MeasureComparisonGrid::getMeasure1(int index) {
	if (m_set1 == NULL) {
		return 0.0;
	}
	return (*m_set1)[index].getMeasure();
}



//////////////////////////////
//
// MeasureComparisonGrid::getMeasure2 -- return the measure of the
//     current grid cell (for the second piece being compared).
//

int MeasureComparisonGrid::getMeasure2(int index) {
	if (m_set2 == NULL) {
		return 0.0;
	}
	return (*m_set2)[index].getMeasure();
}



//////////////////////////////
//
// MeasureComparisonGrid::getStartTime1 -- return the start time of the
//     measure at index position in the first compared score.
//

double MeasureComparisonGrid::getStartTime1(int index) {
	if (m_set1 == NULL) {
		return 0.0;
	}
	return (*m_set1)[index].getStartTime();
}



//////////////////////////////
//
// MeasureComparisonGrid::getScoreDuration1 --
//

double MeasureComparisonGrid::getScoreDuration1(void) {
	if (m_set1 == NULL) {
		return 0.0;
	}
	return m_set1->getScoreDuration();
}



//////////////////////////////
//
// MeasureComparisonGrid::getStartTime2 --
//

double MeasureComparisonGrid::getStartTime2(int index) {
	if (m_set2 == NULL) {
		return 0.0;
	}
	return (*m_set2)[index].getStartTime();
}



//////////////////////////////
//
// MeasureComparisonGrid::getStopTime1 --
//

double MeasureComparisonGrid::getStopTime1(int index) {
	if (m_set1 == NULL) {
		return 0.0;
	}
	return (*m_set1)[index].getStopTime();
}



//////////////////////////////
//
// MeasureComparisonGrid::getStopTime2 --
//

double MeasureComparisonGrid::getStopTime2(int index) {
	if (m_set2 == NULL) {
		return 0.0;
	}
	return (*m_set2)[index].getStopTime();
}



//////////////////////////////
//
// MeasureComparisonGrid::getDuration1 --
//

double MeasureComparisonGrid::getDuration1(int index) {
	if (m_set1 == NULL) {
		return 0.0;
	}
	return (*m_set1)[index].getDuration();
}



//////////////////////////////
//
// MeasureComparisonGrid::getDuration2 --
//

double MeasureComparisonGrid::getDuration2(int index) {
	if (m_set2 == NULL) {
		return 0.0;
	}
	return (*m_set2)[index].getDuration();
}



//////////////////////////////
//
// MeasureComparisonGrid::getScoreDuration2 --
//

double MeasureComparisonGrid::getScoreDuration2(void) {
	if (m_set2 == NULL) {
		return 0.0;
	}
	return m_set2->getScoreDuration();
}



//////////////////////////////
//
// MeasureComparisonGrid::printSvgGrid --
//    default value: out = std::cout
//

ostream& MeasureComparisonGrid::printSvgGrid(ostream& out) {
	pugi::xml_document image;
	auto declaration = image.prepend_child(pugi::node_declaration);
	declaration.append_attribute("version") = "1.0";
	declaration.append_attribute("encoding") = "UTF-8";
	declaration.append_attribute("standalone") = "no";

	auto svgnode = image.append_child("svg");
	svgnode.append_attribute("version") = "1.1";
	svgnode.append_attribute("xmlns") = "http://www.w3.org/2000/svg";
	svgnode.append_attribute("xmlns:xlink") = "http://www.w3.org/1999/xlink";
	svgnode.append_attribute("overflow") = "visible";
	svgnode.append_attribute("viewBox") = "0 0 1000 1000";
	svgnode.append_attribute("width") = "1000px";
	svgnode.append_attribute("height") = "1000px";

	auto grid = svgnode.append_child("g");
	grid.append_attribute("id") = "grid";

	double hue = 0.0;
	double saturation = 100;
	double lightness = 75;

	pugi::xml_node crect;
	double width;
	double height;

	stringstream ss;
	stringstream css;
	double x;
	double y;

	double imagewidth = 1000.0;
	double imageheight = 1000.0;

	double sdur1 = getScoreDuration1();
	double sdur2 = getScoreDuration2();

	for (int i=0; i<(int)m_grid.size(); i++) {
		for (int j=0; j<(int)m_grid[i].size(); j++) {
			width = getDuration2(j) / sdur2 * imagewidth;
			height = getDuration1(i) / sdur1 * imageheight;

			x = getStartTime2(j)/sdur2 * imageheight;
			y = getStartTime1(i)/sdur1 * imagewidth;

			getColorMapping(m_grid[i][j].getCorrelation7pc(), hue, saturation, lightness);
			ss << "hsl(" << hue << "," << saturation << "%," << lightness << "%)";
			crect = grid.append_child("rect");
			crect.append_attribute("x") = to_string(x).c_str();
			crect.append_attribute("y") = to_string(y).c_str();
			crect.append_attribute("width") = to_string(width*0.99).c_str();
			crect.append_attribute("height") = to_string(height*0.99).c_str();
			crect.append_attribute("fill") = ss.str().c_str();
			css << "Xm" << getMeasure1(i) << " Ym" << getMeasure2(j);
			css << " X" << getQon1(i)     << " Y" << getQon2(j);
			css << " X" << getQoff1(i)    << " Y" << getQoff2(j);
			crect.append_attribute("class") = css.str().c_str();
			ss.str("");
			css.str("");
		}
	}

	image.save(out);
	return out;
}


///////////////////////////////////////////////////////////////////////////


/////////////////////////////////
//
// Tool_simat::Tool_simat -- Set the recognized options for the tool.
//

Tool_simat::Tool_simat(void) {
	define("r|raw=b", "output raw correlation matrix");
	define("d|diagonal=b", "output diagonal of correlation matrix");
}



/////////////////////////////////
//
// Tool_simat::run -- Primary interfaces to the tool.
//

bool Tool_simat::run(HumdrumFileSet& infiles) {
	bool status = true;
	if (infiles.getCount() == 1) {
		status = run(infiles[0], infiles[0]);
	} else if (infiles.getCount() > 1) {
		status = run(infiles[0], infiles[1]);
	} else {
		status = false;
	}
	return status;
}


bool Tool_simat::run(const string& indata1, const string& indata2, ostream& out) {
	HumdrumFile infile1(indata1);
	HumdrumFile infile2;
	bool status;
	if (indata2.empty()) {
		infile2.read(indata2);
		status = run(infile1, infile2);
	} else {
		status = run(infile1, infile1);
	}
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile1;
		out << infile2;
	}
	return status;
}


bool Tool_simat::run(HumdrumFile& infile1, HumdrumFile& infile2, ostream& out) {
	bool status;
	if (infile2.getLineCount() == 0) {
		status = run(infile1, infile1);
	} else {
		status = run(infile1, infile2);
	}
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile1;
		out << infile2;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_simat::run(HumdrumFile& infile1, HumdrumFile& infile2) {
	if (infile2.getLineCount() == 0) {
		processFile(infile1, infile1);
	} else {
		processFile(infile1, infile2);
	}

	return true;
}



//////////////////////////////
//
// Tool_simat::processFile --
//

void Tool_simat::processFile(HumdrumFile& infile1, HumdrumFile& infile2) {
	m_data1.parse(infile1);
	m_data2.parse(infile2);
	m_grid.analyze(m_data1, m_data2);
	if (getBoolean("raw")) {
		m_grid.printCorrelationGrid(m_free_text);
		suppressHumdrumFileOutput();
	} else if (getBoolean("diagonal")) {
		m_grid.printCorrelationDiagonal(m_free_text);
		suppressHumdrumFileOutput();
	} else {
		m_grid.printSvgGrid(m_free_text);
		suppressHumdrumFileOutput();
	}
}



// END_MERGE

} // end namespace hum



