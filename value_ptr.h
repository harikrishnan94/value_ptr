#pragma once

#include <boost/compressed_pair.hpp>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace nonstd {
namespace detail {
template <typename T> static constexpr auto size_of() -> std::size_t {
  if constexpr (std::is_empty_v<T>) {
    return 0;
  } else {
    return sizeof(T);
  }
}

struct noop_memops {};

static inline auto add(void *p, std::size_t n) {
  return static_cast<void *>(static_cast<char *>(p) + n);
}
static inline auto add(const void *p, std::size_t n) {
  return static_cast<const void *>(static_cast<const char *>(p) + n);
}

template <typename T>
static constexpr auto is_final_v =
    std::disjunction_v<std::is_final<T>, std::negation<std::is_polymorphic<T>>>;

enum class MemOp { CLONE, DESTROY };

template <typename T, typename Allocator, typename MemOps = noop_memops> struct mem_ops {
  using pair = boost::compressed_pair<MemOps, T>;
  using rebind_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<pair>;

  static auto alloc(T obj, MemOps ops, Allocator &allocator) -> void * {
    void *mem = rebind_alloc(allocator).allocate(1);
    new (mem) MemOps{std::move(ops)};
    new (add(mem, size_of<MemOps>())) T{std::move(obj)};
    return mem;
  }

  static auto do_oper(void *ptr, Allocator &allocator, MemOp op) -> void * {
    switch (op) {
    case MemOp::CLONE:
      if constexpr (is_final_v<T>) {
        return alloc(*static_cast<const T *>(add(ptr, size_of<MemOps>())), {}, allocator);
      } else {
        return alloc(*static_cast<const T *>(add(ptr, size_of<MemOps>())), do_oper, allocator);
      }
    case MemOp::DESTROY:
      std::destroy_at(static_cast<MemOps *>(ptr));
      std::destroy_at(static_cast<T *>(add(ptr, size_of<MemOps>())));
      rebind_alloc(allocator).deallocate(static_cast<pair *>(ptr), 1);
      return nullptr;
    default:
      std::terminate();
    }
  }

  auto operator()(void *src, Allocator &allocator, MemOp op) const -> void * {
    return do_oper(src, allocator, op);
  }
};

template <typename T, typename Allocator>
using memops_t =
    std::conditional_t<is_final_v<T>, mem_ops<T, Allocator>, void *(*)(void *, Allocator &, MemOp)>;

template <typename T, typename Allocator> static inline auto get_memops() {
  if constexpr (is_final_v<T>) {
    return mem_ops<T, Allocator>();
  } else {
    return mem_ops<T, Allocator, memops_t<T, Allocator>>::do_oper;
  }
}

template <typename T, typename Allocator> static inline auto get_memops(void *ptr) {
  if constexpr (is_final_v<T>) {
    return mem_ops<T, Allocator>();
  } else {
    return *static_cast<memops_t<T, Allocator> *>(ptr);
  }
}

template <typename T, typename Allocator> static inline auto get_ptr(const void *ptr) {
  return static_cast<const T *>(add(ptr, size_of<memops_t<T, Allocator>>()));
}
template <typename T, typename Allocator> static inline auto get_ptr(void *ptr) {
  return static_cast<T *>(add(ptr, size_of<memops_t<T, Allocator>>()));
}

struct poly_alloc_t {};
struct direct_alloc_t {};

template <typename T, typename Allocator, typename... Args>
static inline auto alloc_ptr(direct_alloc_t /*unused*/, Allocator &alloc, Args &&...args)
    -> void * {
  static_assert(std::is_constructible_v<T, Args...>, "cannot construct T from Args");
  return mem_ops<T, Allocator, memops_t<T, Allocator>>::alloc(T(std::forward<Args>(args)...),
                                                              get_memops<T, Allocator>(), alloc);
}

template <typename IFace, typename Concrete, typename Allocator>
static inline auto alloc_ptr(poly_alloc_t /*unused*/, Concrete &&val, Allocator &alloc) -> void * {
  static_assert(std::is_base_of_v<IFace, Concrete>, "IFace is not base of Concrete");
  return mem_ops<Concrete, Allocator, memops_t<Concrete, Allocator>>::alloc(
      std::forward<Concrete>(val), get_memops<Concrete, Allocator>(), alloc);
}
} // namespace detail

