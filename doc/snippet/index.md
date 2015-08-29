---
layout: default
breadcrumbs: [
                ['/',             'home'],
                ['/doc',          'documentation'],
                ['/doc/tutorial', 'tutorial'],
                ['/doc/class',    'classes'],
                ['/doc/example',  'examples'],
                ['/doc/snippet',  'snippets']
        ]
title: Code snippets
---

Here are examples of how to access data in a Humdrum score using the
minHumdrum classes:


1: Read Humdrum data from a file, string or istream:

```cpp
HumdrumFile infile;
infile.read(char* filename);
infile.read(string filename);
```
To read content from a char* or string:

```cpp
infile.parse(stringstream content);
infile.parse(istream content);
infile.read(std::cin);
infile.parse(char* content);
infile.parse(string content);
```

2: Access the token in the second field of the fourth line (`12e` in the above example). This can be done in two ways: either address the HumdrumLine by the [] operator of HumdrumFile and then the data files with HumdrumLine::token, or access the HumdrumToken directly from the HumdrumFile::token function, giving the line and field index as arguments.

```cpp
infile[3].token(1);    // 12e
infile.token(3, 1);    // 12e
```

3: HumdrumTokens inherit from std::string, so the text of the token can be accessed in several ways:

```cpp
(std::string)infile.token(3, 1);   // "12e"
infile.token(3, 1).c_str();        // "12e"
```

4: To access the parsed duration of the token, use the HumdrumToken::getDuration function.  The return value of getDuration() is a "HumNum" which is a rational number, or fraction, consisting of an integer numerator and denominator.  The HumNum::getFloat() function will return the duration as a double:

```cpp
infile.token(3, 1).getDuration();              // 1/3    for "12e"
infile.token(3, 1).getDuration().getFloat();   // 0.3333 for "12e"
```

5: HumdrumLines also possess duration:

```cpp
infile[i].getDuration();  // 1/6 (1/6th of a quarter note, due to the polyrhythm between the parts)
```

6: HumdrumFiles also possess duration:

```cpp
infile.getDuration();    // 3 (one measure of 3/4)
```

7: When converting Humdrum files into MIDI, MuseData, MusicXML or SKINI, the function HumdrumFile::tpq (ticks per quarter note) will return the minimum number of fractional time units necessary to describe all rhythms in the file as integer durations.

```cpp
infile.tpq();           // 6 = minimum time unit is a triplet sixteenth note for example
```

In the case of the musical example further above, the smallest duration is a triplet eighth note, but the minimum time unit between
both parts is a triplet sixteenth note when considering the polyrhythmic interaction between the parts.

8: Durations can be expressed in ticks by giving the tpq value as an argument to the duration functions:

```cpp
int tpq = infile.tpq();                           // 6 ticks per quarter note
infile.token(3, 1).getDuration(tpq).toInteger();  // 2 ticks for a triplet eighth note
infile[3].getDuration(tpq).toInteger();           // 1 tick for a triplet sixteenth note
infile.getDuration(tpq).toInteger();              // 18 ticks for three quarter notes
```

9: Get the total number of HumdrumLines in a HumdrumFile:

```cpp
infile.getLineCount();    // 12
```

10: Get the total number of token fields on a HumdrumLine:

```cpp
infile[3].getTokenCount();  // 2
infile[3].getFieldCount();  // 2
```

12: Get the total number of spines/tracks in a HumdrumFile:

```cpp
infile.getMaxTrack();   // 2
infile.getMaxSpine();   // 2
```

11: Get the track number for a token:

```cpp
infile[3].getTrack(1);   // 2 = second track in file.
```

The "1" is the field number for the second token on the 4th line, which is in the second track of the file.

12: Get the sub-track number for a token:

```cpp
infile[3].getSubtrack(1);  // 0
```

In this case the spine has not split, so the sub-track assignment is 0.  If there were a spine split, then
the sub-track count would start at 1 for the first token on the line in a track, the next token in the
spine would be sub-track 2, and so on.

13: Get the first token in the second spine/track (second `**kern` token on the first line):

```cpp
infile.getTrackStart(2);
```

Note that this will return a pointer rather than a reference to the token.

14:  Ask the starting token how many tokens precede/follow the starting token in the second spine:

```cpp
HumdrumToken* tok = infile.getTrackStart(2);
tok->getNextTokenCount();           // 1 token following in the spine
tok->getPreviousTokenCount();       // 0 tokens preceding in the spine
tok->getNextToken();                // returns pointer to `*M3/4`, using default value of 0 for argument.
tok->getPreviousToken();            // returns NULL
```

The HumdrumToken::getNextTokenCount() function will return 0 for the last token in a spine/track (which always must
be the characters `*-` (start-minus) which is the data terminator token.
