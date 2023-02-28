//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 11 12:33:09 PDT 2002
// Last Modified: Thu Oct 13 20:50:11 PDT 2022
// Filename:      PixelColor.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/PixelColor.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Class for managing an image pixel.
//

#include "PixelColor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cctype>
#include <cmath>
#include <iostream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// PixelColor::PixelColor --
//

PixelColor::PixelColor(void) {
	// do nothing
}


PixelColor::PixelColor(const PixelColor& color) {
	Red   = color.Red;
	Green = color.Green;
	Blue  = color.Blue;
}

PixelColor::PixelColor(const string& color) {
	setColor(color);
}

PixelColor::PixelColor(int red, int green, int blue) {
	Red   = (unsigned int)limit(red, 0, 255);
	Green = (unsigned int)limit(green, 0, 255);
	Blue  = (unsigned int)limit(blue, 0, 255);
}

PixelColor::PixelColor(float red, float green, float blue) {
	Red   = (unsigned int)floatToChar(red);
	Green = (unsigned int)floatToChar(green);
	Blue  = (unsigned int)floatToChar(blue);
}

PixelColor::PixelColor(double red, double green, double blue) {
	Red   = (unsigned int)limit(floatToChar((float)red), 0, 255);
	Green = (unsigned int)limit(floatToChar((float)green), 0, 255);
	Blue  = (unsigned int)limit(floatToChar((float)blue), 0, 255);
}



//////////////////////////////
//
// PixelColor::~PixelColor --
//

PixelColor::~PixelColor() {
	// do nothing
}



//////////////////////////////
//
// PixelColor::invert -- negate the color.
//

void PixelColor::invert(void) {
	Red   = ~Red;
	Green = ~Green;
	Blue  = ~Blue;
}



//////////////////////////////
//
// PixelColor::getRed --
//

int PixelColor::getRed(void) {
	return Red;
}



//////////////////////////////
//
// PixelColor::getGreen --
//

int PixelColor::getGreen(void) {
	return Green;
}



//////////////////////////////
//
// PixelColor::getBlue --
//

int PixelColor::getBlue(void) {
	return Blue;
}



//////////////////////////////
//
// PixelColor::setRed --
//

void PixelColor::setRed(int value) {
	Red = (unsigned char)limit(value, 0, 255);
}



//////////////////////////////
//
// PixelColor::setGreen --
//

void PixelColor::setGreen(int value) {
	Green = (unsigned char)limit(value, 0, 255);
}



//////////////////////////////
//
// PixelColor::setBlue --
//

void PixelColor::setBlue(int value) {
	Blue = (unsigned char)limit(value, 0, 255);
}



//////////////////////////////
//
// PixelColor::getRedF --
//

float PixelColor::getRedF(void) {
	return charToFloat(Red);
}


//////////////////////////////
//
// PixelColor::getGreenF --
//

float PixelColor::getGreenF(void) {
	return charToFloat(Green);
}


//////////////////////////////
//
// PixelColor::getBlueF --
//

float PixelColor::getBlueF(void) {
	return charToFloat(Blue);
}


//////////////////////////////
//
// PixelColor::setRedF --
//

void PixelColor::setRedF(float value) {
	Red = (unsigned int)floatToChar(value);
}


//////////////////////////////
//
// PixelColor::setGreenF --
//

void PixelColor::setGreenF(float value) {
	Green = (unsigned int)floatToChar(value);
}


//////////////////////////////
//
// PixelColor::setBlueF --
//

void PixelColor::setBlueF(float value) {
	Blue = (unsigned int)floatToChar(value);
}


//////////////////////////////
//
// PixelColor::setColor --
//

void PixelColor::setColor(PixelColor& color) {
	Red   = color.Red;
	Green = color.Green;
	Blue  = color.Blue;
}



//////////////////////////////
//
// PixelColor::setColor --
//

PixelColor& PixelColor::setColor(int red, int green, int blue) {
	Red   = (unsigned int)limit(red, 0, 255);
	Green = (unsigned int)limit(green, 0, 255);
	Blue  = (unsigned int)limit(blue, 0, 255);
	return *this;
}



//////////////////////////////
//
// PixelColor::setColor -- set the contents to the specified value.
//

PixelColor& PixelColor::setColor(const string& colorstring) {
	PixelColor color;
	color = getColor(colorstring);
	Red   = color.Red;
	Green = color.Green;
	Blue  = color.Blue;

	return *this;
}


//////////////////////////////
//
// PixelColor::makeGrey --
//

PixelColor& PixelColor::makeGrey(void) {
	unsigned char average = limit((int)(((int)Red+(int)Green+(int)Blue)/3.0+0.5),0,255);
	Red = Green = Blue = average;
	return *this;
}


