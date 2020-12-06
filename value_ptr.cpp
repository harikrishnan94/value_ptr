#include "value_ptr.h"
#include "shapes.h"
#include <iostream>

using Numeric = double;
using Shape = nonstd::value_ptr<IShape<Numeric>>;

auto main() -> int {
  auto shape = nonstd::make_polymorphic_value<IShape<Numeric>>(Rectangle(2.5, 2.0));
  std::cout << shape->Area() << "\n";
  shape = Circle(2.0);
  std::cout << shape->Area() << "\n";

  nonstd::value_ptr<double> d(2);
  std::cout << *d << "\n";
  d = 4;
  std::cout << *d << "\n";
}