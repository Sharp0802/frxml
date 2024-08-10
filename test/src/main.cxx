#include <iostream>
#include <frxml.h>

int main()
{
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
    std::cout << "================================\n";
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
}