//////////////////////////////
//
// PixelColor::setGrayNormalized --  input in the range from 0.0 to 1.0.
//

PixelColor& PixelColor::setGrayNormalized(double value) {
	int graylevel = int(value * 256.0);
	if (graylevel >= 256) {
		graylevel = 255;
	}
	if (graylevel < 0) {
		graylevel = 0;
	}
	Red = Green = Blue = graylevel;
	return *this;
}

PixelColor& PixelColor::setGreyNormalized(double value) {
	return setGrayNormalized(value);
}


//////////////////////////////
//
// PixelColor::makeGray --
//

PixelColor& PixelColor::makeGray(void) {
	return makeGrey();
}


//////////////////////////////
//
// PixelColor::operator> --
//

int PixelColor::operator>(int number) {
	if (Red   <= number) return 0;
	if (Green <= number) return 0;
	if (Blue  <= number) return 0;
	return 1;
}



//////////////////////////////
//
// PixelColor::operator< --
//

int PixelColor::operator<(int number) {
	if (Red   >= number) return 0;
	if (Green >= number) return 0;
	if (Blue  >= number) return 0;
	return 1;
}



//////////////////////////////
//
// PixelColor::operator== --
//

int PixelColor::operator==(PixelColor& color) {
	if (Red != color.Red) {
		return 0;
	}
	if (Green != color.Green) {
		return 0;
	}
	if (Blue != color.Blue) {
		return 0;
	}
	return 1;
}



//////////////////////////////
//
// PixelColor::operator!= --
//

int PixelColor::operator!=(PixelColor& color) {
	if ((Red == color.Red) && (Green == color.Green) && (Blue == color.Blue)) {
		return 0;
	} else {
		return 1;
	}
}



//////////////////////////////
//
// PixelColor::operator*= --
//

PixelColor& PixelColor::operator*=(double number) {
	Red = (unsigned char)limit(floatToChar(charToFloat(Red)*number),     0, 255);
	Green = (unsigned char)limit(floatToChar(charToFloat(Green)*number), 0, 255);
	Blue = (unsigned char)limit(floatToChar(charToFloat(Blue)*number),   0, 255);
	return *this;
}



//////////////////////////////
//
// PixelColor::operator= --
//

PixelColor& PixelColor::operator=(PixelColor color) {
	if (this == &color) {
		return *this;
	}
	Red   = color.Red;
	Green = color.Green;
	Blue  = color.Blue;
	return *this;
}


PixelColor& PixelColor::operator=(int value) {
	Red   = (unsigned char)limit(value, 0, 255);
	Green = Red;
	Blue  = Red;
	return *this;
}



//////////////////////////////
//
// PixelColor::operator+ --
//

PixelColor PixelColor::operator+(PixelColor& color) {
	PixelColor output;
	output.Red   = (unsigned char)limit((int)Red   + color.Red,   0, 255);
	output.Green = (unsigned char)limit((int)Green + color.Green, 0, 255);
	output.Blue  = (unsigned char)limit((int)Blue  + color.Blue,  0, 255);
	return output;
}



//////////////////////////////
//
// PixelColor::operator+= --
//

PixelColor& PixelColor::operator+=(int number) {
	setRed(getRed()     + number);
	setGreen(getGreen() + number);
	setBlue(getBlue()   + number);
	return *this;
}

//////////////////////////////
//
// PixelColor::operator- --
//

PixelColor PixelColor::operator-(PixelColor& color) {
	PixelColor output;
	output.Red   = (unsigned char)limit((int)Red   - color.Red,   0, 255);
	output.Green = (unsigned char)limit((int)Green - color.Green, 0, 255);
	output.Blue  = (unsigned char)limit((int)Blue  - color.Blue,  0, 255);
	return output;
}



//////////////////////////////
//
// PixelColor::operator* --
//

PixelColor PixelColor::operator*(PixelColor& color) {
	PixelColor output;
	output.Red   = (unsigned char)limit(floatToChar(charToFloat(Red)*charToFloat(color.Red)), 0, 255);
	output.Green = (unsigned char)limit(floatToChar(charToFloat(Green)*charToFloat(color.Green)), 0, 255);
	output.Blue  = (unsigned char)limit(floatToChar(charToFloat(Blue)*charToFloat(color.Blue)), 0, 255);
	return output;
}


