#pragma once

#include <functional>
#include <type_traits>
#include <variant>

namespace nonstd {
namespace detail {
template <std::ptrdiff_t Index, std::ptrdiff_t Count, typename Visitor> struct visit_helper {
  template <typename Variant> static constexpr auto visit(Visitor &&visitor, Variant &&variant) {
    if (auto *el = std::get_if<Index - 1>(&std::forward<Variant>(variant)))
      return std::invoke(visitor, *el);
    return visit_helper<Index + 1, Count, Visitor>::visit(std::forward<Visitor>(visitor),
                                                          std::forward<Variant>(variant));
  }
};

template <std::ptrdiff_t Count, typename Visitor> struct visit_helper<Count, Count, Visitor> {
  template <typename Variant> static constexpr auto visit(Visitor &&visitor, Variant &&variant) {
    return std::invoke(visitor, std::get<Count - 1>(std::forward<Variant>(variant)));
  }
};
} // namespace detail

static constexpr auto VISIT_SWITCH_THRESHOLD = 100;

template <typename Visitor, typename Variant>
constexpr auto visit(Visitor &&visitor, Variant &&variant) {
  constexpr auto SIZE = std::variant_size_v<std::decay_t<Variant>>;
  if constexpr (SIZE <= VISIT_SWITCH_THRESHOLD) {
    return detail::visit_helper<1, SIZE, Visitor>::visit(std::forward<Visitor>(visitor), variant);
  } else {
    return std::visit(std::forward<Visitor>(visitor), std::forward<Variant>(variant));
  }
}
} // namespace nonstd