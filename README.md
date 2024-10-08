# FRXML : Fast/Robust XML parser

FRXML is DOM-based light-weight & robust XML parser, written in C++17.

## Features

- zero-copy/single-pass parsing
- UTF-8 supported (without BOM)
- limited XML 1.0

Currently, FRXML doesn't support XML 1.0 specification yet.
Below is implemented XML format with EBNF format:

```ebnf
document ::= element

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

### Parsing

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

- Result

```
<xml0>
	<xml1 attr="0asdfas" attr0="1">
		<xml3/>
		<!-- HELLO! -->
	</xml1>
	<?test-pcinstr Hello World!?>
	<xml2 attr="c"/>
</xml0>
```

### Manipulation

```c++
#include <iostream>
#include <frxml.h>

int main()
{
    auto root = frxml::dom::element("xml0", {
        { "xmlns", "test-xmlns" }
    }, {
        frxml::dom::comment(" Hello, FRXML! "),
        frxml::dom::pcinstr("xml1", "test pcinstr"),
        frxml::dom::element("test-element")
    });

    frxml::doc doc{ root };
    std::cout << static_cast<std::string>(doc);
}
```

- Result

```
<xml0 xmlns="test-xmlns">
	<!-- Hello, FRXML! -->
	<?xml1 test pcinstr?>
	<test-element/>
</xml0>
```
