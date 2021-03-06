#include <benchmark/benchmark.h>
#include <memory>
#include <type_traits>
#include <utility>

#include "polymorphic_value.h"
#include "shapes.h"
#include "value_ptr.h"

template <typename Interface> class NonCopyableImplementation {
public:
  template <typename Concrete, typename = std::enable_if_t<std::is_move_constructible_v<Concrete>>>
  explicit NonCopyableImplementation(Concrete conc) : obj_(new Concrete(std::move(conc))) {}

  constexpr auto operator->() const noexcept -> const Interface * { return obj_.get(); }
  constexpr auto operator->() noexcept -> Interface * { return obj_.get(); }

private:
  std::unique_ptr<Interface> obj_;
};

using Numeric = double;

static void BM_Virtual(benchmark::State &s) {
  using Shape = NonCopyableImplementation<IShape<Numeric>>;
  Shape shape(Rectangle(1.0, 1.0));
  Numeric area = 0;

  for (auto _ : s) {
    area += shape->Area();
    shape = std::move(shape);
  }

  benchmark::DoNotOptimize(area);
}

static void BM_PolyValue(benchmark::State &s) {
  using Shape = nonstd::polymorphic_value<IShape<Numeric>>;
  Shape shape(Rectangle(1.0, 1.0));
  Numeric area = 0;

  for (auto _ : s) {
    area += shape->Area();
    shape = std::move(shape);
  }

  benchmark::DoNotOptimize(area);
}

static void BM_ValuePtr(benchmark::State &s) {
  auto shape = nonstd::make_polymorphic_value<IShape<Numeric>>(Rectangle(1.0, 1.0));
  Numeric area = 0;

  for (auto _ : s) {
    area += shape->Area();
    shape = std::move(shape);
  }

  benchmark::DoNotOptimize(area);
}

BENCHMARK(BM_Virtual);
BENCHMARK(BM_PolyValue);
BENCHMARK(BM_ValuePtr);
