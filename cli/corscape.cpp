//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov  9 14:24:19 PST 2020
// Last Modified: Mon Nov  9 08:33:32 PST 2020
// Filename:      corscape.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/corscape.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Calculate a correlation scape from two input vectors.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile      (HumdrumFile& infile, Options& options);
void   printInputData   (vector<double>& x, vector<double>& y);
void   printInputData   (vector<double>& x);
void   extractData      (HumdrumFile& infile, vector<double>& x, int xindex, vector<double>&y, int yindex);
bool   getCorrelation   (double& output, vector<double>& x, int xstart, vector<double>& y, int ystart, int len);
bool   getCorrelation2  (double& output, vector<double>& x, int xstart, vector<double>& y);
void   printRawAnalysis (vector<vector<double>>& analysis);
void   doRowAnalysis    (vector<double>& row, int windowlen, vector<double>& x, vector<double>& y);
void   doArchRowAnalysis(vector<double>& row, int windowlen, vector<double>& x);
void   printCorrelationScape(vector<vector<double>>& correlations,
                         vector<double>& x, vector<double>& xsmooth,
                         vector<double>& y, vector<double>& ysmooth);
void   printPixelRow    (ostream& out, vector<PixelColor>& row, int repeat, bool adjust = false);
void   getPixelRow      (vector<PixelColor>& row, vector<double>& cor);
void   getArch          (vector<double>& arch);
void   printInputPlot   (vector<double>& x, vector<double>& y, int cols, int crepeat, int rows);
void   printInputPlot2  (vector<double>& x, vector<double>& y, int cols, int crepeat, int rows);
void   printInputPlotSmooth(vector<double>& x, vector<double>& xsmooth, int cols,
                         int crepeat, int plotrows);
void   getMinMax        (double& minvalue, double& maxvalue, vector<double>& x,
                         vector<double>& y);
int    scaleValue       (double input, double minvalue, double maxvalue, int maxout);
void   storeDataInPlot  (vector<vector<int>>& plot, vector<double>& x, int xpoint,
		                   double minvalue, double maxvalue, int crepeat);
void   printColorMap    (int maxcols, int crepeat, int plotrows);
vector<double> smoothSequence(vector<double>& input);

