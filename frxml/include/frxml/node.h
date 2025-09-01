#pragma once

#include <cstdint>

namespace frxml {
  enum class node_type {
    None,
    Element,
    Attribute,
    Comment,
    PI,
    Text,
    ETag
  };

  constexpr const char *to_string(const node_type type) {
    switch (type) {
#define __ENTRY(t) case node_type::t: return #t;
    __ENTRY(None)
    __ENTRY(Element)
    __ENTRY(Attribute)
    __ENTRY(Comment)
    __ENTRY(PI)
    __ENTRY(Text)
    __ENTRY(ETag)
#undef __ENTRY
    default: return nullptr;
    }
  }

  namespace details {
    template<typename T>
    struct NodeIteratorItem;

    template<typename T>
    constexpr node_type get_type();

    constexpr size_t get_size(node_type type);

    constexpr const void *next_node(const void *p);
  }

#define __FIELD(type, name) private: type _##name; public: [[nodiscard]] type name() const { return _##name; }
#define __DEFAULT_NEXT [[nodiscard]] const void *next() const { return this + sizeof(*this); }

  template<typename... T>
  struct Variant {
    __FIELD(node_type, type);

    Variant() : _type(node_type::None) {}

    template<typename To>
    static constexpr bool allows() {
      return (std::is_same_v<To, T> || ...);
    }

    [[nodiscard]]
    constexpr size_t size() const {
      return details::get_size(_type);
    }

    template<typename To>
    [[nodiscard]]
    const To *as() const {
      static_assert(allows<To>(), "Casting target should be one of variadic template argument");

      if (_type != details::get_type<To>())
        return nullptr;

      return reinterpret_cast<const To*>(this);
    }
  };

  template<typename T>
  class NodeIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = typename details::NodeIteratorItem<T>::T;
    using pointer = const value_type*;
    using reference = const value_type&;

  private:
    pointer _base;

  public:
    // NOLINTNEXTLINE(*-explicit-constructor)
    NodeIterator(const void *p) : _base(static_cast<pointer>(p)) {}

    reference operator*() const { return *_base; }
    pointer operator->() { return _base; }

    NodeIterator& operator++() {
      _base = details::next_node(_base);
      if (details::NodeIteratorItem<T>::is_end(_base->type())) {
        _base = nullptr;
      }

      return *this;
    }

    NodeIterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    friend bool operator== (const NodeIterator& a, const NodeIterator& b) { return a._base == b._base; }
    friend bool operator!= (const NodeIterator& a, const NodeIterator& b) { return a._base != b._base; }
  };

  struct Attribute;

  struct Element {
    __FIELD(node_type, type);
    __FIELD(std::string_view, tag);

    // NOLINTNEXTLINE(*-explicit-constructor)
    Element(const std::string_view tag) : _type(node_type::Element), _tag(tag) {
    }

    [[nodiscard]] NodeIterator<Attribute> attrs() const;
    [[nodiscard]] NodeIterator<Element> children() const;
  };

  struct Attribute {
    __FIELD(node_type, type);
    __FIELD(std::string_view, name)
    __FIELD(std::string_view, value)

    Attribute(const std::string_view name, const std::string_view value)
      : _type(node_type::Attribute), _name(name), _value(value) {
    }
  };

  struct Comment {
    __FIELD(node_type, type);
    __FIELD(std::string_view, content);

    // NOLINTNEXTLINE(*-explicit-constructor)
    Comment(const std::string_view content) : _type(node_type::Comment), _content(content) {
    }
  };

  struct PI {
    __FIELD(node_type, type);
    __FIELD(std::string_view, content);

    // NOLINTNEXTLINE(*-explicit-constructor)
    PI(const std::string_view content) : _type(node_type::PI), _content(content) {
    }
  };

  struct Text {
    __FIELD(node_type, type);
    __FIELD(std::string_view, content);

    // NOLINTNEXTLINE(*-explicit-constructor)
    Text(const std::string_view content) : _type(node_type::Text), _content(content) {
    }
  };

  struct ETag {
    __FIELD(node_type, type);
    __FIELD(std::string_view, tag);

    // NOLINTNEXTLINE(*-explicit-constructor)
    ETag(const std::string_view tag) : _type(node_type::ETag), _tag(tag) {
    }
  };

  using Node = Variant<Element, Attribute, Comment, PI, Text, ETag>;

#undef __DEFAULT_NEXT
#undef __FIELD

  namespace details {
    template<>
    struct NodeIteratorItem<Element> {
      using T = Variant<Element, Comment, PI, Text>;

      static constexpr bool is_end(const node_type t) {
        return t == node_type::ETag || t == node_type::None;
      }
    };

    template<>
    struct NodeIteratorItem<Attribute> {
      using T = Attribute;

      static constexpr bool is_end(const node_type t) {
        return t != node_type::Attribute;
      }
    };

    template<typename T>
    constexpr node_type get_type() {
      if constexpr (std::is_same_v<T, Attribute>) return node_type::Attribute;
      else if constexpr (std::is_same_v<T, Element>) return node_type::Element;
      else if constexpr (std::is_same_v<T, Comment>) return node_type::Comment;
      else if constexpr (std::is_same_v<T, PI>) return node_type::PI;
      else if constexpr (std::is_same_v<T, Text>) return node_type::Text;
      else if constexpr (std::is_same_v<T, ETag>) return node_type::ETag;
      else return node_type::None;
    }

    constexpr size_t get_size(const node_type type) {
      switch (type) {
#define __ENTRY(t) case node_type::t: return sizeof(t);
      __ENTRY(Element)
      __ENTRY(Attribute)
      __ENTRY(Comment)
      __ENTRY(PI)
      __ENTRY(Text)
      __ENTRY(ETag)
#undef __ENTRY
      default: return 0;
      }
    }

    constexpr node_type get_type(const void *p) {
      return *static_cast<const node_type*>(p);
    }

    constexpr const void *next_node_raw(const void *p) {
      return static_cast<const uint8_t*>(p) + get_size(get_type(p));
    }

    constexpr const void *next_node(const void *p) {
      const void *next = p;

      size_t level = 1;
      do {
        next = next_node_raw(next);
        switch (get_type(next)) {
        case node_type::Element:
          level++;
          break;
        case node_type::ETag:
          level--;
          break;
        case node_type::None:
          next = nullptr;
          break;
        default:
          break;
        }
      } while (next && level > 0);

      return next;
    }
  }

  inline void print(std::ostream &os, const Node *p) {
    switch (details::get_type(p)) {
    case node_type::None:
      os << "None";
      break;
    case node_type::Element:
      os << "Element " << p->as<Element>()->tag();
      break;
    case node_type::Attribute:
      os << "Attribute " << p->as<Attribute>()->name() << "=\"" << p->as<Attribute>()->value() << '"';
      break;
    case node_type::Comment:
      os << "Comment " << p->as<Comment>()->content();
      break;
    case node_type::PI:
      os << "PI " << p->as<PI>()->content();
      break;
    case node_type::Text:
      os << "Text " << p->as<Text>()->content();
      break;
    case node_type::ETag:
      os << "ETag " << p->as<ETag>()->tag();
      break;
    }
  }
}
