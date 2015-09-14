<span class="title-snippet">Ticks per quarter note</span>

When converting Humdrum files into MIDI, MuseData, MusicXML or
SKINI, the function 
<span class="mhcf noc paren dot">HumdrumFileBase::tpq</span> (ticks per 
quarter note) will return the minimum number of fractional time 
units necessary to describe all rhythms in the file as integer durations.

```cpp
infile.tpq();           // 6 = minimum time unit is a triplet 16th note
```

<span style="cursor:pointer; color:#1e6bb8" class="example1" title='/doc/snippet/example1.html'>example data</span>

In the case of the musical example further above, the smallest
duration is a triplet eighth note, but the minimum time unit between
both parts is a triplet sixteenth note when considering the
polyrhythmic interaction between the parts.