PixelColor PixelColor::operator*(double number) {
	PixelColor output;
	output.Red   = (unsigned char)limit(floatToChar(charToFloat(Red)*number),   0, 255);
	output.Green = (unsigned char)limit(floatToChar(charToFloat(Green)*number), 0, 255);
	output.Blue  = (unsigned char)limit(floatToChar(charToFloat(Blue)*number),  0, 255);
	return output;
}


PixelColor PixelColor::operator*(int number) {
	PixelColor output;
	output.Red   = (unsigned char)limit(floatToChar(charToFloat(Red)*number),   0, 255);
	output.Green = (unsigned char)limit(floatToChar(charToFloat(Green)*number), 0, 255);
	output.Blue  = (unsigned char)limit(floatToChar(charToFloat(Blue)*number),  0, 255);
	return output;
}



//////////////////////////////
//
// PixelColor::operator/ --
//

PixelColor PixelColor::operator/(double number) {
	PixelColor output;
	output.Red   = (unsigned char)limit(floatToChar(charToFloat(Red)/number),   0, 255);
	output.Green = (unsigned char)limit(floatToChar(charToFloat(Green)/number), 0, 255);
	output.Blue  = (unsigned char)limit(floatToChar(charToFloat(Blue)/number),  0, 255);
	return output;
}

PixelColor PixelColor::operator/(int number) {
	PixelColor output;
	output.Red   = (unsigned char)limit(floatToChar(charToFloat(Red)/(double)number),   0, 255);
	output.Green = (unsigned char)limit(floatToChar(charToFloat(Green)/(double)number), 0, 255);
	output.Blue  = (unsigned char)limit(floatToChar(charToFloat(Blue)/(double)number),  0, 255);
	return output;
}



//////////////////////////////
//
// PixelColor::getColor -- Switch named colors to SVG list.
//

