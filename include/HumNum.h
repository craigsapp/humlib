//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Sun Aug  9 21:03:12 PDT 2015
// Filename:      HumNum.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/HumNum.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Rational number class for durations.
//

#ifndef _HUMNUM_H_INCLUDED
#define _HUMNUM_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class HumNum {
	public:
		         HumNum             (void);
		         HumNum             (int value);
		         HumNum             (int numerator, int denominator);
		         HumNum             (const HumNum& rat);
		         HumNum             (const std::string& ratstring);
		         HumNum             (const char* ratstring);
		        ~HumNum             ();

		bool     isNegative         (void) const;
		bool     isPositive         (void) const;
		bool     isZero             (void) const;
		bool     isNonZero          (void) const;
		bool     isNonNegative      (void) const;
		bool     isNonPositive      (void) const;
		bool     isInfinite         (void) const;
		bool     isFinite           (void) const;
		bool     isNaN              (void) const;
		bool     isInteger          (void) const;
		bool     isPowerOfTwo       (void) const;
		double   getFloat           (void) const;
		double   toFloat  (void) const { return getFloat(); }
		int      getInteger         (double round = 0.0) const;
		int      toInteger (double round = 0.0) const {
		                                            return getInteger(round); }
		int      getNumerator       (void) const;
		int      getDenominator     (void) const;
		HumNum   getRemainder       (void) const;
		void     setValue           (int numerator);
		void     setValue           (int numerator, int denominator);
		void     setValue           (const std::string& ratstring);
		void     setValue           (const char* ratstring);
		void     invert             (void);
		HumNum   getAbs             (void) const;
		HumNum&  makeAbs            (void);
		HumNum&  operator=          (const HumNum& value);
		HumNum&  operator=          (int value);
		HumNum&  operator+=         (const HumNum& value);
		HumNum&  operator+=         (int value);
		HumNum&  operator-=         (const HumNum& value);
		HumNum&  operator-=         (int value);
		HumNum&  operator*=         (const HumNum& value);
		HumNum&  operator*=         (int value);
		HumNum&  operator/=         (const HumNum& value);
		HumNum&  operator/=         (int value);
		HumNum   operator-          (void) const;
		HumNum   operator+          (const HumNum& value) const;
		HumNum   operator+          (int value) const;
		HumNum   operator-          (const HumNum& value) const;
		HumNum   operator-          (int value) const;
		HumNum   operator*          (const HumNum& value) const;
		HumNum   operator*          (int value) const;
		HumNum   operator/          (const HumNum& value) const;
		HumNum   operator/          (int value) const;
		bool     operator==         (const HumNum& value) const;
		bool     operator==         (double value) const;
		bool     operator==         (int value) const;
		bool     operator!=         (const HumNum& value) const;
		bool     operator!=         (double value) const;
		bool     operator!=         (int value) const;
		bool     operator<          (const HumNum& value) const;
		bool     operator<          (double value) const;
		bool     operator<          (int value) const;
		bool     operator<=         (const HumNum& value) const;
		bool     operator<=         (double value) const;
		bool     operator<=         (int value) const;
		bool     operator>          (const HumNum& value) const;
		bool     operator>          (double value) const;
		bool     operator>          (int value) const;
		bool     operator>=         (const HumNum& value) const;
		bool     operator>=         (double value) const;
		bool     operator>=         (int value) const;
		std::ostream& printFraction      (std::ostream& = std::cout) const;
		std::ostream& printMixedFraction (std::ostream& out = std::cout,
		                             std::string separator = "_") const;
		std::ostream& printList          (std::ostream& out) const;
		std::ostream& printTwoPart  (std::ostream& out, const std::string& spacer = "+") const;

	protected:
		void     reduce             (void);
		int      gcdIterative       (int a, int b);
		int      gcdRecursive       (int a, int b);

	private:
		int top;
		int bot;
};


std::ostream& operator<<(std::ostream& out, const HumNum& number);

template <typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<A>& v);


// END_MERGE

} // end namespace hum

#endif /* _HUMNUM_H_INCLUDED */



