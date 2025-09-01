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
  state parse_element_like(context &ctx, view &src, tstring<begin_chs...> bt, tstring<end_chs...> et) {
    if (!src.start_with(bt)) {
      return NIL;
    }

    const auto begin = src.begin() + bt.size();

    src.skip_until(et);
    if (src.eof())
      return UNEXPECTED_EOF;

    const auto end = src.begin();

    src.forward<et.size()>();

    const node_type c = {
      .content = view(begin, end).to_std()
    };

    callback(ctx, &c, sizeof c);

    return OKAY;
  }

  template<auto callback, typename context>
  state parse_element(context &ctx, view &src) {
    if (!src.start_with<'<'>()) {
      return NIL;
    }

    src.forward<1>();

#define __TRY_PARSE(t, begin, end) if (const auto s = parse_element_like<t, callback, context>(ctx, src, begin##_t, end##_t); s != NIL) { return s; }
    __TRY_PARSE(comment, "!--", "-->");
    __TRY_PARSE(pi, "?", "?>");
    __TRY_PARSE(text, "!CDATA[", "]]>"); // cdata is considered as just text
    __TRY_PARSE(etag, "/", ">"); // validation is for validator, not parser
#undef __TRY_PARSE

    const auto tag_begin = src.begin();
    src.skip_until<SPACE, '/', '>'>();
    const auto tag_end = src.begin();

    if (src.eof())
      return UNEXPECTED_EOF;

    const element e = {
      .tag = view(tag_begin, tag_end).to_std()
    };
    callback(ctx, &e, sizeof e);

    while (true) {
      const auto begin = src.begin();
      src.skip_space();

      if (src.start_with<'/'>()) {
        const etag etag = {
          .content = view(tag_begin, tag_end).to_std()
        };
        callback(ctx, &etag, sizeof etag);
        break;
      }
      if (src.start_with<'>'>()) {
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

      const auto value_begin = src.begin();
      const char *value_end;

      if (src.start_with<'"'>()) {
        src.skip_until<'"'>();
        value_end = src.begin();
      }
      else if (src.start_with<'\''>()) {
        src.skip_until<'\''>();
        value_end = src.begin();
      }
      else {
        return EXPECTED_QUOTE;
      }

      if (src.eof())
        return UNEXPECTED_EOF;

      const attr attr = {
        .name = view(name_begin, name_end).to_std(),
        .value = view(value_begin, value_end).to_std()
      };
      callback(ctx, &attr, sizeof attr);
    }

    return OKAY;
  }
}

template<auto callback, typename context>
state parse(const std::string_view str, context &ctx) {
  using namespace details;

  view view(str);

  state s;
  do {
    view.skip_space();
    s = parse_element<callback, context>(ctx, view);
  } while (!view.eof() && s == OKAY);

  return s;
}
