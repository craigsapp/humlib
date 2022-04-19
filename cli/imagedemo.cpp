//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon 18 Apr 2022 06:34:46 PM UTC
// Last Modified: Mon 18 Apr 2022 06:34:49 PM UTC
// Filename:      imagedemo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/imagedemo.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Demonstration of how to use PixelColor class
//                to create simple images.
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void   processFile         (HumdrumFile& infile, Options& options);
double calculateNpviRhythm (HumdrumFile& infile, Options& option);
double calculateCvPitch    (HumdrumFile& infile, Options& options);
string getReference        (HumdrumFile& infile, const string& targetKey);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("x|columns=i:4", "Number of columns in image");
	options.define("y|rows=i:4", "Number of rows in image");
	options.process(argc, argv);

	int rows = options.getInteger("rows");
	int cols = options.getInteger("columns");

	vector<vector<PixelColor>> image(rows);
	for (int y=0; y<(int)image.size(); y++) {
			image[y].resize(cols);
			for (int x=0; x<(int)image[y].size(); x++) {
				image[y][x].setRedF((double)y/rows);
				image[y][x].setBlueF((double)x/cols);
			}
	}

	// Output image header:
	cout << "P3" << endl;
	cout << cols << " " << rows << endl;
	cout << 255 << endl;

	// Output image pixels:
	for (int y=0; y<(int)image.size(); y++) {
			for (int x=0; x<(int)image[y].size(); x++) {
				cout << image[y][x] << " ";
			}
			cout << endl;
	}

	return 0;
}


