#pragma once

namespace frxml::details {
  template <char... chs>
  using tstring = std::integer_sequence<char, chs...>;

  template <typename T, T... chs>
  constexpr tstring<chs...> operator""_t() { return { }; }

  class view {
    const char *_begin;
    const char *_end;

  public:
    view(const char *begin, const char *end) : _begin(begin), _end(end) {
    }

    // NOLINTNEXTLINE(*-explicit-constructor)
    view(const std::string_view s) : _begin(s.begin()), _end(s.end()) {
    }

    // NOLINTNEXTLINE(*-explicit-constructor)
    view(const std::string &s) : _begin(s.data()), _end(s.data() + s.size()) {
    }

    [[nodiscard]]
    view substr(size_t i, size_t size) const;

    template<char... chs>
    [[nodiscard]]
    bool start_with() const;

    template<char... chs>
    [[nodiscard]]
    bool start_with(tstring<chs...>) const;

    template<char... chs>
    void skip();

    template<char... chs>
    void until();

    template<char... chs>
    void until(tstring<chs...>);

    [[nodiscard]]
    bool eof() const {
      return _begin == _end;
    }
  };

  inline view view::substr(size_t i, size_t size) const {
    return view(_begin + i, _begin + i + size);
  }

  template<char... chs>
  bool view::start_with() const {
    const auto ch = *_begin;
    return ((ch == chs) || ...);
  }

  template<char... chs>
  bool view::start_with(tstring<chs...>) const {
    constexpr char CHS[] = {chs...};
    for (auto i = 0; i < sizeof...(chs) && _begin + i != _end; ++i) {
      if (_begin[i] != CHS[i]) {
        return false;
      }
    }
    return true;
  }

  template<char... chs>
  void view::skip() {
    for (; start_with<chs...>() && !eof(); ++_begin) {
    }
  }

  template<char... chs>
  void view::until() {
    for (; !start_with<0, chs...>() && !eof(); ++_begin) {
    }
  }

  template<char... chs>
  void view::until(tstring<chs...> t) {
    for (; !start_with(t) && !eof(); ++_begin) {
    }
  }
}