Options options;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	vector<double> testing(100);
	getArch(testing);
	options.define("d|data=b", "print input data");
	options.define("n|min=i:2", "minimum correlation vector size to process");
	options.define("m|max=i:0", "maximum correlation vector size to process");
	options.define("c|correlations=b", "print raw correlation data");
	options.define("l|lowest=d:-1.0", "lowest correlation value");
	options.define("x=i:1", "first sequence to use in analysis");
	options.define("y=i:2", "second sequence to use in analysis");
	options.define("coolest=d:0.80", "coolest hue");
	options.define("arch=b", "Do an arch correlation plot");
	options.define("plot=b", "Display plot of input data under triangle plot, overlaid");
	options.define("plot2=b", "Display plot of input data under triangle plot, separately");
	options.define("aspect-ratio=d:2.59", "Apspect ratio for input data plot");
	options.define("color-map=b", "print color map for testing");
	options.define("map-rows=i:25", "height of color map");
	options.define("s|smooth=b", "smooth input data");
	options.define("S|sf|smooth-factor=d:0.25", "smoothing factor");
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile, options);
		// only allowing one file analysis for now
		break;
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile, Options& options) {
	vector<double> x;
	vector<double> y;

	int xcol = options.getInteger("x") - 1;
	int ycol = options.getInteger("y") - 1;
	if (options.getBoolean("arch")) {
		ycol = -1;
	}

	extractData(infile, x, xcol, y, ycol);

	if (x.empty()) {
		return;
	}

	if (y.empty()) {
		if (!options.getBoolean("arch")) {
			return;
		}
	}

	int smoothQ = options.getBoolean("smooth");
	vector<double> xsmooth;
	vector<double> ysmooth;
	if (smoothQ) {
		xsmooth = smoothSequence(x);
		ysmooth = smoothSequence(y);
	}

	if (options.getBoolean("data")) {
		if (options.getBoolean("arch")) {
			if (smoothQ) {
				printInputData(x, xsmooth);
			} else {
				printInputData(x);
			}
		} else {
			if (smoothQ) {
				printInputData(xsmooth, ysmooth);
			} else {
				printInputData(x, y);
			}
		}
		return;
	}

	int tsize = (int)x.size();
	vector<vector<double>> analysis;
	analysis.resize(x.size()-1);
	for (int i=0; i<tsize-1; i++) {
		analysis.at(i).resize(i+1);
		fill(analysis.at(i).begin(), analysis.at(i).end(), -123456789.0);
		if (options.getBoolean("arch")) {
			if (smoothQ) {
				doArchRowAnalysis(analysis.at(i), tsize-i, xsmooth);
			} else {
				doArchRowAnalysis(analysis.at(i), tsize-i, x);
			}
		} else {
			// Regular correlation plot comparing two sequences
			if (smoothQ) {
				doRowAnalysis(analysis.at(i), tsize-i, xsmooth, ysmooth);
			} else {
				doRowAnalysis(analysis.at(i), tsize-i, x, y);
			}
		}
	}

	// Set the min an max row.  Make more efficient by not
	// calculating values outside of the min/max range.

	if (options.getBoolean("max")) {
		int max = options.getInteger("max");
		if (max > 0) {
			// absolute value maxiumum
			max = (int)analysis.size() - max;
			if (max < analysis.size()) {
				analysis.erase(analysis.begin(), analysis.begin() + max);
			}
		} else if (max < 0) {
			// subtract from top of triangle
			max = -max;
			if (max < analysis.size()) {
				analysis.erase(analysis.begin(), analysis.begin() + max);
			}
		}
	}

	int min = options.getInteger("min") - 1;
	if ((min > 0) && (min < (int)analysis.size())) {
		analysis.resize((int)analysis.size() - min);
	}

	if (options.getBoolean("correlations")) {
		printRawAnalysis(analysis);
		return;
	}

	printCorrelationScape(analysis, x, xsmooth, y, ysmooth);
}



//////////////////////////////
//
// doArchRowAnalysis --
//

void doArchRowAnalysis(vector<double>& row, int windowlen, vector<double>& x) {
	vector<double> y(windowlen);
	getArch(y);

	for (int i=0; i<(int)row.size(); i++) {
		double value;
		int status;
		status = getCorrelation2(value, x, i, y);
		if (!status) {
			cerr << "Error" << endl;
			return;
		}
		if (Convert::isNaN(value)) {
			// suppressing 0/0 cases (converting them to zeros).
			// This will happen most likely at length-2 correlations, but
			// can happen with vastly decreasing likelihood for larger
			// correlations when comparing flat sequences that have a
			// zero standard devaition.
			row.at(i) = -0.0;
		} else {
			row.at(i) = value;
		}
	}
}



//////////////////////////////
//
// doRowAnalysis --
//

void doRowAnalysis(vector<double>& row, int windowlen, vector<double>& x, vector<double>& y) {
	for (int i=0; i<(int)row.size(); i++) {
		double value;
		int status;
		status = getCorrelation(value, x, i, y, i, windowlen);
		if (!status) {
			cerr << "Error" << endl;
			return;
		}
		if (Convert::isNaN(value)) {
			// suppressing 0/0 cases (converting them to zeros).
			// This will happen most likely at length-2 correlations, but
			// can happen with vastly decreasing likelihood for larger
			// correlations when comparing flat sequences that have a
			// zero standard devaition.
			row.at(i) = -0.0;
		} else {
			row.at(i) = value;
		}
	}
}



//////////////////////////////
//
// printRawAnalysis --
//

void printRawAnalysis(vector<vector<double>>& analysis) {
	for (int i=0; i<(int)analysis.size(); i++) {
		for (int j=0; j<(int)analysis[i].size(); j++) {
			double datum = analysis.at(i).at(j);
			// limit to two decimal places:
			if (datum > 0) {
				datum = int(datum * 100.0 + 0.5) / 100.0;
			} else {
				datum = -int(-datum * 100.0 + 0.5) / 100.0;
			}
			cout << datum;
			if (j < (int)analysis[i].size() - 1) {
				cout << "\t";
			}
		}
		cout << endl;
	}
}



