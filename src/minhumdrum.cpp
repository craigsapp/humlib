//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 12 17:44:39 PDT 2015
// Filename:      /include/minhumdrum.cpp
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/minhumdrum.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Source file for minHumdrum library.
//

#include "minhumdrum.h"


//////////////////////////////
//
// HumNum::HumNum --
//

HumNum::HumNum(void){
	setValue(0);
}


HumNum::HumNum(int value){
	setValue(value);
}


HumNum::HumNum(int numerator, int denominator){
	setValue(numerator, denominator);
}


HumNum::HumNum(const HumNum& rat) {
	*this = rat;
}



//////////////////////////////
//
// HumNum::~HumNum --
//

HumNum::~HumNum() {
	// do nothing
}



//////////////////////////////
//
// HumNum::isNegative -- Returns true if value is negative.
//

bool HumNum::isNegative(void) const {
	return isFinite() && (top < 0);
}



//////////////////////////////
//
// HumNum::isPositive -- Returns true if value is positive.
//

bool HumNum::isPositive(void) const {
	return isFinite() && (top > 0);
}



//////////////////////////////
//
// HumNum::isZero -- Returns true if value is zero.
//

bool HumNum::isZero(void) const {
	return isFinite() && (top == 0);
}



//////////////////////////////
//
// HumNum::isNonZero -- Returns true if value is zero.
//

bool HumNum::isNonZero(void) const {
	return isFinite() && (top != 0);
}



//////////////////////////////
//
// HumNum::isNonNegative -- Returns true if value is non-negative.
//

bool HumNum::isNonNegative(void) const {
	return isFinite() && (top >= 0);
}



//////////////////////////////
//
// HumNum::isNonPositive -- Returns true if value is non-positive.
//

bool HumNum::isNonPositive(void) const {
	return isFinite() && (top >= 0);
}



//////////////////////////////
//
// HumNum::getFloat -- Return the floating-point equivalent of the
//     rational number.
//

double HumNum::getFloat(void) const {
	return (double)top/(double)bot;
}



//////////////////////////////
//
// HumNum::getInteger -- Return the integral part of the fraction.
//    Default value: round = 0.0
//    Optional parameter is a rounding factor.
//    Examples:
//       8/5 | round=0.0 ==  1
//      -8/5 | round=0.0 == -1
//			8/5 | roudn=0.5 ==  1
//      -8/5 | round=0.5 == -1
//

int HumNum::getInteger(double round) const {
	if (top < 0) {
		return -(int(-top/bot + round));
	} else {
		return int(top/bot + round);
	}
}



//////////////////////////////
//
// HumNum::getNumerator --
//

int HumNum::getNumerator(void) const {
	return top;
}



//////////////////////////////
//
// HumNum::getDenominator --
//

int HumNum::getDenominator(void) const {
	return bot;
}


//////////////////////////////
//
// HumNum::setValue --
//

void HumNum::setValue(int numerator) {
	top = numerator;
	bot = 1;
}


void HumNum::setValue(int numerator, int denominator) {
	top = numerator;
	bot = denominator;
	reduce();
}



//////////////////////////////
//
// HumNum::getAbs -- returns the absolute value of the rational number.
//

HumNum HumNum::getAbs(void) const {
	HumNum rat(top, bot);
	if (isNegative()) {
		rat.setValue(-top, bot);
	}
	return rat;
}



//////////////////////////////
//
// HumNum::makeAbs -- Make the rational number non-negative.
//

HumNum& HumNum::makeAbs(void) {
	if (!isNonNegative()) {
		top = -top;
	}
	return *this;
}



//////////////////////////////
//
// HumNum::reduce -- simplify the fraction.
//

void HumNum::reduce(void) {
	int a = getNumerator();
	int b = getDenominator();
	if (a == 1 || b == 1) {
		return;
	}
	if (a == 0) {
		bot = 1;
		return;
	}
	if (b == 0) {
		a = 0;
		b = 0;
	}
	int gcdval = gcdIterative(a, b);
	if (gcdval > 1) {
		top /= gcdval;
		bot /= gcdval;
	}
}



//////////////////////////////
//
// HumNum::gcdIterative -- Return the greatest common divisor of two
//      numbers using an iterative algorithm.
//

int HumNum::gcdIterative(int a, int b) {
	int c;
	while (b) {
		c = a;
		a = b;
		b = c % b;
	}
	return a < 0 ? -a : a;
}



//////////////////////////////
//
// HumNum::gcdRecursive -- Return the greatest common divisor of two
//      numbers using a recursive algorithm.
//

int HumNum::gcdRecursive(int a, int b) {
	if (a < 0) {
		a = -a;
	}
	if (!b) {
		return a;
	} else {
		return gcdRecursive(b, a % b);
	}
}



//////////////////////////////
//
// HumNum::isInfinite -- Returns true if the denominator is zero.
//

bool HumNum::isInfinite(void) const {
	return (bot == 0) && (top != 0);
}



//////////////////////////////
//
// HumNum::isNaN -- Returns true if the numerator and denominator are zero.
//

bool HumNum::isNaN(void) const {
	return (bot == 0) && (top == 0);
}



//////////////////////////////
//
// HumNum::isFinite -- Returns true if the denominator is not zero.
//

bool HumNum::isFinite(void) const {
	return bot != 0;
}



//////////////////////////////
//
// HumNum::isInteger -- Return true if an integer.
//

bool HumNum::isInteger(void) const {
	return bot == 1;
}



//////////////////////////////
//
// HumNum::operator+ -- Addition operator.
//

