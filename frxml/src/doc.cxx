#include "frxml.h"
#include "chariterator.h"
#include "domparser.h"

#include <sstream>

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

frxml::dom& frxml::doc::root()
{
    return m_root;
}