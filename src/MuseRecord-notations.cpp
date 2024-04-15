//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jun 30 21:44:58 PDT 1998
// Last Modified: Mon Sep 23 22:49:43 PDT 2019 Convert to STL
// Filename:      humlib/src/MuseRecord.cpp
// URL:           http://github.com/craigsapp/humlib/blob/master/src/MuseRecord.cpp
// Syntax:        C++11
// vim:           ts=3
//
//////////////////////////////
//
// Columns 32-42 for notes and rests. Codes in this area:
//
// Ties, Slurs, Tuples
// --------------------
// -  = tie
// J  = overhand back tie/slur (New 22 April 2008)
// K  = underhand back tie/slur (New 22 April 2008)
// (  = open slur1
// )  = close slur1
// [  = open slur2
// ]  = close slur2
// {  = open slur3
// }  = close slur3
// z  = open slur4
// x  = close slur4
// *  = start tuplet
// !  = stop tuplet
//
// Ornaments
// ----------
// t  = tr.
// r  = turn
// k  = delayed turn
// w  = shake
// ~  = wavy line (trill)
// c  = continue wavy line
// M  = mordant
// j  = slide
// T  = tremulo (New 7 Jan 2006)
//
// Technical Indications
// ---------------------
// v  = up bow
// n  = down bow
// o  = harmonic
// 0  = open string
// Q  = thumb position (cello)
// 1,2,3,4,5  = fingering
// :  = next fingering is a
//    substitution e.g.,
//    5:42:1 = 5-4
//             2-1
//
// Articulations and Accents
// --------------------------
// A  = vertical accent (/\)
// V  = vertical accent (\/)
// >  = horizontal accent
// .  = staccato
// _  = legato
// =  = line with dot under it
// i  = spiccato
// ,  = breath mark
// z  = open slur4
// x  = close slur4
//
// Accidentals on ornaments (must follow directly after ornament)
// --------------------------------
// s  = sharp   (ss = double sharp)
// h  = natural
// b  = flat    (bb = double flat)
// u  = next accidental is below rather than above ornament.
//      In case accidentals appear above and below ornament,
//      the accidental above should encoded first.
// U  = next accidental follows tr. on same line (used only with trills)
//
// Other Indications and Codes
// ---------------------------
// S  = arpeggiate (chords)
// F  = upright fermata
// E  = inverted fermata
// G  = G. P. (grand pause)
// p  = piano (pp, ppp, etc.)
// f  = forte (ff, fff, etc., fp)
// m  = mezzo (mp, mf)
// Z  = sfz (also sf)
// Zp = sfp
// R  = rfz
// ^  = accidental above note
// +  = cautionary/written out accidental
//
//////////////////////////////
//
// Codes sorted alphabetically:
// 
// A  = vertical accent (/\)      a  = unassigned
// B  = unassigned                b  = flat (bb = double flat) for ornaments
// C  = unassigned                c  = continue wavy line
// D  = unassigned                d  = unassigned
// E  = inverted fermata          e  = unassigned
// F  = upright fermata           f  = forte (ff, fff, etc., fp)
// G  = G. P. (grand pause)       g  = unassigned
// H  = unassigned                h  = natural for ornaments
// I  = unassigned                i  = spiccato
// J  = overhand back tie/slur    j  = slide
// K  = underhand back tie/slur   k  = delayed turn
// L  = unassigned                l  = unassigned
// M  = mordant                   m  = mezzo (mp, mf)
// N  = unassigned                n  = down bow
// O  = unassigned                o  = harmonic
// P  = unassigned                p  = piano (pp, ppp, etc.)
// Q  = thumb position (cello)    q  = unassigned
// R  = rfz                       r  = turn
// S  = arpeggiate (chords)       s  = sharp (ss = double sharp) for ornaments
// T  = tremulo (New 01/07/06)    t  = tr.
// U  = next accidental follows   u  = next accidental is below for ornaments
// V  = vertical accent (\/)      v  = up bow
// W  = unassigned                w  = shake
// X  = unassigned                x  = close slur4
// Y  = unassigned                y  = unassigned
// Z  = sfz (also sf)             z  = open slur4
// Zp = sfp
//
// Organized alphabetically
// ------------------
// ASCII    Symbol/Meaning
// 33       !  = stop tuplet    
// 34       "                  
// 35       #                 
// 36       $                
// 37       %               
// 38       &  = editorial switch  
// 39       '                     
// 40       (  = open slur1      
// 41       )  = close slur1    
// 42       *  = start tuplet  
// 43       +  = cautionary accidental
// 44       ,  = breath mark         
// 45       -  = tie                
// 46       .  = staccato          
// 47       /                     
// 48       0  = open string     
// 49       1  = fingering      
// 50       2  = fingering     
// 51       3  = fingering    
// 52       4  = fingering   
// 53       5  = fingering  
//
// Non-alphabetic
// ------------------
// 33       !  = stop tuplet      
// 34       "                    
// 35       #                   
// 36       $                  
// 37       %                 
// 38       &  = editorial switch  
// 39       '                     
// 40       (  = open slur1      
// 41       )  = close slur1    
// 42       *  = start tuplet  
// 43       +  = cautionary accidental
// 44       ,  = breath mark         
// 45       -  = tie                
// 46       .  = staccato          
// 47       /                     
// 48       0  = open string     
// 49       1  = fingering      
// 50       2  = fingering     
// 51       3  = fingering    
// 52       4  = fingering   
// 53       5  = fingering  
// 54       6
// 55       7
// 56       8
// 57       9
// 58       :  = next fingering
// 59       ;
// 60       <
// 61       =  = line with dot under it
// 62       >  = horizontal accent
// 63       ?  = dead space (do not assign)
// 64       @  = dead space (do not assign)
// 91       [  = open slur2
// 92       \
// 93       ]  = close slur2
// 94       ^  = accidental above note
// 95       _  = legato
// 96       `
// 123      {  = open slur3
// 124      |
// 125      }  = close slur3
// 126      ~  = wavy line (trill)
// 

