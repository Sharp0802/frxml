#include "frxml.h"

#include <sstream>

void frxml::dom::tostring(std::stringstream& ss, int indentc) const
{
    std::string indent(indentc, '\t');

    switch (m_type)
    {
        case T_ELEMENT:
        {
            ss << indent << '<' << m_tag.view();
            for (const auto& [key, value]: m_attr)
            {
                const auto quote = value.view().find('"') == std::string::npos ? '"' : '\'';
                ss << ' ' << key.view() << '=' << quote << value.view() << quote;
            }

            if (m_children.empty())
                ss << '/';
            ss << ">\n";

            for (const auto& m_child: m_children)
                m_child.tostring(ss, indentc + 1);
            if (!m_children.empty())
                ss << indent << "</" << m_tag.view() << ">\n";

            break;
        }
        case T_COMMENT:
            ss << indent << "<!--" << m_content.view() << "-->\n";
            break;
        case T_PCINSTR:
            ss << indent << "<?" << m_tag.view() << ' ' << m_content.view() << "?>\n";
            break;
    }
}

int frxml::dom::type() const
{
    return m_type;
}

frxml::safestringview frxml::dom::tag() const
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

frxml::safestringview& frxml::dom::tag()
{
    return m_tag;
}

frxml::attrmap& frxml::dom::attr()
{
    return m_attr;
}

std::vector<frxml::dom>& frxml::dom::children()
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
