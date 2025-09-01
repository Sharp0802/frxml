#pragma once

#include <cstdint>
#include <string>

#include "frxml/string.h"

namespace frxml {
  enum class node_type {
    NONE           = 0,
    ELEMENT        = 1,
    ATTR           = 2,
    COMMENT        = ELEMENT | 4,
    PI             = ELEMENT | 8,
    TEXT           = ELEMENT | 16,
    END_OF_ELEMENT = 32
  };

  enum state {
    OKAY = 0,

    NIL,
    UNEXPECTED_EOF,
    EXPECTED_SPACE,
    EXPECTED_QUOTE
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

    void dump(std::ostream &os) const {
      os << "- ATTR " << name << "=" << value;
    }
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

    union {
      /**
       * Tag of this node.
       */
      std::string_view tag;

      /**
       * Tag of this node (required for compatibility with other node types).
       */
      std::string_view content;
    };

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
    node_iterator<element> children() const;

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
    node_iterator<attr> attrs() const;

    void dump(std::ostream &os) const {
      os << "ELEM " << tag;
    }
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

    void dump(std::ostream &os) const {
      os << "COMMENT " << content;
    }
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

    void dump(std::ostream &os) const {
      os << "PI " << content;
    }
  };

  /**
   * A struct for text node.
   */
  struct text {
    static constexpr auto TYPE = node_type::TEXT;

    /**
     * Type of this node.
     *
     * Must be `TEXT`.
     */
    node_type type = TYPE;

    /**
     * Content of text.
     */
    std::string_view content;

    void dump(std::ostream &os) const {
      os << "TEXT " << content;
    }
  };

  /**
   * A struct that indicates end of element in byte stream.
   */
  struct etag {
    static constexpr auto TYPE = node_type::END_OF_ELEMENT;

    /**
     * Type of this node.
     *
     * Must be `END_OF_ELEMENT`.
     */
    node_type type = TYPE;

    union {
      /**
       * Tag of this node.
       */
      std::string_view tag;

      /**
       * Tag of this node (required for compatibility with other node types).
       */
      std::string_view content;
    };

    void dump(std::ostream &os) const {
      os << "ETAG " << tag;
    }
  };

  template<typename t>
  struct context_type {
    using type = t&;
  };

  template<typename t>
  struct context_type<t*> {
    using type = t*;
  };

  template<typename t>
  using context_type_t = typename context_type<t>::type;

  /**
   * Parses xml document.
   *
   * Returns zero if parsing succeeded;
   * Otherwise, returns non-zero error code.
   *
   * See `state` for more information about error codes.
  *
   * @tparam callback Callback function
   * @tparam context Type of context object
   * @param str String view to parse
   * @param ctx Context object
   *
   * @return Zero if parsing succeeded; Otherwise, non-zero error code.
   */
  template<auto callback, typename context>
  state parse(std::string_view str, context_type_t<context> ctx);

  inline void dump(std::ostream &os, const void *p);

  namespace details {
    inline node_type operator&(node_type lhs, node_type rhs) {
      const auto l = static_cast<std::underlying_type_t<node_type>>(lhs);
      const auto r = static_cast<std::underlying_type_t<node_type>>(rhs);
      return static_cast<node_type>(l & r);
    }

    inline const void *skip_node(const void *p) {
      const auto t = *static_cast<const node_type*>(p);
      if ((t & node_type::ELEMENT) == node_type::ELEMENT) {
        return static_cast<const uint8_t*>(p) + sizeof(element);
      }

      switch (t) {
      case node_type::ATTR:
        return static_cast<const uint8_t*>(p) + sizeof(attr);
      case node_type::END_OF_ELEMENT:
        return static_cast<const uint8_t*>(p) + sizeof(etag);
      default:
        return nullptr;
      }
    }
  }

  inline node_iterator<element> element::children() const {
    using namespace details;

    const void *next = this;
    do {
      next = skip_node(next);
    } while (*static_cast<const node_type*>(next) == node_type::ATTR);

    return node_iterator{static_cast<const element*>(next)};
  }

  inline node_iterator<attr> element::attrs() const {
    using namespace details;

    const void *next = skip_node(this);
    return node_iterator{static_cast<const attr*>(next)};
  }

  inline void dump(std::ostream &os, const void *p) {
    switch (const auto v = *static_cast<const node_type*>(p)) {
    case node_type::NONE:
      os << "NONE";
      break;
    case node_type::ELEMENT:
      static_cast<const element*>(p)->dump(os);
      break;
    case node_type::ATTR:
      static_cast<const attr*>(p)->dump(os);
      break;
    case node_type::COMMENT:
      static_cast<const comment*>(p)->dump(os);
      break;
    case node_type::PI:
      static_cast<const pi*>(p)->dump(os);
      break;
    case node_type::TEXT:
      static_cast<const text*>(p)->dump(os);
      break;
    case node_type::END_OF_ELEMENT:
      static_cast<const etag*>(p)->dump(os);
      break;
    default:
      os << "UNKNOWN '" << static_cast<std::underlying_type_t<node_type>>(v) << '\'';
      break;
    }

    os << '\n';
  }

#define FRXML_INCLUDE_PARSE
#include "parse.inc"
#undef FRXML_INCLUDE_PARSE
}
