//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Apr 11 12:33:09 PDT 2002
// Last Modified: Fri Apr 12 18:13:46 PDT 2002
// Last Modified: Sat May 12 17:18:03 PDT 2007 added invert
// Last Modified: Sun Feb 20 18:05:23 PST 2011 added ostream
// Last Modified: Wed Nov 11 12:45:14 PST 2020 port to humlib
// Filename:      ...sig/maint/code/base/PixelColor/PixelColor.h
// Web Address:   http://sig.sapp.org/include/sigBase/PixelColor.h
// Documentation: http://sig.sapp.org/doc/classes/PixelColor
// Syntax:        C++
//

#ifndef _PIXELCOLOR_H_INCLUDED
#define _PIXELCOLOR_H_INCLUDED

#include <ostream>

namespace hum {

// START_MERGE

class PixelColor {
	public:
		             PixelColor     (void);
		             PixelColor     (const std::string& color);
		             PixelColor     (const PixelColor& color);
		             PixelColor     (int red, int green, int blue);
		             PixelColor     (float red, float green, float blue);
		             PixelColor     (double red, double green, double blue);
		            ~PixelColor     ();

		void         invert         (void);
		PixelColor&  setColor       (const std::string& colorstring);
		PixelColor&  setColor       (int red, int green, int blue);
		int          getRed         (void);
		int          getGreen       (void);
		int          getBlue        (void);
		void         setRed         (int value);
		void         setGreen       (int value);
		void         setBlue        (int value);
		float        getRedF        (void);
		float        getGreenF      (void);
		float        getBlueF       (void);
		void         setRedF        (float value);
		void         setGreenF      (float value);
		void         setBlueF       (float value);
		void         setColor       (PixelColor color);
		PixelColor&  setHue         (float value);
		PixelColor&  setTriHue      (float value);
		PixelColor&  makeGrey       (void);
		PixelColor&  makeGray       (void);
		PixelColor&  setGrayNormalized(double value);
		PixelColor&  setGreyNormalized(double value);
		int          operator>      (int number);
		int          operator<      (int number);
		int          operator==     (PixelColor color);
		int          operator!=     (PixelColor color);
		PixelColor&  operator=      (PixelColor color);
		PixelColor&  operator=      (int value);
		PixelColor   operator+      (PixelColor color);
		PixelColor&  operator+=     (int number);
		PixelColor   operator-      (PixelColor color);
		PixelColor&  operator*=     (double number);
		PixelColor   operator*      (PixelColor color);
		PixelColor   operator*      (double color);
		PixelColor   operator*      (int color);
		PixelColor   operator/      (double number);
		PixelColor   operator/      (int number);

		static PixelColor getColor  (const std::string& colorstring);

		void         writePpm6      (std::ostream& out);
		void         writePpm3      (std::ostream& out);

	public:
		unsigned char   Red;
		unsigned char   Green;
		unsigned char   Blue;

	private:
		float   charToFloat         (int value);
		int     floatToChar         (float value);
		int     limit               (int value, int min, int max);
};


// for use with P3 ASCII pnm images: print red green blue triplet.
std::ostream& operator<<(std::ostream& out, PixelColor apixel);


// END_MERGE

} // end namespace hum

#endif  /* _PIXELCOLOR_H_INCLUDED */



