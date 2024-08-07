#include "frxml.h"

#include <iostream>
#include <locale>
#include <utility>
#include <vector>
#include <sstream>
#include <string.h>

#include "chariterator.h"

namespace frxml
{
    class domparser
    {
    public:
        [[nodiscard]]
        static int parseelementlike(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parsepcinstr(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parsecomment(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parseelement(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);
    };
}

std::string_view rangetoview(frxml::char_iterator beg, frxml::char_iterator end)
{
    return {
        beg.operator->(),
        static_cast<size_t>(end.operator->() - beg.operator->())
    };
}

bool isnamechar(const char ch, int idx)
{
    if (isalpha(ch) || ch == '_' || ch == ':')
        return true;
    return idx && (isdigit(ch) || ch == '.' || ch == '-');
}

void skipspace(frxml::char_iterator& cur, const frxml::char_iterator end)
{
    for (; cur != end && isspace(*cur); ++cur)
    {
    }
}

void getname(frxml::char_iterator& cur, const frxml::char_iterator end)
{
    for (auto i = 0; cur != end && isnamechar(*cur, i); ++cur, ++i)
    {
    }
}

int parseattr(
    frxml::char_iterator&      cur,
    const frxml::char_iterator end,
    std::string_view&          name,
    std::string_view&          value)
{
    auto begin = cur;
    getname(cur, end);
    if (begin == cur)
        return frxml::E_NONAME;
    name = rangetoview(begin, cur);

    if (!cur.reserve(1) || *cur++ != '=')
        return frxml::E_NOEQ;

    if (!cur.reserve(1))
        return frxml::E_NOQUOTE;
    const auto quote = *cur++;
    if (quote != '"' && quote != '\'')
        return frxml::E_NOQUOTE;

    begin = cur;
    for (; cur != end && *cur != quote; ++cur)
    {
    }
    if (cur == end)
        return frxml::E_QUOTENOTCLOSED;
    value = rangetoview(begin, cur);
    ++cur;

    return frxml::S_OK;
}

int parseetag(
    frxml::char_iterator&      cur,
    const frxml::char_iterator end,
    std::string_view&          etag)
{
    if (!cur.reserve(2) || *cur++ != '<' || *cur++ != '/')
        return frxml::E_NOTAG;

    const auto begin = cur;
    getname(cur, end);
    if (begin == cur)
        return frxml::E_NONAME;
    etag = rangetoview(begin, cur);

    skipspace(cur, end);
    if (!cur.reserve(1) || *cur++ != '>')
        return frxml::E_TAGNOTCLOSED;

    return frxml::S_OK;
}

/************************************************/

int frxml::domparser::parseelementlike(char_iterator& cur, const char_iterator end, dom& dom)
{
    skipspace(cur, end);

    if (!cur.reserve(3) || *cur != '<')
        return E_NOTAG;

    auto p = cur.operator->();

    if (memcmp(p, "<!-", 3) == 0)
    {
        dom.m_type = T_COMMENT;
        return parsecomment(cur, end, dom);
    }
    if (memcmp(p, "<?", 2) == 0)
    {
        dom.m_type = T_PCINSTR;
        return parsepcinstr(cur, end, dom);
    }

    dom.m_type = T_ELEMENT;
    return parseelement(cur, end, dom);
}

int frxml::domparser::parsepcinstr(char_iterator& cur, const char_iterator end, dom& dom)
{
    if (!cur.reserve(2) || memcmp(cur.operator->(), "<?", 2) != 0)
        return E_NOTAG;
    for (auto i = 0; i < 2; ++i)
        ++cur;

    auto begin = cur;
    getname(cur, end);
    if (begin == cur)
        return E_NONAME;
    dom.m_tag = rangetoview(begin, cur);

    skipspace(cur, end);

    begin = cur;
    while (true)
    {
        if (!cur.reserve(2))
            return E_TAGNOTCLOSED;
        if (memcmp(cur.operator->(), "?>", 2) == 0)
            break;
        ++cur;
    }
    dom.m_content = rangetoview(begin, cur);

    for (auto i = 0; i < 2; ++i)
        ++cur;
    return S_OK;
}

int frxml::domparser::parsecomment(char_iterator& cur, const char_iterator end, dom& dom)
{
    if (!cur.reserve(4) || memcmp(cur.operator->(), "<!--", 4) != 0)
        return E_NOTAG;
    for (auto i = 0; i < 4; ++i)
        ++cur;

    const auto begin = cur;
    while (true)
    {
        if (!cur.reserve(2))
            return E_TAGNOTCLOSED;
        if (memcmp(cur.operator->(), "--", 2) == 0)
            break;
        ++cur;
    }
    dom.m_content = rangetoview(begin, cur);

    for (auto i = 0; i < 2; ++i)
        ++cur;
    if (!cur.reserve(1) || *cur++ != '>')
        return E_TAGNOTCLOSED;

    return S_OK;
}

int frxml::domparser::parseelement(char_iterator& cur, const char_iterator end, dom& dom)
{
    skipspace(cur, end);
    if (!cur.reserve(1) || *cur++ != '<')
        return E_NOTAG;

    const auto begin = cur;
    getname(cur, end);
    if (begin == cur)
        return E_NONAME;
    dom.m_tag = rangetoview(begin, cur);

    while (cur.reserve(1) && isspace(*cur))
    {
        skipspace(cur, end);
        if (cur == end)
            return E_TAGNOTCLOSED;

        if (isnamechar(*cur, 0))
        {
            std::string_view name, value;
            if (auto ret = parseattr(cur, end, name, value))
                return ret;
            if (dom.m_attr.count(name))
                return E_DUPATTR;
            dom.m_attr.insert({ name, value });
        }
    }

    if (!cur.reserve(1))
        return E_TAGNOTCLOSED;
    const auto empty = *cur == '/';
    if (empty)
        ++cur;
    if (!cur.reserve(1) || *cur++ != '>')
        return E_TAGNOTCLOSED;
    if (empty)
        return S_OK;

    while (cur != end)
    {
        skipspace(cur, end);
        if (!cur.reserve(2))
            return E_ELEMNOTCLOSED;

        if (auto tmp = cur; *tmp++ == '<' && *tmp == '/')
            break;

        auto& child = dom.m_children.emplace_back();
        if (const auto ret = parseelementlike(cur, end, child))
            return ret;
    }

    std::string_view etag;
    if (const auto ret = parseetag(cur, end, etag))
        return ret;
    if (etag != dom.m_tag)
        return E_INVETAG;

    return S_OK;
}

/************************************************/

void frxml::dom::tostring(std::stringstream& ss, int indentc) const
{
    std::string indent(indentc, '\t');

    switch (m_type)
    {
        case T_ELEMENT:
        {
            ss << indent << '<' << m_tag;
            for (auto [key, value]: m_attr)
            {
                const auto quote = value.find('"') == std::string::npos ? '"' : '\'';
                ss << ' ' << key << '=' << quote << value << quote;
            }

            if (m_children.empty())
                ss << '/';
            ss << ">\n";

            for (const auto& m_child: m_children)
                m_child.tostring(ss, indentc + 1);
            if (!m_children.empty())
                ss << indent << "</" << m_tag << ">\n";

            break;
        }
        case T_COMMENT:
            ss << indent << "<!--" << m_content << "-->\n";
            break;
        case T_PCINSTR:
            ss << indent << "<?" << m_tag << ' ' << m_content << "?>\n";
            break;
    }
}

int frxml::dom::type() const
{
    return m_type;
}

std::string_view frxml::dom::tag() const
{
    return m_tag;
}

const frxml::attrmap& frxml::dom::attr() const
{
    return m_attr;
}

const std::vector<frxml::dom>& frxml::dom::children() const
{
    return m_children;
}

frxml::dom frxml::dom::element(const std::string& tag, attrmap attr, std::vector<dom> children)
{
    dom obj;
    obj.m_type = T_ELEMENT;
    obj.m_tag = tag;
    obj.m_attr = std::move(attr);
    obj.m_children = std::move(children);
    return obj;
}

frxml::dom frxml::dom::pcinstr(const std::string& target, const std::string& content)
{
    dom obj;
    obj.m_type = T_PCINSTR;
    obj.m_tag = target;
    obj.m_content = content;
    return obj;
}

frxml::dom frxml::dom::comment(const std::string& content)
{
    dom obj;
    obj.m_type = T_COMMENT;
    obj.m_content = content;
    return obj;
}

frxml::doc::doc(dom root) : m_error(), m_root(std::move(root))
{
}

frxml::doc::doc(std::string_view str) : m_error()
{
    char_iterator beg{ str.data(), str.size() }, end{ str.data() + str.size(), 0 };

    m_root.m_type   = T_ELEMENT;
    const auto code = domparser::parseelement(beg, end, m_root);

    m_error = { code, beg.operator->() };
}

frxml::doc::operator std::string() const
{
    std::stringstream ss;
    m_root.tostring(ss, 0);
    return ss.str();
}

bool frxml::doc::operator!() const
{
    return m_error.code;
}

frxml::error frxml::doc::error() const
{
    return m_error;
}

const frxml::dom& frxml::doc::root() const
{
    return m_root;
}
