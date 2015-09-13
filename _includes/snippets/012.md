<span class="title-snippet">Get the sub-track number for a token.</span>

```cpp
infile[3].getSubtrack(1);  // 0
```

In this case the spine has not split, so the sub-track assignment
is 0.  If there were a spine split, then the sub-track count would
start at 1 for the first token on the line in a track, the next
token in the spine would be sub-track 2, and so on.

