//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 11 12:33:09 PDT 2002
// Last Modified: Wed Nov 11 12:40:11 PST 2020
// Filename:      PixelColor.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/PixelColor.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Class for managing an image pixel.
//

#include "PixelColor.h"

#include <string.h>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>

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

void PixelColor::setColor(PixelColor color) { 
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

int PixelColor::operator==(PixelColor color) {
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

int PixelColor::operator!=(PixelColor color) {
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

PixelColor PixelColor::operator+(PixelColor color) { 
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

PixelColor PixelColor::operator-(PixelColor color) { 
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

PixelColor PixelColor::operator*(PixelColor color) { 
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
	int i = 0;
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
		for (i=start; i<length; i++) {
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
		char rv[3] = {0};
		char gv[3] = {0};
		char bv[3] = {0};
		string piece1 = colorstring.substr(start, 2);
		string piece2 = colorstring.substr(start+2, 2);
		string piece3 = colorstring.substr(start+4, 2);

		strcpy(rv, piece1.c_str());
		strcpy(gv, piece1.c_str());
		strcpy(bv, piece1.c_str());
		int rval = strtol(rv, NULL, 16);
		int gval = strtol(gv, NULL, 16);
		int bval = strtol(bv, NULL, 16);
		output.setColor(rval, gval, bval);
		return output;
	}

	// color string
	char buffer[128] = {0};
	strncpy(buffer, colorstring.c_str(), 100);
	length = strlen(buffer);
	for (i=0; i<length; i++) {
		buffer[i] = std::tolower(buffer[i]);
	}
	output.setColor(0,0,0);

	if (strcmp("aliceblue",          buffer) == 0)  return getColor("#f0f8ff");
	if (strcmp("antiquewhite",       buffer) == 0)  return getColor("#faebd7");
	if (strcmp("aqua",               buffer) == 0)  return getColor("#00ffff");
	if (strcmp("aquamarine",         buffer) == 0)  return getColor("#7fffd4");
	if (strcmp("azure",              buffer) == 0)  return getColor("#f0ffff");
	if (strcmp("beige",              buffer) == 0)  return getColor("#f5f5dc");
	if (strcmp("bisque",             buffer) == 0)  return getColor("#ffe4c4");
	if (strcmp("black",              buffer) == 0)  return getColor("#000000");
	if (strcmp("blanchediamond",     buffer) == 0)  return getColor("#ffebcd");
	if (strcmp("blue",               buffer) == 0)  return getColor("#0000ff");
	if (strcmp("blueviolet",         buffer) == 0)  return getColor("#8a2be2");
	if (strcmp("brown",              buffer) == 0)  return getColor("#a52a2a");
	if (strcmp("burlywood",          buffer) == 0)  return getColor("#ffe4c4");
	if (strcmp("cadetblue",          buffer) == 0)  return getColor("#5f9ea0");
	if (strcmp("chartreuse",         buffer) == 0)  return getColor("#7fff00");
	if (strcmp("coral",              buffer) == 0)  return getColor("#ff7f50");
	if (strcmp("cornflowerblue",     buffer) == 0)  return getColor("#6495ed");
	if (strcmp("cornsilk",           buffer) == 0)  return getColor("#fff8dc");
	if (strcmp("crimson",            buffer) == 0)  return getColor("#dc143c");
	if (strcmp("cyan",               buffer) == 0)  return getColor("#00ffff");
	if (strcmp("darkblue",           buffer) == 0)  return getColor("#00008b");
	if (strcmp("darkcyan",           buffer) == 0)  return getColor("#008b8b");
	if (strcmp("darkgoldenrod",      buffer) == 0)  return getColor("#b8860b");
	if (strcmp("darkgray",           buffer) == 0)  return getColor("#a9a9a9");
	if (strcmp("darkgreen",          buffer) == 0)  return getColor("#006400");
	if (strcmp("darkkhaki",          buffer) == 0)  return getColor("#bdb76b");
	if (strcmp("darkmagenta",        buffer) == 0)  return getColor("#8b008b");
	if (strcmp("darkolivegreen",     buffer) == 0)  return getColor("#556b2f");
	if (strcmp("darkorange",         buffer) == 0)  return getColor("#ff8c00");
	if (strcmp("darkorchid",         buffer) == 0)  return getColor("#9932cc");
	if (strcmp("darkred",            buffer) == 0)  return getColor("#8b0000");
	if (strcmp("darksalmon",         buffer) == 0)  return getColor("#e9967a");
	if (strcmp("darkseagreen",       buffer) == 0)  return getColor("#8dbc8f");
	if (strcmp("darkslateblue",      buffer) == 0)  return getColor("#483d8b");
	if (strcmp("darkslategray",      buffer) == 0)  return getColor("#2e4e4e");
	if (strcmp("darkturquoise",      buffer) == 0)  return getColor("#00ded1");
	if (strcmp("darkviolet",         buffer) == 0)  return getColor("#9400d3");
	if (strcmp("deeppink",           buffer) == 0)  return getColor("#ff1493");
	if (strcmp("deepskyblue",        buffer) == 0)  return getColor("#00bfff");
	if (strcmp("dimgray",            buffer) == 0)  return getColor("#696969");
	if (strcmp("dodgerblue",         buffer) == 0)  return getColor("#1e90ff");
	if (strcmp("firebrick",          buffer) == 0)  return getColor("#b22222");
	if (strcmp("floralwhite",        buffer) == 0)  return getColor("#fffaf0");
	if (strcmp("forestgreen",        buffer) == 0)  return getColor("#228b22");
	if (strcmp("fuchsia",            buffer) == 0)  return getColor("#ff00ff");
	if (strcmp("gainsboro",          buffer) == 0)  return getColor("#dcdcdc");
	if (strcmp("ghostwhite",         buffer) == 0)  return getColor("#f8f8ff");
	if (strcmp("gold",               buffer) == 0)  return getColor("#ffd700");
	if (strcmp("goldenrod",          buffer) == 0)  return getColor("#daa520");
	if (strcmp("gray",               buffer) == 0)  return getColor("#808080");
	if (strcmp("gray",               buffer) == 0)  return getColor("#808080");
	if (strcmp("green",              buffer) == 0)  return getColor("#008000");
	if (strcmp("greenyellow",        buffer) == 0)  return getColor("#adff2f");
	if (strcmp("honeydew",           buffer) == 0)  return getColor("#f0fff0");
	if (strcmp("hotpink",            buffer) == 0)  return getColor("#ff69b4");
	if (strcmp("indianred",          buffer) == 0)  return getColor("#cd5c5c");
	if (strcmp("indigo",             buffer) == 0)  return getColor("#4b0082");
	if (strcmp("ivory",              buffer) == 0)  return getColor("#fffff0");
	if (strcmp("khaki",              buffer) == 0)  return getColor("#f0e68c");
	if (strcmp("lavenderblush",      buffer) == 0)  return getColor("#fff0f5");
	if (strcmp("lavender",           buffer) == 0)  return getColor("#e6e6fa");
	if (strcmp("lawngreen",          buffer) == 0)  return getColor("#7cfc00");
	if (strcmp("lemonchiffon",       buffer) == 0)  return getColor("#fffacd");
	if (strcmp("lightblue",          buffer) == 0)  return getColor("#add8e6");
	if (strcmp("lightorange",        buffer) == 0)  return getColor("#ff9c00");
	if (strcmp("lightcoral",         buffer) == 0)  return getColor("#f08080");
	if (strcmp("lightcyan",          buffer) == 0)  return getColor("#e0ffff");
	if (strcmp("lightgoldenrodyellow",buffer)== 0)  return getColor("#fafad2");
	if (strcmp("lightgreen",         buffer) == 0)  return getColor("#90ee90");
	if (strcmp("lightgrey",          buffer) == 0)  return getColor("#d3d3d3");
	if (strcmp("lightpink",          buffer) == 0)  return getColor("#ffb6c1");
	if (strcmp("lightsalmon",        buffer) == 0)  return getColor("#ffa07a");
	if (strcmp("lightseagreen",      buffer) == 0)  return getColor("#20b2aa");
	if (strcmp("lightskyblue",       buffer) == 0)  return getColor("#87cefa");
	if (strcmp("lightslategray",     buffer) == 0)  return getColor("#778899");
	if (strcmp("lightsteelblue",     buffer) == 0)  return getColor("#b0c4de");
	if (strcmp("lightyellow",        buffer) == 0)  return getColor("#ffffe0");
	if (strcmp("lime",               buffer) == 0)  return getColor("#00ff00");
	if (strcmp("limegreen",          buffer) == 0)  return getColor("#32cd32");
	if (strcmp("linen",              buffer) == 0)  return getColor("#faf0e6");
	if (strcmp("magenta",            buffer) == 0)  return getColor("#ff00ff");
	if (strcmp("maroon",             buffer) == 0)  return getColor("#800000");
	if (strcmp("maroon",             buffer) == 0)  return getColor("#800000");
	if (strcmp("mediumaquamarine",   buffer) == 0)  return getColor("#66cdaa");
	if (strcmp("mediumblue",         buffer) == 0)  return getColor("#0000cd");
	if (strcmp("mediumorchid",       buffer) == 0)  return getColor("#ba55d3");
	if (strcmp("mediumpurple",       buffer) == 0)  return getColor("#9370db");
	if (strcmp("mediumseagreen",     buffer) == 0)  return getColor("#3cb371");
	if (strcmp("mediumslateblue",    buffer) == 0)  return getColor("#7b68ee");
	if (strcmp("mediumspringgreen",  buffer) == 0)  return getColor("#00fa9a");
	if (strcmp("mediumturquoise",    buffer) == 0)  return getColor("#48d1cc");
	if (strcmp("mediumvioletred",    buffer) == 0)  return getColor("#c71585");
	if (strcmp("midnightblue",       buffer) == 0)  return getColor("#191970");
	if (strcmp("mintcream",          buffer) == 0)  return getColor("#f5fffa");
	if (strcmp("mistyrose",          buffer) == 0)  return getColor("#ffe4e1");
	if (strcmp("moccasin",           buffer) == 0)  return getColor("#ffe4b5");
	if (strcmp("navajowhite",        buffer) == 0)  return getColor("#ffdead");
	if (strcmp("navy",               buffer) == 0)  return getColor("#000080");
	if (strcmp("navy",               buffer) == 0)  return getColor("#000080");
	if (strcmp("oldlace",            buffer) == 0)  return getColor("#fdf5e6");
	if (strcmp("olive",              buffer) == 0)  return getColor("#6b8e23");
	if (strcmp("olivedrab",          buffer) == 0)  return getColor("#6b8e23");
	if (strcmp("orange",             buffer) == 0)  return getColor("#ff4500");
	if (strcmp("orangered",          buffer) == 0)  return getColor("#ff4500");
	if (strcmp("orchid",             buffer) == 0)  return getColor("#da70d6");
	if (strcmp("palegoldenrod",      buffer) == 0)  return getColor("#eee8aa");
	if (strcmp("palegreen",          buffer) == 0)  return getColor("#98fb98");
	if (strcmp("paleturquoise",      buffer) == 0)  return getColor("#afeeee");
	if (strcmp("palevioletred",      buffer) == 0)  return getColor("#db7093");
	if (strcmp("papayawhip",         buffer) == 0)  return getColor("#ffefd5");
	if (strcmp("peachpuff",          buffer) == 0)  return getColor("#ffdab9");
	if (strcmp("peru",               buffer) == 0)  return getColor("#cd853f");
	if (strcmp("pink",               buffer) == 0)  return getColor("#ffc8cb");
	if (strcmp("plum",               buffer) == 0)  return getColor("#dda0dd");
	if (strcmp("powderblue",         buffer) == 0)  return getColor("#b0e0e6");
	if (strcmp("purple",             buffer) == 0)  return getColor("#800080");
	if (strcmp("purple",             buffer) == 0)  return getColor("#800080");
	if (strcmp("quartz",             buffer) == 0)  return getColor("#c9c9f3");
	if (strcmp("red",                buffer) == 0)  return getColor("#ff0000");
	if (strcmp("rosybrown",          buffer) == 0)  return getColor("#bc8f8f");
	if (strcmp("royalblue",          buffer) == 0)  return getColor("#4169e1");
	if (strcmp("saddlebrown",        buffer) == 0)  return getColor("#8b4513");
	if (strcmp("salmon",             buffer) == 0)  return getColor("#fa8072");
	if (strcmp("sandybrown",         buffer) == 0)  return getColor("#f4a460");
	if (strcmp("seagreen",           buffer) == 0)  return getColor("#2e8b57");
	if (strcmp("seashell",           buffer) == 0)  return getColor("#fff5ee");
	if (strcmp("sienna",             buffer) == 0)  return getColor("#a0522d");
	if (strcmp("silver",             buffer) == 0)  return getColor("#c0c0c0");
	if (strcmp("silver",             buffer) == 0)  return getColor("#c0c0c0");
	if (strcmp("skyblue",            buffer) == 0)  return getColor("#87ceeb");
	if (strcmp("slateblue",          buffer) == 0)  return getColor("#6a5acd");
	if (strcmp("snow",               buffer) == 0)  return getColor("#fffafa");
	if (strcmp("steelblue",          buffer) == 0)  return getColor("#4682b4");
	if (strcmp("tan",                buffer) == 0)  return getColor("#d2b48c");
	if (strcmp("teal",               buffer) == 0)  return getColor("#008080");
	if (strcmp("thistle",            buffer) == 0)  return getColor("#d8bfd8");
	if (strcmp("tomato",             buffer) == 0)  return getColor("#ff6347");
	if (strcmp("turquoise",          buffer) == 0)  return getColor("#40e0d0");
	if (strcmp("violet",             buffer) == 0)  return getColor("#ee82ee");
	if (strcmp("wheat",              buffer) == 0)  return getColor("#f5deb3");
	if (strcmp("white",              buffer) == 0)  return getColor("#ffffff");
	if (strcmp("white",              buffer) == 0)  return getColor("#ffffff");
	if (strcmp("whitesmoke",         buffer) == 0)  return getColor("#f5f5f5");
	if (strcmp("yellow",             buffer) == 0)  return getColor("#ffff00");
	if (strcmp("yellowgreen",        buffer) == 0)  return getColor("#9acd32");

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
		Red   = 0;
		Green = (unsigned char)limit(floatToChar(6.0 * (fraction - 4.0/6.0)), 0,255);
		Blue  = 255;
	} else {
		Red   = 255;
		Green = 0;
		Blue  = (unsigned char)limit(255 - floatToChar(6.0 * (fraction - 5.0/6.0)), 0,255);
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