//////////////////////////////
//
// getCorrelation -- return value is status: true if successful; false otherwise.
//

bool getCorrelation(double& output, vector<double>& x, int xstart, vector<double>& y, int ystart, int len) {
	if ((xstart == 0) && (len == (int)x.size())) {
		if ((ystart == 0) && (len == (int)y.size())) {
			output = Convert::pearsonCorrelation(x, y);
			return 1;
		}
	}

	if (xstart + len > (int)x.size()) {
		cerr << "Error: Cannot go beyond end of vector x: " << xstart + len << " max is " << ((int)x.size()) << endl;
		cerr << "XSTART " << xstart << " LEN " << len << " vector size: " << x.size() << endl;
		return 0;
	}

	if (ystart + len > (int)y.size()) {
		cerr << "Error: Cannot go beyond end of vector y: " << ystart + len << " max is " << ((int)y.size()) << endl;
		cerr << "YSTART " << ystart << " LEN " << len << " vector size: " << y.size() << endl;
		return 0;
	}

	auto x1 = x.begin() + xstart;
	auto x2 = x.begin() + xstart + len;
	auto y1 = y.begin() + ystart;
	auto y2 = y.begin() + xstart + len;

	vector<double> xx(x1, x2);
	vector<double> yy(y1, y2);

	output = Convert::pearsonCorrelation(xx, yy);
	return 1;
}



//////////////////////////////
//
// getCorrelation2 -- return value is status: true if successful; false otherwise.  This version
//    is used to correlate a subsequence of x with the entire y sequence (for the arch analysis).
//

bool getCorrelation2(double& output, vector<double>& x, int xstart, vector<double>& y) {
	int ystart = 0;
	int len = (int)y.size();

	if ((xstart == 0) && (len == (int)x.size())) {
		if ((ystart == 0) && (len == (int)y.size())) {
			output = Convert::pearsonCorrelation(x, y);
			return 1;
		}
	}

	if (xstart + len > (int)x.size()) {
		cerr << "Error: Cannot go beyond end of vector x: " << xstart + len << " max is " << ((int)x.size()) << endl;
		cerr << "XSTART " << xstart << " LEN " << len << endl;
		return 0;
	}

	auto x1 = x.begin() + xstart;
	auto x2 = x.begin() + xstart + len;

	vector<double> xx(x1, x2);

	output = Convert::pearsonCorrelation(xx, y);
	return 1;
}



//////////////////////////////
//
// extractData --
//

void extractData(HumdrumFile& infile, vector<double>& x, int xindex, vector<double>&y, int yindex) {
	x.clear();
	y.clear();
	x.reserve(infile.getLineCount());
	y.reserve(infile.getLineCount());
	int archQ = options.getBoolean("arch");
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].isAllNull()) {
			continue;
		}
		if (archQ) {
			if (infile[i].getFieldCount() < 1) {
				cerr << "Error: data file needs at least one spine, but has only"
				     << infile[i].getFieldCount() << endl;
				return;
			}
		} else {
			if (infile[i].getFieldCount() < 2) {
				cerr << "Error: data file needs at least two spines, but has only"
				     << infile[i].getFieldCount() << endl;
				return;
			}
		}

		HTp token1 = NULL;
		HTp token2 = NULL;

		if ((xindex >= 0) && (xindex < infile[i].getFieldCount())) {
			token1 = infile.token(i, xindex);
		}
		if ((!archQ) && (yindex >= 0) && (yindex < infile[i].getFieldCount())) {
			token2 = infile.token(i, yindex);
		}
		if ((token1 == NULL) && (token2 == NULL)) {
			cerr << "Problem extrcting tokens.  All indexes are out of range" << endl;
			cerr << "x-index = " << xindex << endl;
			cerr << "y-index = " << yindex << endl;
			return;
		}

		double value1 = 0.0;
		double value2 = 0.0;

		if (token1 && (token1->isNull())) {
			cerr << "Token in column " << token1->getFieldNumber() << " on line "
			     << token1->getLineNumber() << " is empty.  Giving up." << endl;
		}
		if (token2 && (token2->isNull())) {
			cerr << "Token in column " << token2->getFieldNumber() << " on line "
			     << token2->getLineNumber() << " is empty.  Giving up." << endl;
		}
		if (token1 && !hre.search(token1, "([+-]?\\d*\\.?\\d+)")) {
			cerr << "Cannot find number in token : " << token1 << " in column "
			     << token1->getFieldNumber() << " on line "
			     << token1->getLineNumber() << " Giving up." << endl;
		}
		if (token1) {
			value1 = hre.getMatchDouble(1);
			x.push_back(value1);
		}
		if (token2 && !hre.search(token2, "([+-]?\\d*\\.?\\d+)")) {
			cerr << "Cannot find number in token : " << token2 << " in column "
			     << token2->getFieldNumber() << " on line "
			     << token2->getLineNumber() << " Giving up." << endl;
		}
		if (token2) {
			value2 = hre.getMatchDouble(1);
			y.push_back(value2);
		}
	}
}