PixelColor PixelColor::getColor(const string& colorstring) {
	PixelColor output;
	int start = 0;
	int hasdigit  = 0;
	int length = (int)colorstring.size();
	if (length > 128) {
		cout << "ERROR: color string too long: " << colorstring << endl;
		exit(1);
	}
	if (length == 7) {
		if (colorstring[0] == '#') {
			hasdigit = 1;
			start = 1;
		}
	} else if (length == 6) {
		int allxdigit = 1;
		start = 0;
		for (int i=start; i<length; i++) {
			allxdigit = allxdigit && isxdigit(colorstring[i]);
		}
		if (allxdigit) {
			hasdigit = 1;
		} else {
			hasdigit = 0;
		}
	}

	// check for decimal strings with spaces around numbers: "255 255 255"
	if ((colorstring.find(' ') != string::npos) ||
		 (colorstring.find('\t') != string::npos)) {
		char buffer[256] = {0};
		strcpy(buffer, colorstring.c_str());
		char* ptr = strtok(buffer, " \t\n:;");
		int tred   = -1;
		int tgreen = -1;
		int tblue  = -1;
		if (ptr != NULL) {
			sscanf(ptr, "%d", &tred);
			ptr = strtok(NULL, " \t\n;:");
		}
		if (ptr != NULL) {
			sscanf(ptr, "%d", &tgreen);
			ptr = strtok(NULL, " \t\n;:");
		}
		if (ptr != NULL) {
			sscanf(ptr, "%d", &tblue);
			ptr = strtok(NULL, " \t\n;:");
		}
		if (tred > 0 && tgreen > 0 && tblue > 0) {
			output.setColor(tred, tgreen, tblue);
			return output;
		}
	}

	if (hasdigit) {
		string piece1 = colorstring.substr(start,   2);
		string piece2 = colorstring.substr(start+2, 2);
		string piece3 = colorstring.substr(start+4, 2);

		int rval = (int)strtol(piece1.c_str(), NULL, 16);
		int gval = (int)strtol(piece2.c_str(), NULL, 16);
		int bval = (int)strtol(piece3.c_str(), NULL, 16);

		output.setColor(rval, gval, bval);
		return output;
	}

	// color string
	output.setColor(0,0,0);
	const string& cs = colorstring;

	char fc = '\0';
	if (!cs.empty()) {
		fc = cs[0];
	}

	switch (fc) {
	case 'a':
	if (cs == "aliceblue"           ) { output.setColor("#f0f8ff");  return output; }
	if (cs == "antiquewhite"        ) { output.setColor("#faebd7");  return output; }
	if (cs == "aqua"                ) { output.setColor("#00ffff");  return output; }
	if (cs == "aquamarine"          ) { output.setColor("#7fffd4");  return output; }
	if (cs == "azure"               ) { output.setColor("#f0ffff");  return output; }
	break; case 'b':
	if (cs == "beige"               ) { output.setColor("#f5f5dc");  return output; }
	if (cs == "bisque"              ) { output.setColor("#ffe4c4");  return output; }
	if (cs == "black"               ) { output.setColor("#000000");  return output; }
	if (cs == "blanchediamond"      ) { output.setColor("#ffebcd");  return output; }
	if (cs == "blue"                ) { output.setColor("#0000ff");  return output; }
	if (cs == "blueviolet"          ) { output.setColor("#8a2be2");  return output; }
	if (cs == "brown"               ) { output.setColor("#a52a2a");  return output; }
	if (cs == "burlywood"           ) { output.setColor("#ffe4c4");  return output; }
	break; case 'c':
	if (cs == "cadetblue"           ) { output.setColor("#5f9ea0");  return output; }
	if (cs == "chartreuse"          ) { output.setColor("#7fff00");  return output; }
	if (cs == "coral"               ) { output.setColor("#ff7f50");  return output; }
	if (cs == "cornflowerblue"      ) { output.setColor("#6495ed");  return output; }
	if (cs == "cornsilk"            ) { output.setColor("#fff8dc");  return output; }
	if (cs == "crimson"             ) { output.setColor("#dc143c");  return output; }
	if (cs == "cyan"                ) { output.setColor("#00ffff");  return output; }
	break; case 'd':
	if (cs == "darkblue"            ) { output.setColor("#00008b");  return output; }
	if (cs == "darkcyan"            ) { output.setColor("#008b8b");  return output; }
	if (cs == "darkgoldenrod"       ) { output.setColor("#b8860b");  return output; }
	if (cs == "darkgray"            ) { output.setColor("#a9a9a9");  return output; }
	if (cs == "darkgrey"            ) { output.setColor("#a9a9a9");  return output; }
	if (cs == "darkgreen"           ) { output.setColor("#006400");  return output; }
	if (cs == "darkkhaki"           ) { output.setColor("#bdb76b");  return output; }
	if (cs == "darkmagenta"         ) { output.setColor("#8b008b");  return output; }
	if (cs == "darkolivegreen"      ) { output.setColor("#556b2f");  return output; }
	if (cs == "darkorange"          ) { output.setColor("#ff8c00");  return output; }
	if (cs == "darkorchid"          ) { output.setColor("#9932cc");  return output; }
	if (cs == "darkred"             ) { output.setColor("#8b0000");  return output; }
	if (cs == "darksalmon"          ) { output.setColor("#e9967a");  return output; }
	if (cs == "darkseagreen"        ) { output.setColor("#8dbc8f");  return output; }
	if (cs == "darkslateblue"       ) { output.setColor("#483d8b");  return output; }
	if (cs == "darkslategray"       ) { output.setColor("#2e4e4e");  return output; }
	if (cs == "darkslategrey"       ) { output.setColor("#2e4e4e");  return output; }
	if (cs == "darkturquoise"       ) { output.setColor("#00ded1");  return output; }
	if (cs == "darkviolet"          ) { output.setColor("#9400d3");  return output; }
	if (cs == "deeppink"            ) { output.setColor("#ff1493");  return output; }
	if (cs == "deepskyblue"         ) { output.setColor("#00bfff");  return output; }
	if (cs == "dimgray"             ) { output.setColor("#696969");  return output; }
	if (cs == "dimgrey"             ) { output.setColor("#696969");  return output; }
	if (cs == "dodgerblue"          ) { output.setColor("#1e90ff");  return output; }
	break; case 'f':
	if (cs == "firebrick"           ) { output.setColor("#b22222");  return output; }
	if (cs == "floralwhite"         ) { output.setColor("#fffaf0");  return output; }
	if (cs == "forestgreen"         ) { output.setColor("#228b22");  return output; }
	if (cs == "fuchsia"             ) { output.setColor("#ff00ff");  return output; }
	break; case 'g':
	if (cs == "gainsboro"           ) { output.setColor("#dcdcdc");  return output; }
	if (cs == "ghostwhite"          ) { output.setColor("#f8f8ff");  return output; }
	if (cs == "gold"                ) { output.setColor("#ffd700");  return output; }
	if (cs == "goldenrod"           ) { output.setColor("#daa520");  return output; }
	if (cs == "gray"                ) { output.setColor("#808080");  return output; }
	if (cs == "grey"                ) { output.setColor("#808080");  return output; }
	if (cs == "green"               ) { output.setColor("#008000");  return output; }
	if (cs == "greenyellow"         ) { output.setColor("#adff2f");  return output; }
	break; case 'h':
	if (cs == "honeydew"            ) { output.setColor("#f0fff0");  return output; }
	if (cs == "hotpink"             ) { output.setColor("#ff69b4");  return output; }
	if (cs == "indianred"           ) { output.setColor("#cd5c5c");  return output; }
	break; case 'i':
	if (cs == "indigo"              ) { output.setColor("#4b0082");  return output; }
	if (cs == "ivory"               ) { output.setColor("#fffff0");  return output; }
	break; case 'k':
	if (cs == "khaki"               ) { output.setColor("#f0e68c");  return output; }
	break; case 'l':
	if (cs == "lavenderblush"       ) { output.setColor("#fff0f5");  return output; }
	if (cs == "lavender"            ) { output.setColor("#e6e6fa");  return output; }
	if (cs == "lawngreen"           ) { output.setColor("#7cfc00");  return output; }
	if (cs == "lemonchiffon"        ) { output.setColor("#fffacd");  return output; }
	if (cs == "lightblue"           ) { output.setColor("#add8e6");  return output; }
	if (cs == "lightorange"         ) { output.setColor("#ff9c00");  return output; }
	if (cs == "lightcoral"          ) { output.setColor("#f08080");  return output; }
	if (cs == "lightcyan"           ) { output.setColor("#e0ffff");  return output; }
	if (cs == "lightgoldenrodyellow") { output.setColor("#fafad2");  return output; }
	if (cs == "lightgreen"          ) { output.setColor("#90ee90");  return output; }
	if (cs == "lightgrey"           ) { output.setColor("#d3d3d3");  return output; }
	if (cs == "lightpink"           ) { output.setColor("#ffb6c1");  return output; }
	if (cs == "lightsalmon"         ) { output.setColor("#ffa07a");  return output; }
	if (cs == "lightseagreen"       ) { output.setColor("#20b2aa");  return output; }
	if (cs == "lightskyblue"        ) { output.setColor("#87cefa");  return output; }
	if (cs == "lightslategray"      ) { output.setColor("#778899");  return output; }
	if (cs == "lightslategrey"      ) { output.setColor("#778899");  return output; }
	if (cs == "lightsteelblue"      ) { output.setColor("#b0c4de");  return output; }
	if (cs == "lightyellow"         ) { output.setColor("#ffffe0");  return output; }
	if (cs == "lime"                ) { output.setColor("#00ff00");  return output; }
	if (cs == "limegreen"           ) { output.setColor("#32cd32");  return output; }
	if (cs == "linen"               ) { output.setColor("#faf0e6");  return output; }
	break; case 'm':
	if (cs == "magenta"             ) { output.setColor("#ff00ff");  return output; }
	if (cs == "maroon"              ) { output.setColor("#800000");  return output; }
	if (cs == "maroon"              ) { output.setColor("#800000");  return output; }
	if (cs == "mediumaquamarine"    ) { output.setColor("#66cdaa");  return output; }
	if (cs == "mediumblue"          ) { output.setColor("#0000cd");  return output; }
	if (cs == "mediumorchid"        ) { output.setColor("#ba55d3");  return output; }
	if (cs == "mediumpurple"        ) { output.setColor("#9370db");  return output; }
	if (cs == "mediumseagreen"      ) { output.setColor("#3cb371");  return output; }
	if (cs == "mediumslateblue"     ) { output.setColor("#7b68ee");  return output; }
	if (cs == "mediumspringgreen"   ) { output.setColor("#00fa9a");  return output; }
	if (cs == "mediumturquoise"     ) { output.setColor("#48d1cc");  return output; }
	if (cs == "mediumvioletred"     ) { output.setColor("#c71585");  return output; }
	if (cs == "midnightblue"        ) { output.setColor("#191970");  return output; }
	if (cs == "mintcream"           ) { output.setColor("#f5fffa");  return output; }
	if (cs == "mistyrose"           ) { output.setColor("#ffe4e1");  return output; }
	if (cs == "moccasin"            ) { output.setColor("#ffe4b5");  return output; }
	break; case 'n':
	if (cs == "navajowhite"         ) { output.setColor("#ffdead");  return output; }
	if (cs == "navy"                ) { output.setColor("#000080");  return output; }
	if (cs == "navy"                ) { output.setColor("#000080");  return output; }
	break; case 'o':
	if (cs == "oldlace"             ) { output.setColor("#fdf5e6");  return output; }
	if (cs == "olive"               ) { output.setColor("#6b8e23");  return output; }
	if (cs == "olivedrab"           ) { output.setColor("#6b8e23");  return output; }
	if (cs == "orange"              ) { output.setColor("#ff4500");  return output; }
	if (cs == "orangered"           ) { output.setColor("#ff4500");  return output; }
	if (cs == "orchid"              ) { output.setColor("#da70d6");  return output; }
	break; case 'p':
	if (cs == "palegoldenrod"       ) { output.setColor("#eee8aa");  return output; }
	if (cs == "palegreen"           ) { output.setColor("#98fb98");  return output; }
	if (cs == "paleturquoise"       ) { output.setColor("#afeeee");  return output; }
	if (cs == "palevioletred"       ) { output.setColor("#db7093");  return output; }
	if (cs == "papayawhip"          ) { output.setColor("#ffefd5");  return output; }
	if (cs == "peachpuff"           ) { output.setColor("#ffdab9");  return output; }
	if (cs == "peru"                ) { output.setColor("#cd853f");  return output; }
	if (cs == "pink"                ) { output.setColor("#ffc8cb");  return output; }
	if (cs == "plum"                ) { output.setColor("#dda0dd");  return output; }
	if (cs == "powderblue"          ) { output.setColor("#b0e0e6");  return output; }
	if (cs == "purple"              ) { output.setColor("#800080");  return output; }
	if (cs == "purple"              ) { output.setColor("#800080");  return output; }
	break; case 'q':
	if (cs == "quartz"              ) { output.setColor("#c9c9f3");  return output; }
	break; case 'r':
	if (cs == "red"                 ) { output.setColor("#ff0000");  return output; }
	if (cs == "rosybrown"           ) { output.setColor("#bc8f8f");  return output; }
	if (cs == "royalblue"           ) { output.setColor("#4169e1");  return output; }
	break; case 's':
	if (cs == "saddlebrown"         ) { output.setColor("#8b4513");  return output; }
	if (cs == "salmon"              ) { output.setColor("#fa8072");  return output; }
	if (cs == "sandybrown"          ) { output.setColor("#f4a460");  return output; }
	if (cs == "seagreen"            ) { output.setColor("#2e8b57");  return output; }
	if (cs == "seashell"            ) { output.setColor("#fff5ee");  return output; }
	if (cs == "sienna"              ) { output.setColor("#a0522d");  return output; }
	if (cs == "silver"              ) { output.setColor("#c0c0c0");  return output; }
	if (cs == "silver"              ) { output.setColor("#c0c0c0");  return output; }
	if (cs == "skyblue"             ) { output.setColor("#87ceeb");  return output; }
	if (cs == "slateblue"           ) { output.setColor("#6a5acd");  return output; }
	if (cs == "snow"                ) { output.setColor("#fffafa");  return output; }
	if (cs == "steelblue"           ) { output.setColor("#4682b4");  return output; }
	break; case 't':
	if (cs == "tan"                 ) { output.setColor("#d2b48c");  return output; }
	if (cs == "teal"                ) { output.setColor("#008080");  return output; }
	if (cs == "thistle"             ) { output.setColor("#d8bfd8");  return output; }
	if (cs == "tomato"              ) { output.setColor("#ff6347");  return output; }
	if (cs == "turquoise"           ) { output.setColor("#40e0d0");  return output; }
	break; case 'v':
	if (cs == "violet"              ) { output.setColor("#ee82ee");  return output; }
	break; case 'w':
	if (cs == "wheat"               ) { output.setColor("#f5deb3");  return output; }
	if (cs == "white"               ) { output.setColor("#ffffff");  return output; }
	if (cs == "white"               ) { output.setColor("#ffffff");  return output; }
	if (cs == "whitesmoke"          ) { output.setColor("#f5f5f5");  return output; }
	break; case 'y':
	if (cs == "yellow"              ) { output.setColor("#ffff00");  return output; }
	if (cs == "yellowgreen"         ) { output.setColor("#9acd32");  return output; }
	}

	// References:
	//            http://netdancer.com/rgbblk.htm
	//            http://www.htmlhelp.com/cgi-bin/color.cgi?rgb=FFFFFF
	//            http://www.brobstsystems.com/colors1.htm

	return output;
}



