<span class="title-snippet">Token/line/score durations in ticks per quarter note.</span>

Durations can be expressed in ticks by giving the tpq value as an
argument to the duration functions:

```cpp
int tpq = infile.tpq();                           // 6 ticks per quarter note
infile.token(3, 1).getDuration(tpq).toInteger();  // 2 ticks for a triplet 8th note
infile[3].getDuration(tpq).toInteger();           // 1 tick for a triplet 16th note
infile.getDuration(tpq).toInteger();              // 18 ticks for 3 quarter notes
```

<span style="cursor:pointer; color:#1e6bb8" class="example1" title='/doc/snippet/example1.html'>example data</span>



