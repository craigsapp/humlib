
The <span class="class-HumdrumFile">HumdrumFile</span> class is used to 
store data from a Humdrum file, or a data string in Humdrum syntax. 


To load content from a file, use the 
<span class="mhcf paren noc dot">HumdrumFileBase::read</span> 
function.  The following program loads content from a file 
called "file.krn" and then prints the content (as a standard 
Humdrum file) to console out.

```cpp
#include "minhumdrum.h"

int main(void) {
	minHumdrum::HumdrumFile infile;
	infile.read("file.krn");
	std::cout << infile;
	return 0;
}
```


The minHumdrum namespace can be omitted by adding `using namespace minHumdrum;`
at the top of the file:

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	infile.read("file.krn");
	cout << infile;
	return 0;
}
```

The HumdrumFile class can also read from standard input as well as
input file streams or `stringstreams`.


```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	infile.read(cin);
	cout << infile;
	return 0;
}
```

Data can be read from an `ifstream` (or `stringstream`):

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	ifstream instream;
	instream.open("file.krn");
	infile.read(instream);
	cout << infile;
	return 0;
}
```

To read content from a string, use the 
<span class="mhcf paren">HumdrumFileBase::readString</span> 
function (since a string is interpreted as a filename in the
read function).

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	string contents;
	contents = "**kern\n1c\n*-";
	infile.readString(contents);
	cout << infile;
	return 0;
}
```

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	const char* contents = "**kern\n1c\n*-";
	infile.readString(contents);
	cout << infile;
	return 0;
}
```

More information about reading data can be found in the 
<span class="ref-read">reference manual</span>.




