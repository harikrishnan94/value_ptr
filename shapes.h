#pragma once

#include <numbers>
#include <variant>

template <typename Numeric, typename Derived> struct AbstractShape {
  [[nodiscard]] constexpr auto Area() const -> Numeric { return impl()->compute_area(); }

private:
  [[nodiscard]] auto impl() const -> const Derived * { return static_cast<const Derived *>(this); }
};

template <typename Numeric> class Rectangle : public AbstractShape<Numeric, Rectangle<Numeric>> {
public:
  constexpr Rectangle(Numeric len, Numeric breth) : len_(len), breth_(breth) {}

  friend struct AbstractShape<Numeric, Rectangle>;

private:
  [[nodiscard]] constexpr auto compute_area() const -> Numeric { return len_ * breth_; }

  Numeric len_;
  Numeric breth_;
};

template <typename Numeric> class Circle : public AbstractShape<Numeric, Circle<Numeric>> {
public:
  constexpr explicit Circle(Numeric radius) : radius_(radius) {}

  friend struct AbstractShape<Numeric, Circle>;

private:
  [[nodiscard]] constexpr auto compute_area() const -> Numeric {
    return 2 * radius_ * std::numbers::pi;
  }

  Numeric radius_;
};

template <typename Numeric> class Triangle : public AbstractShape<Numeric, Triangle<Numeric>> {
public:
  constexpr Triangle(Numeric base, Numeric heit) : base_(base), heit_(heit) {}

  friend struct AbstractShape<Numeric, Triangle>;

private:
  [[nodiscard]] constexpr auto compute_area() const -> Numeric { return (base_ * heit_) / 2; }

  Numeric base_;
  Numeric heit_;
};
