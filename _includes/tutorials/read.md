
Humdrum data can be loaded into the <span class="mhc">HumdrumFile</span> class
in several ways, either from a file or parsed from a text string.  In addition,
<span class="topic-cvs">CSV</span> variants of Humdrum files can be loaded
directly into the HumdrumFile class, as well as
<span class="topic-humdrumxml">HumdrumXML</span> files in the future.


<h2> Reading from a file </h2>

Data can be read through the <span class="mhc">HumdrumFile</span>
constructors.  Either pass
a string containing a filename to read, or an istream to read a previously
opened file (or to read from a stringstream).

<table style="width:auto;">
<tr valign="top"><td>

Reading from filename:

{% highlight cpp %}
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile("file.krn");
	cout << infile;
	return 0;
}
{% endhighlight %}

</td><td>

Reading from istream:

{% highlight cpp %}
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	ifstream instream;
	instream.open("file.krn");
	HumdrumFile infile(instream);
	cout << infile;
	return 0;
}
{% endhighlight %}

</td></tr></table>

The `using namespace` lines at the start of the file allow use of functions
and classes in the minHumdrum library to be used without their full 
namespace scope.  Here is what the code will look like without such
shorthands:


```cpp
#include "minhumdrum.h"

int main(void) {
	minHumdrum::HumdrumFile infile("file.krn");
	std::cout << infile;
	return 0;
}
```

When loading invalid HumdrumData, error messages will be sent to
the console.  You can check that there were no errors when parsing
the Humdrum file contents, by calling the 
<span class="mhcf paren">HumdrumFileBase::isValid</span> function
after reading data.

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile("file.krn");
	if (infile.isValid()) {
		cout << infile;
	} else {
		cout  << "Error reading file.krn" << endl;
		exit(1);
	}
	return 0;
}
```

In addition to initializing the contents of a HumdrumFile through
a constructor, data can be read after the object has been created.

<table style="width:auto;">
<tr valign="top"><td>

Reading from filename:

{% highlight cpp %}
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	infile.read("file.krn");
	cout << infile;
	return 0;
}
{% endhighlight %}

</td><td>

Reading from istream:

{% highlight cpp %}
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
{% endhighlight %}

</td></tr></table>

This allows the object to be filled multiple times.

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
	infile.read("file2.krn");
	cout << infile;
	return 0;
}
```


<h2>Handling parsing errors</h2>

By default reading incorrectly formatted data will cause an error 
message to be sent to standard error.  To suppress these messages, the
<span class="mhcf paren">HumdrumFileBase::setQuietParsing</span>
can be called anytime before parsing the data.  This will prevent
the error message from being sent to standard error.  The function
<span class="mhcf paren">HumdrumFileBase::setNoisyParsing</span>
will return the error reporting method back to the default behavior.


After data has been read, the object can be checked for errors by
calling the
<span class="mhcf paren">HumdrumFileBase::isValid</span>.
This function will return true if there was no parsing error
generated while reading the data; otherwise, false will be returned.
If data is read in quiet mode, the resulting parse
error can be displayed by calling the
<span class="mhcf paren">HumdrumFileBase::getParseError</span>
function as illustrated in the following program:

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
    HumdrumFile infile;
    infile.setQuietParsing();
    infile.read("file.krn");
    if (infile.isValid()) {
        cout << infile;
    } else {
        cout << infile.getParseError() << endl;
    }
    return 0;
}
```




<h2>Reading from a file without parsing rhythms</h2>

The default reading methods parse the rhythmic structure of a score.
If the Humdrum data does not contain rhythmic spines or contains
known irregularities in the rhythmic syntax, 
use <span class="mhcf paren">HumdrumFileStructure::readNoRhythm</span> 
instead.

```cpp
#include "minhumdrum.h"
using namespace minHumdrum;
using namespace std;

int main(void) {
	HumdrumFile infile;
	infile.readNoRhythm("file.krn");
	out << infile;
	return 0;
}
```



<h2> Reading from a string </h2>




<h2> Reading CSV </h2>




