#pragma once

#include <functional>
#include <type_traits>

namespace nonstd {
template <typename IFace> class polymorphic_value { // NOLINT
public:
  using value_type = IFace;

  polymorphic_value() = default;

  template <typename Concrete, typename = std::enable_if_t<std::is_copy_constructible_v<Concrete>>>
  explicit polymorphic_value(Concrete conc)
      : obj_([obj = std::move(conc)]() mutable -> IFace * { return &obj; }) {
    static_assert(std::is_base_of_v<IFace, Concrete>);
  }

  ~polymorphic_value() = default;

  polymorphic_value(const polymorphic_value &o) : obj_(o.obj_) {}
  polymorphic_value(polymorphic_value &&o) noexcept : obj_(std::move(o.obj_)) {}

  auto operator=(polymorphic_value o) -> polymorphic_value & {
    o.swap(*this);
    return *this;
  }

  constexpr auto operator*() const noexcept -> const IFace & { return *ptr_; }
  constexpr auto operator*() noexcept -> IFace & { return *ptr_; }

  constexpr auto operator->() const noexcept -> const IFace * { return ptr_; }
  constexpr auto operator->() noexcept -> IFace * { return ptr_; }

  friend inline void swap(polymorphic_value &a, polymorphic_value &b) noexcept { a.swap(b); }

private:
  void swap(polymorphic_value &o) noexcept {
    std::swap(obj_, o.obj_);
    std::swap(ptr_, o.ptr_);
  }

  std::function<IFace *()> obj_;
  IFace *ptr_ = obj_();
};
} // namespace nonstd