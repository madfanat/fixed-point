#include <iostream>

enum class Rounding {
  kTowardZero,
  kTowardNearestEven,
  kTowardPositiveInfinity,
  kTowardNegativeInfinity
};

class FixedPoint {
 public:
  FixedPoint(const uint32_t value, const uint8_t integer,
             const uint8_t fractional)
      : value_(value & ((1ULL << (integer + fractional)) - 1)),
        integer_(integer),
        fractional_(fractional),
        total_(integer + fractional),
        is_div_by_zero_(false) {}

  static void SetRounding(const Rounding rounding) { rounding_ = rounding; }

  FixedPoint operator+(const FixedPoint& other) const {
    return {value_ + other.value_, integer_, fractional_};
  }

  FixedPoint operator-() const {
    return {(~value_ + 1), integer_, fractional_};
  }

  FixedPoint operator-(const FixedPoint& other) const {
    return {value_ - other.value_, integer_, fractional_};
  }

  FixedPoint operator*(const FixedPoint& other) const {
    const bool is_negative = IsNegative() ^ other.IsNegative();

    uint64_t product = GetAbsolute() * other.GetAbsolute();
    const uint64_t remainder = product & fractional_mask_;
    product >>= fractional_;

    product = Round(product, remainder, 1ULL << fractional_, is_negative);

    if (is_negative) {
      product = (~product + 1);
    }

    product &= total_mask_;

    return {static_cast<uint32_t>(product), integer_, fractional_};
  }

  FixedPoint operator/(const FixedPoint& other) const {
    if (other.value_ == 0) {
      return DivByZero(integer_, fractional_);
    }

    const bool is_negative = IsNegative() ^ other.IsNegative();

    const uint64_t dividend = GetAbsolute();
    const uint64_t divisor = other.GetAbsolute();

    uint64_t quotient = (dividend << fractional_) / divisor;
    const uint64_t remainder = (dividend << fractional_) % divisor;

    quotient = Round(quotient, remainder, divisor, is_negative);

    if (is_negative) {
      quotient = (~quotient + 1);
    }

    quotient &= total_mask_;

    return {static_cast<uint32_t>(quotient), integer_, fractional_};
  }

  static FixedPoint DivByZero(uint8_t integer, uint8_t fractional) {
    FixedPoint div_by_zero(0, integer, fractional);
    div_by_zero.is_div_by_zero_ = true;

    return div_by_zero;
  }

  friend std::ostream& operator<<(std::ostream& output,
                                  const FixedPoint& number) {
    if (number.is_div_by_zero_) {
      output << "div_by_zero";
      return output;
    }

    const bool is_negative = number.IsNegative();
    const uint64_t value = number.GetAbsolute();

    uint64_t scaled = value * 1000;
    const uint64_t remainder = scaled & ((1ULL << number.fractional_) - 1);
    scaled >>= number.fractional_;

    scaled = Round(scaled, remainder, 1ULL << number.fractional_, is_negative);

    const uint64_t integer = scaled / 1000;
    const uint64_t fractional = scaled % 1000;

    if (is_negative && (integer != 0 || fractional != 0)) {
      output << '-';
    }

    output << integer << '.';
    if (fractional < 10) {
      output << "00" << fractional;
    } else if (fractional < 100) {
      output << "0" << fractional;
    } else {
      output << fractional;
    }

    return output;
  }

 private:
  [[nodiscard]] bool IsNegative() const { return (value_ >> (total_ - 1)) & 1; }

  [[nodiscard]] uint64_t GetAbsolute() const {
    uint64_t value = value_;

    if (IsNegative()) {
      value = (~value + 1) & total_mask_;
    }

    return value;
  }

  static uint64_t Round(uint64_t number, const uint64_t remainder,
                        const uint64_t divisor, const bool is_negative) {
    if (remainder != 0) {
      switch (rounding_) {
        case Rounding::kTowardZero:
          break;

        case Rounding::kTowardNearestEven: {
          if ((remainder << 1) > divisor ||
              ((remainder << 1) == divisor && (number & 1))) {
            ++number;
          }
          break;
        }

        case Rounding::kTowardPositiveInfinity:
          if (!is_negative) {
            ++number;
          }
          break;

        case Rounding::kTowardNegativeInfinity:
          if (is_negative) {
            ++number;
          }
          break;
      }
    }

    return number;
  }

  const uint32_t value_;
  const uint8_t integer_;
  const uint8_t fractional_;
  const uint8_t total_;

  const uint32_t fractional_mask_ = (1ULL << fractional_) - 1;
  const uint32_t total_mask_ = (1ULL << total_) - 1;

  bool is_div_by_zero_;

  inline static Rounding rounding_ = Rounding::kTowardZero;
};