#ifndef FRXML_LIBRARY_H
#define FRXML_LIBRARY_H

#include <map>
#include <string>
#include <vector>

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

    using attrmap = std::map<const std::string_view, const std::string_view>;

    class dom
    {
        friend class domparser;
        friend class doc;

        int              m_type;
        std::string_view m_tag;
        std::string_view m_content;
        attrmap          m_attr;
        std::vector<dom> m_children;

        void tostring(std::stringstream& ss, int indent) const;

    public:
        [[nodiscard]]
        int type() const;

        [[nodiscard]]
        std::string_view tag() const;

        [[nodiscard]]
        const attrmap& attr() const;

        [[nodiscard]]
        const std::vector<dom>& children() const;
    };

    class doc
    {
        friend class domparser;

        error m_error;
        dom   m_root;

    public:
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
