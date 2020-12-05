#include <benchmark/benchmark.h>

#include "shapes.h"
#include "visit.h"

using Number = double;

template <typename Numeric, std::size_t I>
struct DummyShape : public AbstractShape<Numeric, DummyShape<Numeric, I>> {
  [[nodiscard]] constexpr auto compute_area() const -> Numeric { return I; }
};

using Shape = std::variant<DummyShape<Number, 0>, DummyShape<Number, 1>, DummyShape<Number, 2>,
                           DummyShape<Number, 3>, DummyShape<Number, 4>, DummyShape<Number, 5>,
                           DummyShape<Number, 6>, DummyShape<Number, 7>, DummyShape<Number, 8>,
                           DummyShape<Number, 9>>;

static constexpr auto DIMENSION1 = 5.0;
static constexpr auto DIMENSION2 = 4.0;

struct ShapeUpdater {
  static constexpr auto REUSE_COUNT = 100;

  explicit ShapeUpdater(Shape &shape) : shape(shape) {}

  void operator()() noexcept {
    if (++count % REUSE_COUNT == 0) {
      switch (++next_shape % 3) {
      case 0:
        shape = DummyShape<Number, 0>{};
        break;
      case 1:
        shape = DummyShape<Number, 1>{};
        break;
      case 2:
        shape = DummyShape<Number, 2>{};
        break;
      case 3:
        shape = DummyShape<Number, 3>{};
        break;
      case 4:
        shape = DummyShape<Number, 4>{};
        break;
      case 5:
        shape = DummyShape<Number, 5>{};
        break;
      case 6:
        shape = DummyShape<Number, 6>{};
        break;
      case 7:
        shape = DummyShape<Number, 7>{};
        break;
      case 8:
        shape = DummyShape<Number, 8>{};
        break;
      case 9:
        shape = DummyShape<Number, 9>{};
        break;
      }
    }
  }

private:
  int count = 0;
  int next_shape = 0;
  Shape &shape;
};

static void BM_StdVisit(benchmark::State &s) {
  Number area = 0;

  Shape shape;
  ShapeUpdater updater(shape);

  for (auto _ : s) {
    area += std::visit([](auto &s) { return s.Area(); }, shape);
    updater();
  }

  benchmark::DoNotOptimize(area);
}

static void BM_MyVisit(benchmark::State &s) {
  Number area = 0;

  Shape shape;
  ShapeUpdater updater(shape);

  for (auto _ : s) {
    area += nonstd::visit([](auto &s) { return s.Area(); }, shape);
    updater();
  }

  benchmark::DoNotOptimize(area);
}

BENCHMARK(BM_StdVisit);
BENCHMARK(BM_MyVisit);