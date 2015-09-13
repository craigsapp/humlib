<span class="title-snippet">Get starting token in track/spine.</span>

Get the first token in the second spine/track (second `**kern`
token on the first line):

```cpp
infile.getTrackStart(2);
```

Note that this will return a pointer rather than a reference to the token.

