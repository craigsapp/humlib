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

void   processFile     (HumdrumFile& infile, Options& options);
void   printInputData  (vector<double>& x, vector<double>& y);
void   extractData     (HumdrumFile& infile, vector<double>& x, vector<double>&y);
bool   getCorrelation  (double& output, vector<double>& x, int xstart, vector<double>& y, int ystart, int len);
void   printRawAnalysis(vector<vector<double>>& analysis);
void   doRowAnalysis   (vector<double>& row, int windowlen, vector<double>& x, vector<double>& y);
void   printCorrelationScape(vector<vector<double>>& correlations);
void   printPixelRow   (ostream& out, vector<PixelColor>& row, int repeat);
void   getPixelRow     (vector<PixelColor>& row, vector<double>& cor);

Options options;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	options.define("d|data=b", "print input data");
   options.define("n|min=i:2", "minimum correlation vector size to process");
   options.define("m|max=i:0", "maximum correlation vector size to process");
	options.define("c|correlations=b", "print raw correlation data");
	options.define("l|lowest=d:-1.0", "lowest correlation value");
	options.define("coolest=d:0.95", "coolest hue");
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
	extractData(infile, x, y);
	if (x.empty()) {
		return;
	}
	if (y.empty()) {
		return;
	}
	if (options.getBoolean("data")) {
      printInputData(x, y);
      return;
   }

	int tsize = (int)x.size();
	vector<vector<double>> analysis;
	analysis.resize(x.size()-1);
	for (int i=0; i<tsize-1; i++) {
		analysis.at(i).resize(i+1);
		fill(analysis.at(i).begin(), analysis.at(i).end(), -123456789.0);
		doRowAnalysis(analysis.at(i), tsize-i, x, y);
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

	printCorrelationScape(analysis);

/*
	double cor;
	bool status = getCorrelation(cor, x, 0, y, 0, (int)x.size());
	if (!status) {
		cerr << "Error calculating correlation: out of bounds." << endl;
		return;
	}
	cout << cor << endl;
*/

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
		cerr << "XSTART " << xstart << " LEN " << len << endl;
		return 0;
	}

	if (ystart + len > (int)y.size()) {
		cerr << "Error: Cannot go beyond end of vector y: " << ystart + len << " max is " << ((int)y.size()) << endl;
		cerr << "YSTART " << ystart << " LEN " << len << endl;
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
// extractData --
//

void extractData(HumdrumFile& infile, vector<double>& x, vector<double>&y) {
	x.clear();
	y.clear();
	x.reserve(infile.getLineCount());
	y.reserve(infile.getLineCount());
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
      if (infile[i].isAllNull()) {
         continue;
      }
		if (infile[i].getFieldCount() < 2) {
			cerr << "Error: data file needs at least two spines, but has only" 
			     << infile[i].getFieldCount() << endl;
			return;
		}
		HTp token1 = infile.token(i, 0);
		HTp token2 = infile.token(i, 1);
		double value1;
		double value2;
		if (token1->isNull()) {
			cerr << "Token in column " << token1->getFieldNumber() << " on line "
			     << token1->getLineNumber() << " is empty.  Giving up." << endl;
		}
		if (token2->isNull()) {
			cerr << "Token in column " << token2->getFieldNumber() << " on line "
			     << token2->getLineNumber() << " is empty.  Giving up." << endl;
		}
      if (!hre.search(token1, "([+-]?\\d*\\.?\\d+)")) {
			cerr << "Cannot find number in token : " << token1 << " in column "
			     << token1->getFieldNumber() << " on line "
			     << token1->getLineNumber() << " Giving up." << endl;
		}
		value1 = hre.getMatchDouble(1);
      if (!hre.search(token2, "([+-]?\\d*\\.?\\d+)")) {
			cerr << "Cannot find number in token : " << token2 << " in column "
			     << token2->getFieldNumber() << " on line "
			     << token2->getLineNumber() << " Giving up." << endl;
		}
		value2 = hre.getMatchDouble(1);
      x.push_back(value1);
      y.push_back(value2);
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



//////////////////////////////
//
// printCorrelationScape --
//

void printCorrelationScape(vector<vector<double>>& correlations) {
	int rrepeat = 2;
	int crepeat = 2;
	int maxrows = (int)correlations.size();
	int maxcols = (int)correlations.back().size();

   cout << "P3\n";
   cout << (crepeat * maxcols) << " " << (rrepeat * maxrows) << "\n";
   cout << "255\n";


	vector<PixelColor> row(maxcols);

	for (int i=0; i<correlations.size(); i++) {
		getPixelRow(row, correlations[i]);
		for (int j=0; j<rrepeat; j++) {
			printPixelRow(cout, row, crepeat);
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
	if (offset < 0) {
		offset = 0;
	}
	for (int i=0; i<(int)cor.size(); i++) {
		int ii = i + offset;
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

void printPixelRow(ostream& out, vector<PixelColor>& row, int repeat) {
	for (int i=0; i<(int)row.size(); i++) {
		for (int j=0; j<repeat; j++) {
			out << row[i] << ' ';
		}
	}
	out << "\n";
}



