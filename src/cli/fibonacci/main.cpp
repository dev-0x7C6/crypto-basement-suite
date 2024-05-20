
#include <array>
#include <cmath>
#include <cstddef>
#include <fmt/format.h>

namespace fibonacci {

template <typename T = double>
constexpr T fib(std::uint64_t n) {
    T a = 0;
    T b = 1;

    for (std::uint64_t i = 0; i < n; i++) {
        b += a;
        a = b - a;
    }

    return b;
}

template <std::size_t N, typename T = double>
constexpr auto to_array() -> std::array<T, N> {
    std::array<T, N> values;
    for (std::size_t i{}; i < N; ++i)
        values[i] = fib(i);
    return values;
}

constexpr static auto fib_6854 = fib(64) / fib(60);
constexpr static auto fib_4236 = fib(64) / fib(61);
constexpr static auto fib_3618 = fib(64) / fib(62) + 1.0;
constexpr static auto fib_2618 = fib(64) / fib(62);
constexpr static auto fib_1618 = fib(64) / fib(63);
constexpr static auto fib_1000 = fib(64) / fib(64);
const static auto fib_0786 = std::sqrt(fib(64) / fib(65));
constexpr static auto fib_0618 = fib(64) / fib(65);
constexpr static auto fib_0500 = fib_1000 / 2.0;
constexpr static auto fib_0381 = fib(64) / fib(66);
constexpr static auto fib_0236 = fib(64) / fib(67);
constexpr static auto fib_0145 = fib(64) / fib(68);

} // namespace fibonacci

template <typename T = double>
auto inverse(const T a) {
    return static_cast<T>(1.0) / a;
}

template <typename T = double>
auto golden_ratio() {
    constexpr static auto a = static_cast<T>(1.0);
    constexpr static auto b = static_cast<T>(2.0);
    constexpr static auto s = static_cast<T>(5.0);

    const auto phi = (a + std::sqrt(s)) / b;
    return phi;
}

template <typename T = double>
auto golden_ratio_inversed() {
    return inverse(golden_ratio());
}

auto main(int argc, char **argv) -> int {
    auto table = fibonacci::to_array<128>();

    for (auto i = 0; i < 40; ++i) {
        fmt::print("fibonacci({}) = {}\n", i, table[i]);
        if (i > 0)
            fmt::print("    ratio({}) = {}\n", i, table[i] / table[i - 1]);
        fmt::print("\n");
    }

    fmt::print("golden ratio = {}\n", golden_ratio<double>());
    fmt::print("golden ratio inverted = {}\n", golden_ratio_inversed());
    fmt::print("\n");

    fmt::print("fibonacci::fib_6854 = {}\n", fibonacci::fib_6854);
    fmt::print("fibonacci::fib_4236 = {}\n", fibonacci::fib_4236);
    fmt::print("fibonacci::fib_3618 = {}\n", fibonacci::fib_3618);
    fmt::print("fibonacci::fib_2618 = {}\n", fibonacci::fib_2618);
    fmt::print("fibonacci::fib_1618 = {}\n", fibonacci::fib_1618);
    fmt::print("fibonacci::fib_1000 = {}\n", fibonacci::fib_1000);
    fmt::print("fibonacci::fib_0786 = {}\n", fibonacci::fib_0786);
    fmt::print("fibonacci::fib_0618 = {}\n", fibonacci::fib_0618);
    fmt::print("fibonacci::fib_0500 = {}\n", fibonacci::fib_0500);
    fmt::print("fibonacci::fib_0381 = {}\n", fibonacci::fib_0381);
    fmt::print("fibonacci::fib_0236 = {}\n", fibonacci::fib_0236);
    fmt::print("fibonacci::fib_0145 = {}\n", fibonacci::fib_0145);

    return 0;
}