//////////////////////////////
//
// printInputData --
//

void printInputData(vector<double>& x, vector<double>& y) {
	cout << "# x\t# y\n";
	for (int i=0; i<(int)x.size(); i++) {
		cout << x.at(i) << "\t" << y.at(i) << endl;
	}
}

void printInputData(vector<double>& x) {
	cout << "# x\n";
	for (int i=0; i<(int)x.size(); i++) {
		cout << x.at(i) << endl;
	}
}



//////////////////////////////
//
// printCorrelationScape --
//

void printCorrelationScape(vector<vector<double>>& correlations,
		vector<double>& x, vector<double>& xsmooth,
		vector<double>& y, vector<double>& ysmooth) {
	int rrepeat = 1;
	int crepeat = 2;
	int maxrows = (int)correlations.size();
	int maxcols = (int)correlations.back().size();

	bool archQ = options.getBoolean("arch");
	bool smoothQ = options.getBoolean("smooth");
	bool plotQ = options.getBoolean("plot");
	bool plot2Q = options.getBoolean("plot2");
	double aspectratio = options.getDouble("aspect-ratio");

	int plotrows = maxcols / aspectratio;
	if (plotrows < 100) {
		plotrows = 100;
	}
	int prows = plotrows;
	if (plot2Q) {
		prows *= 2;
	}
	if (!(plotQ || plot2Q)) {
		plotrows = 0;
	}
	if (plotQ) {
		// force the width of the plot to match the full sequence length
		maxcols = (int)x.size();
	}

	bool colormapQ = options.getBoolean("color-map");
	int maprows = options.getInteger("map-rows");
	if (!colormapQ) {
		maprows = 0;
	}

	cout << "P3\n";
	cout << (crepeat * maxcols) << " " << (rrepeat * maxrows + prows + maprows) << "\n";
	cout << "255\n";

	vector<PixelColor> row(maxcols);

	for (int i=0; i<correlations.size(); i++) {
		getPixelRow(row, correlations[i]);
		for (int j=0; j<rrepeat; j++) {
			printPixelRow(cout, row, crepeat, !(i%2));
		}
	}

	if (colormapQ) {
		printColorMap(maxcols, crepeat, maprows);
	}
	if (plotQ) {
		if (archQ && smoothQ) {
			printInputPlotSmooth(x, xsmooth, maxcols, crepeat, plotrows);
		} else {
			printInputPlot(x, y, maxcols, crepeat, plotrows);
		}
	} else if (plot2Q) {
		if (archQ && !smoothQ) {
			printInputPlot(x, y, maxcols, crepeat, plotrows);
		} else if (archQ && smoothQ) {
			vector<double> empty;
			printInputPlotSmooth(x, empty, maxcols, crepeat, plotrows);
			printInputPlotSmooth(empty, xsmooth, maxcols, crepeat, plotrows);
		} else {
			printInputPlot2(x, y, maxcols, crepeat, plotrows);
		}
	}
}



//////////////////////////////
//
// printColorMap --
//

