//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:32 PDT 2015
// Filename:      Convert-math.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-math.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
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



//////////////////////////////
//
// Convert::isNaN -- needed due to compiler differences.
//

bool Convert::isNaN(double value) {
	union { uint64_t u; double f; } ieee754;
	ieee754.f = value;
	return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
           ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
}



//////////////////////////////
//
// Tool_transpose::pearsonCorrelation --
//

double Convert::pearsonCorrelation(vector<double> x, vector<double> y) {
	double sumx  = 0.0;
	double sumy  = 0.0;
	double sumco = 0.0;
	double meanx = x[0];
	double meany = y[0];
	double sweep;
	double deltax;
	double deltay;

	int size = (int)x.size();
	if ((int)y.size() < size) {
		size = (int)y.size();
	}

	for (int i=2; i<=size; i++) {
		sweep = (i-1.0) / i;
		deltax = x[i-1] - meanx;
		deltay = y[i-1] - meany;
		sumx  += deltax * deltax * sweep;
		sumy  += deltay * deltay * sweep;
		sumco += deltax * deltay * sweep;
		meanx += deltax / i;
		meany += deltay / i;
	}

	double popsdx = sqrt(sumx / size);
	double popsdy = sqrt(sumy / size);
	double covxy  = sumco / size;

	return covxy / (popsdx * popsdy);
}



//////////////////////////////
//
// Convert::romanNumeralToInteger -- Convert a roman numeral into an integer.
//

int Convert::romanNumeralToInteger(const string& roman) {
	int rdigit;
	int sum = 0;
	char previous='_';
	for (int i=(int)roman.length()-1; i>=0; i--) {
		switch (roman[i]) {
			case 'I': case 'i': rdigit =    1; break;
			case 'V': case 'v': rdigit =    5; break;
			case 'X': case 'x': rdigit =   10; break;
			case 'L': case 'l': rdigit =   50; break;
			case 'C': case 'c': rdigit =  100; break;
			case 'D': case 'd': rdigit =  500; break;
			case 'M': case 'm': rdigit = 1000; break;
			default:  rdigit =   -1;
		}
		if (rdigit < 0) {
			continue;
		} else if (rdigit < sum && (roman[i] != previous)) {
			sum -= rdigit;
		} else {
			sum += rdigit;
		}
		previous = roman[i];
	}

	return sum;
}



// END_MERGE

} // end namespace hum



