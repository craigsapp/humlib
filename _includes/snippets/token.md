<span class="title-snippet">Access a particular token.</span>

Access the token in the second field of the fourth line (`12e` in
the above example). This can be done in two ways: either address
the HumdrumLine by the [] operator of HumdrumFile and then the data
files with 
<span class="mhcf noc dot paren">HumdrumLine::token</span>, or access 
the <span class="class-HumdrumToken">HumdrumToken</span> directly
from the 
<span class="mhcf noc dot paren">HumdrumFile::token</span> function
directly from the 
<span class="class-HumdrumFile">HumdrumFile</span> class,
giving the line and field index as arguments.

```cpp
infile[3].token(1);    // 12e
infile.token(3, 1);    // 12e
```

<span style="cursor:pointer; color:#1e6bb8" class="example1" title='/doc/snippet/example1.html'>example data</span>



