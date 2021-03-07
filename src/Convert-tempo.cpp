//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jan 26 22:40:59 PST 2021
// Last Modified: Tue Jan 26 22:41:07 PST 2021
// Filename:      Convert-tempo.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-tempo.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Functions for tempo
//

#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// Convert::tempoNameToMm -- Guess what the MM tempo should be given
//    a tempo name.  Returns 0 if no guess is made.
// 
// Also add cases where there is a tempo marking, such as [quarter] = 132 
// in the input text.
//

int Convert::tempoNameToMm (const string& name, int bot, int top) {
	HumRegex hre;
	int output = -1;

	if      (hre.search(name, "larghissimo"      , "i")) { output = 24;  }
	else if (hre.search(name, "adagissimo"       , "i")) { output = 35;  }
	else if (hre.search(name, "all.*molto"       , "i")) { output = 146; }
	else if (hre.search(name, "all.*vivace"      , "i")) { output = 144; }
	else if (hre.search(name, "all.*moderato"    , "i")) { output = 116; }
	else if (hre.search(name, "all.*fuoco"       , "i")) { output = 138; }
	else if (hre.search(name, "all.*presto"      , "i")) { output = 160; }
	else if (hre.search(name, "grave"            , "i")) { output = 40;  }
	else if (hre.search(name, "largo"            , "i")) { output = 45;  }
	else if (hre.search(name, "lento?"           , "i")) { output = 50;  }
	else if (hre.search(name, "larghetto"        , "i")) { output = 63;  }
	else if (hre.search(name, "adagio"           , "i")) { output = 70;  }
	else if (hre.search(name, "adagietto"        , "i")) { output = 74;  }
	else if (hre.search(name, "andantino"        , "i")) { output = 90;  }
	else if (hre.search(name, "marcia moderato"  , "i")) { output = 85;  }
	else if (hre.search(name, "andante moderato" , "i")) { output = 92;  }
	else if (hre.search(name, "allegretto"       , "i")) { output = 116; }
	else if (hre.search(name, "rasch"            , "i")) { output = 128; }
	else if (hre.search(name, "vivo"             , "i")) { output = 152; }
	else if (hre.search(name, "vif"              , "i")) { output = 152; }
	else if (hre.search(name, "vivace"           , "i")) { output = 164; }
	else if (hre.search(name, "schnell"          , "i")) { output = 164; }
	else if (hre.search(name, "vivacissimo"      , "i")) { output = 172; }
	else if (hre.search(name, "allegrissimo"     , "i")) { output = 176; }
	else if (hre.search(name, "moderato"         , "i")) { output = 108; }
	else if (hre.search(name, "andante"          , "i")) { output = 88;  }
	else if (hre.search(name, "presto"           , "i")) { output = 180; }
	else if (hre.search(name, "allegro"          , "i")) { output = 128; }
	else if (hre.search(name, "prestissimo"      , "i")) { output = 208; }
	else if (hre.search(name, "bewegt"           , "i")) { output = 144; }
	else if (hre.search(name, "all(?!a)"         , "i")) { output = 128; }

	if (output <= 0) {
		return output;
	}

	if (hre.search(name, "ma non troppo", "i") || hre.search(name, "non tanto")) {
		if (output > 100) {
			output = int(output * 0.93 + 0.5);
		} else {
			output = int(output / 0.93 + 0.5);
		}
	}

	if (bot == 2) {
		output = int(output * 1.75 + 0.5);
	} else if (bot == 1) {
		output = int (output * 3.0 + 0.5);
	} else if ((bot == 8) && (top % 3 == 0)) {
		output = int(output * 1.5 + 0.5);
	} else if (bot == 8) {
		output = int(output * 0.75 + 0.5);
	} else if ((bot == 16) && (top % 3 == 0)) {
		output = int(output * 1.5 / 2.0 + 0.5);
	} else if (bot == 16) {
		output = int(output / 2.0 + 0.5);
	} else if ((bot == 32) && (top % 3 == 0)) {
		output = int(output * 1.5 / 4.0 + 0.5);
	} else if (bot == 32) {
		output = int(output / 4.0 + 0.5);
	}

	if ((bot == 2) && (top % 3 == 0)) {
		output = int(output * 1.5 + 0.5);
	}

	return output;
}



// END_MERGE

} // end namespace hum



