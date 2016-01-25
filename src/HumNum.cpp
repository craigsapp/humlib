//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      Convert.h
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumNum.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Rational number class for duration processing in
//                HumdrumFiles.  The HumNum class manages a fractional
//                number formed from two ints.  The fractional
//                number will be kept in reduced for, such as
//                the number 3/6 which can be simplified to 1/2.
//

#include "HumNum.h"

namespace humlib {

// START_MERGE

//////////////////////////////
//
// HumNum::HumNum -- HumNum Constructor.  Set the default value
//   of the number to zero, or the given number if specified.
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


HumNum::HumNum(const string& ratstring) {
	setValue(ratstring);
}


HumNum::HumNum(const char* ratstring) {
	setValue(ratstring);
}


HumNum::HumNum(const HumNum& rat) {
	*this = rat;
}



//////////////////////////////
//
// HumNum::~HumNum -- HumNum deconstructor.
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
// HumNum::isNonZero -- Returns true if value is not zero.
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
// HumNum::getFloat -- Returns the floating-point equivalent of the
//     rational number.
//

double HumNum::getFloat(void) const {
	return (double)top/(double)bot;
}



//////////////////////////////
//
// HumNum::getInteger -- Returns the integral part of the fraction.
//    Default value: round = 0.0
//    Optional parameter is a rounding factor.
//    Examples:
//       8/5 | round=0.0 ==  1
//      -8/5 | round=0.0 == -1
//       8/5 | round=0.5 ==  1
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
// HumNum::getNumerator -- Returns the top integer in the fraction.
//

int HumNum::getNumerator(void) const {
	return top;
}



//////////////////////////////
//
// HumNum::getDenominator -- Returns the bottom integer in the fraction.
//

int HumNum::getDenominator(void) const {
	return bot;
}



//////////////////////////////
//
// HumNum::getRemainder -- Returns the non-integer fractional part of the value.
//

HumNum HumNum::getRemainder(void) const {
	return (*this) - toInteger();
}



//////////////////////////////
//
// HumNum::setValue -- Set the number to the given integer.
//    For the two-parameter version, set the top and bottom
//    values for the number, reducing if necessary.  For the
//    string version, parse an integer or fraction from the
//    string and reduce if necessary.
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


void HumNum::setValue(const string& ratstring) {
	int buffer[2];
	buffer[0] = 0;
	buffer[1] = 0;
	int slash = 0;
	for (int i=0; i<ratstring.size(); i++) {
		if (ratstring[i] == '/') {
			slash = 1;
			continue;
		}
		if (!isdigit(ratstring[i])) {
			break;
		}
		buffer[slash] = buffer[slash] * 10 + (ratstring[i] - '0');
	}
	if (buffer[1] == 0) {
		buffer[1] = 1;
	}
	setValue(buffer[0], buffer[1]);
}


void HumNum::setValue(const char* ratstring) {
	string realstring = ratstring;
	setValue(realstring);
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
// HumNum::reduce -- simplify the fraction.  For example, 4/24 will
//    reduce to 1/6 since a factor of 4 is common to the numerator
//    and denominator.
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
// HumNum::gcdIterative -- Returns the greatest common divisor of two
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
// HumNum::gcdRecursive -- Returns the greatest common divisor of two
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
// HumNum::isNaN -- Returns true if the numerator and denominator
//     are both zero.
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
// HumNum::isInteger -- Returns true if number is an integer.
//

bool HumNum::isInteger(void) const {
	return isFinite() && (bot == 1);
}



//////////////////////////////
//
// HumNum::operator+ -- Addition operator which adds HumNum
//    to another HumNum or with a integers.
//

HumNum HumNum::operator+(const HumNum& value) const {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2 + a2 * b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator+(int value) const {
	HumNum output(value * bot + top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Subtraction operator to subtract
//     HumNums from each other and to subtrack integers from
//     HumNums.
//

HumNum HumNum::operator-(const HumNum& value) const {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2 - a2*b1;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator-(int value) const {
	HumNum output(top - value * bot, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator- -- Unary negation operator to generate
//   the negative version of a HumNum.
//

HumNum HumNum::operator-(void) const {
	HumNum output(-top, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator* -- Multiplication operator to multiply
//   two HumNums together or a HumNum and an integer.
//

HumNum HumNum::operator*(const HumNum& value) const {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*a2;
	int bo = b1*b2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator*(int value) const {
	HumNum output(top * value, bot);
	return output;
}



//////////////////////////////
//
// HumNum::operator/ -- Division operator to divide two
//     HumNums together or divide a HumNum by an integer.
//

HumNum HumNum::operator/(const HumNum& value) const {
	int a1  = getNumerator();
	int b1  = getDenominator();
	int a2  = value.getNumerator();
	int b2  = value.getDenominator();
	int ao = a1*b2;
	int bo = b1*a2;
	HumNum output(ao, bo);
	return output;
}


HumNum HumNum::operator/(int value) const {
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
// HumNum::operator= -- Assign the contents of a HumNum
//    from another HumNum.
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
// HumNum::operator+= -- Add a HumNum or integer to a HumNum.
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
// HumNum::operator-= -- Subtract a HumNum or an integer from
//    a HumNum.
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
// HumNum::operator*= -- Multiply a HumNum by a HumNum or integer.
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
// HumNum::operator/= -- Divide a HumNum by a HumNum or integer.
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
// HumNum::operator< -- Less-than equality for a HumNum and
//   a HumNum, integer, or float.
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
// HumNum::operator<= -- Less-than-or-equal equality for a
//     HumNum with a HumNum, integer or float.
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
// HumNum::operator> -- Greater-than equality for a HumNum
//     compared to a HumNum, integer, or float.
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
// HumNum::operator>= -- Greater-than-or-equal equality
//    comparison for a HumNum to another HumNum, integer, or float.
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
// HumNum::operator== -- Equality test for HumNums compared to
//   another HumNum, integer or float.
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
// HumNum::operator!= -- Inequality test for HumNums compared
//   to other HumNums, integers or floats.
//

bool HumNum::operator!=(const HumNum& value) const {
	if (this == &value) {
		return false;
	}
	return getFloat() != value.getFloat();
}


bool HumNum::operator!=(int value) const {
	return getFloat() != value;
}


bool HumNum::operator!=(double value) const {
	return getFloat() != value;
}



//////////////////////////////
//
// HumNum::printFraction -- Print HumNum as a fraction,
//    such as 3/2.  If the HumNum is an integer, then do
//    not print the denominator.
//      default parameter: out = cout;
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
// HumNum::printMixedFraction -- Print as an integer plus fractional
//     remainder.  If absolute value is less than one, will only
//     print the fraction.  The second parameter is the output stream
//     for printing, and the third parameter is a separation string
//     between the integer and remainder fraction.
//        default parameter: out = cout;
//        default parameter: separator = "_"
//

ostream& HumNum::printMixedFraction(ostream& out,
		string separator) const {
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
// HumNum::printList -- Print as a list of two numbers, such as
//    "(1, 2)" for 1/2.
// default value: out = cout;
//

ostream& HumNum::printList(ostream& out) const {
	out << '(' << top << ", " << bot << ')';
	return out;
}



//////////////////////////////
//
// operator<< -- Default printing behavior for HumNums.
//

ostream& operator<<(ostream& out, const HumNum& number) {
	number.printFraction(out);
	return out;
}


// END_MERGE

} // end namespace std;



