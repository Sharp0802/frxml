#include "frxml.h"

#define ENSURE_BOUND { if (start >= end) return E_EARLY_EOF; }
#define CHECK_EOF { if (start >= end) return I_EOF; }
#define VIEW(begin, end) std::string_view{ begin, static_cast<size_t>(end - begin) }

char32_t UTF8(const char* str, size_t size, size_t& length)
{
    const unsigned char byte = str[0];

    if (byte < 0x80)
        length = 1;
    else if (byte >> 5 == 0x6)
        length = 2;
    else if (byte >> 4 == 0xE)
        length = 3;
    else if (byte >> 3 == 0x1E)
        length = 4;
    else
        length = 0;

    if (!length || length > size)
        return 0;

    char32_t codepoint = 0;
    switch (length)
    {
        case 1:
            codepoint = byte;
            break;
        case 2:
            codepoint = (str[0] & 0x1F) << 6 | str[1] & 0x3F;
            break;
        case 3:
            codepoint = (str[0] & 0x0F) << 12 | (str[1] & 0x3F) << 6 | str[2] & 0x3F;
            break;
        case 4:
            codepoint = (str[0] & 0x07) << 18 | (str[1] & 0x3F) << 12 | (str[2] & 0x3F) << 6 | str[3] & 0x3F;
            break;
    }

    return codepoint;
}

inline bool IsSpace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

inline void SkipSpace(const char* __restrict& start, const char* __restrict end)
{
    for (; start != end && IsSpace(*start); ++start)
    {
    }
}

inline frxml::error frxml::doc::ParsePI(FRXML_STD_PARAMS)
{
    // get target
    auto begin = start;
    for (; start != end - 1 && !IsSpace(*start) && (start[0] != '?' || start[1] != '>'); ++start)
    {
    }
    ENSURE_BOUND;

    const auto target = VIEW(begin, start);

    // find '?>'
    begin = start;
    for (; start != end - 1 && (start[0] != '?' || start[1] != '>'); ++start)
    {
    }
    ENSURE_BOUND;

    const auto value = VIEW(begin, start);

    m_buffer.emplace_back(std::in_place_type_t<pi>(), target, value);
    size++;

    // skip '?>'
    start += 2;
    return S_OK;
}

inline frxml::error frxml::doc::ParseComment(FRXML_STD_PARAMS)
{
    // get content
    auto begin = start;
    for (; start != end - 1 && (start[0] != '-' || start[1] != '-'); ++start)
    {
    }
    ENSURE_BOUND;

    const auto content = VIEW(begin, start);

    // skip '--'
    start += 2;
    ENSURE_BOUND;

    // skip '>'
    if (*start != '>')
        return E_NO_END;
    start++;
    ENSURE_BOUND;

    m_buffer.emplace_back(std::in_place_type_t<comment>(), content);
    size++;

    return S_OK;
}

inline frxml::error frxml::doc::ParseETag(FRXML_STD_PARAMS, std::string_view* tag)
{
    const auto begin = start;
    for (; start != end && *start != '>' && !IsSpace(*start); ++start)
    {
    }
    ENSURE_BOUND;

    if (const auto etag = VIEW(begin, start); !tag || *tag != etag)
        return E_INVALID_ETAG;

    SkipSpace(start, end);
    // skip '>'
    start++;
    ENSURE_BOUND;

    return I_ETAG;
}

inline frxml::error frxml::doc::ParseMisc(FRXML_STD_PARAMS, std::string_view* tag)
{
    SkipSpace(start, end);
    CHECK_EOF;
    if (*start != '<')
        return E_NO_BEGIN;

    start++;
    ENSURE_BOUND;

    switch (*start)
    {
        case '/':
            return ParseETag(++start, end, size, tag);
        case '?':
            return ParsePI(++start, end, size);
        case '!':
        {
            if (start >= end - 2)
                return E_EARLY_EOF;
            if (start[1] != '-' || start[2] != '-')
                return E_INVALID_SEQ;
            start += 3;
            return ParseComment(start, end, size);
        }
        default:
        {
            start--;
            return E_NO_SUCH;
        }
    }
}

inline frxml::error frxml::doc::ParseMiscVec(FRXML_STD_PARAMS)
{
    error err;
    while ((err = ParseMisc(start, end, size, nullptr)) == S_OK)
    {
    }
    if (err == E_NO_SUCH)
        err = S_OK;
    return err;
}

inline frxml::error frxml::doc::ParseElementLike(FRXML_STD_PARAMS, std::string_view* tag)
{
    SkipSpace(start, end);
    CHECK_EOF;
    if (*start != '<')
        return E_NO_BEGIN;

    start++;
    ENSURE_BOUND;

    switch (*start)
    {
        case '/':
            return ParseETag(++start, end, size, tag);
        case '?':
            return ParsePI(++start, end, size);
        case '!':
        {
            if (start >= end - 2)
                return E_EARLY_EOF;
            if (start[1] != '-' || start[2] != '-')
                return E_INVALID_SEQ;
            start += 3;
            return ParseComment(start, end, size);
        }
        default:
        {
            return ParseElement(start, end, size);
        }
    }
}