//////////////////////////////
//
// PixelColor::writePpm6 -- write the pixel in PPM 6 format.
//

void PixelColor::writePpm6(ostream& out) {
	out << (unsigned char)getRed() << (unsigned char)getGreen() << (unsigned char)getBlue();
}

void PixelColor::writePpm3(ostream& out) {
	out << (int)getRed()   << " "
		 << (int)getGreen() << " "
		 << (int)getBlue()  << " ";
}



//////////////////////////////
//
// PixelColor::setHue --
//

PixelColor& PixelColor::setHue(float value) {
	double fraction = value - (int)value;
	if (fraction < 0) {
		fraction = fraction + 1.0;
	}

	if (fraction < 1.0/6.0) {
		Red   = 255;
		Green = (unsigned char)limit(floatToChar(6.0 * fraction), 0, 255);
		Blue  = 0;
	} else if (fraction < 2.0/6.0) {
		Red   = (unsigned char)limit(255 - floatToChar(6.0 * (fraction - 1.0/6.0)), 0,255);
		Green = 255;
		Blue  = 0;
	} else if (fraction < 3.0/6.0) {
		Red   = 0;
		Green = 255;
		Blue  = (unsigned char)limit(floatToChar(6.0 * (fraction - 2.0/6.0)), 0,255);
	} else if (fraction < 4.0/6.0) {
		Red   = 0;
		Blue  = 255;
		Green = (unsigned char)limit(255 - floatToChar(6.0 * (fraction - 3.0/6.0)), 0,255);
	} else if (fraction < 5.0/6.0) {
		Red   = (unsigned char)limit(floatToChar(6.0 * (fraction - 4.0/6.0)), 0,255);
		Green = 0;
		Blue  = 255;
	} else if (fraction <= 6.0/6.0) {
		Red   = 255;
		Green = 0;
		Blue  = (unsigned char)limit(255 - floatToChar(6.0 * (fraction - 5.0/6.0)), 0,255);
	} else {
		Red   = 0;
		Green = 0;
		Blue  = 0;
	}

	return *this;
}



