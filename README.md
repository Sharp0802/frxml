# FRXML : Fast/Robust XML parser

FRXML is DOM-based light-weight & robust XML parser, written in C++17.

## Features

- zero-copy/single-pass parsing
- limited XML 1.0

Currently, FRXML doesn't support XML 1.0 specification yet.
Below is implemented XML format with EBNF format:

```ebnf
document ::= element

          CHAR ::= #x9 | #xA | #xD | [#x20-#x7F]
NAME_STARTCHAR ::= ":" | [A-Z] | "_" | [a-z]
     NAME_CHAR ::= NAME_STARTCHAR | "-" | "." | [0-9]

name ::= NAME_STARTCHAR NAME_CHAR*

node ::= element | comment | pcinstr

      element ::= norm_element | empty_element
 norm_element ::= '<' name (S attr)* S? '>' content '</' name '>'
empty_element ::= '<' name (S attr)* S? '/>'
      comment ::= '<!--' '<!--' ((CHAR - '-') | ('-' (CHAR - '-')))* '-->'
      pcinstr ::= '<?' name (S (CHAR* - (CHAR* '?>' CHAR*)))? '?>'
      
content ::= node (S? content)?
```

## Usage

```c++
#include <iostream>
#include <frxml.h>

int main()
{
    std::string xml = R"(
<xml0>
    <xml1 attr="0asdfas" attr0="1"><xml3 />
<!-- HELLO! -->
    </xml1>
    <?test-pcinstr Hello World!?>
    <xml2 attr="c"/>
</xml0>
)";
    frxml::doc doc{ xml };
    if (!doc)
    {
        auto error = doc.error();
        std::cout << std::string_view(error.source - 1, 3) << ',' << error.code << '\n';
    }

    std::cout << static_cast<std::string>(doc);
}
```
