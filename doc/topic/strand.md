

You can uniquely iterate through all tokens in a Humdrum file using
two methods. The simplest one involves iterating first by line and
then by field within each line. Below is a short program that
demonstrates this process by echoing the input Humdrum file contents
in the same format as a standard TSV Humdrum file 

```cpp
#include "humlib.h"
using namespace std;
using namespace hum;
int main(int argc, char** argv) {
	HumdrumFile infile;
	Options options;
	options.process(argc, argv);
	if (options.getArgCount() > 0) {
		infile.read(options.getArg(1));
	} else {
		infile.read(cin);
	}
	for (int i=0; i<infile.getLineCount(); i++) {
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			cout << infile.token(i, j);
			if (j < infile[i].getFieldCount()) {
				cout << '\t';
			}
		}
		cout << '\n';
	}
	return 0;
}
```

You can copy this code to `cli/humecho.cpp` and then in the base
directory of humlib type `make library && make humecho` to compile
to `bin/humecho`.

The lines in the file represent a sequence of data that is sorted
in chronological order.  This row/column iteration method is suitable
for temporal sequence processing of all spine tokens simultaneously
on each line.


To iterate through all spines in an different order, the humlib library
introduces the concept of *strands*. A strand is a sequence of
tokens in a particular spine that does not include spine splits or
merges. The following figure shows example Humdrum data 
with individual strands highlighted in different colors.

![Strand example](strand.svg)



Each spine consists of a primary strand which is continuous
throughout the total length of the spine.  When a spine
splits into sub-spines, a new strand starts at the beginning
of the right-side branch of the split, while the previous
strand continues along the left-side branch.

The strand segments can be used to iterate through all tokens in
the file (excluding non-spine lines, which are global comments,
reference records and empty lines).  A one dimensional iteration
through all tokens is illustrated in the following code:

```cpp

Humdrum infile;
HumdrumToken* tok;
for (int i=0; i<infile.getStrandCount(); i++) {
	tok = infile.getStrandStart(i);
	while (!tok->isStrandEnd()) {
		cout << tok << endl;
		tok->getNextToken();
	}
}

```

The above illustration contains seven 1D strands, so the above code
will generate this data sequence (removing duplicate adjacent tokens in
the sequence):

<span style="font-weight:900">
<span style="color:#F59DB8">a a a a a1 a1 a a a a a1 a1 a a</span>
<span style="color:#F176A3">a2 a2 </span>
<span style="color:#F26451">a2 a2 </span>
<span style="color:#A8CCA3">b b b1 b1 b1 b1 b1 b1 b1,21 b1,21 b b b b </span>
<span style="color:#D7DF23">b2 b2 b21 b21 b21 b21</span>
<span style="color:#39B54A">b22 b22 b22 b22 b22 b22 </span>
<span style="color:#81D4F7">c c c c c c c c c c c c c c</span>
</span>

A two-dimensional iteration through the spine tokens can generate the same
ordering.  In the following example the strands are first iterated through
by spine index, and then by strands in the spines, always starting with
the primary strand.

```cpp
Humdrum infile;
HumdrumToken* tok;
for (int i=0; i<infile.getSpineCount(); i++) {
	for (int j=0; j<infile.getStrandCount(i); j++) {
	tok = infile.getStrandStart(i, j);
	while (!tok->isStrandEnd()) {
		cout << *tok << endl;
		tok->getNextToken();
	}
}
```

When spines do not split or merge, then strands are equivalent to
spines.  The following code examples will generate the same data
ordering:

```cpp
for (i=0; i<infile.getSpineCount(); i++) {
	tok = infile.getSpineStart(i);
	while (!tok->isTerminator()) {
		cout << *tok << endl;
		tok = tok->getNextToken();
	}
}

for (int i=0; i<infile.getStrandCount(); i++) {
	tok = infile.getStrandStart(i);
	while (!tok->isStrandEnd()) {
		cout << *tok << endl;
		tok->getNextToken();
	}
}
```

Primary strands always start with an exclusive interpretation (interpretation
token which starts with \*\* followed by the data type), and are ended with the
terminate manipulator (`*-`).  Secondary strands always start with a 
non-exclusive interpretation, and typically end at a merge manipulator
(`*v`), although unmerged sub-strands will end with a terminate token.