void printColorMap(int maxcols, int crepeat, int plotrows) {
	double coolest = options.getDouble("coolest");
	PixelColor pc;
	int colval = maxcols * crepeat;
	for (int i=0; i<plotrows; i++) {
		for (int j=0; j<colval; j++) {
			pc.setHue((double)(colval - j - 1) / colval * coolest);
			cout << pc << " ";
		}
		cout << endl;
	}
}




//////////////////////////////
//
// storeDataInPlot --
//

void storeDataInPlot(vector<vector<int>>& plot, vector<double>& x,
		int xpoint, double minvalue, double maxvalue, int crepeat) {
	if (crepeat != 2) {
		// requring crepeat to be 2.
		return;
	}
	int psize = (int)plot.at(0).size() / crepeat;
	int maxindex = (int)x.size();
	if (maxindex > psize) {
		maxindex = psize;
	}
	int lastvalue = -1;
	for (int i=0; i<maxindex; i++) {
		int yvalue = scaleValue(x.at(i), minvalue, maxvalue, (int)plot.size() - 1);
		if ((yvalue >= 0) && (yvalue < (int)plot.size())) {
			for (int j=0; j<crepeat; j++) {
				plot.at(yvalue).at(i * crepeat + j) += xpoint;
			}
			if ((lastvalue >= 0) && (i > 0)) {
				if (lastvalue < yvalue) {
					for (int k=lastvalue + 1; k < yvalue; k++) {
						plot.at(k).at((i-1) * crepeat + crepeat - 1) += xpoint + 100;
					}
				} else if (lastvalue > yvalue) {
					for (int k=lastvalue - 1; k > yvalue; k--) {
						plot.at(k).at((i-1) * crepeat + crepeat - 1) += xpoint + 200;
					}
				}
			}
			lastvalue = yvalue;
		}
	}
}



//////////////////////////////
//
// scaleValue --
//

int scaleValue(double input, double minvalue, double maxvalue, int maxout) {
	double range = maxvalue - minvalue;
	if (range <= 0.0) {
		return -1;
	}
	double value = (input - minvalue) / range;
	// flip the range
	value = 1.0 - value;
	int ivalue = int(value * maxout + 0.5);
	if (ivalue < 0) {
		ivalue = 0;
	}
	if (ivalue > maxout) {
		ivalue = maxout;
	}
	return ivalue;
}



//////////////////////////////
//
// printInputPlot --
//

void printInputPlot2(vector<double>& x, vector<double>& y, int cols,
		int crepeat, int rows) {
	vector<double> empty;
	printInputPlot(x, empty, cols, crepeat, rows);
	printInputPlot(empty, y, cols, crepeat, rows);
}


void printInputPlotSmooth(vector<double>& x, vector<double>& xsmooth, int cols,
	int crepeat, int rows) {
	printInputPlot(x, xsmooth, cols, crepeat, rows);
}


void printInputPlot(vector<double>& x, vector<double>& y, int cols, int crepeat, int rows) {
	if (crepeat != 2) {
		// requiring crepeat to be 2
		return;
	}
	vector<vector<int>> plot;
	plot.resize(rows);
	char xpoint = 1;
	char ypoint = 2;
	// char xypoint = 3;
	char background = 0;
	for (int i=0; i<(int)plot.size(); i++) {
		plot[i].resize(cols * crepeat);
		fill (plot[i].begin(), plot[i].end(), background);
	}

	double minvalue = 0.0;
	double maxvalue = 0.0;
	getMinMax(minvalue, maxvalue, x, y);

	if (!x.empty()) {
		storeDataInPlot(plot, x, xpoint, minvalue, maxvalue, crepeat);
	}
	if (!y.empty()) {
		storeDataInPlot(plot, y, ypoint, minvalue, maxvalue, crepeat);
	}

	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols * crepeat; j++) {
			switch (plot[i][j]) {
				case 1:
					cout << "0 0 250 ";
					break;
				case 101:
				case 201:
					cout << "200 200 250 ";
					break;
				case 102:
				case 202:
					cout << "250 200 200 ";
					break;
				case 103:
				case 203:
				case 303:
				case 403:
					cout << "240 200 240 ";
					break;
				case 2:
					cout << "250 0 0 ";
					break;
				case 3:
					cout << "220 0 220 ";
					break;
				case 0:
					cout << "255 255 255 ";
					break;
				default:
					cerr << "UNKNOWN COLOR CODE: " << (int)plot[i][j] << endl;
					// Some problem: unknown pixel type
					cout << "0 0 0 ";
			}
		}
		cout << endl;
	}
}



