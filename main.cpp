#include <iostream>
#include <string>

#include "fixed_point/fixed_point.h"

int main(const int argc, char* argv[]) {
  if (argc != 4 && argc != 6) {
    std::cerr << "Error: wrong amount of arguments." << std::endl;
    return 1;
  }

  std::string input(argv[1]);
  const size_t delimiter = input.find('.');
  if (delimiter == std::string::npos) {
    std::cerr << "Error: wrong integer/fractional format." << std::endl;
    return 1;
  }

  uint8_t integer, fractional;
  try {
    integer = std::stoi(input.substr(0, delimiter));
    fractional = std::stoi(input.substr(delimiter + 1));
  } catch (...) {
    std::cerr << "Error: wrong numbers format." << std::endl;
    return 1;
  }

  uint8_t rounding_index;
  try {
    rounding_index = std::stoi(argv[2]);
  } catch (...) {
    std::cerr << "Error: wrong rounding option." << std::endl;
    return 1;
  }

  Rounding rounding;
  switch (rounding_index) {
    case 0:
      rounding = Rounding::kTowardZero;
      break;
    case 1:
      rounding = Rounding::kTowardNearestEven;
      break;
    case 2:
      rounding = Rounding::kTowardPositiveInfinity;
      break;
    case 3:
      rounding = Rounding::kTowardNegativeInfinity;
      break;
    default:
      std::cerr << "Error: wrong rounding option." << std::endl;
      return 1;
  }

  FixedPoint::SetRounding(rounding);

  switch (argc) {
    case 4: {
      unsigned long value;
      try {
        value = std::stoul(argv[3], nullptr, 16);
      } catch (...) {
        std::cerr << "Error: wrong number." << std::endl;
        return 1;
      }

      const FixedPoint number(value, integer, fractional);
      std::cout << number << std::endl;
      break;
    }
    case 6: {
      const std::string operation(argv[3]);

      unsigned long a_value;
      unsigned long b_value;
      try {
        a_value = std::stoul(argv[4], nullptr, 16);
      } catch (...) {
        std::cerr << "Error: wrong number." << std::endl;
        return 1;
      }
      try {
        b_value = std::stoul(argv[5], nullptr, 16);
      } catch (...) {
        std::cerr << "Error: wrong number." << std::endl;
        return 1;
      }

      const FixedPoint a(static_cast<uint32_t>(a_value), integer, fractional);
      const FixedPoint b(static_cast<uint32_t>(b_value), integer, fractional);

      if (operation == "+") {
        std::cout << a + b << std::endl;
      } else if (operation == "-") {
        std::cout << a - b << std::endl;
      } else if (operation == "*") {
        std::cout << a * b << std::endl;
      } else if (operation == "/") {
        std::cout << a / b << std::endl;
      } else {
        std::cerr << "Error: wrong operation type." << std::endl;
        return 1;
      }
      break;
    }
  }

  return 0;
}