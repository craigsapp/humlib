<span class="title-snippet">Get the number of tokens which following next in track/spine.</span>

Ask the starting token how many tokens precede/follow the starting
token in the second spine:

```cpp
HumdrumToken* tok = infile.getTrackStart(2);
tok->getNextTokenCount();           // 1 token following in the spine
tok->getPreviousTokenCount();       // 0 tokens preceding in the spine
tok->getNextToken();                // returns pointer to `*M3/4`, using default value of 0 for argument.
tok->getPreviousToken();            // returns NULL
```

The HumdrumToken::getNextTokenCount() function will return 0 for
the last token in a spine/track (which always must be the characters
`*-` (start-minus) which is the data terminator token.
