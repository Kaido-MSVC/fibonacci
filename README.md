# 費波那契數列計算器

使用 Boost.Multiprecision 的 `cpp_int` 和 Fast Doubling 計算 F(N)。N 可介於 0 與 1,000,000。主控台版輸入的 N 超過 10,000 時，按 Enter 顯示前後各 50 位，輸入 `y` 或 `Y` 可顯示完整數字。

WinUI 3 版的啟動與單檔發行方式請參考 [WinUI 3 說明](winui/README.md)。

## Windows 建置

建置使用 Visual Studio 2026、MSVC x64 工具集 14.52、Windows SDK 10.0.28000.0 和 Boost 1.92.0。請從 [Boost 官方發行頁](https://www.boost.org/releases/1.92.0/) 取得 `boost_1_92_0.7z`，以 `Get-FileHash -Algorithm SHA256` 確認 SHA-256 為 `57f2ded2390068a44d88748425067e01a8c285257a4d9611d04ade9d8ec68615`，再解壓縮至 `%LOCALAPPDATA%\Fibonacci`。建置會從 `%LOCALAPPDATA%\Fibonacci\boost_1_92_0\boost` 讀取標頭檔。這個專案的 `cpp_int` 用法只需要 Boost 標頭檔。

用 VS Code 開啟專案資料夾，按 `Ctrl+Shift+B` 執行預設建置工作。也可以在專案資料夾執行 `.\build.cmd`。兩種方式都會以完整路徑啟動 MSVC x64 的編譯器及連結器，產生已啟用 Control Flow Guard 的主控台版 `build\fibonacci.exe` 和 GUI 版 `build\fibonacci_gui.exe`。多帳戶電腦請將原始碼與建置產物放在可控制整段路徑寫入權限的位置，例如 `%LOCALAPPDATA%\Fibonacci\workspace`。

## 原始碼結構

- `fibonacci_core.hpp`：各版本共用的輸入解析（`trim`、`parseIndex`、`MAX_N`）與 Fast Doubling 計算。
- `fibonacci.cpp`：主控台版。
- `fibonacci_gui.cpp`：Win32 GUI 版，使用 Windows SDK 的 C++/WinRT 剪貼簿 API。
- `winui/`：C#／XAML 的 WinUI 3 版，透過原生 DLL 共用 C++ 計算核心。

## GUI 版

執行 `build\fibonacci_gui.exe`。在輸入框填入 0 到 1,000,000 的項次，按 Enter 或「計算」，下方會顯示完整的 F(N)，狀態列會顯示位數和計算耗時。按「複製結果」可把整個數字複製到剪貼簿。

- 計算在背景進行，期間會停用輸入與複製。開始新一輪計算會清除上一筆結果，完成並成功顯示後才能複製。
- 輸入最多 20 個字元，可含前後 ASCII 空白。項次只接受 ASCII 十進位數字，格式錯誤或超過上限時會顯示在狀態列。
- 支援各螢幕不同 DPI 縮放（Per-Monitor V2），並使用 Common Controls v6 視覺樣式。
- GUI 版使用 Windows Runtime 剪貼簿，連結 Windows SDK 的 `user32.lib`、`gdi32.lib` 和 `windowsapp.lib`。Boost 仍只使用標頭檔。

## WinUI 3 實驗版

`winui` 是獨立的 C# / XAML 專案，透過原生 DLL 共用 `fibonacci_core.hpp`。建置與啟動方式見 [WinUI 3 實驗版說明](winui/README.md)。

## C++23 功能

- `std::expected` 回傳有效項次或具體的解析錯誤，區分格式錯誤與超出上限。
- `std::print` 和 `std::println` 負責格式化中文提示、結果和錯誤訊息。
- 整數版 `std::from_chars` 在 C++23 可於編譯期執行，程式用 `static_assert` 驗證解析器的成功、溢位和格式錯誤案例。

主控台每行最多讀取 256 個位元組。項次可含前後 ASCII 空白，數值本身只接受 ASCII 十進位數字。`-1` 離開，EOF 也會結束程式。Windows 主控台會在正常離開時還原原有的輸入和輸出字碼頁。

## 授權

本專案自行撰寫的原始碼採用 [MIT 授權](LICENSE)，著作權為 2026 Kaido-MSVC。Boost.Multiprecision 依 [Boost Software License 1.0](licenses/Boost-1.92.0.txt) 授權，建置時從本機讀取 Boost 標頭檔。

WinUI 3 的部分專案檔與圖檔（含 AppIcon.ico）源自 Microsoft 的 [WinUI 3 C# Blank App 範本](https://www.nuget.org/packages/Microsoft.WindowsAppSDK.WinUI.CSharp.Templates/0.0.8-alpha)。本專案保留 [Microsoft 著作權與 MIT 授權聲明](licenses/WinUI-Templates-MIT.txt)。

Git 追蹤原始碼、範本素材與套件參照。Windows App SDK 等執行階段由建置時還原，本機的 ZIP 與其他建置產物由 Git 排除。自行封裝 WinUI 執行檔時，請參考 [單檔 EXE 的授權與散布條件](winui/README.md#單檔-exe)。
