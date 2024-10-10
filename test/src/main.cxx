#include <iostream>
#include <frxml.h>

int main()
{
    {
        std::string xml = R"(
<xml0>
    <유니코드 attr="안녕하세요" attr0="1"><xml3 />
<!-- HELLO! -->
    </유니코드>
    <?ユニコードは 好きですか??>
    <xml2 attr="c"/>
</xml0>
)";
        frxml::doc doc{ xml };
        if (!doc)
        {
            auto [code, source] = doc.error();
            std::cout << std::string_view(source - 1, 3) << ',' << code << '\n';
        }
        doc.root().children().push_back(frxml::dom::element("added-elem"));

        std::cout << static_cast<std::string>(doc);
    }
    std::cout << "================================\n";
    {
        auto root = frxml::dom::element("xml0", {
            { "xmlns", "test-xmlns" }
        }, {
            frxml::dom::comment(" Hello, FRXML! "),
            frxml::dom::pcinstr("xml1", "유니코드 유니코드"),
            frxml::dom::element("test-element")
        });
        root.children().push_back(frxml::dom::element("added-elem"));

        frxml::doc doc{ root };
        std::cout << static_cast<std::string>(doc);
    }
}
