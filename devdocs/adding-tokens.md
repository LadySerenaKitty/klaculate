Adding Tokens
==
With the Token Data System, adding new tokens is a pretty simple task.

What you need:
- token name (no moar than 4 characters or `sizeof(int)`)
- a set of rules for how the token should be placed (see [TokenData.h](/src/util/TokenData.h) for an idea of what you need)
- some code (maybe)

Example Token: pi
=
`pi` was added as the first numeric token.  The commit that happned in is a bit too large to link here, so we'll go over the basics.

First, add the `TokenData` to [TokenData.cpp](/src/util/TokenData.cpp):
```cpp
tdata.insert(std::pair<int, TokenData>(ttype(tsize{'p', 'i', 0, 0}).num,
	TokenData{true, true, token{TOKEN_ADD, 0}, token{TOKEN_ADD, 0}, token{TOKEN_NUMBER, 3.14159...493}}));
```
Second, add the token to the regular expressions in [klac.cpp](/src/klac.cpp):
```cpp
rtoken << "|pi";
```

What is going on here?
With `tdata.insert(....)`, you are adding the new token to the Token Data System.
- `ttype(tsize{'p', 'i', 0, 0}).num` :: Tokens are added by their 4-character identifier defined by the token name you have selected.
In this case, we use the characters for `pi`, but since there are only 2, we have to pad the character array with zeroes.
- `TokenData{true, true,` :: The start of the `TokenData` structure.  These first 2 booleans tell the fixup system how to handle your token.
In this case, both are `true` because we want this token to be seen as a number when looking at it from both sides.
  - The first `true` means it is viewed *as a number* when looking at it from the left side.
  - The second `true` means it is viewed *as a number* when looking at it from the right side.
  - Simply use `false` to make it an operator, instead.
- `token{TOKEN_ADD, 0}, token{TOKEN_ADD, 0}` :: If a token needs to be inserted either **before** or **after** your token, these 2 fields
define what should be added.
-  `token{TOKEN_NUMBER, 3.14159...493}` :: This is the **identity** of the token.  Having an identity isn't important for operator tokens,
but it is very important for number tokens.  This field allows the number to always be output as its token name instead of as a number.
Don't worry, the value field of the token (where the numbers are) will hold the numeric value of your token.
You should always use zero here for operator tokens, since this is ignored for operator tokens.

If you are adding an operator token, you need to add the relevant code in (solver.cpp)[/src/solver.cpp].  Since all the code for the existing
operator tokens is already there, you have a huge set of reference material here.
**A Note On Using `switch()`**: Since `switch(int)` is limited to integer types, and `case` is limited to using constants (no code!), you
may need to convert your token name into a number.  Luckily, this is done for you.  Once you have added your token to the
Token Data System, compiling and running `klaculate` will list all the tokens in the Token Data System:
```txt
37	0, 0, token{#, 1}, token{#, 1}, token{*, 0}
40	1, 0, token{*, 0}, token{#, 1}, token{(, 0}
41	0, 1, token{#, 1}, token{*, 0}, token{), 0}
42	0, 0, token{#, 1}, token{#, 1}, token{*, 0}
43	0, 0, token{#, 0}, token{#, 0}, token{+, 0}
45	0, 0, token{#, 0}, token{#, 0}, token{-, 0}
47	0, 0, token{#, 1}, token{#, 1}, token{/, 0}
94	0, 0, token{#, 1}, token{#, 1}, token{^, 0}
26992	1, 1, token{+, 0}, token{+, 0}, token{#, 3.14159}
```
Here we can see the letters `pi` turn into the value `26992` because they were added as a token type conerted to an integer earlier.
