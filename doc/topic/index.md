---
layout: default
breadcrumbs: [
                ['/', 'home'],
                ['/doc', 'documentation'],
                ['/doc/tutorial', 'tutorial'],
                ['/doc/class',    'classes'],
                ['/doc/example',  'examples'],
                ['/doc/snippet',  'snippets'],
                ['/doc/topic', 'topics']
        ]
title: Topics
---


Parsing extensions
==================

Standard Humdrum files are encoded as
[TSV](https://en.wikipedia.org/wiki/Tab-separated_values) (Tab
Separated Values).  The minHumdurm library can also read and write
data in the [CSV](https://en.wikipedia.org/wiki/Comma-separated_values)
(Comma Separate Values) format.  This serves two purposes: (1) for
people who cannot wrap their minds around the concept of a tab
character, and (2) for text processing which may munge tabs
(particularly when copy/pasting Humdrum files into email messages).

Use the HumdrumFile::printCSV() funtion to print Humdrum data in the
CSV format.  The example file in CSV format:

```
**kern,**kern
*M4/4,*M4/4
8C,12d
.,12e
8B,.
.,12f
*,*^
4A,2g,4d
4G,.,4c
*,*v,*v
=,=
*-,*-
```

CSV files can be read into a HumdrumFile object by appending "CSV" to the
various read functions, such as readCSV() and parseCSV().



