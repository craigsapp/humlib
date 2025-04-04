//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Feb 16 13:22:15 PST 2025
// Last Modified: Sun Mar  2 11:57:59 PST 2025
// Filename:      tool-autocadence.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-autocadence.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Automatic detection of cadences in Renaissance music.
//

/*
 * Documentation:
 *

https://github.com/HCDigitalScholarship/intervals/blob/main/crim_intervals/data/cadences/CVFLabels.csv

Syntax:

* comma + space separate the slices

* each slice starts with a harmonic interval between two voices

* all intervals are diatonic without quality

* harmonic intervals are negative if the voice that is on a lower
  staff in the score is actually higher in pitch

* _ the underscore separates harmonic intervals from the melodic
  intervals in that slice

* : the colon separates the melodic interval of the lower voice
  (lower on the page) from the higher voice

* since we don't care about melodic intervals after the perfection,
  the last slice is always just a single harmonic interval

* D only appears after harmonic fourths, and it means that the
  fourth is a dissonant fourth. In this system, a fourth is dissonant
  if it's against the lowest pitch in the texture, even if that lowest
  pitch isn't in the pair being analyzed. I believe our dissonance
  classifier also has this requirement though, so you probably don't
  need the fourths as it will be enough to know that a fourth was
  labeled a suspension.

It would be cool if there were an option to have the full cadential
voice function names, but the single letters are probably the best
default. Logically there would be 24, but not all of them are
possible so there are only 15. Here's the meaning:

Realized Cadential Voice Functions (uppercase letters):
	C - Cantizans
	A - Altizans
	T - Tenorizans
	B - Bassizans (I believe Meier spells it Basizans but that looks wrong to me)
	L - Leaping Contratenor
	P - Plagal Bassizans
	S - Sestizans
	Q - Quintizans

Evaded Cadential Voice Functions (lowercase letters):
	c - Evaded Cantizans
	a - Evaded Altizans
	t - Evaded Tenorizans
	b - Evaded Bassizans
	u - Evaded Bassizans (down third)

Abandoned Cadential Voice Functions (also lowercase letters):
	x - Abandoned Bassizans
	y - Abandoned Cantizans
	z - Abandoned Tenorizans


First entry, which is an unornamented clausula vera:

"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_-2:2, 8",T,C

(?:R_1|7_1|[\-]?._[\-]?[^1]):1 == means 7 in 7_1:-2 is a suspension.


7_1:-2 = slice interval is a seventh with the lower voice staying on the
same pitch, and the upper voice going down a second (so the next harmonic
interval will be 6.

A "D" after a harmonic interval indicates a fourth above the lowest
note in the score (considering all parts).

With dissonance analysis, instead look for a suspension where the
"s" and "g" (patient and agent) dissonance types form a seventh,
and then the two-voice pattern from those two voices goes on to be
7_1:-2, 6_-2:2, 8.

Note maximum module length is 6 in the list below.

At the end of the list there are a few patterns with no labels and
one with only one label. I believe that was to fix cases of
over-triggering in CRIM. In those cases, the blanks were supposed
to overwrite any labels that might have been assigned. I left those
in for now, but I don't think they will be necessary in Humdrum
since we have proper dissonance detection.

Ngram,LowerCVF,UpperCVF
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:2, 8_1:-3, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:2, 8_1:-3, 6_1:1, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:-2, 5_-2:3, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:2, 8_1:-3, 6_1:-2, 5_-2:3, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:-2, 5_1:2, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:-2, 5_1:2, 6_1:1, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_-2:3, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_1:2, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:2, 7_1:2, 8_1:-3, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:2, 7_1:2, 8_1:-3, 6_1:-2, 5_-2:3, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:2, 7_1:-2, 6_1:-2, 5_-2:3, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:3, 8_1:-3, 6_-2:2, 8",T,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-3, 5_1:2, 6_-2:2, 8",T,C
"^6_1:2, 7_1:-2, 6_-2:2, 8",T,C
"^8_1:-2, 7_1:-2, 6_-2:2, 8",T,C
"^(?:[\-]?.|R)_1:[\-]?., -7_-2:1, -6_2:-2, -8",C,T
"^(?:[\-]?.|R)_1:[\-]?., -7_-2:1, -6_-2:1, -5_3:-2, -8",C,T
"^(?:[\-]?.|R)_1:[\-]?., -7_-2:1, -6_-2:1, -5_2:1, -6_2:-2, -8",C,T
"^(?:[\-]?.|R)_1:[\-]?., -7_-2:1, -6_1:1, -6_-2:1, -5_3:-2, -8",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_2:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_-2:1, 4D?_3:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_1:1, 3_-2:1, 4D?_3:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_-2:1, 4D?_2:1, 3_2:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_1:1, 3_-2:1, 4D?_2:1, 3_2:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_1:1, 3_2:-2, (?:1|8)",C,T
"^3_2:1, 2_-2:1, 3_2:-2, (?:1|8)",C,T
"^8_-2:1, 2_-2:1, 3_2:-2, (?:1|8)",C,T
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_-2:-2, 3",c,T
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_-2:2, (?:1|-8)",T,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:-2, -4D?_-2:3, (?:1|-8)",T,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:1, -3_1:-2, -4D?_-2:3, (?:1|-8)",T,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:-2, -4D?_1:2, -3_-2:2, (?:1|-8)",T,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_2:2, 6",t,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_1:2, 6_2:2, 6",t,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_2:2, -3",t,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:-2, -4D?_1:2, -3_2:2, -3",t,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_-2:R, R",T,y
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_2:2, 3",C,t
"^3_2:1, 2_-2:1, 3_2:2, 3",C,t
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_1:1, 3_-2:1, 4D?_2:1, 3_2:2, 3",C,t
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_-2:1, 4D?_2:1, 3_1:1, 3_2:2, 3",C,t
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_R:-2, R",z,c
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_R:-2, R",z,c
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_R:R, R",z,y
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:1, -3_R:R, R",z,y
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_2:R, R",C,z
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_R:R, R",y,z
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_R:2, R",z,C
"^(?:R_1|7_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_R:2, R",z,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_R:2, R",z,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_1:1, -3_R:2, R",z,C
"^(?:[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_R:2, R",x,C
"^(?:R_1|4_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_R:-2, R",x,c
"^3_1:2, 4D_1:-2, 3_R:2, R",x,C
"^5_1:-2, 4D_1:-2, 3_R:2, R",x,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-2:2, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-2:2, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-2:2, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_-2:3, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-2:3, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-2:2, 5",T,A
"^(?:R_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_1:1, 3_-2:2, 5",T,A
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_2:-2, -5",A,T
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_3:-2, -5",A,T
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_2:1, -3_2:-2, -5",A,T
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-2, -5",A,T
"^(?:[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-2:-2, 3",T,a
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-5:2, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_-5:2, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_8:2, (?:4|-5)",L,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_-5:3, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_8:3, (?:4|-5)",L,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-5:3, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_8:3, (?:4|-5)",L,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-5:2, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_8:2, (?:4|-5)",L,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_4:2, (?:1|8)",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_-5:2, 8",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_8:2, (?:4|-5)",L,C
"^3_1:2, 4D_1:-2, 3_(?:4|-5):2, (?:1|8)",B,C
"^5_1:-2, 4D_1:-2, 3_(?:4|-5):2, (?:1|8)",B,C
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_2:-5, -8",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_2:4, 1",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_2:4, -8",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_3:-5, -8",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_3:4, (?:1|-8)",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-5, -8",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_1:1, -3_-2:1, -2_3:4, (?:1|-8)",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_2:1, -3_2:(?:-5|4), -8",C,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_2:1, -3_2:4, (?:1|-8)",C,B
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:2, 4D_1:2, 5_1:-3, 3_2:2, 3",b,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_4:2, (?:8|1)",B,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_8:2, (?:4|-5)",L,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-3:2, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_-3:2, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_-3:3, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-3:3, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-3:2, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_-3:2, 6",u,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_-2:1, 4D_-2:2, 6",u,C
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_2:-3, -6",C,u
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_3:-3, -6",C,u
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-3, -6",C,u
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:1, -2_2:1, -3_2:-3, -6",C,u
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, 8_(?:5|-4):2, 5",P,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, 1_-4:2, 5",P,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, 8_1:1, 8_(?:5|-4):2, 5",P,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_(?:5|-4):2, 3",p,C
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]):1, 4D_1:-2, 3_(?:4|-5):(?:4|-5), 3",B,c
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_(?:4|-5):4, -3",c,B
"^(?:[\-]?.|R)_1:[\-]?., -4D_-2:1, -3_-2:(?:4|-5), (?:-6|3)",c,B
"^(?:R_1|4D?_1|[\-]?._[\-]?[^1]), 4D_1:-2, 3_1:-2, 2_1:2, 3_1:1, 3_-5:2, 8",B,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_-2:4, 3",T,c
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_(?:-5|4):2, 4D?",Q,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 7_1:-2, 6_1:1, 6_(?:-5|4):2, 4D?",Q,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_-5:2, 4D?",Q,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, -2_1:-2, -3_4:2, -5",Q,C
"^(?:[\-]?.|R)_1:[\-]?., 2_-2:1, 3_2:(?:-5|4), 5",C,Q
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, 8_-2:2, 3",s,
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, (?:1|8)_-3:2, 4D?",S,C
"^(?:R_1|[\-]?._[\-]?[^1]):1, 2_1:-2, (?:1|8)_1:1, (?:1|8)_-3:2, 4D?",S,C
"^[^R]_1, 7_1:-2, 6_-2:4, 3",c,T
"^[^R]_1, 2_1:-2, (?:1|8)_-3:2, 4D?",,
"^[^R]_1:, 4D_1:-2, 3_1:-2, 2_1:2, 3_-3:2, 6",,
"^[^R]_1:[\-]?., 7_1:-2, 6_-2:2, 8",,
"^(?:[\-]?.|R)_1:[\-]?., 7_1:-2, 6_R:-2, R",,

 */

#include "tool-autocadence.h"
#include "Convert.h"
#include "HumRegex.h"
#include "tool-dissonant.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_autocadence::Tool_autocadence -- Set the recognized options for the tool.
//

