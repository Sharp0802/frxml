#ifndef FRXML_INCLUDE_PARSE
#error Do not include 'frxml/parse.inc' directly; Use 'frxml/frxml.h'
#endif

namespace details {
  template<
    typename node_type,
    auto callback,
    typename context,
    char... begin_chs,
    char... end_chs
  >
  state parse_element_like(context *ctx, view &src, tstring<begin_chs...> bt, tstring<end_chs...> et) {
    if (!src.start_with(bt)) {
      return NIL;
    }

    const auto begin = src.begin() + bt.size();

    src.skip_until(et);
    if (src.eof())
      return UNEXPECTED_EOF;

    const auto end = src.begin();

    src.forward<et.size()>();

    const node_type c(view(begin, end).to_std());
    callback(ctx, &c, sizeof c);

    return OKAY;
  }

  template<auto callback, typename context>
  state parse_element(context *ctx, view &src) {
    const auto super_begin = src.begin();
    src.skip_space();
    if (src.eof())
      return NIL;

    if (!src.start_with<'<'>()) {
      // Use super_begin to preserve whitespace
      // const auto begin = src.begin();
      src.skip_until<'<'>();
      const auto end = src.begin();

      const Text t(view(super_begin, end).to_std());
      callback(ctx, &t, sizeof t);

      return OKAY;
    }

    src.forward<1>();

#define __TRY_PARSE(t, begin, end) if (const auto s = parse_element_like<t, callback, context>(ctx, src, begin##_t, end##_t); s != NIL) { return s; }
    __TRY_PARSE(Comment, "!--", "-->");
    __TRY_PARSE(PI, "?", "?>");
    __TRY_PARSE(Text, "!CDATA[", "]]>"); // cdata is considered as just text
    __TRY_PARSE(ETag, "/", ">"); // validation is for validator, not parser
#undef __TRY_PARSE

    const auto tag_begin = src.begin();
    src.skip_until<SPACE, '/', '>'>();
    const auto tag_end = src.begin();

    if (src.eof())
      return UNEXPECTED_EOF;

    const Element e(view(tag_begin, tag_end).to_std());
    callback(ctx, &e, sizeof e);

    while (true) {
      const auto begin = src.begin();
      src.skip_space();

      if (src.start_with("/>"_t)) {
        const ETag etag(view(tag_begin, tag_end).to_std());
        callback(ctx, &etag, sizeof etag);
        src.forward<2>();
        break;
      }
      if (src.start_with<'>'>()) {
        src.forward<1>();
        break;
      }

      if (begin == src.begin()) {
        return EXPECTED_SPACE;
      }

      const auto name_begin = src.begin();
      src.skip_until<'='>();
      const auto name_end = src.begin();
      src.forward<1>();

      if (src.eof())
        return UNEXPECTED_EOF;

      const char *value_begin;

      if (src.start_with<'"'>()) {
        src.forward<1>();
        value_begin = src.begin();
        src.skip_until<'"'>();
      }
      else if (src.start_with<'\''>()) {
        src.forward<1>();
        value_begin = src.begin();
        src.skip_until<'\''>();
      }
      else {
        return EXPECTED_QUOTE;
      }

      const char *value_end = src.begin();
      src.forward<1>();

      if (src.eof())
        return UNEXPECTED_EOF;

      const Attribute attr(
        view(name_begin, name_end).to_std(),
        view(value_begin, value_end).to_std()
      );
      callback(ctx, &attr, sizeof attr);
    }

    return OKAY;
  }
}

template<auto callback, typename context>
state parse(const std::string_view str, context *ctx) {
  using namespace details;

  view view(str);

  state s;
  do {
    view.skip_space();
    s = parse_element<callback>(ctx, view);
  } while (!view.eof() && s == OKAY);

  return s;
}