inline frxml::error frxml::doc::ParseElement(FRXML_STD_PARAMS)
{
    auto begin = start;
    for (; start != end && !IsSpace(*start) && *start != '>' && *start != '/'; ++start)
    {
    }
    ENSURE_BOUND;

    auto tag = VIEW(begin, start);

    auto local = m_buffer.all().size();
    m_buffer.emplace_back(std::in_place_type_t<element>(), tag, 0);

    size_t localSize = 0;

    while (true)
    {
        SkipSpace(start, end);
        ENSURE_BOUND;

        if (*start == '>')
        {
            start++;
            ENSURE_BOUND;
            break;
        }

        if (*start == '/')
        {
            start++;
            ENSURE_BOUND;
            if (*start != '>')
                return E_NO_END;
            start++;
            ENSURE_BOUND;
            return S_OK;
        }

        // find '='
        begin = start;
        for (; start != end && *start != '='; ++start)
        {
        }
        ENSURE_BOUND;

        auto key = VIEW(begin, start);

        start++;
        ENSURE_BOUND;

        auto quote = *start;
        if (quote != '\'' && quote != '"')
            return E_NO_QUOTE;
        start++;
        ENSURE_BOUND;

        // find quote
        begin = start;
        for (; start != end && *start != quote; ++start)
        {
        }
        ENSURE_BOUND;

        auto value = VIEW(begin, start);

        // skip quote
        start++;
        ENSURE_BOUND;

        m_buffer.emplace_back(std::in_place_type_t<attribute>(), key, value);
        localSize++;
    }

    error err;
    while ((err = ParseElementLike(
        start, end,
        localSize /* do NOT use `local.m_size`: `local` is in non-const vector */,
        &tag /* do NOT use `local.m_tag`: `local` is in non-const vector */)) == S_OK)
    {
    }

    std::get<element>(m_buffer.at_raw(local)).m_size += localSize;

    return err == I_ETAG ? S_OK : err;
}

frxml::doc::doc(std::string_view str) : m_size(0)
{
    auto begin = str.begin();

    if ((m_code = ParseMiscVec(begin, str.end(), m_size)) != S_OK)
        goto ERROR;
    // An element should be placed because all of MISC were taken by ParseMiscVec
    if ((m_code = ParseElementLike(begin, str.end(), m_size, nullptr)) != S_OK)
        goto ERROR;
    if (m_code = ParseMiscVec(begin, str.end(), m_size); m_code != I_EOF)
        goto ERROR;

    m_error = 0;

ERROR:
    m_error = begin - str.begin();
}

bool IsNameStartChar(char32_t ch)
{
    return
        ch == ':' ||
        ch == '_' ||
        ('A' <= ch && ch <= 'Z') ||
        ('a' <= ch && ch <= 'z') ||
        (0x00C0 <= ch && ch <= 0x00D6) ||
        (0x00D8 <= ch && ch <= 0x00F6) ||
        (0x00F8 <= ch && ch <= 0x02FF) ||
        (0x0370 <= ch && ch <= 0x037D) ||
        (0x037F <= ch && ch <= 0x1FFF) ||
        (0x200C <= ch && ch <= 0x200D) ||
        (0x2070 <= ch && ch <= 0x218F) ||
        (0x2C00 <= ch && ch <= 0x2FEF) ||
        (0x3001 <= ch && ch <= 0xD7FF);
}

bool IsNameChar(char32_t ch)
{
    return
        IsNameStartChar(ch) ||
        ch == '-' ||
        ch == '.' ||
        ('0' <= ch && ch <= '9') ||
        ch == 0xB7 ||
        (0x0300 <= ch && ch <= 0x036F) ||
        (0x203F <= ch && ch <= 0x2040);
}

bool ValidateName(std::string_view view)
{
    size_t length;
    for (size_t i = 0; i < view.length(); i += length)
    {
        auto ch = UTF8(view.data() + i, view.length() - i, length);

        if (i == 0 && !IsNameStartChar(ch))
            return false;
        if (!IsNameChar(ch))
            return false;
        if (length <= 0)
            return false;
    }

    return true;
}

bool frxml::doc::validate()
{
    for (auto i = 0; i < m_buffer.all().size(); i++)
    {
        auto& nodeV = m_buffer.at_raw(i);

        std::string_view name;
        switch (nodeV.index())
        {
            case 1:
                name = std::get<attribute>(nodeV).key();
                break;
            case 2:
                name = std::get<element>(nodeV).tag();
                break;
            case 3:
                name = std::get<pi>(nodeV).target();
                break;
            default:
                continue;
        }

        if (!ValidateName(name))
        {
            m_error = i;
            m_code  = E_INVALID_SEQUENCE;
            return false;
        }
    }

    return true;
}

bool frxml::doc::operator!() const
{
    return m_error < 0;
}

frxml::error frxml::doc::exception() const
{
    return m_code;
}

size_t frxml::doc::offset() const
{
    return m_error;
}

decltype(frxml::doc::m_buffer)& frxml::doc::children()
{
    return m_buffer;
}

const decltype(frxml::doc::m_buffer)& frxml::doc::children() const
{
    return m_buffer;
}
