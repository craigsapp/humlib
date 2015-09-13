<span class="title-snippet">Get token string contents.</span>

HumdrumTokens inherit from std::string, so the text of the token
can be accessed in several ways:

```cpp
(std::string)infile.token(3, 1);   // "12e"
infile.token(3, 1).c_str();        // "12e"
```

