<span class="title-snippet">Get starting token in track/spine.</span>

Get the first token in the second spine/track (second `**kern`
token on the first line):

```cpp
infile.getTrackStart(2);
```

<span style="cursor:pointer; color:#1e6bb8" class="example1" title='/doc/snippet/example1.html'>example data</span>

Note that this will return a pointer rather than a reference to the token.