//////////////////////////////
//
// PixelColor::setTriHue -- Red, Green, Blue with a little overlap
//

PixelColor& PixelColor::setTriHue(float value) {
	double fraction = value - (int)value;
	if (fraction < 0) {
		fraction = fraction + 1.0;
	}
	if (fraction < 1.0/3.0) {
		Green = (unsigned char)limit(floatToChar(3.0 * fraction), 0, 255);
		Red   = (unsigned char)limit(255 - Green, 0, 255);
		Blue  = 0;
	} else if (fraction < 2.0/3.0) {
		setBlue(floatToChar(3.0 * (fraction - 1.0/3.0)));
		setGreen(255 - getBlue());
		setRed(0);
	} else {
		setRed(floatToChar(3.0 * (fraction - 2.0/3.0)));
		setBlue(255 - Red);
		setGreen(0);
	}

	return *this;
}




//////////////////////////////////////////////////////////////////////////
//
// private functions:
//


//////////////////////////////
//
// PixelColor::charToFloat --
//

float PixelColor::charToFloat(int value) {
	return value / 255.0;
}


//////////////////////////////
//
// PixelColor::floatToChar --
//

int PixelColor::floatToChar(float value) {
	return limit((int)(value * 255.0 + 0.5), 0, 255);
}


//////////////////////////////
//
// limit --
//

