//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Wed Aug 19 00:06:32 PDT 2015
// Filename:      Convert-math.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-math.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert between various data representations.
//

#include "Convert.h"

#include <cmath>
#include <cstdint>

using namespace std;

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
// Convert::isPowerOfTwo --
//

bool Convert::isPowerOfTwo(int value) {
	if (value < 0) {
		return (-value & (-value - 1)) == 0;
	} else if (value > 0) {
		return (value & (value - 1)) == 0;
	} else {
		return false;
	}
}



//////////////////////////////
//
// Convert::pearsonCorrelation --
//

double Convert::pearsonCorrelation(const vector<double>& x, const vector<double>& y) {
	double sumx  = 0.0;
	double sumy  = 0.0;
	double sumco = 0.0;
	double meanx = x[0];
	double meany = y[0];

	int size = (int)x.size();
	if ((int)y.size() < size) {
		size = (int)y.size();
	}

	for (int i=2; i<=size; i++) {
		double sweep = (i-1.0) / i;
		double deltax = x[i-1] - meanx;
		double deltay = y[i-1] - meany;
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
// Convert::standardDeviation --
//

double Convert::standardDeviation(const vector<double>& x) {
	double sum = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		sum += x[i];
	}
	double mean = sum / x.size();
	double variance = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		variance += pow(x[i] - mean, 2);
	}
	variance = variance / x.size();
	return sqrt(variance);
}


double Convert::standardDeviation(const vector<int>& x) {
	double sum = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		sum += x[i];
	}
	double mean = sum / x.size();
	double variance = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		variance += pow(x[i] - mean, 2);
	}
	variance = variance / x.size();
	return sqrt(variance);
}



//////////////////////////////
//
// Convert::standardDeviationSample -- Similar to Convert::standardDeviation,
//     but divide by (size-1) rather than (size).
//

double Convert::standardDeviationSample(const vector<double>& x) {
	double sum = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		sum += x[i];
	}
	double mean = sum / x.size();
	double variance = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		variance += pow(x[i] - mean, 2);
	}
	variance = variance / ((int)x.size()-1);
	return sqrt(variance);
}


double Convert::standardDeviationSample(const vector<int>& x) {
	double sum = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		sum += x[i];
	}
	double mean = sum / x.size();
	double variance = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		variance += pow(x[i] - mean, 2);
	}
	variance = variance / ((int)x.size()-1);
	return sqrt(variance);
}



//////////////////////////////
//
// Convert::mean -- calculate the mean (average) of a list of numbers.
//

double Convert::mean(const std::vector<double>& x) {
	double output = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		output += x[i];
	}
	return output / (int)x.size();
}


double Convert::mean(const std::vector<int>& x) {
	double output = 0.0;
	for (int i=0; i<(int)x.size(); i++) {
		output += x[i];
	}
	return output / (int)x.size();
}



//////////////////////////////
//
// Convert::coefficientOfVariationPopulation -- Standard deviation divided by
//    mean.  From: Patel, Iversen & Rosenberg (2006): Comparing the
//    rhythm and melody of speech and music: The case of British
//    English and French.  JASA 119(5), May 2006, pp. 3034-3047.
//

double Convert::coefficientOfVariationPopulation(const std::vector<double>& x) {
	double sd = Convert::standardDeviation(x);
	double mean = Convert::mean(x);
	return sd / mean;
}



//////////////////////////////
//
// Convert::coefficientOfVariationSample -- Standard deviation divided by
//    mean.  From: Patel, Iversen & Rosenberg (2006): Comparing the
//    rhythm and melody of speech and music: The case of British
//    English and French.  JASA 119(5), May 2006, pp. 3034-3047.
//

double Convert::coefficientOfVariationSample(const std::vector<double>& x) {
	double sd = Convert::standardDeviationSample(x);
	double mean = Convert::mean(x);
	return sd / mean;
}



//////////////////////////////
//
// Convert::nPvi -- normalized pairwise variablity index.
//    See: Linguistic: Grabe & Lowe 2002.
//    See: Daniele & Patel 2004.
//    See: Patel, Iversen & Rosenberg (2006): Comparing the
//    rhythm and melody of speech and music: The case of British
//    English and French.  JASA 119(5), May 2006, pp. 3034-3047.
//

double Convert::nPvi(const std::vector<double>& x) {
	double output = 0.0;
	for (int i=0; i<(int)x.size() - 1; i++) {
		output += fabs((x[i] - x[i+1]) / (x[i] + x[i+1]));
	}
	output *= 200.0 / ((int)x.size() - 1);
	return output;
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



