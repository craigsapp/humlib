<span class="title-snippet">Ticks per quarter note</span>

When converting Humdrum files into MIDI, MuseData, MusicXML or
SKINI, the function HumdrumFile::tpq (ticks per quarter note) will
return the minimum number of fractional time units necessary to
describe all rhythms in the file as integer durations.

```cpp
infile.tpq();           // 6 = minimum time unit is a triplet sixteenth note for example
```

In the case of the musical example further above, the smallest
duration is a triplet eighth note, but the minimum time unit between
both parts is a triplet sixteenth note when considering the
polyrhythmic interaction between the parts.

