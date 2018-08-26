//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Jul 15 09:57:12 CEST 2018
// Last Modified: Sun Jul 15 09:57:16 CEST 2018
// Filename:      tool-periodicity.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-periodicity.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   periodicity tool.
//

#include "tool-periodicity.h"
#include "Convert.h"

#include "pugixml.hpp"
#include <math.h>

using namespace std;

namespace hum {

// START_MERGE



/////////////////////////////////
//
// Tool_periodicity::Tool_periodicity -- Set the recognized options for the tool.
//

Tool_periodicity::Tool_periodicity(void) {
	define("m|min=b", "minimum time unit (other than grace notes)");
	define("t|track=i:0", "track to analyze");
	define("attacks=b", "extract attack grid)");
	define("raw=b", "show only raw period data");
	define("s|svg=b", "output svg image");
	define("p|power=d:2.0", "scaling power for visual display");
	define("1|one=b", "composite rhythms are not weighted by attack");
}



/////////////////////////////////
//
// Tool_periodicity::run -- Primary interfaces to the tool.
//

bool Tool_periodicity::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status;
	status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_periodicity::run(HumdrumFile& infile, ostream& out) {
	bool status;
	status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

//
// In-place processing of file:
//

bool Tool_periodicity::run(HumdrumFile& infile) {
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_periodicity::processFile --
//

void Tool_periodicity::processFile(HumdrumFile& infile) {
	HumNum minrhy = infile.tpq() * 4;
	if (getBoolean("min")) {
		m_free_text << minrhy << endl;
		return;
	}

	vector<vector<double>> attackgrids;
	attackgrids.resize(infile.getTrackCount()+1);
	fillAttackGrids(infile, attackgrids, minrhy);
	if (getBoolean("attacks")) {
		printAttackGrid(m_free_text, infile, attackgrids, minrhy);
		return;
	}

	int atrack = getInteger("track");
	vector<vector<double>> analysis;
	doPeriodicityAnalysis(analysis, attackgrids[atrack], minrhy);

	if (getBoolean("raw")) {
		printPeriodicityAnalysis(m_free_text, analysis);
		return;
	}

	printSvgAnalysis(m_free_text, analysis, minrhy);

// Tool_periodicity::printSvgGrid --
}



//////////////////////////////
//
// Tool_periodicity::printPeriodicityAnalysis --
//

void Tool_periodicity::printPeriodicityAnalysis(ostream& out, vector<vector<double>>& analysis) {
	for (int i=0; i<(int)analysis.size(); i++) {
		for (int j=0; j<(int)analysis[i].size(); j++) {
			out << analysis[i][j];
			if (j < (int)analysis[i].size() - 1) {
				out << "\t";
			}
		}
		out << "\n";
	}
}



//////////////////////////////
//
// Tool_periodicity::doPeriodicAnalysis --
//

void Tool_periodicity::doPeriodicityAnalysis(vector<vector<double>> &analysis, vector<double>& grid, HumNum minrhy) {
	analysis.resize(minrhy.getNumerator());
	for (int i=0; i<(int)analysis.size(); i++) {
		doAnalysis(analysis, i, grid);
	}
}



//////////////////////////////
//
// Tool_periodicity::doAnalysis --
//

void Tool_periodicity::doAnalysis(vector<vector<double>>& analysis, int level, vector<double>& grid) {
	int period = level + 1;
	analysis[level].resize(period);
	std::fill(analysis[level].begin(), analysis[level].end(), 0.0);
	for (int i=0; i<period; i++) {
		int j = i;
		while (j < (int)grid.size()) {
			analysis[level][i] += grid[j];
			j += period;
		}
	}
}



//////////////////////////////
//
// Tool_periodicity::printAttackGrid --
//

void Tool_periodicity::printAttackGrid(ostream& out, HumdrumFile& infile, vector<vector<double>>& grids, HumNum minrhy) {
	out << "!!!minrhy: " << minrhy << endl;
	out << "**all";
	for (int i=1; i<(int)grids.size(); i++) {
		out << "\t**track";
	}
	out << "\n";
	for (int j=0; j<(int)grids[0].size(); j++) {
		for (int i=0; i<(int)grids.size(); i++) {
			out << grids[i][j];
			if (i < (int)grids.size() - 1) {
				out << "\t";
			}
		}
		out << "\n";
	}
	for (int i=0; i<(int)grids.size(); i++) {
		out << "*-";
		if (i < (int)grids.size() - 1) {
			out << "\t";
		}
	}
	out << "\n";
	
}



//////////////////////////////
//
// Tool_periodicity::fillAttackGrids -- 
//

void Tool_periodicity::fillAttackGrids(HumdrumFile& infile, vector<vector<double>>& grids, HumNum minrhy) {
	HumNum elements = minrhy * infile.getScoreDuration() / 4;

	for (int t=0; t<(int)grids.size(); t++) {
		grids[t].resize(elements.getNumerator());
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		HumNum position = infile[i].getDurationFromStart() / 4 * minrhy;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (!token->isNoteAttack()) {
				continue;
			}
			int track = token->getTrack();
			grids.at(track).at(position.getNumerator()) += 1;
		}
	}

	bool oneQ = getBoolean("one");
	for (int j=0; j<(int)grids.at(0).size(); j++) {
		grids.at(0).at(j) = 0;
		for (int i=0; i<(int)grids.size(); i++) {
			if (!grids.at(i).at(j)) {
				continue;
			}
			if (oneQ) {
				grids.at(0).at(j) = 1;
			} else {
				grids.at(0).at(j) += grids.at(i).at(j);
			}
		}
	}
}



//////////////////////////////
//
// Tool_periodicity::printSvgAnalysis --
//

void Tool_periodicity::printSvgAnalysis(ostream& out, vector<vector<double>>& analysis, HumNum minrhy) {
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

	auto style = svgnode.append_child("style");
	style.text().set(".label { font: 14px sans-serif; alignment-baseline: middle; text-anchor: middle; }");

	auto grid = svgnode.append_child("g");
	grid.append_attribute("id") = "grid";

	auto labels = svgnode.append_child("g");

	double hue = 0.0;
	double saturation = 100;
	double lightness = 75;

	pugi::xml_node crect;
	double width;
	double height;

	stringstream ss;
	stringstream ssl;
	//stringstream css;
	double x;
	double y;

	double imagewidth = 1000.0;
	double imageheight = 1000.0;

	double sdur = (double)analysis.back().size();

	double maxscore = 0.0;
	for (int i=0; i<(int)analysis.size(); i++) {
		for (int j=0; j<(int)analysis[i].size(); j++) {
			if (maxscore < analysis[i][j]) {
				maxscore = analysis[i][j];
			}
		}
	}

	double power = getDouble("power");
	for (int i=0; i<(int)analysis.size(); i++) {
		for (int j=0; j<(int)analysis[i].size(); j++) {
			width = 1 / sdur * imagewidth;
			height = 1 / sdur * imageheight;

			x = j/sdur * imageheight;
			y = i/sdur * imagewidth;

			double value = analysis[i][j]/maxscore;
			value = pow(value, 1.0/power);

			getColorMapping(value, hue, saturation, lightness);
			ss << "hsl(" << hue << "," << saturation << "%," << lightness << "%)";
			crect = grid.append_child("rect");
			crect.append_attribute("x") = to_string(x).c_str();
			crect.append_attribute("y") = to_string(y).c_str();
			crect.append_attribute("width") = to_string(width*0.99).c_str();
			crect.append_attribute("height") = to_string(height*0.99).c_str();
			crect.append_attribute("fill") = ss.str().c_str();
			//css << "Xm" << getMeasure1(i) << " Ym" << getMeasure2(j);
			//css << " X" << getQon1(i)     << " Y" << getQon2(j);
			//css << " X" << getQoff1(i)    << " Y" << getQoff2(j);
			//crect.append_attribute("class") = css.str().c_str();
			ss.str("");
			//css.str("");
		}

		pugi::xml_node label = labels.append_child("text");
		label.append_attribute("class") = "label";

		HumNum rval = (i+1);
		rval /= minrhy;
		rval *= 4;
	

		std::string rhythm = Convert::durationToRecip(rval);
cerr << "MINRHY " << minrhy << " analysize=" << analysis.size() << " i = " << i << " rval=> " << rval  << " rhythm => " << rhythm << endl;
		label.text().set(rhythm.c_str());
		x = (i+1+0.5)/sdur * imageheight;
		y = (i+0.5)/sdur * imagewidth;
		label.append_attribute("x") = to_string(x).c_str();
		label.append_attribute("y") = to_string(y).c_str();
	}

	image.save(out);
}



//////////////////////////////
//
// Tool_periodicity::getColorMapping --
//

void Tool_periodicity::getColorMapping(double input, double& hue,
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


// END_MERGE

} // end namespace hum