Tool_autocadence::Tool_autocadence(void) {
	define("c|color-cadence-notes=b", "Color cadence formula notes that match");
	define("d|show-formula-index=b",  "Append formula index after CVF label");
	define("e|even-note-spacing=b",   "Compress notation as much as possible");
	define("i|include-intervals=b",   "Display interval strings for notes in score (no further analysis)");
	define("m|matches=b",             "Give list of matching sequences only");
	define("p|pitches=b",             "Display extracted base-7 pitches only");
	define("r|regex=b",               "Display table of cadence formula regexes");
	define("s|sequences=b",           "Give list of extracted sequences only");
	define("I|intervals-only=b",      "Display interval strings for notes in score (no further analysis)");
	define("color=s:dodgerblue",      "Color cadence formula notes with given color");
	define("count|match-count=b",     "Return number of cadence formulas that match");
	define("B|no-back-highlight=b",   "Do not color start of sustain note at start of cadence definition");
	define("S|no-suspensions=b",      "Do not use suspensions from dissonance analysis");
}



/////////////////////////////////
//
// Tool_autocadence::run -- Do the main work of the tool.
//

bool Tool_autocadence::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_autocadence::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autocadence::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_autocadence::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_autocadence::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_autocadence::initialize(void) {
	m_printRawDiatonicPitchesQ =  getBoolean("pitches");
	m_intervalsOnlyQ           =  getBoolean("intervals-only");
	m_intervalsQ               =  getBoolean("include-intervals");
	m_matchesQ                 =  getBoolean("matches");
	m_printSequenceInfoQ       =  getBoolean("sequences");
	m_countQ                   =  getBoolean("match-count");
	m_colorQ                   =  getBoolean("color-cadence-notes");
	m_color                    =  getString("color");
	m_showFormulaIndexQ        =  getBoolean("show-formula-index");
	m_evenNoteSpacingQ         =  getBoolean("even-note-spacing");
	m_regexQ                   =  getBoolean("regex");
	m_nobackQ                  = !getBoolean("no-back-highlight");
	m_suspensionsQ             = !getBoolean("no-suspensions");

	prepareCadenceDefinitions();
	prepareCvfNames();
	prepareDissonanceNames();
}



//////////////////////////////
//
// Tool_autocadence::processFile --
//

void Tool_autocadence::processFile(HumdrumFile& infile) {


	// fill m_pitches and m_lowestPitch
	preparePitchInfo(infile);
	if (m_printRawDiatonicPitchesQ) {
		printExtractedPitchInfo(infile);
		return;
	}

	// identify dissonances
	if (m_suspensionsQ) {
		prepareDissonances(infile);
	}

	// fill m_intervals
	prepareIntervalInfo(infile);
	if (m_intervalsOnlyQ) {
		printExtractedIntervalInfo(infile);
		return;
	}

	// fill m_sequences
	prepareIntervalSequences(infile);
	if (m_printSequenceInfoQ) {
		printSequenceInfo();
		return;
	}

	// identify cadences
	searchIntervalSequences();
	if (m_matchesQ) {
		printSequenceMatches();
		return;
	} else if (m_countQ) {
		printMatchCount();
		return;
	}

	// markup score with matches and CVF
	markupScore(infile);
	printScore(infile);

}



//////////////////////////////
//
// Tool_autocadence::printScore --
//

void Tool_autocadence::printScore(HumdrumFile& infile) {
	infile.createLinesFromTokens();

	prepareAbbreviations(infile);
	vector<HTp> kspines;
	infile.getKernSpineStartList(kspines);
	int kcount = (int)kspines.size();
	if (!m_intervalsQ) {
		kcount = 0;
	}

	for (int i=0; i < infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
		} else if (infile[i].isEmpty()) {
			m_humdrum_text << endl;
		} else if (infile[i].isManipulator()) {
			printIntervalManipulatorLine(infile, i, kcount);
		} else if (infile[i].isData()) {
			printIntervalDataLineScore(infile, i, kcount);
		} else if (infile[i].isBarline()) {
			string bartok = *infile.token(i, 0);
			printIntervalLine(infile, i, kcount, bartok);
		} else if (infile[i].isCommentLocal()) {
			printIntervalLine(infile, i, kcount, "!");
		} else if (infile[i].isInterpretation()) {
			printIntervalLine(infile, i, kcount, "*");
		} else {
			m_humdrum_text << "!! UNKNOWN LINE: " << infile[i] << endl;
		}
	}

	if (m_colorQ) {
		m_humdrum_text << "!!!RDF**kern: " << m_marker << " = marked note, color=" << m_color << endl;
	}
	if (m_hasSuspensionMarkersQ) {
		m_humdrum_text << "!!!RDF**kern: " << m_suspensionMarker << " = marked note, color=" << m_suspensionColor << endl;
	}
	if (m_evenNoteSpacingQ) {
		m_humdrum_text << "!!!verovio: evenNoteSpacing" << endl;
	}

	if (m_regexQ) {
		printRegexTable();
	}
}



//////////////////////////////
//
// Tool_autocadence::printRegexTable --
//

void Tool_autocadence::printRegexTable(void) {

	// pair::first: index into m_definitions;
	// pair::second: occurrence count for given cadence in score.
	set<int> definitionList;
	prepareDefinitionList(definitionList);

	if (definitionList.empty()) {
		m_humdrum_text << "!!@@BEGIN: PREHTML" << endl;
		m_humdrum_text << "!!@CONTENT: <i style='color:red'>No cadences found in music</i>" << endl;
		m_humdrum_text << "!!@@END: PREHTML" << endl;
		return;
	}

	m_humdrum_text << "!!@@BEGIN: PREHTML" << endl;
	m_humdrum_text << "!!@CONTENT:" << endl;
	m_humdrum_text << "!! <table class=regex>" << endl;

	m_humdrum_text << "!! <tr>" << endl;
	m_humdrum_text << "!! <th class=lcvf title='lower cadence vocal function'>LCVF</th>" << endl;
	m_humdrum_text << "!! <th class=ucvf title='upper cadence vocal function'>UCVF</th>" << endl;
	m_humdrum_text << "!! <th class=name title='cadence name (abbreviation)'>Name</th>" << endl;
	m_humdrum_text << "!! <th class='index' title='cadence enumeration'>Index</th>" << endl;
	m_humdrum_text << "!! <th title='regular expression definition for cadence formula'>Cadence formula</th>" << endl;
	m_humdrum_text << "!! </tr>" << endl;

	for (int index : definitionList) {
		printDefinitionRow(index);
	}

	m_humdrum_text << "!! </table>" << endl;
	m_humdrum_text << "!!@@END: PREHTML" << endl;

}



//////////////////////////////
//
// Tool_autocadence::printDefinitionRow --
//

void Tool_autocadence::printDefinitionRow(int index) {
	if (index > (int)m_definitions.size() - 1) {
		cerr << "ERROR: definition index out of range: " << index << endl;
		cerr << "Maximum value is " << (m_definitions.size() - 1) << endl;
		return;
	}
	auto& def = m_definitions.at(index);

	m_humdrum_text << "!! <tr>" << endl;

	string nameL = m_functionNames[def.m_funcL];
	string nameU = m_functionNames[def.m_funcU];

	m_humdrum_text << "!! <td class=lcvf";
	if (!nameL.empty()) {
		m_humdrum_text << " title='" << nameL << "'";
	}
	m_humdrum_text << ">" << def.m_funcL << "</td>" << endl;


	m_humdrum_text << "!! <td class=ucvf";
	if (!nameU.empty()) {
		m_humdrum_text << " title='" << nameU << "'";
	}
	m_humdrum_text << ">" << def.m_funcU << "</td>" << endl;

	m_humdrum_text << "!! <td class=name>" << def.m_name  << "</td>" << endl;
	m_humdrum_text << "!! <td class=index>" << index << "</td>" << endl;
	m_humdrum_text << "!! <td class=definition>" << def.m_regex << "</td>" << endl;

	m_humdrum_text << "!! </tr>" << endl;

	string style = R"(!! <style>
!! table.regex tr:hover { background: orange; }
!! table.regex {
!!    border-collapse: collapse;
!!    border: 1px solid orange;
!!    margin-top: 40px;
!!    margin-left: auto;
!!    margin-right: auto;
!!    max-width: 800px;
!! }
!! table.regex td.definition {
!!    word-spacing: 5px;
!!    padding-left: 30px;
!!    text-indent: -30px;
!! }
!! table.regex td.lcvf,
!! table.regex td.ucvf,
!! table.regex td.name {
!!    text-align: center;
!!    cursor: help;
!! }
!! table.regex th {
!!    vertical-align: top;
!!    background-color: bisque;
!!    padding-right: 10px;
!!    text-align: left;
!! }
!! table.regex td {
!!    vertical-align: top;
!!    text-align: left;
!!    padding-right: 20px;
!! }
!! table.regex td.index, table.regex th.index {
!!    text-align: right;
!! }
!! </style>)";
	m_humdrum_text << style << endl;


}



//////////////////////////////
//
// Tool_autocadence::markupScore --
//

void Tool_autocadence::markupScore(HumdrumFile& infile) {
	for (int i=0; i<(int)m_matches.size(); i++) {
		addMatchToScore(infile, i);
	}
}



//////////////////////////////
//
// Tool_autocadence::addMatchToScore --
//

void Tool_autocadence::addMatchToScore(HumdrumFile& infile, int matchIndex) {
	vector<int>& coord = m_matches.at(matchIndex);
	int vindex = coord.at(0);
	int pindex = coord.at(1);
	int nindex = coord.at(2);
	auto& info = m_sequences.at(vindex).at(pindex).at(nindex);
	// get<0> is the sequence string.
	HTp startL = get<1>(info);  // starting token of cadence formula, lower voice
	HTp startU = get<2>(info);  // starting token of cadence formula, upper voice

	if (startL == NULL) {
		cerr << "WARNING: startL is NULL" << endl;
		return;
	}
	if (startU == NULL) {
		cerr << "WARNING: startU is NULL" << endl;
		return;
	}

	int lindex = startL->getLineIndex();
	vector<int>& dindexes = get<3>(info);
	if (dindexes.empty()) {
		cerr << "WARNING: dindexes is empty" << endl;
		return;
	}
	// Ignoring any secondary matches for same sequence for now:
	int dindex = dindexes.at(0);
	Tool_autocadence::CadenceDefinition& definition = m_definitions.at(dindex);
	string& funcL = definition.m_funcL; // CVF of lower voice
	string& funcU = definition.m_funcU; // CVF of upper voice
	// string& name  = definition.m_name;  // name of cadence formula
	string& regex = definition.m_regex; // regular expression defining formula
	int count = getRegexSliceCount(regex);
	HTp endL = NULL;  // ending token of cadence formula, lower voice
	HTp endU = NULL;  // ending token of cadence formula, upper voice
	int status = getCadenceEndSliceNotes(endL, endU, count, infile, lindex, vindex, pindex);
	if (!status) {
		cerr << "Problem extracting slide end for cadence" << endl;
		return;
	}

	// cerr << "ENDING LOWER: " << endL << "\tENDING UPPER: " << endU << endl;

	// Add CVF to last note of cadence formulas:
	string valueL = endL->getValue("auto", "cvf");
	if (valueL.empty()) {
		valueL = funcL;
	} else {
		valueL += ",";
		valueL += funcL;
	}
	if (m_showFormulaIndexQ) {
		valueL += to_string(dindex);
	}
	// Indicate voice indexes of two voices involved in CVF pair:
	// valueL += "V" + to_string(vindex);
	// valueL += "P" + to_string(pindex);
	endL->setValue("auto", "cvf", valueL);

	string valueU = endU->getValue("auto", "cvf");
	if (valueU.empty()) {
		valueU = funcU;
	} else {
		valueU += ",";
		valueU += funcU;
	}
	if (m_showFormulaIndexQ) {
		valueU += to_string(dindex);
	}
	endU->setValue("auto", "cvf", valueU);

	if (m_colorQ) {
		colorNotes(startL, endL);
		colorNotes(startU, endU);
	}

}



