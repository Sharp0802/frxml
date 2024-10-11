#ifndef FRXML_LIBRARY_H
#define FRXML_LIBRARY_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <variant>

#define FRXML_EXPORT __attribute__((visibility("default")))

namespace frxml
{
    class safestringview
    {
        std::variant<std::shared_ptr<std::string>, std::string_view> m_content;

    public:
        FRXML_EXPORT safestringview();

        // NOLINTNEXTLINE(*-explicit-constructor)
        FRXML_EXPORT safestringview(const std::string& str);

        // NOLINTNEXTLINE(*-explicit-constructor)
        FRXML_EXPORT safestringview(std::string_view view);

        template<int N>
        // NOLINTNEXTLINE(*-explicit-constructor)
        safestringview(const char (&str)[N]) : safestringview(std::string_view{ str, strnlen(str, N) })
        {
        }

        [[nodiscard]]
        FRXML_EXPORT std::string_view view() const;
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
        E_SKIPPED,

        E_NONAME,

        E_NOEQ,
        E_NOQUOTE,
        E_QUOTENOTCLOSED,

        E_INVCHAR,
        E_INVSEQ,

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

        FRXML_EXPORT void tostring(std::stringstream& ss, int indent) const;

    public:
        [[nodiscard]]
        FRXML_EXPORT int type() const;

        [[nodiscard]]
        FRXML_EXPORT safestringview tag() const;

        [[nodiscard]]
        FRXML_EXPORT const attrmap& attr() const;

        [[nodiscard]]
        FRXML_EXPORT const std::vector<dom>& children() const;

        [[nodiscard]]
        FRXML_EXPORT safestringview& tag();

        [[nodiscard]]
        FRXML_EXPORT attrmap& attr();

        [[nodiscard]]
        FRXML_EXPORT std::vector<dom>& children();


        [[nodiscard]]
        FRXML_EXPORT static dom element(const std::string& tag, attrmap attr = {}, std::vector<dom> children = {});

        [[nodiscard]]
        FRXML_EXPORT static dom pcinstr(const std::string& target, const std::string& content);

        [[nodiscard]]
        FRXML_EXPORT static dom comment(const std::string& content);
    };

    class doc
    {
        friend class domparser;

        error            m_error;
        std::vector<dom> m_children;

    public:
        FRXML_EXPORT doc(std::vector<dom> root);

        FRXML_EXPORT doc(std::string_view str);

        [[nodiscard]]
        FRXML_EXPORT operator std::string() const;

        [[nodiscard]]
        FRXML_EXPORT bool operator!() const;

        [[nodiscard]]
        FRXML_EXPORT error error() const;

        [[nodiscard]]
        FRXML_EXPORT const std::vector<dom>& children() const;

        [[nodiscard]]
        FRXML_EXPORT std::vector<dom>& children();

        [[nodiscard]]
        FRXML_EXPORT const dom& root() const;

        [[nodiscard]]
        FRXML_EXPORT dom& root();
    };
}

#endif //FRXML_LIBRARY_H
