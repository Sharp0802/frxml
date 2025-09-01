#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifdef SPACE
#warning 'SPACE' macro is already occupied; Result may malfunction.
#else
#define SPACE ' ', '\t', '\r', '\n'
#endif

namespace frxml::details {
  template<char... chs>
  using tstring = std::integer_sequence<char, chs...>;

#if __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif
  template<typename T, T... chs>
  constexpr tstring<chs...> operator""_t() { return {}; }
#if __GNUG__
#pragma GCC diagnostic pop
#endif

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
    const char *begin() const { return _begin; }

    [[nodiscard]]
    const char *end() const { return _end; }

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
    void skip_until();

    template<char... chs>
    void skip_until(tstring<chs...>);

    template<size_t N>
    void forward();

    [[nodiscard]]
    bool eof() const {
      return _begin == _end;
    }

    [[nodiscard]]
    std::string_view to_std() const {
      return {_begin, static_cast<std::size_t>(_end - _begin)};
    }
  };

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
  void view::skip_until() {
    for (; !eof() && !start_with<chs...>(); ++_begin) {
    }
  }

  template<char... chs>
  void view::skip_until(tstring<chs...> t) {
    for (; !eof() && !start_with(t); ++_begin) {
    }
  }

  template<size_t N>
  void view::forward() {
    _begin += N;
    if (_begin > _end)
      _begin = _end;
  }
}