//////////////////////////////
//
// Tool_autocadence::colorNotes -- Color notes inclusive between two notes.
//

void Tool_autocadence::colorNotes(HTp startTok, HTp endTok) {
	if (startTok == NULL) {
		cerr << "Warning: startTok is null" << endl;
	}
	if (endTok == NULL) {
		cerr << "Warning: endTok is null" << endl;
	}
	int startTrack = startTok->getTrack();
	int endTrack = startTok->getTrack();
	if (startTrack != endTrack) {
		cerr << "Start and ending tracks are not the same: " << startTrack << " and " << endTrack << endl;
		return;
	}
	// cerr << "\n\tcolor notes startLine = " << startTok->getLineIndex() << endl;
	// cerr << "\tcolor notes endLine   = " << endTok->getLineIndex() << endl;
	HTp current = startTok;
	while (current) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}
		//	if (current->isRest()) {
		//		current = current->getNextToken();
		//		continue;
		//	}
		string text = *current;
		text += m_marker;
		current->setText(text);
	
		if (current == endTok) {
			break;
		}

		current = current->getNextToken();
	}

	if (m_nobackQ && (startTok->isNullToken() || startTok->isSustainedNote())) {
		highlightNoteAttack(startTok);
	}
}



//////////////////////////////
//
// Tool_autocadence::highlightNoteAttack -- search backards from a sustain
//    to highlight start of note.
//

void Tool_autocadence::highlightNoteAttack(HTp startTok) {
	bool markedQ = true;
	if (startTok->isNullToken()) {
		HTp resolved = startTok->resolveNullToken();
		if (!resolved) {
			return;
		}
		if (resolved->isRest()) {
			// maybe color?
			return;
		}
		if (resolved->isNoteAttack()) {
			string text = resolved->getText();
			text += m_marker;
			resolved->setText(text);
			return;
		} else {
			startTok = resolved;
			string text = resolved->getText();
			text += m_marker;
			resolved->setText(text);
			markedQ = true;
		}
	}
	if (startTok->isRest()) {
		return;
	}
	int b40 = Convert::kernToBase40(startTok);
	if (!markedQ) {
		string text = startTok->getText();
		text += m_marker;
		startTok->setText(text);
	}
	HTp current = startTok->getPreviousToken();
	while (current) {
		if (!current->isData()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isNullToken()) {
			current = current->getPreviousToken();
			continue;
		}
		if (current->isRest()) {
			break;
		}
		int b40t = Convert::kernToBase40(current);
		if (b40t != b40) {
			break;
		}
		string text = current->getText();
		text += m_marker;
		current->setText(text);
		if (current->isNoteAttack()) {
			break;
		}
		current = current->getPreviousToken();
	}
}



//////////////////////////////
//
// Tool_autocadence::getCadenceEndSliceNotes --
//    lindex = starting line of the cadence definition
//    vindex = voice index (nth **kern spine, starting with lowest, indexed by 0
//    pindex = pairing index between two voices
//
// m_intervals: The counterpoint intervals for each pair of notes.
// The data is store in a 3-D vector, where the first dimension is the
// line in the score, the second dimension is the voice (kern spine) index in
// the score for the lower voice.  For example, if the score has four **kern
// spines, the second dimension size will be four, and the third dimensions
// will be 3, 2, 1 and 0.  
// Dimensions:
//     0: line index in the Humdrum data
//     1: voice index of the lower voice
//     2: pairing interval of two voice generating the interval
// Parameters for tuple:
//     0: interval string for the note pair (or empty if no interval.
//     1: the token for the lower voice note
//     2: the token for the upper voice note
// std::vector<std::vector<std::vector<std::tuple<std::string, HTp, HTp>>>> m_intervals;

bool Tool_autocadence::getCadenceEndSliceNotes(HTp& endL, HTp& endU, int count,
		HumdrumFile& infile, int lindex, int vindex, int pindex) {
	endL = NULL;
	endU = NULL;
	int counter = 0;
	int lineIndex = lindex;
	int endIndex = -1;
	while ((counter < count) && (lineIndex < (int)m_intervals.size())) {
		if (!infile[lineIndex].isData()) {
			lineIndex++;
			continue;
		}
		string& interval = get<0>(m_intervals.at(lineIndex).at(vindex).at(pindex));
		if (!interval.empty()) {
			counter++;
			if (counter == count) {
				endIndex = lineIndex;
				break;
			}
		}
		lineIndex++;
	}
	if (counter != count) {
		cerr << "ERROR: Could not find full cadence definition in score" << endl;
		return false;
	}

	// find the lower voice token:
	int fieldIndex = -1;
	int kcounter = 0;
	for (int i=0; i<infile[endIndex].getFieldCount(); i++) {
		HTp token = infile.token(endIndex, i);
		if (!token->isKern()) {
			continue;
		}
		int subtrack = token->getSubtrack();
		if (subtrack > 1) {
			// Multiple layers on staff, ignore secondary layers:
			continue;
		}
		if (kcounter == vindex) {
			fieldIndex = i;
			endL = token;
			break;
		} else {
			kcounter++;
		}
	}

	if (endL == NULL) {
		cerr << "ERROR: Problem finding lower voice ending token" << endl;
		return false;
	}

	int pcounter = 0;
	for (int i=fieldIndex+1; i<infile[endIndex].getFieldCount(); i++) {
		HTp token = infile.token(endIndex, i);
		if (!token->isKern()) {
			continue;
		}
		int subtrack = token->getSubtrack();
		if (subtrack > 1) {
			// Multiple layers on staff, ignore secondary layers:
			continue;
		}
		if (pcounter == pindex) {
			endU = token;
			break;
		} else {
			pcounter++;
		}
	}
	
	if (endU == NULL) {
		cerr << "ERROR: Problem finding upper voice ending token" << endl;
		return false;
	}

	return true;
}



//////////////////////////////
//
// Tool_autocadence::getRegexSliceCount --
//      Example: R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:2, 8_1:-3, 6_1:1, 6_-2:2, 8_)"
//      Count the number of commas and add 1.
//      Maybe need to be careful with look behinds, but probably ok.
//

int Tool_autocadence::getRegexSliceCount(const string& regex) {
	int count = (int)std::count(regex.begin(), regex.end(), ',');
	return count + 1;

}



//////////////////////////////
//
// Tool_autocadence::printMatchCount -- Print number of cadence formula
//      match counts for score.  If there are duplicate matches for any
//      sequence (more than one cadence formula matches a single sequence),
//      then these secondary matches are give after the main number of
//      matches in parentheses.
//

void Tool_autocadence::printMatchCount(void) {
	int count = (int)m_matches.size();
	int subcount = 0;
	for (int i=0; i<(int)m_matches.size(); i++) {
		auto& info = m_sequences.at(m_matches[i][0]).at(m_matches[i][1]).at(m_matches[i][2]);
		vector<int>& matches = get<3>(info);
		subcount += (int)matches.size() - 1;
	}

	m_humdrum_text << count;
	if (subcount > 0) {
		m_humdrum_text << "(" << subcount << ")";
	}
	m_humdrum_text << endl;
}



//////////////////////////////
//
// Tool_autocadence::searchIntervalSequences --
//
//
// m_sequences dimensions:
// 	0: voice index
// 	1: voice pair index
//    2: an array of tuples containing sequences
// Parameters for tuple:
// 	0: sequence to search
// 	1: first note of sequence in score (lower voice)
// 	2: first note of sequence in score (upper voice)
//    3: vector of ints: int is the m_definitions cadence index
//

void Tool_autocadence::searchIntervalSequences(void) {
	HumRegex hre;
	m_matches.clear();
	for (int i=0; i<(int)m_sequences.size(); i++) {
		for (int j=0; j<(int)m_sequences[i].size(); j++) {
			for (int k=0; k<(int)m_sequences[i][j].size(); k++) {
				string& feature = get<0>(m_sequences.at(i).at(j).at(k));
				for (int m=0; m<(int)m_definitions.size(); m++) {
					if (hre.search(feature, m_definitions.at(m).m_regex)) {
						vector<int>& matches = get<3>(m_sequences.at(i).at(j).at(k));
						// cerr << "FOUND MATCH: " << m << endl;
						matches.push_back(m);
						m_matches.emplace_back(vector<int>{i, j, k});
					}
				}
			}
		}
	} 
}



//////////////////////////////
//
// Tool_autocadence::prepareDefinitionList -- Extract a list of definition indexes
//    for use with PREHTML table.  Could count number of times definition was
//    used in score.
//

