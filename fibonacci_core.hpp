#pragma once

// 主控台版與 GUI 版共用的輸入解析與 Fast Doubling 計算。

#include <charconv>
#include <cstdint>
#include <expected>
#include <string_view>
#include <system_error>
#include <utility>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;

constexpr std::uint64_t MAX_N = 1'000'000;

enum class InputError { invalid, tooLarge };

constexpr std::string_view trim(std::string_view input) {
    constexpr std::string_view whitespace = " \t\r\n\f\v";
    const auto first = input.find_first_not_of(whitespace);
    if (first == std::string_view::npos)
        return {};
    return input.substr(first, input.find_last_not_of(whitespace) - first + 1);
}

constexpr std::expected<std::uint64_t, InputError> parseIndex(std::string_view input) {
    if (input.empty())
        return std::unexpected(InputError::invalid);

    std::uint64_t n = 0;
    const auto end = input.data() + input.size();
    const auto [ptr, error] = std::from_chars(input.data(), end, n);
    if (ptr != end || error == std::errc::invalid_argument)
        return std::unexpected(InputError::invalid);
    if (error == std::errc::result_out_of_range || n > MAX_N)
        return std::unexpected(InputError::tooLarge);
    return n;
}

// C++23 允許在編譯期用 from_chars 解析整數。
static_assert(parseIndex("1000000").value() == MAX_N);
static_assert(parseIndex("18446744073709551616").error() == InputError::tooLarge);
static_assert(parseIndex("12a").error() == InputError::invalid);

// Fast Doubling 回傳 F(n) 與 F(n+1)。
inline std::pair<cpp_int, cpp_int> fibonacci(std::uint64_t n) {
    if (n == 0)
        return {0, 1};

    auto [a, b] = fibonacci(n / 2);
    cpp_int c = a * (2 * b - a);
    cpp_int d = a * a + b * b;

    if (n % 2 == 0)
        return {std::move(c), std::move(d)};

    cpp_int e = c + d;
    return {std::move(d), std::move(e)};
}
