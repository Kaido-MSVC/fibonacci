#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <string_view>

#include "fibonacci_core.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

constexpr std::uint64_t DISPLAY_LIMIT = 10'000;
constexpr std::size_t PREVIEW_DIGITS = 50;
constexpr std::size_t MAX_LINE_LENGTH = 256;

enum class LineStatus { ok, tooLong, end, error };
using LineBuffer = std::array<char, MAX_LINE_LENGTH + 1>;

LineStatus readLine(std::istream& input, LineBuffer& buffer, std::string_view& line) {
    line = {};
    if (input.getline(buffer.data(), static_cast<std::streamsize>(buffer.size()))) {
        // gcount() 包含換行字元，只有以 EOF 結尾的最後一行例外。
        const auto length = input.gcount() - (input.eof() ? 0 : 1);
        line = {buffer.data(), static_cast<std::size_t>(length)};
        return LineStatus::ok;
    }
    if (input.bad())
        return LineStatus::error;
    if (input.eof())
        return LineStatus::end;

    input.clear(input.rdstate() & ~std::ios::failbit);
    input.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    return input.bad() ? LineStatus::error : LineStatus::tooLong;
}

#ifdef _WIN32
class ConsoleUtf8 {
public:
    ConsoleUtf8() noexcept
        : inputCodePage(GetConsoleCP()), outputCodePage(GetConsoleOutputCP()),
          inputChanged(inputCodePage != 0 && inputCodePage != CP_UTF8 && SetConsoleCP(CP_UTF8)),
          outputChanged(outputCodePage != 0 && outputCodePage != CP_UTF8 && SetConsoleOutputCP(CP_UTF8)) {}

    ~ConsoleUtf8() {
        if (inputChanged)
            SetConsoleCP(inputCodePage);
        if (outputChanged)
            SetConsoleOutputCP(outputCodePage);
    }

    ConsoleUtf8(const ConsoleUtf8&) = delete;
    ConsoleUtf8& operator=(const ConsoleUtf8&) = delete;

private:
    UINT inputCodePage;
    UINT outputCodePage;
    bool inputChanged;
    bool outputChanged;
};
#endif

int main() {
#ifdef _WIN32
    const ConsoleUtf8 consoleUtf8;
#endif

    std::println("================================");
    std::println("  費波那契數列計算器 C++23");
    std::println("  Fast Doubling + Big Integer");
    std::println("================================");

    LineBuffer buffer{};
    std::string_view input;
    while (true) {
        std::print("\n輸入第 N 項 (0-{}，輸入 -1 離開)：", MAX_N);
        std::fflush(stdout);
        const auto status = readLine(std::cin, buffer, input);
        if (status == LineStatus::end)
            return 0;
        if (status == LineStatus::error) {
            std::println(stderr, "無法讀取輸入。");
            return 1;
        }
        if (status == LineStatus::tooLong) {
            std::println("輸入過長，每行最多接受 {} 個位元組。", MAX_LINE_LENGTH);
            continue;
        }

        const auto trimmed = trim(input);
        if (trimmed == "-1")
            break;

        const auto index = parseIndex(trimmed);
        if (!index) {
            if (index.error() == InputError::tooLarge)
                std::println("超過目前設定的最大項次。");
            else
                std::println("輸入錯誤，請輸入有效的非負整數。");
            continue;
        }

        const auto n = *index;
        try {
            std::println("\n正在計算...");
            std::fflush(stdout);
            const cpp_int result = fibonacci(n).first;
            const std::string output = result.convert_to<std::string>();
            std::println("F({}) 的位數：{}", n, output.size());

            bool showFull = n <= DISPLAY_LIMIT || output.size() <= 2 * PREVIEW_DIGITS;
            if (!showFull) {
                std::print("數字較長，是否完整顯示？(y/N)：");
                std::fflush(stdout);
                const auto choiceStatus = readLine(std::cin, buffer, input);
                if (choiceStatus == LineStatus::end)
                    return 0;
                if (choiceStatus == LineStatus::error) {
                    std::println(stderr, "無法讀取輸入。");
                    return 1;
                }
                if (choiceStatus == LineStatus::tooLong)
                    std::println("輸入過長，將顯示摘要。");
                const auto choice = trim(input);
                showFull = choice == "y" || choice == "Y";
            }

            if (showFull) {
                std::println("\nF({}) =\n{}", n, output);
            } else {
                const std::string_view digits = output;
                std::println("\n前 {} 位：{}", PREVIEW_DIGITS, digits.substr(0, PREVIEW_DIGITS));
                std::println("後 {} 位：{}", PREVIEW_DIGITS, digits.substr(digits.size() - PREVIEW_DIGITS));
            }

            std::println("\n計算完成。");
        } catch (const std::exception& e) {
            std::println(stderr, "計算失敗：{}", e.what());
        }
    }

    std::print("\n程式結束，按 Enter 關閉視窗...");
    std::fflush(stdout);
    std::cin.get();
    return 0;
}
