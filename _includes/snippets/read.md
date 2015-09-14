<span class="title-snippet">Read Humdrum data from a file, string or istream.</span>

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

See also the <span class="tutorial-read">tutorial</span> and the
<span class="ref-read">reference manual</span> for more details
about reading Humdrum data.


