#ifndef FRXML_LIBRARY_H
#define FRXML_LIBRARY_H

#include <string>
#include <memory>
#include <variant>
#include <vector>

#define FRXML_EXPORT __attribute__((visibility("default")))

namespace frxml
{
    class comment
    {
        std::string_view m_value;

    public:
        explicit comment(const std::string_view& value) : m_value(value)
        {
        }

        [[nodiscard]] const std::string_view& value() const { return m_value; }
    };

    class attribute
    {
        std::string_view m_key;
        std::string_view m_value;

    public:
        attribute(const std::string_view& key, const std::string_view& value)
            : m_key(key), m_value(value)
        {
        }

        [[nodiscard]] const std::string_view& key() const { return m_key; }
        [[nodiscard]] const std::string_view& value() const { return m_value; }
    };

    class element
    {
        friend class doc;

        std::string_view m_tag;
        size_t           m_start;
        size_t           m_size;

    public:
        explicit element(const std::string_view& tag, size_t start, size_t size)
            : m_tag(tag), m_start(start), m_size(size)
        {
        }

        [[nodiscard]] const std::string_view& tag() const { return m_tag; }
    };

    class pi
    {
        std::string_view m_target;
        std::string_view m_value;

    public:
        explicit pi(const std::string_view& target, const std::string_view& value)
            : m_target(target), m_value(value)
        {
        }

        [[nodiscard]] const std::string_view& target() const { return m_target; }
        [[nodiscard]] const std::string_view& value() const { return m_value; }
    };


    enum error : int8_t
    {
        S_OK = 0,
        I_ETAG,
        I_EOF,

        E_NO_BEGIN = -128,
        E_EARLY_EOF,
        E_INVALID_SEQ,
        E_NO_END,
        E_NO_SUCH,
        E_NO_QUOTE,
        E_INVALID_ETAG
    };

#define STD_PARAMS const char* __restrict& start, const char* __restrict end, size_t& size

    class doc
    {
        std::vector<std::variant<comment, attribute, element, pi>> m_buffer;
        size_t                                                     m_size;
        error                                                      m_code;
        ptrdiff_t                                                  m_error;

        [[nodiscard]] error ParsePI(STD_PARAMS);

        [[nodiscard]] error ParseComment(STD_PARAMS);

        [[nodiscard]] error ParseETag(STD_PARAMS, std::string_view* tag);

        [[nodiscard]] error ParseMisc(STD_PARAMS, std::string_view* tag);

        [[nodiscard]] error ParseMiscVec(STD_PARAMS);

        [[nodiscard]] error ParseElement(STD_PARAMS);

        [[nodiscard]] error ParseElementLike(STD_PARAMS, std::string_view* tag);

    public:
        FRXML_EXPORT explicit doc(std::string_view str);

        FRXML_EXPORT bool operator!() const;

        [[nodiscard]] FRXML_EXPORT error exception() const;

        [[nodiscard]] FRXML_EXPORT size_t offset() const;

        FRXML_EXPORT decltype(m_buffer)& children();

        [[nodiscard]] FRXML_EXPORT const decltype(m_buffer)& children() const;
    };
}

#endif //FRXML_LIBRARY_H
