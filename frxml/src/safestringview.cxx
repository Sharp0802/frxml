#include "frxml.h"

frxml::safestringview::safestringview() = default;

frxml::safestringview::safestringview(const std::string& str) : m_content(std::make_shared<std::string>(str))
{
}

frxml::safestringview::safestringview(std::string_view view) : m_content(view)
{
}

std::string_view frxml::safestringview::view() const
{
    if (m_content.index())
        return std::get<std::string_view>(m_content);

    const auto ptr = std::get<std::shared_ptr<std::string>>(m_content);
    return { ptr->data(), ptr->size() };
}

bool std::less<frxml::safestringview>::operator()(
    const frxml::safestringview& lhs,
    const frxml::safestringview& rhs) const
{
    return lhs.view() < rhs.view();
}
