#pragma once

#include <cstdint>

#include "frxml/node.h"
#include "frxml/string.h"

namespace frxml {
  enum state {
    OKAY = 0,

    NIL,
    UNEXPECTED_EOF,
    EXPECTED_SPACE,
    EXPECTED_QUOTE
  };

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
   * @param ctx Pointer to context object
   *
   * @return Zero if parsing succeeded; Otherwise, non-zero error code.
   */
  template<auto callback, typename context>
  state parse(std::string_view str, context *ctx);

#define FRXML_INCLUDE_PARSE
#include "parse.inc"
#undef FRXML_INCLUDE_PARSE
}
