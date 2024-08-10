#ifndef FRXML_LIBRARY_H
#define FRXML_LIBRARY_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <variant>

namespace frxml
{
    class safestringview
    {
        std::variant<std::shared_ptr<std::string>, std::string_view> m_content;

    public:
        safestringview();

        // NOLINTNEXTLINE(*-explicit-constructor)
        safestringview(const std::string& str);

        // NOLINTNEXTLINE(*-explicit-constructor)
        safestringview(std::string_view view);

        template<int N>
        // NOLINTNEXTLINE(*-explicit-constructor)
        safestringview(const char (&str)[N]) : safestringview(std::string_view{ str, strnlen(str, N) })
        {
        }

        [[nodiscard]]
        std::string_view view() const;
    };
}

template<>
struct std::less<frxml::safestringview>
{
    bool operator()(const frxml::safestringview& lhs, const frxml::safestringview& rhs) const;
};

namespace frxml
{
    enum
    {
        S_OK,

        E_NONAME,

        E_NOEQ,
        E_NOQUOTE,
        E_QUOTENOTCLOSED,

        E_INVCHAR,

        E_NOTAG,
        E_TAGNOTCLOSED,
        E_DUPATTR,
        E_ELEMNOTCLOSED,
        E_INVETAG
    };

    enum
    {
        T_ELEMENT,
        T_COMMENT,
        T_PCINSTR,
    };

    struct error
    {
        int         code;
        const char* source;
    };

    using attrmap = std::map<const safestringview, const safestringview, std::less<safestringview>>;

    class dom
    {
        friend class domparser;
        friend class doc;

        int              m_type;
        safestringview   m_tag;
        safestringview   m_content;
        attrmap          m_attr;
        std::vector<dom> m_children;

        void tostring(std::stringstream& ss, int indent) const;

    public:
        [[nodiscard]]
        int type() const;

        [[nodiscard]]
        safestringview tag() const;

        [[nodiscard]]
        const attrmap& attr() const;

        [[nodiscard]]
        const std::vector<dom>& children() const;


        [[nodiscard]]
        static dom element(const std::string& tag, attrmap attr = {}, std::vector<dom> children = {});

        [[nodiscard]]
        static dom pcinstr(const std::string& target, const std::string& content);

        [[nodiscard]]
        static dom comment(const std::string& content);
    };

    class doc
    {
        friend class domparser;

        error m_error;
        dom   m_root;

    public:
        doc(dom root);

        doc(std::string_view str);

        [[nodiscard]]
        operator std::string() const;

        [[nodiscard]]
        bool operator!() const;

        [[nodiscard]]
        error error() const;

        [[nodiscard]]
        const dom& root() const;
    };
}

#endif //FRXML_LIBRARY_H
