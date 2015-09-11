---
layout: default
breadcrumbs: [
                ['/', 'home'],
                ['/doc',          'documentation'],
                ['/doc/snippet',  'snippets'],
                ['/doc/example',  'examples'],
                ['/doc/topic',    'topics'],
                ['/doc/tutorial', 'tutoials'],
                ['/doc/class',    'classes']
        ]
title: Classes in the minHumdrum library
---


<details open>
<summary>
Overview
</summary>

The minHumdrum library consists of several classes that abstract
various functionalities for the parser.  Briefly, the HumdrumToken
class manages individual cells of data, with the HumdrumLine managing
simultaneously occurring tokens, and the HumdrumFile manages the
sequence of lines in a Humdrum score.  A HumdrumFile class is used to
represent a single continuous movement in a musical score.  A class
for managing multiple movements will be added to the code set in
the future.  The figure on the right shows the relationship between
the classes, and a short description of each class is given below:

Here are the classes defined in the minHumdrum library:

<img width="35%" style="display:float; float:right;" src="/images/class-organization.svg">
{% include class/classsummary.html %}

The following figure shows some sample Humdrum file for a musical
score with overlays illustrating how the class structure of a
HumdrumFile interacts with the data.  The blue box represents a
HumdrumFile.  The file primarily consists of an array of HumdrumLines,
indexed from 0.  The HumdrumLine in turn contains a list of
HumdrumTokens, indexed by a field number on the line (which are
tab-separated strings in the text version of a standard Humdrum
file), which is also indexed from 0.

<center>
<img title="class-organization" width="600" src="https://cdn.rawgit.com/craigsapp/minHumdrum/gh-pages/images/humdrum-file.svg">
</center>

A Humdrum file has a two-dimensional organization.  The vertical
dimension gives a time ordering of the data, while the horizontal
dimension gives information about the musical parts.  A Humdrum
"spine" roughly represents a musical "parts" and the minHumdrum
code provides functionality for iterating through parts between
lines.  The minHumdrum code also has a system for iterating through
the spines as "tracks" which are a simplified descriptions of the
parts that is similar to standard MIDI file tracks or MEI layer
elements.  The first spine (i.e., column or part) in the above
example maps directly into the concept of a "track".  The second
spine splits into two subs-pines starting with the `*^` on line 7
and these two subs-pines merge again into a single spine starting
on line 11 with the pair of `*v` tokens.  Notice how the labeling
of the subs-pines differs slightly from how they are labeled as
sub-tracks.

The bottom right portion of the above figure shows the organization
of the file in terms of parts (or staves) in the data.  The parts
are contained in "spines" which start with a data type, such as
`**kern` and end with a data terminator, `*-` (star-dash, not
star-underscore).  Humdrum spines can split into two subs-pines
(and the sub-spines can split into sub-spines) as shown in the
second spine.  A spine is split using the `*^` spine manipulator
and can be merged again with the merge manipulator `*v`.  A related
concept to spines are "tracks".  Tracks are identical to spines if
spines do not split; otherwise, sub-spines and sub-tracks are
described slightly differently.  Sub-spines are described by strings
which indicate the spine manipulator history of the spine.  For
example when spine `2` splits into two sub-spines, the first sub-spine
on the line is labeled `(2)a` and the second sub-spine is 
`(2)b`.  Likewise if the `(2)b` sub-spine splits again, the two sub-spines
would be labeled `((2)b)a` and `((2)b)b`.  Sub-tracks are enumerated
in the order of their left-to-right occurrence on the line regardless
of sub-spine manipulations, so in this example sub-spine `(2)a` is
called sub-track `2.1` and sub-spine `(2)b` is called sub-track
`2.2`.  If the two sub-spines were to switch their order on the
line with the exchange manipulator `*x`, then the sub-track assignments
would reverse such that `(2)b` would be sub-track `2.1` and `(2)a`
would be sub-track `2.2`.  Note that spine and track labels are
indexed from 1 rather than 0.


</details>


&nbsp;


{% include class/classlist.html %}


