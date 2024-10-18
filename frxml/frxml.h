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
        size_t           m_size;

    public:
        explicit element(const std::string_view& tag, size_t size)
            : m_tag(tag), m_size(size)
        {
        }

        [[nodiscard]] const std::string_view& tag() const { return m_tag; }
        [[nodiscard]] size_t                  size() const { return m_size; }
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
        E_INVALID_ETAG,
        E_INVALID_SEQUENCE
    };

    /**
     * @brief Sibling view of DOM
     */
    class node_view
    {
    public:
        class const_iterator
        {
        public:
            using value_type        = const std::variant<comment, attribute, element, pi>;
            using iterator_category = std::forward_iterator_tag;
            using reference         = const value_type&;
            using pointer           = const value_type*;

        private:
            pointer m_ptr;

        public:
            explicit const_iterator(const std::variant<comment, attribute, element, pi>* p) : m_ptr(p)
            {
            }

            reference operator*() const { return *m_ptr; }
            pointer   operator->() const { return m_ptr; }

            const_iterator& operator++()
            {
                if (const auto elem = std::get_if<element>(m_ptr))
                    m_ptr += elem->size();
                m_ptr++;
                return *this;
            }

            const_iterator operator++(int)
            {
                const const_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.m_ptr == b.m_ptr; };
            friend bool operator!=(const const_iterator& a, const const_iterator& b) { return a.m_ptr != b.m_ptr; };
        };

    private:
        const std::variant<comment, attribute, element, pi>* m_node;
        const const_iterator                                 m_end;

        static const_iterator GetEnd(const std::variant<comment, attribute, element, pi>* node)
        {
            if (node->index() != 2)
                return const_iterator{ node + 1 };
            node++;

            for (auto i = 0; i < std::get<element>(*node).size(); i++)
            {
                if (const auto p = std::get_if<element>(node))
                    node += p->size();
                node++;
            }
            return const_iterator{ node };
        }

    public:
        explicit node_view(const std::variant<comment, attribute, element, pi>* node)
            : m_node(node), m_end(GetEnd(node))
        {
        }

        [[nodiscard]] const_iterator begin() const { return const_iterator{ &m_node[1] }; }
        [[nodiscard]] const_iterator end() const { return m_end; }
        [[nodiscard]] const_iterator cbegin() const { return begin(); }
        [[nodiscard]] const_iterator cend() const { return end(); }
    };

    /**
     * @brief Descendants view of DOM
     */
    class node_descendants_view
    {
    public:
        using const_iterator = const std::variant<comment, attribute, element, pi>*;

    private:
        const std::variant<comment, attribute, element, pi>* m_node;
        const const_iterator                                 m_end;

    public:
        explicit node_descendants_view(const std::variant<comment, attribute, element, pi>* node)
            : m_node(node), m_end(node_view{ node }.end().operator->())
        {
        }

        [[nodiscard]] const_iterator begin() const { return const_iterator{ &m_node[1] }; }
        [[nodiscard]] const_iterator end() const { return m_end; }
        [[nodiscard]] const_iterator cbegin() const { return begin(); }
        [[nodiscard]] const_iterator cend() const { return end(); }
    };

    class node_vector
    {
        friend class doc;

        std::vector<std::variant<comment, attribute, element, pi>> m_data;

        /**
         * @brief Emplace new item to inner vector
         * @tparam Args Non-argument pack parameter for forwarding arguments to inner vector
         * @param args Arguments that are forwarded to inner vector
         * @return Item constructed with given arguments
         */
        template<typename... Args>
        decltype(m_data)::reference emplace_back(Args&&... args)
        {
            return m_data.emplace_back(std::forward<Args>(args)...);
        }

        /**
         * Gets item by raw index
         * @param index Index of item
         * @return Item that is at given index
         */
        decltype(m_data)::reference at_raw(size_t index)
        {
            return m_data[index];
        }

    public:
        /*
         * NOTE: All methods should be *const*: Some structures depends on raw address of `m_data`
         */

        /**
         * @brief Gets raw list of all elements
         * @return Raw list of all elements
         */
        [[nodiscard]] const decltype(m_data)& all() const { return m_data; }

        /**
         * @brief Gets all siblings of the element at zero index
         * @return All siblings of the element at zero index
         */
        [[nodiscard]] node_view view() const { return node_view{ &m_data[0] }; }

        /**
         * @brief Gets all descendants of the element at zero index
         * @return All descendants of the element at zero index
         */
        [[nodiscard]] node_descendants_view descendants_view() const { return node_descendants_view{ &m_data[0] }; }
    };

#define FRXML_STD_PARAMS const char* __restrict& start, const char* __restrict end, size_t& size

    class doc
    {
        node_vector m_buffer;
        size_t      m_size;
        error       m_code;
        ptrdiff_t   m_error;

        [[nodiscard]] error ParsePI(FRXML_STD_PARAMS);

        [[nodiscard]] error ParseComment(FRXML_STD_PARAMS);

        [[nodiscard]] error ParseETag(FRXML_STD_PARAMS, std::string_view* tag);

        [[nodiscard]] error ParseMisc(FRXML_STD_PARAMS, std::string_view* tag);

        [[nodiscard]] error ParseMiscVec(FRXML_STD_PARAMS);

        [[nodiscard]] error ParseElement(FRXML_STD_PARAMS);

        [[nodiscard]] error ParseElementLike(FRXML_STD_PARAMS, std::string_view* tag);

    public:
        /**
         * @brief Parses xml string into DOM
         * @param str A string-view of xml string
         */
        FRXML_EXPORT explicit doc(std::string_view str);

        /**
         * @brief Validates this document
         * @return True if the document is valid
         */
        [[nodiscard]] FRXML_EXPORT bool validate();

        /**
         * @brief Determines if this document can be used
         * @return True if it cannot be used, otherwise false
         */
        FRXML_EXPORT bool operator!() const;

        /**
         * @brief Gets error code
         * @return 0 or greater than 0 when job completed successfully, otherwise less than 0
         */
        [[nodiscard]] FRXML_EXPORT error exception() const;

        /**
         * Gets offset of source that occurs error during job
         * @return Offset of source (source string for parsing, source node for validation)
         */
        [[nodiscard]] FRXML_EXPORT size_t offset() const;

        /**
         * @brief Gets all descendants of this document
         * @return Descendants of this document
         */
        FRXML_EXPORT decltype(m_buffer)& children();

        /**
         * @copydoc children
         */
        [[nodiscard]] FRXML_EXPORT const decltype(m_buffer)& children() const;
    };
}

#endif //FRXML_LIBRARY_H