HumNum HumNum::operator+(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2	+ a2 * b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator+(int value) {
	HumNum output(value * bot + top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Subtraction operator.
//

HumNum HumNum::operator-(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2	- a2*b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator-(int value) {
	HumNum output(top - value * bot, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Unary negation operator
//

HumNum HumNum::operator-(void) {
	HumNum output(-top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator* -- Multiplication operator.
//

HumNum HumNum::operator*(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*a2;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator*(int value) {
	HumNum output(top * value, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator/ -- Division operator.
//

HumNum HumNum::operator/(const HumNum& value) {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2;
	int bo = b1*a2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator/(int value) {
	int a  = getNumerator();
	int b  = getDenominator();
	if (value < 0) {
		a = -a;
		b *= -value;
	} else {
		b *= value;
	}
	HumNum output(a, b);
	return output;
}



//////////////////////////////
//
// HumNum::operator= -- Assign from another value.
//

HumNum& HumNum::operator=(const HumNum& value) {
	if (this == &value) {
		return *this;
	}
	setValue(value.top, value.bot);
	return *this;
}

HumNum& HumNum::operator=(int  value) {
	setValue(value);
	return *this;
}



//////////////////////////////
//
// HumNum::operator+= --
//

HumNum& HumNum::operator+=(const HumNum& value) {
	*this = *this + value;
	return *this;
}


HumNum& HumNum::operator+=(int value) {
	*this = *this + value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator-= --
//

HumNum& HumNum::operator-=(const HumNum& value) {
	*this = *this - value;
	return *this;
}


HumNum& HumNum::operator-=(int value) {
	*this = *this - value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator*= --
//

HumNum& HumNum::operator*=(const HumNum& value) {
	*this = *this * value;
	return *this;
}


HumNum& HumNum::operator*=(int value) {
	*this = *this * value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator/= --
//

HumNum& HumNum::operator/=(const HumNum& value) {
	*this = *this / value;
	return *this;
}


HumNum& HumNum::operator/=(int value) {
	*this = *this / value;
	return *this;
}



//////////////////////////////
//
// HumNum::operator< -- Test less-than equality
//

bool HumNum::operator<(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() < value.getFloat();
}


bool HumNum::operator<(int value) const {
	return getFloat() < value;
}


bool HumNum::operator<(double value) const {
	return getFloat() < value;
}



//////////////////////////////
//
// HumNum::operator<= -- Return less-than-or-equal equality
//

bool HumNum::operator<=(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() <= value.getFloat();
}


bool HumNum::operator<=(int value) const {
	return getFloat() <= value;
}


bool HumNum::operator<=(double value) const {
	return getFloat() <= value;
}



//////////////////////////////
//
// HumNum::operator> -- Test greater-than equality
//

bool HumNum::operator>(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() > value.getFloat();
}


bool HumNum::operator>(int value) const {
	return getFloat() > value;
}


bool HumNum::operator>(double value) const {
	return getFloat() > value;
}



//////////////////////////////
//
// HumNum::operator>= -- Test greater-than-or-equal equality
//

bool HumNum::operator>=(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() >= value.getFloat();
}


bool HumNum::operator>=(int value) const {
	return getFloat() >= value;
}


bool HumNum::operator>=(double value) const {
	return getFloat() >= value;
}



//////////////////////////////
//
// HumNum::operator== -- Test equality.
//

bool HumNum::operator==(const HumNum& value) const {
	if (this == &value) {
		return true;
	}
	return getFloat() == value.getFloat();
}


bool HumNum::operator==(int value) const {
	return getFloat() == value;
}


bool HumNum::operator==(double value) const {
	return getFloat() == value;
}


//////////////////////////////
//
// HumNum::printFraction -- Print as fraction, such as 3/2.
//		default parameter: out = cout;
//

ostream& HumNum::printFraction(ostream& out) const {
	if (this->isInteger()) {
		out << getNumerator();
	} else {
		out << getNumerator() << '/' << getDenominator();
	}
	return out;
}



//////////////////////////////
//
// HumNum::printMixedFration -- Print as an integer plus fractional part.
//		default parameter: out = cout;
//		default parameter: separator = "_"
//

ostream& HumNum::printMixedFraction(ostream& out, string separator) const {
	if (this->isInteger()) {
		out << getNumerator();
	} else if (top > bot) {
		int intval = this->getInteger();
		int remainder = top - intval * bot;
		out << intval << separator << remainder << '/' << bot;
	} else {
		printFraction(out);
	}
	return out;
}



//////////////////////////////
//
// HumNum::printList -- Print as a list of two numbers.
//		default parameter: out = cout;
//

ostream& HumNum::printList(ostream& out) const {
	out << '(' << top << ", " << bot << ')';
	return out;
}



//////////////////////////////
//
// operator<< --
//

ostream& operator<<(ostream& out, const HumNum& number) {
	number.printFraction(out);
	return out;
}

/////////////////////////////
//
// template for printing arrays of items (not pointers to items).
//

template <typename A>
ostream& operator<<(ostream& out, const vector<A>& v) {
	for (unsigned int i=0; i<v.size(); i++) {
		out << v[i];
		if (i < v.size() - 1) {		
			out << '\t';
		}
	}
	return out;
}


//////////////////////////////
//
// HumAddress::HumAddress --
//

HumAddress::HumAddress(void) {
	track      = -1;
	subtrack   = -1;
	lineindex  = -1;
	fieldindex = -1;
}



//////////////////////////////
//
// HumAddress::~HumAddress --
//

HumAddress::~HumAddress() {
	track      = -1;
	subtrack   = -1;
	lineindex  = -1;
	fieldindex = -1;
}



//////////////////////////////
//
// HumAddress::getLineIndex --
//

int  HumAddress::getLineIndex(void) const {
	return lineindex;
}



//////////////////////////////
//
// HumAddress::getLineNumber --
//

int  HumAddress::getLineNumber(void) const {
	return lineindex + 1;
}



//////////////////////////////
//
// HumAddress::getFieldIndex --
//

int  HumAddress::getFieldIndex(void) const {
	return fieldindex;
}



//////////////////////////////
//
// HumAddress::getDataType --
//

string HumAddress::getDataType(void) const {
	return exinterp;
}



//////////////////////////////
//
// HumAddress::getSpineInfo --
//

string HumAddress::getSpineInfo(void) const {
	return spining;
}



//////////////////////////////
//
// HumAddress::getTrack --
//

int HumAddress::getTrack(void) const {
	return track;
}



//////////////////////////////
//
// HumAddress::getSubTrack --
//

int HumAddress::getSubtrack(void) const {
	return subtrack;
}



//////////////////////////////
//
// HumAddress::getTrackString --
//

string HumAddress::getTrackString(void) const {
	string output;
	int thetrack    = getTrack();
	int thesubtrack = getSubtrack();
	output += to_string(thetrack);
	if (thesubtrack > 0) {
		output += '.' + to_string(thesubtrack);
	}
	return output;
}



//////////////////////////////
//
// HumAddress::setLineAddress --
//

void HumAddress::setLineAddress(int aLineIndex, int aFieldIndex) {
	setLineIndex(aLineIndex);
	setFieldIndex(aFieldIndex);
}



//////////////////////////////
//
// HumAddress::setLineIndex --
//

void HumAddress::setLineIndex(int index) {
	lineindex = index;
}



//////////////////////////////
//
// HumAddress::setFieldIndex --
//

void HumAddress::setFieldIndex(int index) {
	lineindex = index;
}



//////////////////////////////
//
// HumAddress::setDataType --
//

void HumAddress::setDataType(const string& datatype) {
	switch (datatype.size()) {
		case 0:
			cerr << "Error: cannot have an empty data type." << endl;
			exit(1);
		case 1:
			if (datatype[0] == '*') {
				cerr << "Error: incorrect data type: " << datatype << endl;
				exit(1);
			} else {
				exinterp = "**" + datatype;
				return;
			}
		case 2:
			if ((datatype[0] == '*') && (datatype[1] == '*')) {
				cerr << "Error: incorrect data type: " << datatype << endl;
				exit(1);
			} else if (datatype[1] == '*') {
				exinterp = "**" + datatype;
				return;
			} else {
				exinterp = '*' + datatype;
				return;
			}
		default:
			if (datatype[0] != '*') {
				exinterp = "**" + datatype;
				return;
			} else if (datatype[1] == '*') {
				exinterp = datatype;
				return;
			} else {
				exinterp = '*' + datatype;
				return;
			}
	}
}



//////////////////////////////
//
// HumAddress::setSpineInfo --
//

void HumAddress::setSpineInfo(const string& spineinfo) {
	spining = spineinfo;
}



//////////////////////////////
//
// HumAddress::setTrack --
//

void HumAddress::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aTrack);
}


void HumAddress::setTrack(int aTrack) {
	if (aTrack < 0) {
		aTrack = 0;
	}
	if (aTrack > 1000) {
		aTrack = 1000;
	}
	track = aTrack;
}



//////////////////////////////
//
// HumAddress::setSubtrack --
//

void HumAddress::setSubtrack(int aSubtrack) {
	if (aSubtrack < 0) {
		aSubtrack = 0;
	}
	if (aSubtrack > 1000) {
		aSubtrack = 1000;
	}
	subtrack = aSubtrack;
}



//////////////////////////////
//
// HumdrumFile::HumdrumFile --
//

HumdrumFile::HumdrumFile(void) { }



//////////////////////////////
//
// HumdrumFile::~HumdrumFile --
//

HumdrumFile::~HumdrumFile() { }



//////////////////////////////
//
// HumdrumFile::operator[] --
//

HumdrumLine& HumdrumFile::operator[](int index) {
	if ((index < 0) || (index >= (int)lines.size())) {
		cerr << "Error: invalid index " << index << " in HumdrumFile" << endl;
		exit(1);
	}
	return *lines[index];
}



//////////////////////////////
//
// HumdrumFile::read -- Load file contents from input stream.
//

bool HumdrumFile::read(istream& infile) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (infile.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine(buffer);
		lines.push_back(s);
	}
	createTokensFromLines();
	if (!analyzeLines())  { return false; }
	if (!analyzeSpines()) { return false; }
	if (!analyzeLinks())  { return false; }
	if (!analyzeTracks()) { return false; }
	return analyzeRhythm();
}


bool HumdrumFile::read(const char* filename) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return read(cin);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return false;
		}
	}
	int status = read(infile);
	infile.close();
	return status;
}


bool HumdrumFile::read(const string& filename) {
	return read(filename.c_str());
}



//////////////////////////////
//
// HumdrumFile::ReadString -- Read contents from a string rather than 
//    an istream or filename.
//

bool HumdrumFile::readString(const string& contents) { 
	stringstream infile;
	infile << contents;
	return read(infile);
}


bool HumdrumFile::readString(const char* contents) { 
	stringstream infile;
	infile << contents;
	return read(infile);
}



//////////////////////////////
//
// HumdrumFile::createTokensFromLines -- Generate token array from
//    current state of lines.  If either state is changed, then the
//    other state becomes invalid.  See createLinesFromTokens for
//		regeneration of lines from tokens.
//

void HumdrumFile::createTokensFromLines(void) {
	int i;
	for (i=0; i<lines.size(); i++) {
		lines[i]->createTokensFromLine();
	}
}



//////////////////////////////
//
// HumdrumFile::createLinesFromTokens -- Generate Humdrum lines from
//   the list of tokens.
//

void HumdrumFile::createLinesFromTokens(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->createLineFromTokens();
	}
}



////////////////////////////
//
// HumdrumFile::append -- Add a line to the file's contents.
//

void HumdrumFile::append(const char* line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}


void HumdrumFile::append(const string& line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}



////////////////////////////
//
// HumdrumFile::size -- Return the number of lines.
//

int HumdrumFile::size(void) const {
	return lines.size();
}



//////////////////////////////
//
// HumdrumFile::getMaxTrack -- returns the number of primary spines in the data.
//

int HumdrumFile::getMaxTrack(void) const {
	return maxtrack;
}



//////////////////////////////
//
// HumdrumFile::printSpineInfo --
//

ostream& HumdrumFile::printSpineInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printSpineInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDataTypeInfo --
//

ostream& HumdrumFile::printDataTypeInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printDataTypeInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printTrackInfo --
//

ostream& HumdrumFile::printTrackInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printTrackInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::printDurationInfo --
//

ostream& HumdrumFile::printDurationInfo(ostream& out) {
	for (int i=0; i<size(); i++) {
		lines[i]->printDurationInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFile::analyzeLines -- Mark the line's index number in the
//    HumdrumFile.
//

bool HumdrumFile::analyzeLines(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->setLineIndex(i);
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeTracks -- Analyze the track structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeTracks(void) {
	for (int i=0; i<lines.size(); i++) {
		int status = lines[i]->analyzeTracks();
		if (!status) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeLinks -- Generate forward and backwards spine links
//    for each token.
//

bool HumdrumFile::analyzeLinks(void) {
	HumdrumLine* next     = NULL;
	HumdrumLine* previous = NULL;

	for (int i=0; i<lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		previous = next;
		next = lines[i];
		if (previous != NULL) {
			if (!stitchLinesTogether(*previous, *next)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::stitchLinesTogether -- Make forward/backward links for
//    tokens on each line.
//

bool HumdrumFile::stitchLinesTogether(HumdrumLine& previous,
		HumdrumLine& next) {
	int i;

   // first handle simple cases where the spine assignments are one-to-one:
	if (!previous.isInterpretation() && !next.isInterpretation()) {
		if (previous.getTokenCount() != next.getTokenCount()) {
			cerr << "Error lines " << (previous.getLineNumber())
			     << " and " << (next.getLineNumber()) << " not same length\n";
			cerr << "Line " << (previous.getLineNumber()) << ": "
			     << previous << endl;
			cerr << "Line " << (next.getLineNumber()) << ": "
			     << next << endl;
			return false;
		}
		for (i=0; i<previous.getTokenCount(); i++) {
			previous.token(i).makeForwardLink(next.token(i));
		}
		return true;
	}

	int ii = 0;
	for (i=0; i<previous.getTokenCount(); i++) {
		if (!previous.token(i).isManipulator()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isSplitInterpretation()) {
			// connect the previous token to the next two tokens.
			previous.token(i).makeForwardLink(next.token(ii++));
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isMergeInterpretation()) {
			// connect multiple previous tokens which are adjacent *v
			// spine manipulators to the current next token.
			while ((i<previous.getTokenCount()) &&
					previous.token(i).isMergeInterpretation()) {
				previous.token(i).makeForwardLink(next.token(ii));
				i++;
			}
			i--;
			ii++;
		} else if (previous.token(i).isExchangeInterpretation()) {
			// swapping the order of two spines.
			if ((i<previous.getTokenCount()) &&
					previous.token(i+1).isExchangeInterpretation()) {
				previous.token(i+1).makeForwardLink(next.token(ii++));
				previous.token(i).makeForwardLink(next.token(ii++));
			}
		} else if (previous.token(i).isTerminateInterpretation()) {
			// No link should be made.  There may be a problem if a
			// new segment is given (this should be handled by a
			// HumdrumSet class, not HumdrumFile.
		} else if (previous.token(i).isAddInterpretation()) {
			// A new data stream is being added, the next linked token
			// should be an exclusive interpretation.
			if (!next.token(i).isExclusiveInterpretation()) {
				cerr << "Error: expecting exclusive interpretation on line "
				     << next.getLineNumber() << " but got "
				     << next.token(i) << endl;
				return false;
			}
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isExclusiveInterpretation()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else {
			cerr << "Error: should not get here" << endl;
			return false;
		}
	}

	if ((i != previous.getTokenCount()) || (ii != next.getTokenCount())) {
		cerr << "Error: cannot stitch lines together due to alignment problem\n";
		cerr << "Line " << previous.getLineNumber() << ": "
		     << previous << endl;
		cerr << "Line " << next.getLineNumber() << ": "
		     << next << endl;
		cerr << "I = " <<i<< " token count " << previous.getTokenCount() << endl;
		cerr << "II = " <<ii<< " token count " << next.getTokenCount() << endl;
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeSpines -- Analyze the spine structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeSpines(void) {
	vector<string> datatype;
	vector<string> sinfo;
	vector<vector<HumdrumToken*> > lastspine;

	bool init = false;
	int i, j;
	for (i=0; i<size(); i++) {
		if (!lines[i]->hasSpines()) {
			lines[i]->token(0).setLineAddress(i, 0);
			continue;
		}
		if ((init == false) && !lines[i]->isExclusive()) {
			cerr << "Error on line: " << (i+1) << ':' << endl;
			cerr << "   Data found before exclusive interpretation" << endl;
			cerr << "   LINE: " << *lines[i] << endl;
			return false;
		}
		if ((init == false) && lines[i]->isExclusive()) {
			// first line of data in file.
			init = true;
			datatype.resize(lines[i]->getTokenCount());
			sinfo.resize(lines[i]->getTokenCount());
			lastspine.resize(lines[i]->getTokenCount());
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				datatype[j] = lines[i]->getTokenString(j);
				sinfo[j]    = to_string(i+1);
				lines[i]->token(j).setDataType(datatype[j]);
				lines[i]->token(j).setSpineInfo(sinfo[j]);
				lines[i]->token(j).setLineAddress(i, j);
				lastspine[j].push_back(&(lines[i]->token(j)));
			}
			maxtrack = datatype.size();
			continue;
		}
		if (datatype.size() != lines[i]->getTokenCount()) {
			cerr << "Error on line " << (i+1) << ':' << endl;
			cerr << "   Expected " << datatype.size() << " fields,"
			     << " but found " << lines[i]->getTokenCount() << endl;
			return false;
		}
		for (j=0; j<lines[i]->getTokenCount(); j++) {
			lines[i]->token(j).setDataType(datatype[j]);
			lines[i]->token(j).setSpineInfo(sinfo[j]);
			lines[i]->token(j).setLineAddress(i, j);
		}
		if (!lines[i]->isManipulator()) {
			continue;
		}
		maxtrack = adjustSpines(*lines[i], datatype, sinfo, maxtrack);
	}
   return true;
}




//////////////////////////////
//
// HumdrumFile::adjustSpines -- adjust datatype and spineinfo based
//   on manipulators.
//

int HumdrumFile::adjustSpines(HumdrumLine& line, vector<string>& datatype,
		vector<string>& sinfo, int trackcount) {
	vector<string> newtype;
	vector<string> newinfo;
	int mergecount = 0;
	int i, j;
	for (i=0; i<line.getTokenCount(); i++) {
		if (line.token(i).isSplitInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 2);
			newinfo[newinfo.size()-2] = '(' + sinfo[i] + ")a";
			newinfo[newinfo.size()-1] = '(' + sinfo[i] + ")b";
		} else if (line.token(i).isMergeInterpretation()) {
			mergecount = 0;
			for (j=i+1; j<line.getTokenCount(); j++) {
				if (line.token(j).isMergeInterpretation()) {
					mergecount++;
				} else {
					break;
				}
			}
			newinfo.resize(newtype.size() + 1);
			newinfo.back() = getMergedSpineInfo(sinfo, i, mergecount);
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			i += mergecount;
		} else if (line.token(i).isAddInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = "";
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = to_string(++mergecount);
		} else if (line.token(i).isExchangeInterpretation()) {
			if (i < line.getTokenCount() - 1) {
				if (line.token(i).isExchangeInterpretation()) {
					// exchange spine information
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i+1];
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i+1];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i];
				} else {
					cerr << "ERROR in *x calculation" << endl;
					exit(1);
				}
			} else {
				cerr << "ERROR in *x calculation" << endl;
				exit(1);
			}
		} else if (line.token(i).isTerminateInterpretation()) {
			// do nothing: the spine is terminating;
		} else if (((string)line.token(i)).substr(0, 2) == "**") {
			newtype.resize(newtype.size() + 1);
			newtype.back() = line.getTokenString(i);
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
		} else {
			// should only be null interpretation, but doesn't matter
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
		}
	}

	datatype.resize(newtype.size());
	sinfo.resize(newinfo.size());
	for (i=0; i<newtype.size(); i++) {
		datatype[i] = newtype[i];
		sinfo[i]    = newinfo[i];
	}

	return trackcount;
}



//////////////////////////////
//
// HumdrumFile::getMergedSpineInfo -- Will only simplify a two-spine
//   merge.  Should be expanded to larger spine mergers.
//

string HumdrumFile::getMergedSpineInfo(vector<string>& info, int starti,
		int extra) {
	string output;
	int len1;
	int len2;
	if (extra == 1) {
		len1 = info[starti].size();
		len2 = info[starti+1].size();
		if (len1 == len2) {
			if (info[starti].substr(0, len1-1) ==
					info[starti+1].substr(0,len2-1)) {
				output = info[starti].substr(1, len1-3);
				return output;
			}
		}
		output = info[starti] + " " + info[starti+1];
		return output;
	}
	output = info[starti];
	for (int i=0; i<extra; i++) {
		output += " " + info[starti+1+extra];
	}
	return output;
}



//////////////////////////////
//
// HumdrumFile::analyzeTokenDurations -- Calculate the duration of
//   all tokens in a file.
//

bool HumdrumFile::analyzeTokenDurations (void) {
	for (int i=0; i<size(); i++) {
		if (!lines[i]->analyzeTokenDurations()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeRhythm -- Analyze the rhythmic structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFile::analyzeRhythm(void) {
	if (!analyzeTokenDurations()) {
		return false;
	}
	vector<HumNum> durstate;
	vector<HumNum> newdurstate;
	vector<HumNum> curdur;
	HumNum linedur;
	HumNum dur;
	HumNum zero(0);

	int i, j, k;
	for (i=0; i<size(); i++) {
		if (lines[i]->isExclusiveInterpretation()) {
			// If an exclusive interpretation line, initialize the durstate.
			if (!getTokenDurations(curdur, i)) { return false; }
			durstate = curdur;
			linedur = 0;
			lines[i]->setDuration(linedur);
			continue;
		} else if (!lines[i]->isManipulator()) {
			if (lines[i]->isData()) {
				if (!getTokenDurations(curdur, i)) { return false; }
				if (curdur.size() != durstate.size()) {
					cerr << "Error on line " << (i+1) 
					     << ": spine problem" << endl;
					cerr << "Line: " << *lines[i] << endl;
					return false;
				}
				linedur = getMinDur(curdur, durstate);
				for (j=0; j<curdur.size(); j++) {
					if (curdur[j].isPositive()) {
						if (durstate[j].isPositive()) {
							cerr << "Error on line " << (i+1) 
					     		<< ": previous rhythm too long in field " << j << endl;
							cerr << "Line: " << *lines[i] << endl;
							return false;
						} else {
							durstate[j] = curdur[j];
						}
					}
				}
				lines[i]->setDuration(linedur);
				if (!decrementDurStates(durstate, linedur, i)) { return false; }
			} else {
				// Not a rhythmic line, so preserve durstate and set the
				// duration of the line to zero.
				lines[i]->setDuration(0);
			}
		} else {
			// Deal with spine manipulators.  The dur state of a spine must
			// be zero when the manipulator is *^, *v, or *-. Exclusive
			// interpretations initialze a new duration state.
			newdurstate.resize(0);
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				if (lines[i]->token(j).isSplitInterpretation()) {
					if (durstate[j].isNonZero()) {
						cerr << "Error on line " << (i+1)
						     << ": notes must end before splitting spine." << endl;
						return false;
					}
					newdurstate.push_back(durstate[j]);
					newdurstate.push_back(durstate[j]);
				} else if (lines[i]->token(j).isMergeInterpretation()) {
					int mergecount = lines[i]->token(j).getNextToken()
							->getPreviousTokenCount();
					if (mergecount <= 1) {
						cerr << "Error on line " << (i+1)
						     << ": merger is incomplete" << endl;
						cerr << "Line: " << lines[i] << endl;
						return false;
					}
					// check that all merger spine running durations
					// are zero; otherwise, there is a rhythmic error in the score.
					for (k=0; k<mergecount; k++) {
						if (durstate[j+k].isNonZero()) {
							cerr << "Error on line " << (i+1)
							     << ": merger is incomplete" << endl;
							cerr << "Line: " << lines[i] << endl;
							return false;
						}
					}
					newdurstate.push_back(durstate[j]);
					j += mergecount - 1;
				} else if (lines[i]->token(j).isExchangeInterpretation()) {
					// switch order of duration states.
					newdurstate.push_back(durstate[j+1]);
					newdurstate.push_back(durstate[j]);
					j++;
				} else if (lines[i]->token(j).isAddInterpretation()) {
					// initialize a new durstate;
					newdurstate.push_back(durstate[j]);
					newdurstate.push_back(zero);
				} else if (lines[i]->token(j).isTerminateInterpretation()) {
					// the durstate should be removed for the current spine.
				} else if (lines[i]->token(j).isExclusiveInterpretation()) {
					// The add interpretation should have added this interpetation,
					// and it should alread by in durstate
					newdurstate.push_back(durstate[j]);
				} else {
					// The manipulator should have a one-to-one mapping with
					// the next token in the spine.
					newdurstate.push_back(durstate[j]);
				}
			}
			// Store the new durstate:
			durstate = newdurstate;
		}
	}

   // Fill in the cumulative duration data:
	HumNum dursum = 0;
	for (i=0; i<lines.size(); i++) {
		lines[i]->setDurationFromStart(dursum);
		dursum += lines[i]->getDuration();
	}

	// Fill in the metrical information
	if (!analyzeMeter()) {
		return false;
	}

   return true;
}



//////////////////////////////
//
// HumdrumFile::analyzeMeter -- Store the times from the last barline
//     to the current line, as well as the time to the next barline.
//     the sum of these two will be the duration of the barline, except
//     for barlines, where the getDurationToBarline() will store the 
//     duration of the measure staring at that barline.  To get the
//     beat, you will have to figure out the current time signature.
//

bool HumdrumFile::analyzeMeter(void) {
	int i;
	HumNum sum = 0;
	for (i=0; i<size(); i++) {
		lines[i]->setDurationFromBarline(sum);
		sum += lines[i]->getDuration();
		if (lines[i]->isBarline()) {
			sum = 0;
		}
	}

	sum = 0;
	for (i=size()-1; i>=0; i--) {
		sum += lines[i]->getDuration();
		lines[i]->setDurationToBarline(sum);
		if (lines[i]->isBarline()) {
			sum = 0;
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFile::getTokenDurations --
//

bool HumdrumFile::getTokenDurations(vector<HumNum>& durs, int line) {
	durs.resize(0);
	for (int i=0; i<lines[line]->getTokenCount(); i++) {
		HumNum dur = lines[line]->token(i).getDuration();
		durs.push_back(dur);
	}
	if (!cleanDurs(durs, line)) {
		return false;
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::decrementDurStates -- Subtract the line duration from
//   the current line of running durations.  If any duration is less
//   than zero, then a rhythm error exists in the data.
//

bool HumdrumFile::decrementDurStates(vector<HumNum>& durs, HumNum linedur,
		int line) {
	if (linedur.isZero()) {
		return true;
	}
	for (int i=0; i<(int)durs.size(); i++) {
		if (!lines[line]->token(i).hasRhythm()) {
			continue;
		}
		durs[i] -= linedur;
		if (durs[i].isNegative()) {
			cerr << "Error: rhythmic error on line " << (line+1)
			     << " field index " << i << endl;
			cerr << "Duration state is: " << durs[i] << endl;
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumFile::getMinDur -- Return the smallest duration on the 
//   line.  If all durations are zero, then return zero; otherwise,
//   return the smallest positive duration.
//

HumNum HumdrumFile::getMinDur(vector<HumNum>& durs, vector<HumNum>& durstate) {
	HumNum mindur = 0;
	for (int i=0; i<(int)durs.size(); i++) {
		if (durs[i].isPositive()) {
			if (mindur.isZero()) {
				mindur = durs[i];
			} else if (mindur > durs[i]) {
				mindur = durs[i];
			}
		}
		if (durstate[i].isPositive()) {
			if (durstate[i].isZero()) {
				mindur = durstate[i];
			} else if (mindur > durstate[i]) {
				mindur = durstate[i];
			}
		}
	}
	return mindur;
}



//////////////////////////////
//
// HumdrumFile::cleanDurs -- Check if there are grace note and regular
//    notes on a line (not allowed), and convert negative durations
//    to zero (negative durations indicate undefined durations).
//

bool HumdrumFile::cleanDurs(vector<HumNum>& durs, int line) {
	bool zero 		= false;
	bool positive = false;
	for (int i=0; i<(int)durs.size(); i++) {
		if      (durs[i].isPositive()) { positive = true; }
		else if (durs[i].isZero())     { zero     = true; }
		else                           { durs[i]  = 0; }
	}
	if (zero && positive) {
		cerr << "Error on line " << (line+1) << " grace note and "
		     << " regular note cannot occur on same line." << endl;
		cerr << "Line: " << *lines[line] << endl;
		return false;
	}
	return true;
}



//////////////////////////////
//
// operator<< -- Print a HumdrumFile.
//

ostream& operator<<(ostream& out, HumdrumFile& infile) {
	for (int i=0; i<infile.size(); i++) {
		out << infile[i] << '\n';
	}
	return out;
}


//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::HumdrumLine(void) : string() { }

HumdrumLine::HumdrumLine(const string& aString) : string(aString) { }

HumdrumLine::HumdrumLine(const char* aString) : string(aString) { }



//////////////////////////////
//
// HumdrumLine::HumdrumLine --
//

HumdrumLine::~HumdrumLine() {
	// free stored HumdrumTokens
	clear();
}



//////////////////////////////
//
// HumdrumLine::clear -- remove stored tokens.
//

void HumdrumLine::clear(void) {
	for (int i=0; i<tokens.size(); i++) {
		delete tokens[i];
		tokens[i] = NULL;
	}
}



//////////////////////////////
//
// HumdrumLine::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumLine::equalChar(int index, char ch) const {
	if (size() <= index) {
		return false;
	}
	if (index < 0) {
		return false;
	}
	if (((string)(*this))[index] == ch) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumLine::isComment -- Returns true if the first character
//   in the string is '!'. Could be local, global, or a reference record.
//

bool HumdrumLine::isComment(void) const {
	return equalChar(0, '!');
}

bool HumdrumLine::isCommentLocal(void) const {
	return equalChar(0, '!') && !equalChar(1, '!');

}

bool HumdrumLine::isCommentGlobal(void) const {
	return equalChar(0, '!') && equalChar(1, '!');
}


//////////////////////////////
//
// HumdrumLine::isExclusive -- Returns true if the first two characters
//     are "**".
//

bool HumdrumLine::isExclusive(void) const {
	return equalChar(1, '*') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isTerminator -- Returns true if first two characters
//    are "*-" and the next character is undefined or a tab character.
//

bool HumdrumLine::isTerminator(void) const {
	return equalChar(1, '!') && equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isInterp -- Returns true if starts with '*' character.
//

bool HumdrumLine::isInterp(void) const {
	return equalChar(0, '*');
}



//////////////////////////////
//
// HumdrumLine::isBarline -- Returns true if starts with '=' character.
//

bool HumdrumLine::isBarline(void) const {
	return equalChar(0, '=');
}



//////////////////////////////
//
// HumdrumLine::isData -- Returns true if data (but not measure).
//

bool HumdrumLine::isData(void) const {
	if (isComment() || isInterp() || isBarline() || isEmpty()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::setLineIndex --
//

void HumdrumLine::setLineIndex(int index) {
   lineindex = index;
}



//////////////////////////////
//
// HumdrumLine::getLineIndex --
//

int HumdrumLine::getLineIndex(void) const {
	return lineindex;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

int HumdrumLine::getLineNumber(void) const {
	return lineindex + 1;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

HumNum HumdrumLine::getDuration(void) const { 
	return duration;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromStart --
//

void HumdrumLine::setDurationFromStart(HumNum dur) {
	 durationFromStart = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromStart --
//

HumNum HumdrumLine::getDurationFromStart(void) const {
	return durationFromStart;
}



//////////////////////////////
//
// HumdrumLine::getDurationFromBarline --
//

HumNum HumdrumLine::getDurationFromBarline(void) const { 
	return durationFromBarline;
}



//////////////////////////////
//
// HumdrumLine::setDurationFromBarline -- Time from the previous
//    barline to the current line.
//

void HumdrumLine::setDurationFromBarline(HumNum dur) { 
	durationFromBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getDurationToBarline -- Time from the starting of the
//     current note to the next barline.
//

HumNum HumdrumLine::getDurationToBarline(void) const { 
	return durationToBarline;
}



//////////////////////////////
//
// HumdrumLine::getBeat -- return the beat number for the data on the
//     current line given the input **recip representation for the duration
//     of a beat.
//  Default value: beatrecip = "4".
//

HumNum HumdrumLine::getBeat(string beatrecip) const {
	HumNum beatdur = Convert::recipToDuration(beatrecip);
	if (beatdur.isZero()) {
		return beatdur;
	}
	HumNum beat = (getDurationFromBarline() / beatdur) + 1;
	return beat;
}



//////////////////////////////
//
// HumdrumLine::setDurationToBarline --
//

void HumdrumLine::setDurationToBarline(HumNum dur) { 
	durationToBarline = dur;
}



//////////////////////////////
//
// HumdrumLine::getLineNumber --
//

void HumdrumLine::setDuration(HumNum aDur) { 
	if (aDur.isNonNegative()) {
		duration = aDur;
	} else {
		duration = 0;
	}
}



//////////////////////////////
//
// HumdrumLine::hasSpines --
//

bool HumdrumLine::hasSpines(void) const {
	if (isEmpty() || isCommentGlobal()) {
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// HumdrumLine::isManipulator --
//

bool HumdrumLine::isManipulator(void) const {
	for (int i=0; i<tokens.size(); i++) {
		if (tokens[i]->isManipulator()) {
			return true;
		}
	}
	return false;
}



//////////////////////////////
//
// HumdrumLine::isEmpty -- Returns true if no characters on line.
//

bool HumdrumLine::isEmpty(void) const {
	if (size() == 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumLine::getTokenCount --
//

int HumdrumLine::getTokenCount(void) const {
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::token --
//

HumdrumToken& HumdrumLine::token(int index) {
	return *tokens[index];
}



//////////////////////////////
//
// HumdrumLine::getTokenString --
//

string HumdrumLine::getTokenString(int index) const {
	return (string(*tokens[index]));
}


//////////////////////////////
//
// HumdrumLine::createTokensFromLine --
//

int HumdrumLine::createTokensFromLine(void) {
	tokens.resize(0);
	HumdrumToken* token = new HumdrumToken();
	char ch;
	for (int i=0; i<size(); i++) {
		ch = getChar(i);
		if (ch == '\t') {
			tokens.push_back(token);
			token = new HumdrumToken();
		} else {
			*token += ch;
		}
	}
	tokens.push_back(token);
	return tokens.size();
}



//////////////////////////////
//
// HumdrumLine::createLineFromTokens --
//

void HumdrumLine::createLineFromTokens(void) {
	string& iline = *this;
	iline.resize(0);
	for (int i=0; i<tokens.size(); i++) {
		iline += (string)(*tokens[i]);
		if (i < tokens.size() - 1) {
			iline += '\t';
		}
	}
}



//////////////////////////////
//
// HumdrumLine::getTokens -- Returns an array of tokens pointers for a 
//   Humdrum line.  This function should not be called on global comments,
//   reference records (which are a sub-cateogry of global comments).  This
//   is because a line's type may contain tabs which are not representing
//   token separators.  Empty lines are ok to input: the output token
//   list will contain one empty string.
//

void HumdrumLine::getTokens(vector<HumdrumToken*>& list) {
	if (tokens.size() == 0) {
		createTokensFromLine();
	}
	list = tokens;
}



//////////////////////////////
//
// HumdrumLine::getChar -- Return character at given index in string, or
//    null if out of range.
//

char HumdrumLine::getChar(int index) const {
	if (index < 0) {
		return '\0';
	}
	if (index >= size()) {
		return '\0';
	}
	return (((string)(*this))[index]);
}



//////////////////////////////
//
// HumdrumLine::printSpineInfo -- Print the spine state information of
//    each token in a file.  Useful for debugging.  The spine info
//    is the track number, such as "1".  When the track splits into
//    subtracks, then there will be two subtracks: "(1)a" and "(1)b".
//    If the second of those subtracks splits again, then its subtracks
//    will be "((1)b)a" and "((1)b)b". If two different tracks merge, such
//    as "1" and "(2)a", then the spine info will be "1 (2)a".
//
//    default value: out = cout
//

ostream& HumdrumLine::printSpineInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getSpineInfo();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printDataTypeInfo -- Print the datatype of each token in
//     the file.  Useful for debugging.  The datatype prefix "**" is removed;
//     otherwise, it is given when you call HumdrumToken::getDataType().
//
//     default value: out = cout
//

ostream& HumdrumLine::printDataTypeInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getDataType().substr(2, string::npos);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::analyzeTokenDurations -- Calculate the duration of
//    all tokens on a line.
//

bool HumdrumLine::analyzeTokenDurations(void) {
	if (!hasSpines()) {
		return true;
	}
	for (int i=0; i<tokens.size(); i++) {
		if (!tokens[i]->analyzeDuration()) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::analyzeTracks --
//

bool HumdrumLine::analyzeTracks(void) {
	if (!hasSpines()) {
		return true;
	}

	string info;
	int track;
	int maxtrack = 0;
	int i, j, k;

	for (i=0; i<tokens.size(); i++) {
		info = tokens[i]->getSpineInfo();
		track = 0;
		for (j=0; j<info.size(); j++) {
			if (!isdigit(info[j])) {
				continue;
			}
			track = info[j] - '0';
			for (k=j+1; k<info.size(); k++) {
				if (isdigit(info[k])) {
					track = track * 10 + (info[k] - '0');
				} else {
					break;
				}
			}
			break;
		}
		if (maxtrack < track) {
			maxtrack = track;
		}
		tokens[i]->setTrack(track);
	}

	int subtrack;
	vector<int> subtracks;
	vector<int> cursub;

	subtracks.resize(maxtrack+1);
	cursub.resize(maxtrack+1);
	fill(subtracks.begin(), subtracks.end(), 0);
	fill(cursub.begin(), cursub.end(), 0);

	for (i=0; i<tokens.size(); i++) {
		subtracks[tokens[i]->getTrack()]++;
	}
	for (i=0; i<tokens.size(); i++) {
		subtrack = subtracks[tokens[i]->getTrack()];
		if (subtrack > 1) {
			tokens[i]->setSubtrack(++cursub[tokens[i]->getTrack()]);
		} else {
			tokens[i]->setSubtrack(0);
		}
	}
	return true;
}



//////////////////////////////
//
// HumdrumLine::printDurationInfo -- Print the analyzed duration of each
//     token in a file (for debugging).  If a token has an undefined
//     duration, then its duration is -1.  If a token represents
//     a grace note, then its duration is 0 (regardless of whether it
//     includes a visual duration).
//     default value: out = cout
//

ostream& HumdrumLine::printDurationInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			tokens[i]->getDuration().printMixedFraction(out);
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// HumdrumLine::printTrackInfo -- Print the analyzed track information.
//     The first (left-most) spine in a Humdrum file is track 1, the
//     next is track 2, etc.  The track value is shared by all subspines,
//     so there may be duplicate track numbers on a line if the spine
//     has split.  When the spine splits, a subtrack number is given
//     after a "." character in the printed output from this function.
//     Subtrack==0 means that there is only one subtrack.
//     Examples:
//         "1"  == Track 1, subtrack 1 (and there are no more subtracks)
//	        "1.1" == Track 1, subtrack 1 (and there are more subtracks)
//	        "1.2" == Track 1, subtrack 2 (and there may be more subtracks)
//	        "1.10" == Track 1, subtrack 10 (and there may be subtracks)
//     Each starting exclusive interpretation is assigned to a unique
//     track number.  When a *+ manipulator is given, the new exclusive
//     interpretation on the next line is give the next higher track
//     number.
//
//     default value: out = cout
//

ostream& HumdrumLine::printTrackInfo(ostream& out) {
	if (isManipulator()) {
		out << *this;
	} else {
		for (int i=0; i<tokens.size(); i++) {
			out << tokens[i]->getTrackString();
			if (i < tokens.size() - 1) {
				out << '\t';
			}
		}
	}
	return out;
}



//////////////////////////////
//
// operator<< -- Print a HumdrumLine.
//

ostream& operator<<(ostream& out, HumdrumLine& line) {
	out << (string)line;
	return out;
}


// spine mainipulators:
#define SPLIT_TOKEN       "*^"
#define MERGE_TOKEN       "*v"
#define EXCHANGE_TOKEN    "*x"
#define TERMINATE_TOKEN   "*-"
#define ADD_TOKEN         "*+"
// Also exclusive interpretations which start "**" followed by the data type.

// other special tokens:
#define NULL_DATA            "."
#define NULL_INTERPRETATION  "*"
#define NULL_COMMENT_LOCAL   "!"
#define NULL_COMMENT_GLOBAL  "!!"



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::HumdrumToken(void) : string() {
	// do nothing
}

HumdrumToken::HumdrumToken(const string& aString) : string(aString) {
	// do nothing
}

HumdrumToken::HumdrumToken(const char* aString) : string(aString) {
	// do nothing
}



//////////////////////////////
//
// HumdrumToken::equalChar -- return true if the character at the given
//     index is the given char.
//

bool HumdrumToken::equalChar(int index, char ch) const {
	if (size() <= index) {
		return false;
	}
	if (index < 0) {
		return false;
	}
	if (((string)(*this))[index] == ch) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// HumdrumToken::HumdrumToken --
//

HumdrumToken::~HumdrumToken() {
	// do nothing
}



//////////////////////////////
//
// HumdrumToken::setDataType -- Set the exclusive interpretation type.
//

void HumdrumToken::setDataType(const string& datatype) {
	address.setDataType(datatype);
}



//////////////////////////////
//
// HumdrumToken::getDataType -- Get the exclusive interpretation type.
//

string HumdrumToken::getDataType(void) const {
	return address.getDataType();
}



//////////////////////////////
//
// HumdrumToken::setSpineInfo -- Set the spine manipulation history string.
//

void HumdrumToken::setSpineInfo(const string& spineinfo) {
	address.setSpineInfo(spineinfo);
}



//////////////////////////////
//
// HumdrumToken::getSpineInfo --
//

string HumdrumToken::getSpineInfo(void) const {
	return address.getSpineInfo();
}



//////////////////////////////
//
// HumdrumToken::getLineIndex --
//

int HumdrumToken::getLineIndex(void) const { 
	return address.getLineIndex();
}



//////////////////////////////
//
// HumdrumToken::getLineNumber --
//

int HumdrumToken::getLineNumber(void) const {
	return address.getLineNumber();
}



//////////////////////////////
//
// HumdrumToken::setLineAddress --
//

void HumdrumToken::setLineAddress(int aLineIndex, int aFieldIndex) {
	setLineIndex(aLineIndex);
	setFieldIndex(aFieldIndex);
}



//////////////////////////////
//
// HumdrumToken::setLineIndex --
//

void HumdrumToken::setLineIndex(int index) {
	address.setLineIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setFieldIndex --
//

void HumdrumToken::setFieldIndex(int index) {
	address.setFieldIndex(index);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track (similar to a staff in MEI).
//

void HumdrumToken::setTrack(int aTrack) {
	address.setTrack(aTrack);
}



//////////////////////////////
//
// HumdrumToken::setTrack -- Set the track and subtrack (similar to a
//     staff and layer in MEI).
//

void HumdrumToken::setTrack(int aTrack, int aSubtrack) {
	setTrack(aTrack);
	setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::getTrack -- Get the track (similar to a staff in MEI).
//

int HumdrumToken::getTrack(void) const {
	return address.getTrack();
}



//////////////////////////////
//
// HumdrumToken::setSubtrack -- Set the subtrack (similar to a layer
//    in MEI).
//

void HumdrumToken::setSubtrack(int aSubtrack) {
	address.setSubtrack(aSubtrack);
}



//////////////////////////////
//
// HumdrumToken::setPreviousToken --
//

void HumdrumToken::setPreviousToken(HumdrumToken* aToken) {
	previousTokens.resize(1);
	previousTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::setNextToken --
//

void HumdrumToken::setNextToken(HumdrumToken* aToken) {
	nextTokens.resize(1);
	nextTokens[0] = aToken;
}



//////////////////////////////
//
// HumdrumToken::getNextToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getNextToken(int index) const {
	if ((index >= 0) && (index < (int)nextTokens.size())) {
		return nextTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::getNextTokens --
//

vector<HumdrumToken*> HumdrumToken::getNextTokens(void) const {
	return nextTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokens --
//

vector<HumdrumToken*> HumdrumToken::getPreviousTokens(void) const {
	return previousTokens;
}



//////////////////////////////
//
// HumdrumToken::getPreviousToken --
//    default value: index = 0
//

HumdrumToken* HumdrumToken::getPreviousToken(int index) const {
	if ((index >= 0) && (index < (int)previousTokens.size())) {
		return previousTokens[index];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumToken::analyzeDuration -- Currently reads the duration of
//   **kern and **recip data.  Add more data types here.
//

bool HumdrumToken::analyzeDuration(void) {
	if ((*this) == NULL_DATA) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'!')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'*')) {
		duration.setValue(-1);
		return true;
	}
	if (equalChar(0 ,'=')) {
		duration.setValue(-1);
		return true;
	}
	string dtype = getDataType();
	if ((dtype == "**kern") || (dtype == "**recip")) {
		duration = Convert::recipToDuration((string)(*this));
	} else {
		duration.setValue(-1);
	}
	return true;
}



///////////////////////////////
//
// HumdrumToken::isManipulator -- returns true if token is one of:
//    SPLIT_TOKEN     = "*^"  == spine splitter
//    MERGE_TOKEN     = "*v"  == spine merger
//    EXCHANGE_TOKEN  = "*x"  == spine exchanger
//    ADD_TOKEN       = "*+"  == spine adder
//    TERMINATE_TOKEN = "*-"  == spine terminator
//    **...  == exclusive interpretation
//

bool HumdrumToken::isManipulator(void) const {
	if (isSplitInterpretation())     { return true; }
	if (isMergeInterpretation())     { return true; }
	if (isExchangeInterpretation())  { return true; }
	if (isAddInterpretation())       { return true; }
	if (isTerminateInterpretation()) { return true; }
	if (isExclusiveInterpretation()) { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getDuration --
//

HumNum HumdrumToken::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// HumdrumToken::hasRhythm --
//

bool HumdrumToken::hasRhythm(void) const {
	string type = getDataType();
	if (type == "**kern") {
		return true;
	}
	if (type == "**recip") {
		return true;
	}
	return false;
}



//////////////////////////////
//
// HumdrumToken::isExclusive -- Returns true if first two characters
//     are "**".
//

bool HumdrumToken::isExclusive(void) const {
	const string& tok = (string)(*this);
	return tok.substr(0, 2) == "**";
}



//////////////////////////////
//
// HumdrumToken::isSplitInterpretation --
// 

bool HumdrumToken::isSplitInterpretation(void) const {
	return ((string)(*this)) == SPLIT_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isMergeInterpretation --
// 

bool HumdrumToken::isMergeInterpretation(void) const { 
	return ((string)(*this)) == MERGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isExchangeInterpretation --
// 

bool HumdrumToken::isExchangeInterpretation(void) const { 
	return ((string)(*this)) == EXCHANGE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isTerminateInterpretation --
// 

bool HumdrumToken::isTerminateInterpretation(void) const { 
	return ((string)(*this)) == TERMINATE_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isAddInterpretation --
// 

bool HumdrumToken::isAddInterpretation(void) const { 
	return ((string)(*this)) == ADD_TOKEN;
}



//////////////////////////////
//
// HumdrumToken::isNull -- Returns true if the token is a null token,
//   either for data, comments, or interpretations.  Does not consider
//   null global comments since they are not part of the spine structure.
//

bool HumdrumToken::isNull(void) const {
	const string& tok = (string)(*this);
   if (tok == NULL_DATA)           { return true; }
   if (tok == NULL_INTERPRETATION) { return true; }
   if (tok == NULL_COMMENT_LOCAL)  { return true; }
	return false;
}



//////////////////////////////
//
// HumdrumToken::getSubtrack -- Get the subtrack (similar to a layer
//    in MEI).
//

int HumdrumToken::getSubtrack(void) const {
	return address.getSubtrack();
}



//////////////////////////////
//
// HumdrumToken::getTrackString -- Get "track.subtrack" as a string.
//

string HumdrumToken::getTrackString(void) const {
	return address.getTrackString();
}



/////////////////////////////
//
// HumdrumToken::getSubtokenCount --
//   default value: separator = " "
//

int HumdrumToken::getSubtokenCount(const string& separator) const {
	int count = 0;
	string::size_type start = 0;
	while ((start = string::find(separator, start)) != string::npos) {
		count++;
		start += separator.size();
	}
	return count;
}



/////////////////////////////
//
// HumdrumToken::getSubtoken --
//   default value: separator = " "
//

string HumdrumToken::getSubtoken(int index, const string& separator) const {
	if (index < 0) {
		return "";
	}
	int count = 0;
	int start = 0;
	int end   = 0;
	while ((end = string::find(separator, start)) != string::npos) {
		count++;
		if (count == index) {
			return string::substr(start, end-start);
		}
		start += separator.size();
	}
   if (count == index) {
		return string::substr(start, string::size()-start);
	}
	return "";
}



//////////////////////////////
//
// HumdrumToken::makeForwardLink --
//

void HumdrumToken::makeForwardLink(HumdrumToken& nextToken) {
	nextTokens.push_back(&nextToken);
	nextToken.previousTokens.push_back(this);
}



//////////////////////////////
//
// HumdrumToken::makeBackwarddLink --
//

void HumdrumToken::makeBackwardLink(HumdrumToken& previousToken) {
	previousTokens.push_back(&previousToken);
	previousToken.nextTokens.push_back(this);
}


//////////////////////////////
//
// HumdrumToken::getNextTokenCount -- Return the number of tokens in the
//   spine/subspine which follow this token.  Typically this will be 1,
//   but will be zero for a terminator interpretation (*-), and will be 
//   2 for a split interpretation (*^).
//

int HumdrumToken::getNextTokenCount(void) const {
	return nextTokens.size();
}



//////////////////////////////
//
// HumdrumToken::getPreviousTokenCount -- Return the number of tokens
//   in the spine/subspine which precede this token.  Typically this will 
//   be 1, but will be zero for an exclusive interpretation (starting with 
//   "**"), and will be greater than one for a token which follows a 
//   spine merger (using *v interpretations).
//

int HumdrumToken::getPreviousTokenCount(void) const {
	return previousTokens.size();
}



//////////////////////////////
//
// Convert::recipToDuration --
//     default value: scale = 4 (duration in terms of quarter notes)
//     default value: separator = " "
//

HumNum Convert::recipToDuration(const string& recip, HumNum scale, 
		string separator) {
	size_t loc;
	loc = recip.find(separator);
	string subtok;
	if (loc != string::npos) {
		subtok = recip.substr(0, loc);
	} else {
		subtok = recip;
	}

   loc = recip.find('q');
	if (loc != string::npos) {
		// grace note, ignore printed rhythm
		HumNum zero(0);
		return zero;
	}

	int dotcount = 0;
	int i;
	int numi = -1;
	for (i=0; i<subtok.size(); i++) {
		if (subtok[i] == '.') {
			dotcount++;
		}
		if ((numi < 0) && isdigit(subtok[i])) {
			numi = i;
		}
	}
	loc = subtok.find("%");
	int numerator = 1;
	int denominator = 1;
	HumNum output;
	if (loc != string::npos) {
		// reciporical rhythm
		numerator = 1;
		denominator = subtok[numi++] - '0';
		while ((numi < subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		if ((loc + 1 < subtok.size()) && isdigit(subtok[loc+1])) {
			int xi = loc + 1;
			numerator = subtok[xi++] - '0';
			while ((xi < subtok.size()) && isdigit(subtok[xi])) {
				numerator = numerator * 10 + (subtok[xi++] - '0');
			}
		}
		output.setValue(numerator, denominator);
	} else if (numi < 0) {
		// no rhythm found
		HumNum zero(0);
		return zero;
	} else if (subtok[numi] == '0') {
		// 0-symbol
		int zerocount = 1;
		for (i=numi+1; i<subtok.size(); i++) {
			if (subtok[i] == '0') {
				zerocount++;
			} else {
				break;
			}
			numerator = 4 * (int)pow(2, zerocount);
			output.setValue(numerator, 1);
		}
   } else {
		// plain rhythm
		denominator = subtok[numi++] - '0';
		while ((numi < subtok.size()) && isdigit(subtok[numi])) {
			denominator = denominator * 10 + (subtok[numi++] - '0');
		}
		output.setValue(1, denominator);
	}

	if (dotcount <= 0) {
		return output * scale;
	}

	int bot = (int)pow(2.0, dotcount);
	int top = (int)pow(2.0, dotcount + 1) - 1;
	HumNum factor(top, bot);
	return output * factor * scale;
}

