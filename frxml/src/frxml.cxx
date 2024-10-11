#include "frxml.h"

#include <iostream>
#include <locale>
#include <utility>
#include <vector>
#include <sstream>
#include <cstring>

#include "chariterator.h"
#include "domparser.h"

constexpr int memcmp_(const char* __restrict a, const char* __restrict b)
{
    for (; *a && *b; a++, b++)
        if (*a != *b)
            return -1;
    return 0;
}

inline std::string_view RangeToView(frxml::char_iterator beg, frxml::char_iterator end)
{
    return {
        beg.operator->(),
        static_cast<size_t>(end.operator->() - beg.operator->())
    };
}

int8_t SizeOfUTF8(frxml::char_iterator& cur)
{
    const auto cp = *cur;

    if ((cp & 0x80) == 0)  // 1-byte character (ASCII)
        return 1;
    if ((cp & 0xE0) == 0xC0)  // 2-byte character
        return 2;
    if ((cp & 0xF0) == 0xE0)  // 3-byte character
        return 3;
    if ((cp & 0xF8) == 0xF0)  // 4-byte character
        return 4;

    return -1;  // Invalid UTF-8
}

int8_t UTF8(frxml::char_iterator cur, char32_t& ch)
{
    const auto size = SizeOfUTF8(cur);
    if (size == -1 || !cur.reserve(size))
        return -1;

    if (size == 1)
    {
        ch = *cur;
        return size;
    }

    ch = *cur & (0xFF >> (size + 1));
    for (int8_t i = 1; i < size; ++i)
    {
        ch <<= 6;
        ch |= (*++cur & 0x3F);  // Mask with 0x3F to get last 6 bits
    }

    return size;
}

bool IsNameStartChar(const char32_t ch)
{
    if (ch == ':' || ch == '_' || isalpha(static_cast<int>(ch)))
        return true;

    return
        (0x000C0 <= ch && ch <= 0x000D6) ||
        (0x000D8 <= ch && ch <= 0x000F6) ||
        (0x000F8 <= ch && ch <= 0x002FF) ||
        (0x00370 <= ch && ch <= 0x0037D) ||
        (0x0037F <= ch && ch <= 0x01FFF) ||
        (0x0200C <= ch && ch <= 0x0200D) ||
        (0x02070 <= ch && ch <= 0x0218F) ||
        (0x02C00 <= ch && ch <= 0x02FEF) ||
        (0x03001 <= ch && ch <= 0x0D7FF) ||
        (0x0F900 <= ch && ch <= 0x0FDCF) ||
        (0x0FDF0 <= ch && ch <= 0x0FFFD) ||
        (0x10000 <= ch && ch <= 0xEFFFF);
}

bool IsNameChar(const char32_t ch, int idx)
{
    if (IsNameStartChar(ch))
        return true;

    if (!idx)
        return false;

    return
        ch == '-' ||
        ch == '.' ||
        isdigit(static_cast<int>(ch)) ||
        ch == 0xB7 ||
        (0x0300 <= ch && ch <= 0x036F) ||
        (0x203F <= ch && ch <= 0x2040);
}

void SkipSpace(frxml::char_iterator& cur, const frxml::char_iterator end)
{
    for (; cur != end && isspace(*cur); ++cur)
    {
    }
}

[[nodiscard]]
bool GetName(frxml::char_iterator& cur, const frxml::char_iterator end)
{
    for (auto i = 0; cur != end; ++i)
    {
        char32_t ch;

        const auto size = UTF8(cur, ch);
        if (size < 0)
            return false;

        if (!IsNameChar(ch, size))
            break;

        cur.skip(size);
    }

    return true;
}

int ParseAttribute(
    frxml::char_iterator&      cur,
    const frxml::char_iterator end,
    std::string_view&          name,
    std::string_view&          value)
{
    auto begin = cur;
    if (!GetName(cur, end))
        return frxml::E_INVSEQ;
    if (begin == cur)
        return frxml::E_NONAME;
    name = RangeToView(begin, cur);

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
    value = RangeToView(begin, cur);
    ++cur;

    return frxml::S_OK;
}

int ParseETag(
    frxml::char_iterator&      cur,
    const frxml::char_iterator end,
    std::string_view&          etag)
{
    if (!cur.reserve(2) || *cur++ != '<' || *cur++ != '/')
        return frxml::E_NOTAG;

    const auto begin = cur;
    if (!GetName(cur, end))
        return frxml::E_INVSEQ;
    if (begin == cur)
        return frxml::E_NONAME;
    etag = RangeToView(begin, cur);

    SkipSpace(cur, end);
    if (!cur.reserve(1) || *cur++ != '>')
        return frxml::E_TAGNOTCLOSED;

    return frxml::S_OK;
}

