#include "frxml.h"

#include <cstdint>

namespace frxml {
  node_type operator&(node_type lhs, node_type rhs) {
    const auto l = static_cast<std::underlying_type_t<node_type>>(lhs);
    const auto r = static_cast<std::underlying_type_t<node_type>>(rhs);
    return static_cast<node_type>(l & r);
  }

  const void *skip_node(const void *p) {
    const auto t = *static_cast<const node_type*>(p);
    if ((t & node_type::ELEMENT) == node_type::ELEMENT) {
      return static_cast<const uint8_t*>(p) + sizeof(element);
    }

    switch (t) {
    case node_type::ATTR:
      return static_cast<const uint8_t*>(p) + sizeof(attr);
    case node_type::END_OF_ELEMENT:
      return static_cast<const uint8_t*>(p) + sizeof(end);
    default:
      return nullptr;
    }
  }

  node_iterator<element> element::children() const {
    const void *next = this;
    do {
      next = skip_node(next);
    } while (*static_cast<const node_type*>(next) == node_type::ATTR);

    return node_iterator{static_cast<const element*>(next)};
  }

  node_iterator<attr> element::attrs() const {
    const void *next = skip_node(this);
    return node_iterator{static_cast<const attr*>(next)};
  }
}