int PixelColor::limit(int value, int min, int max) {
	if (value < min) {
		value = min;
	} else if (value > max) {
		value = max;
	}
	return value;
}



//////////////////////////////
//
// PixelColor:mix -- mix two colors together.
//

PixelColor PixelColor::mix(PixelColor& color1, PixelColor& color2) {

	PixelColor p1 = color1.getHsi();
	PixelColor p2 = color2.getHsi();

	PixelColor output;
	unsigned int r = ((unsigned int)color1.Red + (unsigned int)color2.Red)/2;
	unsigned int g = ((unsigned int)color1.Green + (unsigned int)color2.Green)/2;
	unsigned int b = ((unsigned int)color1.Blue + (unsigned int)color2.Blue)/2;

	output.setRed(r);
	output.setGreen(g);
	output.setBlue(b);

	return output;
}



//////////////////////////////
//
// PixelColor::rgb2hsi -- convert from RGB color space to HSI color space.
//     You have to keep track of color space used by pixel since RGB/HSI
//     state is not stored in pixel.
//

PixelColor& PixelColor::rgb2hsi(void) {

	// Convert RGB into range from 0 to 255.0:
	double R = Red   / 255.0;
	double G = Green / 255.0;
	double B = Blue  / 255.0;

	// HSI will be in the range from 0.0 to 1.0;
	double H = 0.0; // will be stored in Red parameter
	double S = 0.0; // will be stored in Green parameter
	double I = 0.0; // will be stored in Blue parameter

	double min = R;
	if (G < min) min = G;
	if (B < min) min = B;

	I = (R+G+B)/3.0;
	S = 1 - min/I;
	if (S == 0.0) {
		H = 0.0;
	} else {
		H = ((R-G)+(R-B))/2.0;
		H = H/sqrt((R-G)*(R-G) + (R-B)*(G-B));
		H = acos(H);
		if (B > G) {
			H = 2*M_PI - H;
		}
		H = H/(2*M_PI);
	}

	// Adjust output range from 0 to 255:
	int h = (int)(H  * 255.0 + 0.5);
	if (h < 0)   { h = 0; }
	if (h > 255) { h = 255; }

	int s = (int)(S  * 255.0 + 0.5);
	if (s < 0)   { s = 0; }
	if (s > 255) { s = 255; }

	int i = (int)(I  * 255.0 + 0.5);
	if (i < 0)   { i = 0; }
	if (i > 255) { i = 255; }

	Red   = h;
	Green = s;
	Blue  = i;

	return *this;
}