/************************************************/

int frxml::domparser::ParseMiscVec(char_iterator& cur, char_iterator& end, std::vector<dom>& vec)
{
    int error = 0;
    while (error == S_OK)
        if (dom misc; (error = ParseMisc(cur, end, misc)) == S_OK)
            vec.emplace_back(misc);

    return error == E_SKIPPED ? S_OK : error;
}

int frxml::domparser::ParseMisc(char_iterator& cur, char_iterator end, dom& dom)
{
    SkipSpace(cur, end);

    if (!cur.reserve(3) || *cur != '<')
        return E_NOTAG;

    auto p = cur.operator->();

    if (memcmp_(p, "<!-") == 0)
    {
        dom.m_type = T_COMMENT;
        return ParseComment(cur, end, dom);
    }
    if (memcmp_(p, "<?") == 0)
    {
        dom.m_type = T_PCINSTR;
        return ParsePI(cur, end, dom);
    }

    return E_SKIPPED;
}

int frxml::domparser::ParseElementLike(char_iterator& cur, const char_iterator end, dom& dom)
{
    SkipSpace(cur, end);

    auto p = cur.operator->();
    if (!cur.reserve(3) || *p != '<')
        return E_NOTAG;

    if (memcmp_(p, "<!-") == 0)
    {
        dom.m_type = T_COMMENT;
        return ParseComment(cur, end, dom);
    }
    if (memcmp_(p, "<?") == 0)
    {
        dom.m_type = T_PCINSTR;
        return ParsePI(cur, end, dom);
    }

    dom.m_type = T_ELEMENT;
    return ParseElement(cur, end, dom);
}

int frxml::domparser::ParsePI(char_iterator& cur, const char_iterator end, dom& dom)
{
    if (!cur.reserve(2) || memcmp_(cur.operator->(), "<?") != 0)
        return E_NOTAG;
    for (auto i = 0; i < 2; ++i)
        ++cur;

    auto begin = cur;
    if (!GetName(cur, end))
        return E_INVSEQ;
    if (begin == cur)
        return E_NONAME;
    dom.m_tag = RangeToView(begin, cur);

    SkipSpace(cur, end);

    begin = cur;
    while (true)
    {
        if (!cur.reserve(2))
            return E_TAGNOTCLOSED;
        if (memcmp_(cur.operator->(), "?>") == 0)
            break;
        ++cur;
    }
    dom.m_content = RangeToView(begin, cur);

    for (auto i = 0; i < 2; ++i)
        ++cur;

    return S_OK;
}

int frxml::domparser::ParseComment(char_iterator& cur, const char_iterator end, dom& dom)
{
    if (!cur.reserve(4) || memcmp_(cur.operator->(), "<!--") != 0)
        return E_NOTAG;
    cur.skip(4);

    const auto begin = cur;
    while (true)
    {
        if (!cur.reserve(2))
            return E_TAGNOTCLOSED;
        if (memcmp_(cur.operator->(), "--") == 0)
            break;
        ++cur;
    }
    dom.m_content = RangeToView(begin, cur);

    for (auto i = 0; i < 2; ++i)
        ++cur;
    if (!cur.reserve(1) || *cur++ != '>')
        return E_TAGNOTCLOSED;

    return S_OK;
}

int frxml::domparser::ParseElement(char_iterator& cur, const char_iterator end, dom& dom)
{
    SkipSpace(cur, end);
    if (!cur.reserve(1) || *cur++ != '<')
        return E_NOTAG;

    const auto begin = cur;
    if (!GetName(cur, end))
        return E_INVSEQ;
    if (begin == cur)
        return E_NONAME;
    dom.m_tag = RangeToView(begin, cur);

    while (cur.reserve(1) && isspace(*cur))
    {
        SkipSpace(cur, end);
        if (cur == end)
            return E_TAGNOTCLOSED;

        if (IsNameChar(*cur, 0))
        {
            std::string_view name, value;
            if (auto ret = ParseAttribute(cur, end, name, value))
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
        SkipSpace(cur, end);
        if (!cur.reserve(2))
            return E_ELEMNOTCLOSED;

        if (auto tmp = cur; *tmp++ == '<' && *tmp == '/')
            break;

        auto& child = dom.m_children.emplace_back();
        if (const auto ret = ParseElementLike(cur, end, child))
            return ret;
    }

    std::string_view etag;
    if (const auto ret = ParseETag(cur, end, etag))
        return ret;
    if (etag != dom.m_tag.view())
        return E_INVETAG;

    return S_OK;
}
