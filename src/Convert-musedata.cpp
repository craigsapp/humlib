//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Sep 24 10:40:19 PDT 2019
// Last Modified: Tue Sep 24 10:40:22 PDT 2019
// Filename:      Convert-musedata.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-musedata.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert between musedata and various Humdrum representations.
//

#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// Convert::museToKern -- Return the Least Common Multiple of a list of numbers.
//

int Convert::museToBase40(const string& pitchString) {
   string temp = pitchString;
   int octave;
   int i = (int)temp.size() - 1;
   while (i >= 0 && !isdigit(temp[i])) {
      i--;
   }

   if (i <= 0) {
      cerr << "Error: could not find octave in string: " << pitchString << endl;
      exit(1);
   }

   octave = temp[i] - '0';
   temp.resize(i);

	for (int i=0; i<(int)temp.size(); i++) {
		if (temp[i] == 'f') {
			temp[i] = '-';
		}
	}
	int kb40 = Convert::kernToBase40(temp);
	if (kb40 < 0) {
		return kb40;
	}
   return kb40 % 40 + 40 * octave;
}



//////////////////////////////
//
// Convert::musePitchToKernPitch -- 
//

string Convert::musePitchToKernPitch(const string& museInput) {
   return base40ToKern(museToBase40(museInput));
}


// END_MERGE

} // end namespace hum



