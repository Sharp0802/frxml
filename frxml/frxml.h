#ifndef FRXML_LIBRARY_H
#define FRXML_LIBRARY_H

#include <cstdint>
#include <string>
#include <vector>

#define FRXML_EXPORT __attribute__((visibility("default")))

namespace frxml {
  enum class node_type {
    NONE           = 0,
    ELEMENT        = 1,
    ATTR           = 2,
    COMMENT        = ELEMENT | 4,
    PI             = ELEMENT | 8,
    END_OF_ELEMENT = 16
  };

  template<typename T>
  class node_iterator {
    const T *_base;

  public:
    explicit node_iterator(const T *base) : _base(base) {
    }

    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::forward_iterator_tag;

    reference operator*() const { return *_base; }
    pointer operator->() const { return _base; }

    node_iterator &operator++() {
      ++_base;
      if (~_base->_type & T::type) {
        _base = nullptr;
      }
      return *this;
    }

    [[nodiscard]]
    node_iterator operator++(int) {
      node_iterator copy = *this;
      ++*this;
      return copy;
    }

    bool operator==(const node_iterator &rhs) const { return _base == rhs._base; }
    bool operator!=(const node_iterator &rhs) const { return !(*this == rhs); }

    [[nodiscard]]
    static constexpr node_iterator end() {
      return node_iterator(nullptr);
    }
  };

  /**
   * A struct for attribute node.
   */
  struct attr {
    static constexpr auto TYPE = node_type::ATTR;

    node_type type = TYPE;
    std::string_view name;
    std::string_view value;
  };

  /**
   * A struct for element node.
   */
  struct element {
    static constexpr auto TYPE = node_type::ELEMENT;

    /**
     * Type of this node.
     *
     * Can be `ELEMENT`, `COMMENT` or `PI`.
     *
     * Depending on this field,
     * Object can safely cast into corresponded type (`comment` and `pi`).
     */
    node_type type = TYPE;

    /**
     * Tag of this node.
     */
    std::string_view tag;

    /**
     * Gets children of this element.
     *
     * It's undefined behaviour that
     * call this method for `comment` or `pi` object.
     * (See `type` field)
     *
     * @return Iterator for children of this element
     */
    [[nodiscard]]
    FRXML_EXPORT node_iterator<element> children() const;

    /**
     * Gets attributes of this element.
     *
     * It's undefined behaviour that
     * call this method for `comment` or `pi` object.
     * (See `type` field)
     *
     * @return Iterator for attributes of this element
     */
    [[nodiscard]]
    FRXML_EXPORT node_iterator<attr> attrs() const;
  };

  /**
   * A struct for comment node.
   */
  struct comment {
    static constexpr auto TYPE = node_type::COMMENT;

    /**
     * Type of this node.
     *
     * Must be `COMMENT`.
     */
    node_type type = TYPE;

    /**
     * Content of comment.
     */
    std::string_view content;
  };

  /**
   * A struct for processing instruction node.
   */
  struct pi {
    static constexpr auto TYPE = node_type::PI;

    /**
     * Type of this node.
     *
     * Must be `PI`.
     */
    node_type type = TYPE;

    /**
     * Content of processing instruction.
     */
    std::string_view content;
  };

  /**
   * A struct that indicates end of element in byte stream.
   */
  struct end {
    static constexpr auto TYPE = node_type::END_OF_ELEMENT;

    /**
     * Type of this node.
     *
     * Must be `END_OF_ELEMENT`.
     */
    node_type type = TYPE;
  };

  /**
   * Parses xml document.
   *
   * When it fails, It returns zero,
   * and An error message will be placed in `buffer`;
   * Otherwise, node data will be stored in `buffer`.
   *
   * @return
   * Root element of document.
   * Non-zero if parsing succeeded; otherwise, zero.
   */
  FRXML_EXPORT const element *parse(std::string_view str, const std::vector<uint8_t> &buffer);
}

#endif //FRXML_LIBRARY_H