//////////////////////////////
//
// getMinMax --
//

void getMinMax(double& minvalue, double& maxvalue, vector<double>& x,
		vector<double>& y) {
	int xsize = (int)x.size();
	int ysize = (int)y.size();
	if (xsize > 0) {
		minvalue = x[0];
		maxvalue = x[0];
	} else if (ysize > 0) {
		minvalue = y[0];
		maxvalue = y[0];
	} else {
		minvalue = 0.0;
		maxvalue = 0.0;
	}

	for (int i=0; i<xsize; i++) {
		if (x[i] < minvalue) {
			minvalue = x[i];
		}
		if (x[i] > maxvalue) {
			maxvalue = x[i];
		}
	}

	for (int i=0; i<ysize; i++) {
		if (y[i] < minvalue) {
			minvalue = y[i];
		}
		if (y[i] > maxvalue) {
			maxvalue = y[i];
		}
	}
}



//////////////////////////////
//
// getPixelRow --  cor vector size is the number of columns for the image
//   (divided by 2).  The correlation values should be centered in this
//   region, with black (or white) for the border color.
//

void getPixelRow(vector<PixelColor>& row, vector<double>& cor) {
	double lowest = options.getDouble("lowest");
	double range = 1.0 - lowest;
	double coolest = options.getDouble("coolest");
	if (range <= 0) {
		lowest = -1.0;
		range = 2.0;
	}
	for (int i=0; i<(int)row.size(); i++) {
		row[i].setRed(0);
		row[i].setGreen(0);
		row[i].setBlue(0);
	}

	int offset = ((int)row.size() - (int)cor.size()) / 2;

	for (int i=0; i<(int)cor.size(); i++) {
		int ii = i + offset;
		if (ii < 0) {
			ii = 0;
		}
		if (ii >= row.size()) {
			break;
		}
		double value = -(cor.at(i) - 1)/range * coolest;
		row.at(ii).setHue(value);
	}
}



//////////////////////////////
//
// printPixelRow --
//

void printPixelRow(ostream& out, vector<PixelColor>& row, int repeat, bool adjust) {
	for (int i=0; i<(int)row.size(); i++) {
		int newrepeat = repeat;
		if (adjust) {
			if (i == 0) {
				newrepeat = repeat + 1;
			}
			if (i == (int)row.size() - 1) {
				newrepeat = repeat - 1;
			}
			if (newrepeat < 0) {
				newrepeat = 0;
			}
		}
		for (int j=0; j<newrepeat; j++) {
			out << row[i] << ' ';
		}
	}
	out << "\n";
}



//////////////////////////////
//
// getArch -- return a half-cycle sine wave. getArch -- return a half-cycle sinewave.
//

void getArch(vector<double>& arch) {
	int vsize = (int)arch.size();
	if (vsize == 1) {
		arch[0] = 0.0;
		return;
	}
	for (int i=0; i<vsize; i++) {
		double value = sin(M_PI * (double)i / (double)(vsize-1));
		arch[i] = value;
	}
}



//////////////////////////////
//
// smoothSequence --
//

vector<double> smoothSequence(vector<double>& input) {
	vector<double> output(input.size());
	if (input.empty()) {
		return output;
	}
	double gain = options.getDouble("smooth-factor");
	double feedback = 1.0 - gain;
	double lastvalue = input[0];
	for (int i=0; i<input.size(); i++) {
		output[i] = gain * input[i] + feedback * lastvalue;
		lastvalue = output[0];
	}
	lastvalue = output.back();
	for (int i=(int)output.size()-1; i>=0; i--) {
		output[i] = gain * output[i] + feedback * lastvalue;
		lastvalue = output[i];
	}
	return output;
}