//////////////////////////////
//
// PixelColor::hsi2rgb -- convert from HSI color space to RGB color space.
//

PixelColor& PixelColor::hsi2rgb(void) {

	// Scale input HSI into the range from 0.0 to 1.0:
	double H = Red   / 255.0;
	double S = Green / 255.0;
	double I = Blue  / 255.0;

	double R = 0.0;
	double G = 0.0;
	double B = 0.0;

	if (H < 1.0/3.0) {
		B = (1-S)/3;
		R = (1+S*cos(2*M_PI*H)/cos(M_PI/3-2*M_PI*H))/3.0;
		G = 1 - (B + R);
	} else if (H < 2.0/3.0) {
		H = H - 1.0/3.0;
		R = (1-S)/3;
		G = (1+S*cos(2*M_PI*H)/cos(M_PI/3-2*M_PI*H))/3.0;
		B = 1 - (R+G);
	} else {
		H = H - 2.0/3.0;
		G = (1-S)/3;
		B = (1+S*cos(2*M_PI*H)/cos(M_PI/3-2*M_PI*H))/3.0;
		R = 1 - (G+B);
	}

	// Adjust output range from 0 to 255:
	int r = (int)(I * R * 3.0 * 255.0 + 0.5);
	if (r < 0)   { r = 0; }
	if (r > 255) { r = 255; }

	int g = (int)(I * G * 3.0 * 255.0 + 0.5);
	if (g < 0)   { g = 0; }
	if (g > 255) { g = 255; }

	int b = (int)(I * B * 3.0 * 255.0 + 0.5);
	if (b < 0)   { b = 0; }
	if (b > 255) { b = 255; }

	Red   = r;
	Green = g;
	Blue  = b;

	return *this;
}



//////////////////////////////
//
// PixelColor::getHsi -- convert from RGB color space to HSI color space.
//     You have to keep track of color space used by pixel since RGB/HSI
//     state is not stored in pixel.
//

PixelColor PixelColor::getHsi(void) {
	PixelColor tempColor = *this;
	tempColor.rgb2hsi();
	return tempColor;
}



//////////////////////////////
//
// PixelColor::getRgb -- convert from HSI color space to RGB color space.
//     You have to keep track of color space used by pixel since RGB/HSI
//     state is not stored in pixel.
//

PixelColor PixelColor::getRgb(void) {
	PixelColor tempColor = *this;
	tempColor.hsi2rgb();
	return tempColor;
}


//////////////////////////////
//
// PixelColor::getHexColor --
//

string PixelColor::getHexColor(void) {
	string output = "#";
	unsigned char redA   = (Red   & 0xF0) >> 4;
	unsigned char redB   = (Red   & 0x0F);
	unsigned char greenA = (Green & 0xF0) >> 4;
	unsigned char greenB = (Green & 0x0F);
	unsigned char blueA  = (Blue  & 0xF0) >> 4;
	unsigned char blueB  = (Blue  & 0x0F);

	if (redA < 10) {
		output += '0' + redA;
	} else {
		output += 'A' + redA - 10;
	}
	if (redB < 10) {
		output += '0' + redB;
	} else {
		output += 'A' + redB - 10;
	}

	if (greenA < 10) {
		output += '0' + greenA;
	} else {
		output += 'A' + greenA - 10;
	}
	if (greenB < 10) {
		output += '0' + greenB;
	} else {
		output += 'A' + greenB - 10;
	}

	if (blueA < 10) {
		output += '0' + blueA;
	} else {
		output += 'A' + blueA - 10;
	}
	if (blueB < 10) {
		output += '0' + blueB;
	} else {
		output += 'A' + blueB - 10;
	}

	return output;
}


///////////////////////////////////////////////////////////////////////////
//
// other functions
//


//////////////////////////////
//
// operator<< --
//
// for use with P3 ASCII pnm images: print red green blue triplet.
//

ostream& operator<<(ostream& out, PixelColor apixel) {
	out << apixel.getRed() << ' ';
	out << apixel.getGreen() << ' ';
	out << apixel.getBlue();
	return out;
}


// END_MERGE

} // end namespace hum


