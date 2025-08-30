#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

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

    [[clang::always_inline]]
    void skip_space() {
      skip<' ', '\r', '\n', '\t'>();
    }

    template<char... chs>
    void until();

    template<char... chs>
    void until(tstring<chs...>);

    [[nodiscard]]
    bool eof() const {
      return _begin == _end;
    }
  };

  inline view view::substr(const size_t i, const size_t size) const {
#if _DEBUG
    if (_begin + i + size > _end) {
      throw std::out_of_range("string::substr() out of range");
    }
#endif
    return {_begin + i, _begin + i + size};
  }

  template<char... chs>
  bool view::start_with() const {
    if (eof())
      return false;

    const auto ch = *_begin;
    return ((ch == chs) || ...);
  }

  template<char... chs>
  bool view::start_with(tstring<chs...>) const {
    if (_end - _begin < sizeof...(chs))
      return false;

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
    for (; !eof() && start_with<chs...>(); ++_begin) {
    }
  }

  template<char... chs>
  void view::until() {
    for (; !eof() && !start_with<chs...>(); ++_begin) {
    }
  }

  template<char... chs>
  void view::until(tstring<chs...> t) {
    for (; !eof() && !start_with(t); ++_begin) {
    }
  }
}