#include "MuseRecord.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// MuseRecord::getOtherNotationsString -- return columns
//   32-42 (for notes and rests, but currently not checking if
//   the correct record type).
//

string MuseRecord::getOtherNotations(void) {
    if ((int)m_recordString.size() < 32) {
        return "";
    } else {
        size_t lengthToExtract = std::min(size_t(12), m_recordString.size() - 31);
        return m_recordString.substr(31, lengthToExtract);
    }
}



//////////////////////////////
//
// MuseRecord::getKernNoteOtherNotations -- Extract note-level ornaments
//    and articulations.  See MuseRecord::getOtherNotation() for list
//    of "other notations".
//

string MuseRecord::getKernNoteOtherNotations(void) {
	string output;
	string notations = getOtherNotations();
	for (int i=0; i<(int)notations.size(); i++) {
		switch(notations[i]) {
			case 'F': // fermata above
				output += ";";
				break;
			case 'E': // fermata below
				output += ";<";
				break;
			case '.': // staccato
				output += "'";
				break;
			case ',': // breath mark
				output += ",";
				break;
			case '=': // tenuto-staccato
				output += "~'";
				break;
			case '>': // accent
				output += "^";
				break;
			case 'A': // heavy accent
				output += "^^";
				break;
			case 'M': // mordent
				output += "M";
				break;
			case 'r': // turn
				output += "S";
				break;
			case 't': // trill
				output += "T";
				break;
			case 'n': // down bow
				output += "u";
				break;
			case 'v': // up bow
				output += "v";
				break;
			case 'Z': // sfz
				output += "zz";
				break;
		}
	}
	return output;
}



//////////////////////////////
//
// MuseRecord::hasFermata -- Return 0 if no fermata
//    E or F in columns 32-43 of a note or rest.  Returns +1 if fermata
//    above (E), or -1 if fermata below (F).  Check for &+ before
//    fermata for editorial.
//

int MuseRecord::hasFermata(void) {
	string notations = getOtherNotations();
	for (int i=0; i<(int)notations.size(); i++) {
		if (notations[i] == 'F') {
			return +1;
		}
		if (notations[i] == 'E') {
			return -1;
		}
	}
	return 0;
}


// END_MERGE

} // end namespace hum



