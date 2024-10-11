#include "chariterator.h"

namespace frxml
{
    char_iterator::char_iterator(const char* ptr, size_t size)
    : m_ptr(ptr), m_size(size)
    {
    }

    bool char_iterator::reserve(size_t size) const
    {
        return m_size >= size;
    }

    size_t char_iterator::skip(size_t size)
    {
        size = std::min(size, m_size);
        m_ptr += size;
        m_size -= size;
        return size;
    }

    char_iterator::reference char_iterator::operator*() const
    {
        return *m_ptr;
    }

    char_iterator::pointer char_iterator::operator->() const
    {
        return m_ptr;
    }

    char_iterator& char_iterator::operator++()
    {
        m_ptr++;
        m_size--;
        return *this;
    }

    char_iterator char_iterator::operator++(int)
    {
        const char_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const char_iterator& a, const char_iterator& b)
    {
        return a.m_ptr == b.m_ptr;
    }

    bool operator!=(const char_iterator& a, const char_iterator& b)
    {
        return !(a == b);
    }
}