void Tool_autocadence::prepareDefinitionList(set<int>& list) {
	list.clear();

	for (int i=0; i<(int)m_matches.size(); i++) {
		int& vindex = m_matches.at(i).at(0);
		int& pindex = m_matches.at(i).at(1);
		int& nindex = m_matches.at(i).at(2);
		auto& info  = m_sequences.at(vindex).at(pindex).at(nindex);
		vector<int>& matches = get<3>(info);
		for (int m=0; m<(int)matches.size(); m++) {
			int dindex = matches.at(m);
			list.insert(dindex);
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::printSequenceMatches -- Print only sequences that are matches to cadence formulas.
// 	The first column indicate which formula index (empty if none).
//
// std::vector< // voice index
// 	std::vector< // voice pair index
// 		std::vector< // list of sequences
// 			std::tuple<std::string, HTp, HTp, std::vector<int>>
// 		>
// 	>
// > m_sequences;
//
// Dimensions of m_sequences:
//    0: voice index
//    1: voice pair index
//    2: an array of tuples containing sequences
// Parameters for tuple:
//    0: sequence to search
//    1: first note of sequence in score (lower voice)
//    2: first note of sequence in score (upper voice)
//    3: vector of ints: int is the m_definitions cadence index
// std::vector<std::vector<int>> m_matches;

void Tool_autocadence::printSequenceMatches(void) {
	for (int i=0; i<(int)m_matches.size(); i++) {
		int& vindex = m_matches.at(i).at(0);
		int& pindex = m_matches.at(i).at(1);
		int& nindex = m_matches.at(i).at(2);
		auto& info = m_sequences.at(vindex).at(pindex).at(nindex);
		vector<int>& matches = get<3>(info);
		if (matches.empty()) {
			continue;
		}


		for (int m=0; m<(int)matches.size(); m++) {
			int dindex = matches.at(m);
			auto& cinfo = m_definitions.at(dindex);
			string& name = cinfo.m_name;
			m_humdrum_text << name;
			if (m<(int)matches.size() - 1) {
				m_humdrum_text << ",";
			}
		}

		m_humdrum_text << "\t";

		for (int m=0; m<(int)matches.size(); m++) {
			m_humdrum_text << matches[m];
			if (m<(int)matches.size() - 1) {
				m_humdrum_text << ",";
			}
		}

		m_humdrum_text << "\t";
		string& sequence = get<0>(info);
		m_humdrum_text << sequence << endl;
	}
}


// Version that prints directly from m_sequences:

void Tool_autocadence::printSequenceMatches2(void) {
	for (int i=0; i<(int)m_sequences.size(); i++) {
		for (int j=0; j<(int)m_sequences.at(i).size(); j++) {
			if (!(i==0 && j==0)) {
				m_humdrum_text << endl;
			}
			m_humdrum_text << "# Matches for voices " << (i+1) << " TO " << (i+1+j+1) << endl;
			for (int k=0; k<(int)m_sequences.at(i).at(j).size(); k++) {
				string& sequence = get<0>(m_sequences.at(i).at(j).at(k));
				vector<int>& matches = get<3>(m_sequences.at(i).at(j).at(k));
				if (matches.empty()) {
					continue;
				}
				for (int m=0; m<(int)matches.size(); m++) {
					m_humdrum_text << matches[m];
					if (m<(int)matches.size() - 1) {
						m_humdrum_text << ",";
					}
				}
				m_humdrum_text << "\t";
				for (int m=0; m<(int)matches.size(); m++) {
					auto& cinfo = m_definitions.at(matches[m]);
					string& name = cinfo.m_name;
					m_humdrum_text << name;
					if (m<(int)matches.size() - 1) {
						m_humdrum_text << ",";
					}
				}
				m_humdrum_text << "\t";
				m_humdrum_text << sequence << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::printSequenceInfo --
//
// std::vector<std::vector<std::vector<std::tuple<std::string, HTp, HTp, std::vector<int>>>>> m_sequences;
// Dimensions of m_sequences:
//    0: voice index
//    1: voice pair index
//    2: an array of tuples containing sequences
// Parameters for tuple:
//    0: sequence to search
//    1: first note of sequence in score (lower voice)
//    2: first note of sequence in score (upper voice)
//    3: vector of ints: int is the m_definitions cadence index

void Tool_autocadence::printSequenceInfo(void) {
	for (int i=0; i<(int)m_sequences.size(); i++) {
		for (int j=0; j<(int)m_sequences[i].size(); j++) {
			m_humdrum_text << endl;
			m_humdrum_text << "# Sequences for voices " << (i+1) << " TO " << (i+1+j+1) << endl;
			for (int k=0; k<(int)m_sequences[i][j].size(); k++) {
				string& sequence = get<0>(m_sequences[i][j][k]);
				m_humdrum_text << sequence << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareIntervalSequences --
// Dimensions of m_seqences:
// 	0: voice index of the lower voice
// 	1: voice pair index for upper voice
// Parameters for tuple:
// 	0: sequence to search
// 	1: first note of sequence in score (lower voice)
// 	2: first note of sequence in score (upper voice)
//    3: vector of ints: int is the m_definitions cadence index
//

void Tool_autocadence::prepareIntervalSequences(HumdrumFile& infile) {
	vector<HTp> kstarts;
	infile.getKernSpineStartList(kstarts);
	int ksize = (int)kstarts.size();

	// Allocate space for m_sequence data:
	m_sequences.clear();
	m_sequences.resize(ksize);
	for (int i=0; i<ksize; i++) {
		m_sequences[i].resize(ksize-1-i);
		for (int j=0; j<(int)m_sequences[i].size(); j++) {
			prepareSinglePairSequences(infile, i, j);
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareSinglePairSequences -- Given a voice index and a pairing index, generate
//     sequences of intervals based with the length determined by m_sequenceLength.
//
// Dimensions of m_intervals:
//     0: line index in the Humdrum data
//     1: voice index of the lower voice
//     2: pairing index for two voices
// Parameters for tuple:
//     0: interval string for the note pair (or empty if no interval).
//     1: the token for the lower voice note
//     2: the token for the upper voice note
//
// Dimensions of m_sequences:
//    0: voice index
//    1: voice pair index
//    2: an array of tuples containing sequences
// Parameters for tuple:
//    0: sequence to search
//    1: first note of sequence in score (lower voice)
//    2: first note of sequence in score (upper voice)
//    3: vector of ints: int is the m_definitions cadence index that matches.
//

void Tool_autocadence::prepareSinglePairSequences(HumdrumFile& infile, int vindex, int pindex) {
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		string interval = get<0>(m_intervals.at(i).at(vindex).at(pindex));
		if (interval.empty()) {
			continue;
		}
		HTp lower = get<1>(m_intervals.at(i).at(vindex).at(pindex));
		HTp upper = get<2>(m_intervals.at(i).at(vindex).at(pindex));
		string sequence = generateSequenceString(infile, i, vindex, pindex);
// cerr << "ADDING SEQUENCE: " << sequence << endl;
		m_sequences.at(vindex).at(pindex).emplace_back(sequence, lower, upper, vector<int>{});
	}
}



//////////////////////////////
//
// Tool_autocadence::generateSequenceString -- Combine multiple interval strings from
//      m_intervals for the given note pairing.  If there are not enough notes at the
//      end of a score, the length will be shortened based on the number of remaining
//      intervals that are available.
//
// Dimensions of m_intervals:
//     0: line index in the Humdrum data
//     1: voice index of the lower voice
//     2: pairing index for two voices
// Parameters for tuple:
//     0: interval string for the note pair (or empty if no interval).
//     1: the token for the lower voice note
//     2: the token for the upper voice note

string Tool_autocadence::generateSequenceString(HumdrumFile& infile, int lindex, int vindex, int pindex) {
	vector<string> pieces;
	for (int i=lindex; i<infile.getLineCount(); i++) {
		string interval = get<0>(m_intervals.at(i).at(vindex).at(pindex));
		if (interval.empty()) {
			continue;
		}
		pieces.push_back(interval);
		if ((int)pieces.size() >= m_sequenceLength) {
			break;
		}
	}

	string output;
	for (int i=0; i<(int)pieces.size(); i++) {
		output += pieces[i];
		if (i < (int)pieces.size() - 1) {
			output += ", ";
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_autocadence::printExtractedPitchInfo -- Print only the extracted pitch infomation.
//

void Tool_autocadence::printExtractedPitchInfo(HumdrumFile& infile) {
	for (int i=0; i < infile.getLineCount(); i++) {
		if (infile[i].isManipulator()) {
			HTp token = infile.token(i, 0);

			if (token->isExInterp()) {
				m_humdrum_text << "**low";
			} else if (token->isTerminator()) {
				m_humdrum_text << "*-";
			} else {
				m_humdrum_text << "*";
			}

			if (token->isExInterp()) {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					HTp tok = infile.token(i, j);
					if (tok->isKern()) {
						m_humdrum_text << "\t" << "**b7";
					}
				}
				m_humdrum_text << endl;
			} else {
				for (int j=0; j<infile[i].getFieldCount(); j++) {
					HTp tok = infile.token(i, j);
					if (tok->isKern()) {
						m_humdrum_text << "\t" << tok;
					}
				}
				m_humdrum_text << endl;

			}
		} else if (infile[i].isData()) {
			m_humdrum_text << m_lowestPitch.at(i);
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok = infile.token(i, j);
				if (!tok->isKern()) {
					continue;
				}
				m_humdrum_text << "\t" << m_pitches.at(i).at(j);
			}
			m_humdrum_text << endl;
		} else if (infile[i].isBarline()) {
			HTp tok = infile.token(i, 0);
			m_humdrum_text << tok;
			for (int j=0; j<infile[i].getFieldCount(); j++) {
				HTp tok2 = infile.token(i, j);
				if (!tok2->isKern()) {
					continue;
				}
				int subtrack = tok2->getSubtrack();
				if (subtrack > 1) {
					continue;
				}
				m_humdrum_text << "\t" << tok;
			}
			m_humdrum_text << endl;
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::printExtractedIntervalInfo -- Print only the extracted interval infomation.
//

void Tool_autocadence::printExtractedIntervalInfo(HumdrumFile& infile) {

	prepareAbbreviations(infile);
	vector<HTp> kspines;
	infile.getKernSpineStartList(kspines);
	int kcount = (int)kspines.size();

	for (int i=0; i < infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			m_humdrum_text << infile[i] << endl;
		} else if (infile[i].isEmpty()) {
			m_humdrum_text << endl;
		} else if (infile[i].isManipulator()) {
			printIntervalManipulatorLine(infile, i, kcount);
		} else if (infile[i].isData()) {
			printIntervalDataLine(infile, i, kcount);
		} else if (infile[i].isBarline()) {
			string bartok = *infile.token(i, 0);
			printIntervalLine(infile, i, kcount, bartok);
		} else if (infile[i].isCommentLocal()) {
			printIntervalLine(infile, i, kcount, "!");
		} else if (infile[i].isInterpretation()) {
			printIntervalLine(infile, i, kcount, "*");
		} else {
			m_humdrum_text << "!! UNKNOWN LINE: " << infile[i] << endl;
		}
	}

}



//////////////////////////////
//
// Tool_autocadence::printIntervalManipulatorLine --
//

void Tool_autocadence::printIntervalManipulatorLine(HumdrumFile& infile, int index, int kcount) {
	stringstream line;
	int fcount = infile[index].getFieldCount();
	for (int i=0; i<fcount; i++) {
		HTp token = infile.token(index, i);
		if (i != 0) {
			line << "\t";
		}
		line << token;
		if (!token->isKern()) {
			continue;
		}
		int track = token->getTrack();
		int ntrack = 0;
		if (i < fcount - 1) {
			ntrack = infile.token(index, i+1)->getTrack();
		}
		if (track == ntrack) {
			continue;
		}
		if (kcount > 0) {
			int vindex = m_trackToVoiceIndex.at(track);
			int tcount = kcount - vindex - 1;
			for (int j=0; j<tcount; j++) {
				string value;
				if (token->isExInterp()) {
					value = "**adata";
					if (!m_abbr.at(vindex+1+j).empty()) {
						value += "-";
						value += m_abbr.at(vindex+1+j);
					}
				} else if (token-> isTerminator()) {
					value = "*-";
				} else {
					value = "*";
				}
				line << "\t" << value;
			}
		}
	}
	m_humdrum_text << line.str() << endl;
}



//////////////////////////////
//
// Tool_autocadence::printIntervalDataLine --
//

void Tool_autocadence::printIntervalDataLine(HumdrumFile& infile, int index, int kcount) {
	stringstream line;
	int fcount = infile[index].getFieldCount();
	for (int i=0; i<fcount; i++) {
		HTp token = infile.token(index, i);
		if (i != 0) {
			line << "\t";
		}
		line << token;
		if (!token->isKern()) {
			continue;
		}
		int track = token->getTrack();
		int ntrack = 0;
		if (i < fcount - 1) {
			ntrack = infile.token(index, i+1)->getTrack();
		}
		if (track == ntrack) {
			continue;
		}

		if (kcount > 0) {
			int vindex = m_trackToVoiceIndex.at(track);
			int tcount = kcount - vindex - 1;
			for (int j=0; j<tcount; j++) {
				string value = get<0>(m_intervals.at(index).at(vindex).at(j));
				if (value == "") {
					value = ".";
				}
				line << "\t" << value;
			}
		}
	}
	m_humdrum_text << line.str() << endl;
}



//////////////////////////////
//
// Tool_autocadence::printIntervalDataLineScore -- Data line printing for score, adding
//     markers for CFV if present.
//

void Tool_autocadence::printIntervalDataLineScore(HumdrumFile& infile,
		int index, int kcount) {

	vector<string> labels(infile[index].getFieldCount());
	bool foundLabelQ = false;

	vector<string> dissonances(infile[index].getFieldCount());
	bool foundDissonanceQ = false; // just for suspensions

	int fcount = infile[index].getFieldCount();

	stringstream dataline;
	for (int i=0; i<fcount; i++) {
		HTp token = infile.token(index, i);
		if (i != 0) {
			dataline << "\t";
		}
		dataline << token;
		if (!token->isKern()) {
			continue;
		}
		string label = token->getValue("auto", "cvf");
		if (!label.empty()) {
			labels.at(i) = label;
			foundLabelQ = true;
		}
		string dissonance = token->getValue("auto", "dissonance");
		if ((dissonance == "s") || (dissonance == "S") || (dissonance == "g") || (dissonance == "G")) {
			if (!dissonance.empty()) {
				dissonances.at(i) = dissonance;
				foundDissonanceQ = true;
			}
		}
		int track = token->getTrack();
		int ntrack = 0;
		if (i < fcount - 1) {
			ntrack = infile.token(index, i+1)->getTrack();
		}
		if (track == ntrack) {
			continue;
		}
		if (kcount > 0) {
			int vindex = m_trackToVoiceIndex.at(track);
			int tcount = kcount - vindex - 1;
			for (int j=0; j<tcount; j++) {
				string value = get<0>(m_intervals.at(index).at(vindex).at(j));
				if (value == "") {
					value = ".";
				}
				dataline << "\t" << value;
			}
		}

	}

	stringstream labelline;
	if (foundLabelQ) {
		for (int i=0; i<fcount; i++) {
			HTp token = infile.token(index, i);
			int track = token->getTrack();
			int vindex = m_trackToVoiceIndex.at(track);
			if (i != 0) {
				labelline << "\t";
			}
			string label = labels.at(i);
			if (label.empty()) {
				labelline << "!";
			} else {
				labelline << "!LO:TX:a:B:cvf";
				labelline << ":color=" << m_color;
				labelline << ":t=" << label;
				if (m_popupQ) {
				 	string fname = getFunctionNames(label);
				 	if (!fname.empty()) {
				 		labelline << ":pop=" << fname;
				 	}
				}
			}
			if ((kcount > 0) && (vindex >= 0))  {
				int tcount = kcount - vindex - 1;
				for (int j=0; j<tcount; j++) {
					labelline << "\t!";
				}
			}
		}
	}

	stringstream dissonanceline;
	if (foundDissonanceQ) {
		for (int i=0; i<fcount; i++) {
			HTp token = infile.token(index, i);
			int track = token->getTrack();
			int vindex = m_trackToVoiceIndex.at(track);
			if (i != 0) {
				dissonanceline << "\t";
			}
			string dissonance = dissonances.at(i);
			if (dissonance.empty()) {
				dissonanceline << "!";
			} else {
				dissonanceline << "!LO:TX:a:diss";
				dissonanceline << ":color=" << m_suspensionColor;
				dissonanceline << ":t=" << dissonance;
				if (m_popupQ) {
				 	string dname = getDissonanceNames(dissonance);
				 	if (!dname.empty()) {
				 		dissonanceline << ":pop=" << dname;
				 	}
				}
			}
			if ((kcount > 0) && (vindex >= 0))  {
				int tcount = kcount - vindex - 1;
				for (int j=0; j<tcount; j++) {
					dissonanceline << "\t!";
				}
			}
		}
	}

	if (!dissonanceline.str().empty()) {
		m_humdrum_text << dissonanceline.str() << endl;
	}
	if (!labelline.str().empty()) {
		m_humdrum_text << labelline.str() << endl;
	}
	if (!dataline.str().empty()) {
		m_humdrum_text << dataline.str() << endl;
	}
}



//////////////////////////////
//
// Tool_cadence::getDissonanceNames --
//

string Tool_autocadence::getDissonanceNames(const string& input) {
	HumRegex hre;
	string output;
	vector<string> pieces;
	hre.split(pieces, input, "[^A-Za-z]");
	map<string, bool> found;
	int counter = 0;
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].empty()) {
			continue;
		}
		if (found[pieces[i]] == true) {
			continue;
		}
		found[pieces[i]] = true;
		string name = m_dissonanceNames[pieces[i]];
		if (name.empty()) {
			name = pieces[i];
		}
		if (counter > 0) {
			output += ", ";
		}
		output += name;
	}
	return output;
}



//////////////////////////////
//
// Tool_autocadence::getFunctionNames -- convert CVF name list into a popup title string.
//     The names can be postfixed with the cadence formula index number.
//        Example: C73,C29
//     In this case the output will be "Cantizans".  
//        Example: C73,T29
//     In this case the output will be "Cantizans, Tenorzans".
//

string Tool_autocadence::getFunctionNames(const string& input) {
	HumRegex hre;
	string output;
	vector<string> pieces;
	hre.split(pieces, input, "[^A-Za-z]");
	map<string, bool> found;
	int counter = 0;
	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].empty()) {
			continue;
		}
		if (found[pieces[i]] == true) {
			continue;
		}
		found[pieces[i]] = true;
		string name = m_functionNames[pieces[i]];
		if (name.empty()) {
			name = pieces[i];
		}
		if (counter > 0) {
			output += ", ";
		}
		output += name;
	}
	return output;
}



//////////////////////////////
//
// Tool_autocadence::printIntervalLine --
//

void Tool_autocadence::printIntervalLine(HumdrumFile& infile, int index, int kcount, const string& tok) {
	stringstream line;
	int fcount = infile[index].getFieldCount();
	for (int i=0; i<fcount; i++) {
		HTp token = infile.token(index, i);
		if (i != 0) {
			line << "\t";
		}
		line << token;
		if (!token->isKern()) {
			continue;
		}
		int track = token->getTrack();
		int ntrack = 0;
		if (i < fcount - 1) {
			ntrack = infile.token(index, i+1)->getTrack();
		}
		if (track == ntrack) {
			continue;
		}
		if (kcount > 0) {
			int vindex = m_trackToVoiceIndex.at(track);
			int tcount = kcount - vindex - 1;
			for (int j=0; j<tcount; j++) {
				line << "\t" << tok;
			}
		}
	}
	m_humdrum_text << line.str() << endl;
}



//////////////////////////////
//
// Tool_autocadence::preparePitchInfo -- Fill in the m_pitches and m_lowestPitch
//     variables.  These are used to generate the counterpoing interval
//     information later.  Gets the lowest pitch for each sonority in the
//     score.   First extract all diatonic note numbers into a parallel
//     structure to the Humdrum file (for computational efficiency).  This
//     function also store diatonic pitches for each note in the score,
//     in a 2-D array that matches the dimensions of the score (so that
//     the pitches do not have to be recalculated later for generating
//     interval chains for searching for cadence patterns.
//

void Tool_autocadence::preparePitchInfo(HumdrumFile& infile) {
	// First extract the diatonic pitches:
	prepareDiatonicPitches(infile);

	// Now find the lowest sounding pitch for each data row in the file:
	prepareLowestPitches();
}



//////////////////////////////
//
// Tool_autocadence::prepareLowestPitches -- Find the lowest sounding pitch for
//     each data row in the file.  The pitch is given as a non-negative integer,
//     with middle C being 28 (7 * 4).  Rests are 0.
//

void Tool_autocadence::prepareLowestPitches(void) {
	m_lowestPitch.clear();
	m_lowestPitch.resize(m_pitches.size());
	std::fill(m_lowestPitch.begin(), m_lowestPitch.end(), 0);

	for (int i=0; i<(int)m_pitches.size(); i++) {
		int lowest = -1;
		for (int j=0; j<(int)m_pitches[i].size(); j++) {
			// Using abs since negative integers represent sustained notes:
			int b7 = std::abs(m_pitches.at(i).at(j));
			if (b7 > 0) {
				if (lowest == -1) {
					lowest = b7;
				} else if (b7 < lowest) {
					lowest = b7;
				}
			}
		}
		if (lowest < 0) {
			m_lowestPitch.at(i) = 0;
		} else {
			m_lowestPitch.at(i) = lowest;
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareTrackToVoiceIndex --
//

void Tool_autocadence::prepareTrackToVoiceIndex(HumdrumFile& infile, vector<HTp> kspines) {
	m_trackToVoiceIndex.resize(infile.getMaxTrack() + 1);
	fill(m_trackToVoiceIndex.begin(), m_trackToVoiceIndex.end(), -1);
	for (int i=0; i<(int)kspines.size(); i++) {
		int track = kspines[i]->getTrack();
		m_trackToVoiceIndex.at(track) = i;
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareIntervalInfo -- Generate intervals used in analysis.
//      m_pitches and m_lowestSlidePitch variables need to be filled in before
//      calling this function.
//
void Tool_autocadence::prepareIntervalInfo(HumdrumFile& infile) {

	// Initialize space to store counterpoint intervals:
	//    hint_mintL:mintU
	//    hint = harmonic interval between two parts
	//    mintL = melodic interval to next note in lower part
	//    mintH = melodic interval to next note in upper part
	//    Intervals are diatonic in the range from 1 to 8 (8 = octave or two octaves, etc.)

	vector<HTp> kspines = infile.getKernSpineStartList();
	prepareTrackToVoiceIndex(infile, kspines);

	// Initialize space for interval strings:
	m_intervals.clear();
	m_intervals.resize(infile.getLineCount());
	for (int i=0; i<(int)m_intervals.size(); i++) {
		int vcount = (int)kspines.size();
		m_intervals[i].resize(vcount);
		for (int j=0; j<vcount; j++) {
			int pcount = vcount - j - 1;
			m_intervals[i][j].resize(pcount);
			for (int k=0; k<pcount; k++) {
				get<1>(m_intervals[i][j][k]) = NULL;
				get<2>(m_intervals[i][j][k]) = NULL;
			}
		}
	}

	for (int i=0; i<(int)kspines.size()-1; i++) {
		for (int j=i+1; j<(int)kspines.size(); j++) {
			generateCounterpointStrings(kspines, i, j);
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareDiatonicPitches -- Extra the absolute diatonic pitches
//      from the score.  These are stored in m_pitches which is a 2-D array that
//      matches the dimentions of the score.  Middle C is number 28 (7 * 4), rests
//      are 0, and sustained notes are negative pitches.
//

void Tool_autocadence::prepareDiatonicPitches(HumdrumFile& infile) {
	m_pitches.clear();
	m_pitches.resize(infile.getLineCount());
	for (int i=0; i<infile.getLineCount(); i++) {
		m_pitches.at(i).resize(infile[i].getFieldCount());
		std::fill(m_pitches[i].begin(), m_pitches[i].end(), 0);
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			int sign = 1;
			HTp resolve = NULL;
			if (token->isNullToken()) {
				resolve = token->resolveNullToken();
				// Negative pitch value is a sustained note:
				sign = -1;

			} else {
				resolve = token;
			}
			if (resolve == NULL) {
				continue;
			}
			if (resolve->isRest()) {
				continue;
			}

			int pitch = Convert::kernToBase7(resolve);
			pitch *= sign;
			m_pitches.at(i).at(j) = pitch;
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::getPairIndex -- Return the index for the interval pairing
//      info stored in m_intervals for this particular pair of voices (array index
//      add the token level).
//

int Tool_autocadence::getPairIndex(HTp lowerExInterp, HTp upperExInterp) {
	int count = -1;
	HTp current = lowerExInterp->getNextFieldToken();
	while (current) {
		if (*current == "**kern") {
			count++;
		}
		if (current == upperExInterp) {
			break;
		}
		current = current->getNextFieldToken();
	}
	return count;
}



//////////////////////////////
//
// Tool_autocadence::generateCounterpointStrings -- Calculate individual
//    counterpoint intervals strings for a pair of voices.
//
//

void Tool_autocadence::generateCounterpointStrings(vector<HTp>& kspines, int vindexL, int vindexU) {
	// cerr << "Creating counterpoint strings for " << vindexL << " and " << vindexU << endl;

	HTp partLstart = kspines.at(vindexL);
	HTp partUstart = kspines.at(vindexU);

	vector<vector<HTp>> pairings(2);
	pairings[0].reserve(10000);
	pairings[1].reserve(10000);
	fillNotes(pairings[0], partLstart);
	fillNotes(pairings[1], partUstart);

	if (pairings[0].size() != pairings[1].size()) {
		cerr << "Error: length of each note sequence is not equal: "
		     << pairings[0].size() << " compared to " << pairings[1].size() << endl;
		return;
	}

	int track = partLstart->getTrack();
	int voiceIndex = m_trackToVoiceIndex.at(track);
	int pairIndex = getPairIndex(kspines.at(vindexL), kspines.at(vindexU));

	for (int i=0; i<(int)pairings[0].size(); i++) {
		if (!(pairings[0][i]->isNoteAttack() || pairings[1][i]->isNoteAttack())) {
			// both notes are sustaining in this slice.
			// deal with rests here as well, allowing one rest slice between note pairings.
			continue;
		}

		string entry = generateCounterpointString(pairings, i);

		int lineIndex = pairings[0][i]->getLineIndex();

		std::get<0>(m_intervals.at(lineIndex).at(voiceIndex).at(pairIndex)) = entry;
		std::get<1>(m_intervals.at(lineIndex).at(voiceIndex).at(pairIndex)) = pairings[0].at(i);
		std::get<2>(m_intervals.at(lineIndex).at(voiceIndex).at(pairIndex)) = pairings[1].at(i);
	}
}



//////////////////////////////
//
// Tool_autocadence::generateCounterpointString -- Generate the counterpoint interval string
// between a pair of voices in the form: hint{low4th}_mintL:mintU.
//

string Tool_autocadence::generateCounterpointString(vector<vector<HTp>>& pairings, int index) {
	HTp lower = pairings[0][index];
	HTp upper = pairings[1][index];

	// Get pitches of the current pair of notes in the parts:
	int lineIndex   = lower->getLineIndex();
	int fieldIndexL = lower->getFieldIndex();
	int fieldIndexU = upper->getFieldIndex();
	int b7L = m_pitches.at(lineIndex).at(fieldIndexL);
	int b7U = m_pitches.at(lineIndex).at(fieldIndexU);

	// Get pitches for the next pair of notes in the two parts:
	int b7Ln = 0;
	int b7Un = 0;
	if (index < (int)pairings[0].size() - 1) {
		HTp lowern = pairings[0][index+1];
		HTp uppern = pairings[1][index+1];
		int nlineIndex   = lowern->getLineIndex();
		int nfieldIndexL = lowern->getFieldIndex();
		int nfieldIndexU = uppern->getFieldIndex();
		b7Ln = std::abs(m_pitches.at(nlineIndex).at(nfieldIndexL));
		b7Un = std::abs(m_pitches.at(nlineIndex).at(nfieldIndexU));
	}

	// Determine if there is a fourth above the lowest sounding note
	// for the current pair of voices:
	int lowL = 0;
	int lowU = 0;
	int lowest = m_lowestPitch.at(lineIndex);
	if (lowest == 0) {
		// do nothing
	} else {
		if (b7L != 0) {
			lowL = getDiatonicInterval(lowest, b7L);
		}
		if (b7U != 0) {
			lowU = getDiatonicInterval(lowest, b7U);
		}
	}
	string dissonant4;
	if ((lowL == 4) || (lowU == 4)) {
		dissonant4 = "D";
	}

	string dissonanceU;
	string dissonanceL;
	if (m_suspensionsQ) {
		dissonanceL = lower->getValue("auto", "dissonance");
		dissonanceU = upper->getValue("auto", "dissonance");
cerr << "DISSONANCES: " << dissonanceL << "\t" << dissonanceU << endl;
	}

	string mintL = "R";
	string mintU = "R";

	string hint  = getDiatonicIntervalString(b7L, b7U);
	if (index < (int)pairings[0].size() - 1){
		mintL = getDiatonicIntervalString(b7L, b7Ln);
		mintU = getDiatonicIntervalString(b7U, b7Un);
	}

	if ((mintL == "R") || (mintU == "R")) {
		dissonant4 = "";
	}

	string output = hint;
	output += dissonant4;
	output += "_";
	output += mintL;
	output += ":";
	output += mintU;

	return output;
}



//////////////////////////////
//
// Tool_autocadence::getDiatonicIntervalString --
//    lower is the lower staff note, or the first note in a melodic sequence.
//    Both numbers are diagonic pitch numbers (28 = middle C).
//

string Tool_autocadence::getDiatonicIntervalString(int lower, int upper) {
	if ((lower == 0) || (upper == 0)) {
		return "R";
	} else {
		int interval = getDiatonicInterval(lower, upper);
		return to_string(interval);
	}
}



//////////////////////////////
//
// Tool_autocadence::getDiatonicInterval -- A negative pitch number means sustained note.
//     upper = pitch from higher staff
//     lower = pitch from lower staff
//     Or for melodic calculations:
//         lower = first pitch (or rest)
//         upper = second pitch (or rest)
//

int Tool_autocadence::getDiatonicInterval(int lower, int upper) {
	int interval = abs(upper) - abs(lower);
	if (interval > 7) {
		interval = interval % 7;
		if (interval == 0) {
			interval = 8;
		} else {
			interval++;
		}
	} else if (interval < 0) {
		interval = -interval % 7;
		if (interval == 0) {
			interval = -8;
		} else {
			interval = -interval -1;
		}
	} else {
		interval++;
	}
	return interval;
}



//////////////////////////////
//
// Tool_autocadence::fillNotes --
//

void Tool_autocadence::fillNotes(vector<HTp>& voice, HTp exinterp) {
	HTp current = exinterp->getNextToken();
	while (current) {
		if (current->isData()) {
			voice.push_back(current);
		}
		current = current->getNextToken();
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareCadenceDefinitions --
//

void Tool_autocadence::prepareCadenceDefinitions(void) {
	m_definitions.clear();
	m_definitions.reserve(200);

	// /* Index */                 LowserCVF, UpperCVF, Name, Regex
	/*   0 */ addCadenceDefinition("", "",		"__1",	R"(^(?:-?\d+|R)_1:-?\d+, 7_1:-2, 6_R:-2, R_)");
	/*   1 */ addCadenceDefinition("", "",		"__2",	R"(^[^R]_1, 2_1:-2, (?:1|8)_-3:2, 4D?_)");
	/*   2 */ addCadenceDefinition("", "",		"__3",	R"(^[^R]_1:, 4D_1:-2, 3_1:-2, 2_1:2, 3_-3:2, 6_)");
	/*   3 */ addCadenceDefinition("", "",		"__4",	R"(^[^R]_1:-?\d+, 7_1:-2, 6_-2:2, 8_)");
	/*   4 */ addCadenceDefinition("A", "T",	"AT1",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_2:1, -3_2:-2, -5_)");
	/*   5 */ addCadenceDefinition("A", "T",	"AT2",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_3:-2, -5_)");
	/*   6 */ addCadenceDefinition("A", "T",	"AT3",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-2, -5_)");
	/*   7 */ addCadenceDefinition("A", "T",	"AT4",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_2:-2, -5_)");
	/*   8 */ addCadenceDefinition("B", "C",	"BC1",	R"("^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_4:2, (?:1|8)_)");
	/*   9 */ addCadenceDefinition("B", "C",	"BC2",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]), 4D_1:-2, 3_1:-2, 2_1:2, 3_1:1, 3_-5:2, 8_)");
	/*  10 */ addCadenceDefinition("B", "C",	"BC3",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-5:2, 8_)");
	/*  11 */ addCadenceDefinition("B", "C",	"BC4",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_-5:3, 8_)");
	/*  12 */ addCadenceDefinition("B", "C",	"BC5",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-5:2, 8_)");
	/*  13 */ addCadenceDefinition("B", "C",	"BC6",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_-5:2, 8_)");
	/*  14 */ addCadenceDefinition("B", "C",	"BC7",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-5:3, 8_)");
	/*  15 */ addCadenceDefinition("B", "C",	"BC8",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_-5:2, 8_)");
	/*  16 */ addCadenceDefinition("B", "C",	"BC9",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_4:2, (?:8|1)_)");
	/*  17 */ addCadenceDefinition("B", "C",	"BC10",	R"(^3_1:2, 4D_1:-2, 3_(?:4|-5):2, (?:1|8)_)");
	/*  18 */ addCadenceDefinition("B", "C",	"BC11",	R"(^5_1:-2, 4D_1:-2, 3_(?:4|-5):2, (?:1|8)_)");
	/*  19 */ addCadenceDefinition("B", "c",	"Bc1",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_(?:4|-5):(?:4|-5), 3_)");
	/*  20 */ addCadenceDefinition("C", "B",	"CB1",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_2:1, -3_2:(?:-5|4), -8_)");
	/*  21 */ addCadenceDefinition("C", "B",	"CB2",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_2:1, -3_2:4, (?:1|-8)_)");
	/*  22 */ addCadenceDefinition("C", "B",	"CB3",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_3:-5, -8_)");
	/*  23 */ addCadenceDefinition("C", "B",	"CB4",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_3:4, (?:1|-8)_)");
	/*  24 */ addCadenceDefinition("C", "B",	"CB5",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-5, -8_)");
	/*  25 */ addCadenceDefinition("C", "B",	"CB6",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_1:1, -3_-2:1, -2_3:4, (?:1|-8)_)");
	/*  26 */ addCadenceDefinition("C", "B",	"CB7",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_2:-5, -8_)");
	/*  27 */ addCadenceDefinition("C", "B",	"CB8",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_2:4, -8_)");
	/*  28 */ addCadenceDefinition("C", "B",	"CB9",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_2:4, 1_)");
	/*  29 */ addCadenceDefinition("C", "Q",	"CQ1",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_2:(?:-5|4), 5_)");
	/*  30 */ addCadenceDefinition("C", "T",	"CT1",	R"(^(?:-?\d+|R)_1:-?\d+, -7_-2:1, -6_-2:1, -5_2:1, -6_2:-2, -8_)");
	/*  31 */ addCadenceDefinition("C", "T",	"CT2",	R"(^(?:-?\d+|R)_1:-?\d+, -7_-2:1, -6_-2:1, -5_3:-2, -8_)");
	/*  32 */ addCadenceDefinition("C", "T",	"CT3",	R"(^(?:-?\d+|R)_1:-?\d+, -7_-2:1, -6_1:1, -6_-2:1, -5_3:-2, -8_)");
	/*  33 */ addCadenceDefinition("C", "T",	"CT4",	R"(^(?:-?\d+|R)_1:-?\d+, -7_-2:1, -6_2:-2, -8_)");
	/*  34 */ addCadenceDefinition("C", "T",	"CT5",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_-2:1, 4D?_2:1, 3_2:-2, (?:1|8)_)");
	/*  35 */ addCadenceDefinition("C", "T",	"CT6",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_-2:1, 4D?_3:-2, (?:1|8)_)");
	/*  36 */ addCadenceDefinition("C", "T",	"CT7",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_1:1, 3_-2:1, 4D?_2:1, 3_2:-2, (?:1|8)_)");
	/*  37 */ addCadenceDefinition("C", "T",	"CT8",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_1:1, 3_-2:1, 4D?_3:-2, (?:1|8)_)");
	/*  38 */ addCadenceDefinition("C", "T",	"CT9",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_1:1, 3_2:-2, (?:1|8)_)");
	/*  39 */ addCadenceDefinition("C", "T",	"CT10",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_2:-2, (?:1|8)_)");
	/*  40 */ addCadenceDefinition("C", "T",	"CT11",	R"(^3_2:1, 2_-2:1, 3_2:-2, (?:1|8)_)");
	/*  41 */ addCadenceDefinition("C", "T",	"CT11",	R"(^8_-2:1, 2_-2:1, 3_2:-2, (?:1|8)_)");
	/*  42 */ addCadenceDefinition("C", "t",	"ct1",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_-2:1, 4D?_2:1, 3_1:1, 3_2:2, 3_)");
	/*  43 */ addCadenceDefinition("C", "t",	"ct2",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_1:1, 3_-2:1, 4D?_2:1, 3_2:2, 3_)");
	/*  44 */ addCadenceDefinition("C", "t",	"ct3",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_2:2, 3_)");
	/*  45 */ addCadenceDefinition("C", "t",	"ct4",	R"(^3_2:1, 2_-2:1, 3_2:2, 3_)");
	/*  46 */ addCadenceDefinition("C", "u",	"Cu1",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_2:1, -3_2:-3, -6_)");
	/*  47 */ addCadenceDefinition("C", "u",	"Cu2",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:1, -2_3:-3, -6_)");
	/*  48 */ addCadenceDefinition("C", "u",	"Cu3",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_1:1, -3_-2:1, -2_3:-3, -6_)");
	/*  49 */ addCadenceDefinition("C", "u",	"Cu4",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_2:-3, -6_)");
	/*  50 */ addCadenceDefinition("C", "z",	"Cz1",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_2:R, R_)");
	/*  51 */ addCadenceDefinition("L", "C",	"LC1",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_8:2, (?:4|-5)_)");
	/*  52 */ addCadenceDefinition("L", "C",	"LC2",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_8:3, (?:4|-5)_)");
	/*  53 */ addCadenceDefinition("L", "C",	"LC3",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_8:2, (?:4|-5)_)");
	/*  54 */ addCadenceDefinition("L", "C",	"LC4",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_8:3, (?:4|-5)_)");
	/*  55 */ addCadenceDefinition("L", "C",	"LC5",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_8:2, (?:4|-5)_)");
	/*  56 */ addCadenceDefinition("L", "C",	"LC6",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_8:2, (?:4|-5)_)");
	/*  57 */ addCadenceDefinition("P", "C",	"PC1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, 1_-4:2, 5_)");
	/*  58 */ addCadenceDefinition("P", "C",	"PC2",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, 8_(?:5|-4):2, 5_)");
	/*  59 */ addCadenceDefinition("P", "C",	"PC3",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, 8_1:1, 8_(?:5|-4):2, 5_)");
	/*  60 */ addCadenceDefinition("Q", "C",	"QC1",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_-5:2, 4D?_)");
	/*  61 */ addCadenceDefinition("Q", "C",	"QC2",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_4:2, -5_)");
	/*  62 */ addCadenceDefinition("Q", "C",	"QC3",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_(?:-5|4):2, 4D?_)");
	/*  63 */ addCadenceDefinition("Q", "C",	"QC4",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_(?:-5|4):2, 4D?_)");
	/*  64 */ addCadenceDefinition("S", "C",	"SC1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, (?:1|8)_-3:2, 4D?_)");
	/*  65 */ addCadenceDefinition("S", "C",	"SC2",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, (?:1|8)_1:1, (?:1|8)_-3:2, 4D?_)");
	/*  66 */ addCadenceDefinition("T", "A",	"TA1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-2:2, 5_)");
	/*  67 */ addCadenceDefinition("T", "A",	"TA2",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-2:2, 5_)");
	/*  68 */ addCadenceDefinition("T", "A",	"TA3",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-2:2, 5_)");
	/*  69 */ addCadenceDefinition("T", "A",	"TA4",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_-2:3, 5_)");
	/*  70 */ addCadenceDefinition("T", "A",	"TA5",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-2:2, 5_)");
	/*  71 */ addCadenceDefinition("T", "A",	"TA6",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_1:1, 3_-2:2, 5_)");
	/*  72 */ addCadenceDefinition("T", "A",	"TA7",	R"(^(?:R_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-2:3, 5_)");
	/*  73 */ addCadenceDefinition("T", "C",	"TC1",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_-2:2, 8_)");
	/*  74 */ addCadenceDefinition("T", "C",	"TC2",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:-2, 5_-2:3, 8_)");
	/*  75 */ addCadenceDefinition("T", "C",	"TC3",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:-2, 5_1:2, 6_-2:2, 8_)");
	/*  76 */ addCadenceDefinition("T", "C",	"TC4",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:-2, 5_1:2, 6_1:1, 6_-2:2, 8_)");
	/*  77 */ addCadenceDefinition("T", "C",	"TC5",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_-2:2, 8_)");
	/*  78 */ addCadenceDefinition("T", "C",	"TC6",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_-2:3, 8_)");
	/*  79 */ addCadenceDefinition("T", "C",	"TC7",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_1:2, 6_-2:2, 8_)");
	/*  80 */ addCadenceDefinition("T", "C",	"TC8",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:2, 7_1:-2, 6_1:-2, 5_-2:3, 8_)");
	/*  81 */ addCadenceDefinition("T", "C",	"TC9",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:2, 7_1:2, 8_1:-3, 6_-2:2, 8_)");
	/*  82 */ addCadenceDefinition("T", "C",	"TC10",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:2, 7_1:2, 8_1:-3, 6_1:-2, 5_-2:3, 8_)");
	/*  83 */ addCadenceDefinition("T", "C",	"TC11",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:3, 8_1:-3, 6_-2:2, 8_)");
	/*  84 */ addCadenceDefinition("T", "C",	"TC12",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-3, 5_1:2, 6_-2:2, 8_)");
	/*  85 */ addCadenceDefinition("T", "C",	"TC13",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:2, 8_1:-3, 6_-2:2, 8_)");
	/*  86 */ addCadenceDefinition("T", "C",	"TC14",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:2, 8_1:-3, 6_1:-2, 5_-2:3, 8_)");
	/*  87 */ addCadenceDefinition("T", "C",	"TC15",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:2, 8_1:-3, 6_1:1, 6_-2:2, 8_)");
	/*  88 */ addCadenceDefinition("T", "C",	"TC16",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_-2:2, (?:1|-8)_)");
	/*  89 */ addCadenceDefinition("T", "C",	"TC17",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:-2, -4D?_-2:3, (?:1|-8)_)");
	/*  90 */ addCadenceDefinition("T", "C",	"TC18",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:-2, -4D?_1:2, -3_-2:2, (?:1|-8)_)");
	/*  91 */ addCadenceDefinition("T", "C",	"TC19",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:1, -3_1:-2, -4D?_-2:3, (?:1|-8)_)");
	/*  92 */ addCadenceDefinition("T", "C",	"TC20",	R"(^6_1:2, 7_1:-2, 6_-2:2, 8_)");
	/*  93 */ addCadenceDefinition("T", "C",	"TC21",	R"(^8_1:-2, 7_1:-2, 6_-2:2, 8_)");
	/*  94 */ addCadenceDefinition("T", "a",	"Ta1",	R"(^(?:-?\d+_-?[^1]):1, 4D_1:-2, 3_-2:-2, 3_)");
	/*  95 */ addCadenceDefinition("T", "c",	"Tc1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_-2:4, 3_)");
	/*  96 */ addCadenceDefinition("T", "y",	"Ty1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_-2:R, R_)");
	/*  97 */ addCadenceDefinition("b", "C",	"bC1",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_2:2, 3_)");
	/*  98 */ addCadenceDefinition("b", "C",	"bC2",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_2:2, 3_)");
	/*  99 */ addCadenceDefinition("b", "C",	"bC3",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_2:2, 3_)");
	/* 100 */ addCadenceDefinition("b", "C",	"bC4",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_2:2, 3_)");
	/* 101 */ addCadenceDefinition("b", "C",	"bC5",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:2, 4D_1:2, 5_1:-3, 3_2:2, 3_)");
	/* 102 */ addCadenceDefinition("b", "C",	"bC6",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_2:2, 3_)");
	/* 103 */ addCadenceDefinition("c", "B",	"cB1",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_(?:4|-5):4, -3_)");
	/* 104 */ addCadenceDefinition("c", "B",	"cB2",	R"(^(?:-?\d+|R)_1:-?\d+, -4D_-2:1, -3_-2:(?:4|-5), (?:-6|3)_)");
	/* 105 */ addCadenceDefinition("c", "T",	"cT1",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_-2:-2, 3_)");
	/* 106 */ addCadenceDefinition("c", "T",	"cT2",	R"(^[^R]_1, 7_1:-2, 6_-2:4, 3_)");
	/* 107 */ addCadenceDefinition("p", "C",	"pC1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_(?:5|-4):2, 3_)");
	/* 108 */ addCadenceDefinition("s", "",		"s_1",	R"(^(?:R_1|-?\d+_-?[^1]):1, 2_1:-2, 8_-2:2, 3_)");
	/* 109 */ addCadenceDefinition("t", "C",	"tC1",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:-2, -4D?_1:2, -3_2:2, -3_)");
	/* 110 */ addCadenceDefinition("t", "C",	"tC2",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_2:2, -3_)");
	/* 111 */ addCadenceDefinition("t", "C",	"tC3",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_1:-2, 5_1:2, 6_2:2, 6_)");
	/* 112 */ addCadenceDefinition("t", "C",	"tC4",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_2:2, 6_)");
	/* 113 */ addCadenceDefinition("u", "C",	"uC1",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-2:1, 4D_-2:2, 6_)");
	/* 114 */ addCadenceDefinition("u", "C",	"uC2",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_-3:2, 6_)");
	/* 115 */ addCadenceDefinition("u", "C",	"uC3",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_-3:3, 6_)");
	/* 116 */ addCadenceDefinition("u", "C",	"uC4",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:-2, 2_1:2, 3_-3:2, 6_)");
	/* 117 */ addCadenceDefinition("u", "C",	"uC5",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_-3:2, 6_)");
	/* 118 */ addCadenceDefinition("u", "C",	"uC6",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_-3:3, 6_)");
	/* 119 */ addCadenceDefinition("u", "C",	"uC7",	R"(^(?:R_1|4D?_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_1:1, 3_1:-2, 2_1:2, 3_-3:2, 6_)");
	/* 120 */ addCadenceDefinition("x", "C",	"xC1",	R"(^(?:-?\d+_-?[^1]):1, 4D_1:-2, 3_R:2, R_)");
	/* 121 */ addCadenceDefinition("x", "C",	"xC2",	R"(^3_1:2, 4D_1:-2, 3_R:2, R_)");
	/* 122 */ addCadenceDefinition("x", "C",	"xC3",	R"(^5_1:-2, 4D_1:-2, 3_R:2, R_)");
	/* 123 */ addCadenceDefinition("x", "c",	"xc1",	R"(^(?:R_1|4_1|-?\d+_-?[^1]):1, 4D_1:-2, 3_R:-2, R_)");
	/* 124 */ addCadenceDefinition("y", "z",	"yz1",	R"(^(?:-?\d+|R)_1:-?\d+, 2_-2:1, 3_R:R, R_)");
	/* 125 */ addCadenceDefinition("z", "C",	"zC2",	R"(^(?:R_1|7_1|-?\d+_-?[^1]):1, 7_1:-2, 6_1:1, 6_R:2, R_)");
	/* 126 */ addCadenceDefinition("z", "C",	"zC3",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:1, -3_R:2, R_)");
	/* 127 */ addCadenceDefinition("z", "C",	"zC4",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_R:2, R_)");
	/* 128 */ addCadenceDefinition("z", "C",	"zC5",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_R:2, R_)");
	/* 129 */ addCadenceDefinition("z", "c",	"zc1",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_R:-2, R_)");
	/* 130 */ addCadenceDefinition("z", "c",	"zc2",	R"(^(?:R_1|-?\d+_-?[^1]):1, 7_1:-2, 6_R:-2, R_)");
	/* 131 */ addCadenceDefinition("z", "y",	"zy1",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_1:1, -3_R:R, R_)");
	/* 132 */ addCadenceDefinition("z", "y",	"zy2",	R"(^(?:R_1|-?\d+_-?[^1]):1, -2_1:-2, -3_R:R, R_)");
}



//////////////////////////////
//
// Tool_autocadence::addCadenceDefinition --
//

void Tool_autocadence::addCadenceDefinition(const std::string& funcL, const std::string& funcU,
		const std::string& name, const std::string& regex) {
	m_definitions.resize(m_definitions.size() + 1);
	m_definitions.back().setDefinition(funcL, funcU, name, regex);
}



//////////////////////////////
//
// Tool_autocadence::prepareAbbreviations --
//

void Tool_autocadence::prepareAbbreviations(HumdrumFile& infile) {
	vector<HTp> kstarts;
	infile.getKernSpineStartList(kstarts);
	m_abbr.clear();
	m_abbr.resize(kstarts.size());
	for (int i=0; i<(int)kstarts.size(); i++) {
		HTp current = kstarts[i]->getNextToken();
		while (current && !current->isData()) {
			if (current->isInstrumentAbbreviation()) {
				m_abbr.at(i) = (*current).substr(3);
				break;
			}
			current = current->getNextToken();
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareDissonanceNames -- Only labeling suspensions for now.
//

void Tool_autocadence::prepareDissonanceNames(void) {
	m_dissonanceNames.clear();

	m_dissonanceNames.emplace("s", "binary suspension");
	m_dissonanceNames.emplace("S", "ternary suspension");
	m_dissonanceNames.emplace("g", "binary suspension agent");
	m_dissonanceNames.emplace("G", "ternary suspension agent");
}



//////////////////////////////
//
// Tool_autocadence::prepareCvfNames -- Counterpoint voice function names and abbreviations.
//

void Tool_autocadence::prepareCvfNames(void) {
	m_functionNames.clear();

	// Realized Cadential Voice Functions (uppercase letters):
	m_functionNames.emplace("C", "Cantizans");
	m_functionNames.emplace("A", "Altizans");
	m_functionNames.emplace("T", "Tenorizans");
	m_functionNames.emplace("B", "Bassizans");
	m_functionNames.emplace("L", "Leaping Contratenor");
	m_functionNames.emplace("P", "Plagal Bassizans");
	m_functionNames.emplace("S", "Sestizans");
	m_functionNames.emplace("Q", "Quintizans");

	// Evaded Cadential Voice Functions (lowercase letters):
	m_functionNames.emplace("c", "Evaded Cantizans");
	m_functionNames.emplace("a", "Evaded Altizans");
	m_functionNames.emplace("t", "Evaded Tenorizans");
	m_functionNames.emplace("b", "Evaded Bassizans");
	m_functionNames.emplace("u", "Evaded Bassizans (down third");

	// Abandoned Cadential Voice Functions (also lowercase letters):
	m_functionNames.emplace("x", "Abandoned Bassizans");
	m_functionNames.emplace("y", "Abandoned Cantizans");
	m_functionNames.emplace("z", "Abandoned Tenorizans");

}



//////////////////////////////
//
// Tool_autocadence::prepareDissonances --
//

void Tool_autocadence::prepareDissonances(HumdrumFile& infile) {
	HumdrumFile dfile;
	stringstream ss;
	ss << infile;
	dfile.readString(ss.str());
   hum::Tool_dissonant dissonant;
	dissonant.run(dfile);
	// cout << dfile;
	int dsize = dfile.getLineCount();
	int isize = infile.getLineCount();
	if (dsize != isize) {
		// number of lines in input/output are expected to be the same.
		cerr << "LINE COUNTS OF FILES FOR DISSONANCE ANALYSIS DO NOT MATCH." << endl;
		m_suspensionsQ = false;
		return;
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			prepareDissonancesForLine(infile[i], dfile[i]);
		}
	}
}



//////////////////////////////
//
// Tool_autocadence::prepareDissonancesForLine -- Transfer dissonance analysis to
//     input file for a single data line.
//

void Tool_autocadence::prepareDissonancesForLine(HumdrumLine& iline, HumdrumLine& dline) {
	vector<HTp> ikern;
	for (int i=0; i<iline.getFieldCount(); i++) {
		HTp token = iline.token(i);
		if (token->isKern()) {
			ikern.push_back(token);
		}
	}

	int kindex = -1;
	for (int i=0; i<dline.getFieldCount(); i++) {
		HTp token = dline.token(i);
		if (token->isKern()) {
			kindex++;
			continue;
		}
		if (token->isDataType("**cdata-rdiss")) {
			if (kindex >= 0) {
				string text = token->getText();
				if (text != ".") {
					ikern.at(kindex)->setValue("auto", "dissonance", text);
				}
			}
		}
	}
}


// END_MERGE

} // end namespace hum



