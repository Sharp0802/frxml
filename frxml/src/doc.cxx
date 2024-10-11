#include "frxml.h"
#include "chariterator.h"
#include "domparser.h"

#include <sstream>

frxml::doc::doc(std::vector<dom> root) : m_error(), m_children(std::move(root))
{
}

frxml::doc::doc(std::string_view str) : m_error()
{
    char_iterator beg{ str.data(), str.size() }, end{ str.data() + str.size(), 0 };

    dom xmlDecl;
    xmlDecl.m_type = T_PCINSTR;

    auto error = domparser::ParsePI(beg, end, xmlDecl);
    if (error != E_NOTAG && error != S_OK)
    {
        m_error = { error, beg.operator->() };
        return;
    }

    if (error == S_OK)
        m_children.emplace_back(xmlDecl);

    error = domparser::ParseMiscVec(beg, end, m_children);
    if (error != S_OK && error != E_NOTAG)
    {
        m_error = { error, beg.operator->() };
        return;
    }

    auto& root  = m_children.emplace_back();
    root.m_type = T_ELEMENT;
    if ((error = domparser::ParseElement(beg, end, root)) != S_OK)
    {
        m_error = { error, beg.operator->() };
        return;
    }

    error = domparser::ParseMiscVec(beg, end, m_children);
    if (error != S_OK && error != E_NOTAG)
    {
        m_error = { error, beg.operator->() };
        return;
    }
    if (error == E_NOTAG)
        error = S_OK;

    m_error = { error, beg.operator->() };
}

frxml::doc::operator std::string() const
{
    std::stringstream ss;
    for (const auto& child : m_children)
        child.tostring(ss, 0);

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

const std::vector<frxml::dom>& frxml::doc::children() const
{
    return m_children;
}

std::vector<frxml::dom>& frxml::doc::children()
{
    return m_children;
}

const frxml::dom& frxml::doc::root() const
{
    static const dom empty = {};

    for (const auto& child : m_children)
        if (child.m_type == T_ELEMENT)
            return child;

    return empty;
}

frxml::dom& frxml::doc::root()
{
    static dom empty = {};

    for (auto& child : m_children)
        if (child.m_type == T_ELEMENT)
            return child;

    return empty;
}
