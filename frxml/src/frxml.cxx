#include "frxml.h"

#include <iostream>
#include <locale>
#include <utility>
#include <vector>
#include <sstream>
#include <cstring>

#include "chariterator.h"
#include "domparser.h"

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
    if (etag != dom.m_tag.view())
        return E_INVETAG;

    return S_OK;
}
