#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <objbase.h>

#include "../fibonacci_core.hpp"

namespace {

// 與 NativeFibonacci.cs 的狀態值一致。
enum class Status : std::int32_t { success, invalid, tooLarge, failed };
constexpr std::int32_t maxInputLength = 20;

}

// 成功時，呼叫端用 CoTaskMemFree 釋放以 NUL 結尾的 ASCII 結果。
extern "C" __declspec(dllexport) Status fibonacci_compute(
    const wchar_t* input, std::int32_t length, std::uint64_t* index, char** digits) noexcept {
    if (!index || !digits)
        return Status::failed;

    *index = 0;
    *digits = nullptr;
    if (!input || length <= 0 || length > maxInputLength)
        return Status::invalid;

    std::array<char, maxInputLength> ascii{};
    for (std::int32_t i = 0; i < length; ++i) {
        if (input[i] >= 0x80)
            return Status::invalid;
        ascii[i] = static_cast<char>(input[i]);
    }

    const auto parsed = parseIndex(trim(std::string_view(ascii.data(), static_cast<std::size_t>(length))));
    if (!parsed)
        return parsed.error() == InputError::tooLarge ? Status::tooLarge : Status::invalid;

    try {
        const std::string result = fibonacci(*parsed).first.convert_to<std::string>();
        const auto bytes = result.size() + 1;
        auto* buffer = static_cast<char*>(CoTaskMemAlloc(bytes));
        if (!buffer)
            return Status::failed;

        std::memcpy(buffer, result.c_str(), bytes);
        *index = *parsed;
        *digits = buffer;
        return Status::success;
    } catch (...) {
        return Status::failed;
    }
}
