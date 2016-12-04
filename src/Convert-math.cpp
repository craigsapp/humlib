//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:32 PDT 2015
// Filename:      Convert-math.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-math.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert between various data representations.
//

#include "Convert.h"
#include <cmath>

namespace hum {

// START_MERGE



//////////////////////////////
//
// Convert::getLcm -- Return the Least Common Multiple of a list of numbers.
//

int Convert::getLcm(const vector<int>& numbers) {
	if (numbers.size() == 0) {
		return 1;
	}
	int output = numbers[0];
	for (int i=1; i<(int)numbers.size(); i++) {
		output = (output * numbers[i]) / getGcd(output, numbers[i]);
	}
	return output;
}



//////////////////////////////
//
// Convert::getGcd -- Return the Greatest Common Divisor of two numbers.
//

int Convert::getGcd(int a, int b) {
	if (b == 0) {
		return a;
	}
	int c = a % b;
	a = b;
	int output = getGcd(a, c);
	return output;
}



//////////////////////////////
//
// Convert::primeFactors -- Return a list of prime factors of a number.
//

void Convert::primeFactors(vector<int>& output, int n) {
	output.clear();
	while (n%2 == 0) {
		output.push_back(2);
		n = n >> 1;
	}
	for (int i=3; i <= sqrt(n); i += 2) {
		while (n%i == 0) {
			output.push_back(i);
			n = n/i;
		}
	}
	if (n > 2) {
		output.push_back(n);
	}
}



//////////////////////////////
//
// Convert::nearIntQuantize -- avoid small deviations from integer values.
//    devault value: delta = 0.00001
//

double Convert::nearIntQuantize(double value, double delta) {
	if ((value + delta) - int(value+delta)  < delta*2) {
		value = (int)(value+delta);
	}
	return value;
}



//////////////////////////////
//
// Convert::significantDigits --
//

double Convert::significantDigits(double value, int digits) {
	double scale = pow(10, digits);
	return (int(value * scale + 0.5))/scale;
}


// END_MERGE

} // end namespace hum



