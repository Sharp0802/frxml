#pragma once
#include <iterator>

namespace frxml
{
    struct char_iterator
    {
    private:
        const char* m_ptr;
        size_t m_size;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = char;
        using pointer           = const char*;
        using reference         = const char&;


        char_iterator(const char* ptr, size_t m_size);


        /// Checks if there is sufficient space greater or equal to specific size
        /// @returns True if there is; otherwise, false
        [[nodiscard]]
        bool reserve(size_t size) const;



        /// Gets reference of the raw pointer
        /// @return A reference of the raw pointer
        reference operator*() const;

        /// Gets a raw pointer of the iterator
        /// @return A raw pointer of the iterator
        pointer operator->() const;



        /// Points the next element
        /// @return A iterator for the next element
        char_iterator& operator++();

        /// Points the next element
        /// @return A iterator for the current element
        char_iterator operator++(int);



        friend bool operator==(const char_iterator& a, const char_iterator& b);

        friend bool operator!=(const char_iterator& a, const char_iterator& b);
    };
}