template <typename Type, typename Allocator = std::allocator<Type>> class value_ptr { // NOLINT
public:
  using value_type = Type;
  using allocator_type = Allocator;

  explicit value_ptr(Allocator allocator = {}) : ptr_{nullptr, std::move(allocator)} {}

  template <typename Value, typename = std::enable_if_t<std::is_copy_constructible_v<Value>>>
  explicit value_ptr(Value &&val, allocator_type alloc = {})
      : ptr_{detail::alloc_ptr<value_type, Allocator>(detail::direct_alloc_t{}, alloc,
                                                      std::forward<Value>(val)),
             std::move(alloc)} {}

  ~value_ptr() {
    if (ptr_.first()) {
      detail::get_memops<value_type, allocator_type>(ptr_.first())(ptr_.first(), ptr_.second(),
                                                                   detail::MemOp::DESTROY);
    }
  }

  value_ptr(const value_ptr &o)
      : ptr_{detail::get_memops<value_type, allocator_type>(o.ptr_.first())(
                 o.ptr_.first(), o.ptr_.second(), detail::MemOp::CLONE),
             o.ptr_.second()} {}

  value_ptr(value_ptr &&o) noexcept : ptr_(std::move(o.ptr_)) { o.ptr_.first() = nullptr; }

  auto operator=(value_ptr o) -> value_ptr & {
    o.swap(*this);
    return *this;
  }

  template <typename Value, typename = std::enable_if_t<std::is_copy_constructible_v<Value>>>
  auto operator=(Value &&val) -> value_ptr & {
    if constexpr (detail::is_final_v<value_type>) {
      if (auto ptr = get()) {
        *ptr = value_type(std::forward<Value>(val));
      } else {
        *this = value_ptr<Type>(std::forward<Value>(val));
      }
    } else {
      using value_t = std::decay_t<Value>;
      static_assert(std::is_base_of_v<value_type, value_t>, "value_type is not base of Value");

      value_ptr other(ptr_.second());
      other.ptr_.first() = detail::alloc_ptr<value_type, value_t, allocator_type>(
          detail::poly_alloc_t{}, std::forward<Value>(val), other.ptr_.second());
      *this = std::move(other);
    }

    return *this;
  }

  auto get_allocator() const noexcept { return ptr_.second(); }

  [[nodiscard]] constexpr auto get() const noexcept -> const void * {
    return detail::get_ptr<value_type, allocator_type>(ptr_.first());
  }
  constexpr auto get() noexcept {
    return detail::get_ptr<value_type, allocator_type>(ptr_.first());
  }

  constexpr auto operator*() const noexcept { return *get(); }
  constexpr auto operator*() noexcept { return *get(); }

  constexpr auto operator->() const noexcept { return get(); }
  constexpr auto operator->() noexcept { return get(); }

  friend inline void swap(value_ptr &a, value_ptr &b) noexcept { a.swap(b); }

private:
  void swap(value_ptr &o) noexcept { std::swap(ptr_, o.ptr_); }

  mutable boost::compressed_pair<void *, allocator_type> ptr_;
};

template <typename IFace, typename Concrete, typename Allocator = std::allocator<IFace>>
auto make_polymorphic_value(Concrete &&val, Allocator alloc = {}) -> value_ptr<IFace, Allocator> {
  value_ptr<IFace, Allocator> pv(std::move(alloc));

  pv = std::forward<Concrete>(val);
  return pv;
}
} // namespace nonstd