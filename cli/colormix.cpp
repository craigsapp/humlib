//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue May  3 18:25:53 PDT 2022
// Last Modified: Tue May  3 18:25:56 PDT 2022
// Filename:      cli/colormix.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Demonstration of averaging two colors together.
//
// Output:
//      #DC143C + #1E90FF = #7D529D
//

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

int main(int argc, char** argv) {
	PixelColor a("crimson");
	PixelColor b("dodgerblue");
	PixelColor c = PixelColor::mix(a, b);
	
	cout << a.getHexColor()<< " + " 
        << b.getHexColor() << " = "
        << c.getHexColor() << endl;
	
	return 0;
}


