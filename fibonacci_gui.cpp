// 費波那契計算器 Win32 GUI 版。計算邏輯與主控台版共用 fibonacci_core.hpp。

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

#include <windows.h>
#include <roapi.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>

#include "fibonacci_core.hpp"

// 啟用 Common Controls v6，讓按鈕與輸入框使用目前的 Windows 視覺樣式。
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

class WindowsRuntimeApartment {
public:
    WindowsRuntimeApartment() noexcept : initialized(SUCCEEDED(RoInitialize(RO_INIT_SINGLETHREADED))) {}
    ~WindowsRuntimeApartment() {
        if (initialized)
            RoUninitialize();
    }

    WindowsRuntimeApartment(const WindowsRuntimeApartment&) = delete;
    WindowsRuntimeApartment& operator=(const WindowsRuntimeApartment&) = delete;

private:
    bool initialized;
};

constexpr wchar_t WINDOW_CLASS[] = L"FibonacciGuiWindow";
constexpr int MAX_INPUT_LENGTH = 20;
constexpr DWORD RESULT_POLL_MS = 25;

enum ControlId : int {
    idInput = 101,
    idCompute = IDOK, // Enter 鍵透過 IsDialogMessage 觸發預設按鈕
    idCopy = 102,
    idStatus = 103,
    idOutput = 104,
    idLabel = 105,
};

struct ComputeResult {
    std::uint64_t n;
    std::wstring digits;
    double seconds;
};

struct AppState {
    HWND label = nullptr;
    HWND input = nullptr;
    HWND compute = nullptr;
    HWND copy = nullptr;
    HWND status = nullptr;
    HWND output = nullptr;
    HFONT uiFont = nullptr;
    HFONT monoFont = nullptr;
    UINT dpi = USER_DEFAULT_SCREEN_DPI;
    std::future<ComputeResult> pendingResult;
    std::wstring displayedDigits;
};

AppState g_app;

int scale(int value) {
    return MulDiv(value, static_cast<int>(g_app.dpi), USER_DEFAULT_SCREEN_DPI);
}

void recreateFonts() {
    if (g_app.uiFont)
        DeleteObject(g_app.uiFont);
    if (g_app.monoFont)
        DeleteObject(g_app.monoFont);

    NONCLIENTMETRICSW metrics{};
    metrics.cbSize = sizeof(metrics);
    if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, g_app.dpi))
        g_app.uiFont = CreateFontIndirectW(&metrics.lfMessageFont);
    else
        g_app.uiFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    g_app.monoFont = CreateFontW(-MulDiv(10, static_cast<int>(g_app.dpi), 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE,
                                 FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 FIXED_PITCH | FF_MODERN, L"Consolas");

    for (HWND control : {g_app.label, g_app.input, g_app.compute, g_app.copy, g_app.status})
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(g_app.uiFont), TRUE);
    SendMessageW(g_app.output, WM_SETFONT, reinterpret_cast<WPARAM>(g_app.monoFont), TRUE);
}

void layout(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;

    const int margin = scale(10);
    const int rowHeight = scale(26);
    const int labelWidth = scale(150);
    const int inputWidth = scale(110);
    const int buttonWidth = scale(90);
    const int gap = scale(8);

    int x = margin;
    const int y = margin;
    MoveWindow(g_app.label, x, y + scale(4), labelWidth, rowHeight - scale(4), TRUE);
    x += labelWidth + gap;
    MoveWindow(g_app.input, x, y, inputWidth, rowHeight, TRUE);
    x += inputWidth + gap;
    MoveWindow(g_app.compute, x, y, buttonWidth, rowHeight, TRUE);
    x += buttonWidth + gap;
    MoveWindow(g_app.copy, x, y, buttonWidth, rowHeight, TRUE);

    const int statusY = y + rowHeight + gap;
    MoveWindow(g_app.status, margin, statusY, width - 2 * margin, rowHeight - scale(4), TRUE);

    const int outputY = statusY + rowHeight;
    MoveWindow(g_app.output, margin, outputY, width - 2 * margin, std::max(0, height - outputY - margin), TRUE);
}

void setBusy(bool busy) {
    EnableWindow(g_app.compute, !busy);
    EnableWindow(g_app.input, !busy);
    EnableWindow(g_app.copy, !busy && !g_app.displayedDigits.empty());
    SetCursor(LoadCursorW(nullptr, busy ? IDC_APPSTARTING : IDC_ARROW));
}

void startCompute() {
    if (g_app.pendingResult.valid())
        return;

    g_app.displayedDigits.clear();
    EnableWindow(g_app.copy, FALSE);
    if (!SetWindowTextW(g_app.output, L"")) {
        SetWindowTextW(g_app.status, L"無法清除上一筆結果。");
        return;
    }

    // 多讀一個字元，避免把截斷後的輸入當成完整項次。
    std::array<wchar_t, MAX_INPUT_LENGTH + 2> wide{};
    const int length = GetWindowTextW(g_app.input, wide.data(), static_cast<int>(wide.size()));
    if (length > MAX_INPUT_LENGTH) {
        SetWindowTextW(g_app.status, L"輸入過長，最多接受 20 個字元。");
        SetFocus(g_app.input);
        return;
    }

    // 只接受 ASCII，非 ASCII 字元直接視為格式錯誤。
    std::array<char, MAX_INPUT_LENGTH> narrow{};
    for (std::size_t i = 0; i < static_cast<std::size_t>(length); ++i)
        narrow[i] = wide[i] < 0x80 ? static_cast<char>(wide[i]) : '?';

    const auto index = parseIndex(trim(std::string_view(narrow.data(), static_cast<std::size_t>(length))));
    if (!index) {
        SetWindowTextW(g_app.status, index.error() == InputError::tooLarge
                                        ? L"超過目前設定的最大項次。"
                                        : L"輸入錯誤，請輸入有效的非負整數。");
        SetFocus(g_app.input);
        return;
    }

    const std::uint64_t n = *index;
    try {
        g_app.pendingResult = std::async(std::launch::async, [n] {
            const auto start = std::chrono::steady_clock::now();
            const std::string digits = fibonacci(n).first.convert_to<std::string>();
            const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            return ComputeResult{n, std::wstring(digits.begin(), digits.end()), seconds};
        });
    } catch (const std::exception&) {
        SetWindowTextW(g_app.status, L"無法啟動背景計算，請再試一次。");
        SetFocus(g_app.input);
        return;
    }

    setBusy(true);
    wchar_t status[96];
    swprintf_s(status, L"正在計算 F(%llu)...", static_cast<unsigned long long>(n));
    SetWindowTextW(g_app.status, status);
}

void finishCompute() {
    using namespace std::chrono_literals;
    if (!g_app.pendingResult.valid() || g_app.pendingResult.wait_for(0ms) != std::future_status::ready)
        return;

    try {
        auto result = g_app.pendingResult.get();
        if (SetWindowTextW(g_app.output, result.digits.c_str())) {
            wchar_t status[160];
            swprintf_s(status, L"F(%llu)：%zu 位數，耗時 %.3f 秒", static_cast<unsigned long long>(result.n),
                       result.digits.size(), result.seconds);
            g_app.displayedDigits = std::move(result.digits);
            SetWindowTextW(g_app.status, status);
        } else {
            SetWindowTextW(g_app.status, L"無法顯示計算結果。");
        }
    } catch (const std::exception&) {
        SetWindowTextW(g_app.status, L"計算失敗，請再試一次。");
    }

    setBusy(false);
    SetFocus(g_app.input);
    SendMessageW(g_app.input, EM_SETSEL, 0, -1);
}

enum class ClipboardResult { failed, copied, persistenceUnconfirmed };

ClipboardResult writeClipboard(const std::wstring& text) noexcept {
    using winrt::Windows::ApplicationModel::DataTransfer::Clipboard;
    using winrt::Windows::ApplicationModel::DataTransfer::DataPackage;

    try {
        const DataPackage data;
        data.SetText(text);
        Clipboard::SetContent(data);
    } catch (...) {
        return ClipboardResult::failed;
    }

    try {
        Clipboard::Flush();
        return ClipboardResult::copied;
    } catch (...) {
        return ClipboardResult::persistenceUnconfirmed;
    }
}

void copyOutput() {
    if (g_app.pendingResult.valid() || g_app.displayedDigits.empty())
        return;
    switch (writeClipboard(g_app.displayedDigits)) {
    case ClipboardResult::copied:
        SetWindowTextW(g_app.status, L"已複製到剪貼簿。");
        break;
    case ClipboardResult::persistenceUnconfirmed:
        SetWindowTextW(g_app.status, L"已寫入剪貼簿，請在關閉程式前貼上。");
        break;
    case ClipboardResult::failed:
        SetWindowTextW(g_app.status, L"無法複製到剪貼簿，請再試一次。");
        break;
    }
}

HWND createControl(HWND parent, const wchar_t* className, const wchar_t* text, DWORD style, int id,
                   DWORD exStyle = 0) {
    return CreateWindowExW(exStyle, className, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, parent,
                           reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        g_app.dpi = GetDpiForWindow(window);
        g_app.label = createControl(window, L"STATIC", (L"第 N 項（0–" + std::to_wstring(MAX_N) + L"）：").c_str(),
                                    SS_RIGHT, idLabel);
        g_app.input = createControl(window, L"EDIT", L"100", WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL, idInput,
                                    WS_EX_CLIENTEDGE);
        g_app.compute = createControl(window, L"BUTTON", L"計算", WS_TABSTOP | BS_DEFPUSHBUTTON, idCompute);
        g_app.copy = createControl(window, L"BUTTON", L"複製結果", WS_TABSTOP | BS_PUSHBUTTON, idCopy);
        g_app.status = createControl(window, L"STATIC", L"輸入項次後按 Enter 或「計算」。", SS_LEFT | SS_NOPREFIX,
                                     idStatus);
        g_app.output = createControl(window, L"EDIT", L"",
                                     WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                     idOutput, WS_EX_CLIENTEDGE);
        if (!g_app.label || !g_app.input || !g_app.compute || !g_app.copy || !g_app.status || !g_app.output)
            return -1;
        SendMessageW(g_app.input, EM_SETLIMITTEXT, MAX_INPUT_LENGTH + 1, 0);
        EnableWindow(g_app.copy, FALSE);
        recreateFonts();
        layout(window);
        SetFocus(g_app.input);
        SendMessageW(g_app.input, EM_SETSEL, 0, -1);
        return 0;

    case WM_SIZE:
        layout(window);
        return 0;

    case WM_GETMINMAXINFO: {
        auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
        info->ptMinTrackSize = {scale(520), scale(260)};
        return 0;
    }

    case WM_DPICHANGED: {
        g_app.dpi = HIWORD(wParam);
        recreateFonts();
        const auto* suggested = reinterpret_cast<const RECT*>(lParam);
        SetWindowPos(window, nullptr, suggested->left, suggested->top, suggested->right - suggested->left,
                     suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
        layout(window);
        return 0;
    }

    case WM_SETCURSOR:
        if (g_app.pendingResult.valid() && LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursorW(nullptr, IDC_APPSTARTING));
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case idCompute:
            startCompute();
            return 0;
        case idCopy:
            copyOutput();
            return 0;
        default:
            break;
        }
        break;

    case WM_CTLCOLORSTATIC:
        // 唯讀輸出框保持白底，比較好閱讀。
        if (reinterpret_cast<HWND>(lParam) == g_app.output) {
            SetBkColor(reinterpret_cast<HDC>(wParam), GetSysColor(COLOR_WINDOW));
            SetTextColor(reinterpret_cast<HDC>(wParam), GetSysColor(COLOR_WINDOWTEXT));
            return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
        }
        break;

    case WM_DESTROY:
        if (g_app.uiFont)
            DeleteObject(g_app.uiFont);
        if (g_app.monoFont)
            DeleteObject(g_app.monoFont);
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int showCommand) {
    const WindowsRuntimeApartment apartment;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    windowClass.lpszClassName = WINDOW_CLASS;
    if (!RegisterClassExW(&windowClass))
        return 1;

    // 先以 96 DPI 的大小建立，再依實際 DPI 調整。
    HWND window = CreateWindowExW(0, WINDOW_CLASS, L"費波那契數列計算器", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                  CW_USEDEFAULT, 720, 480, nullptr, nullptr, instance, nullptr);
    if (!window)
        return 1;

    const UINT dpi = GetDpiForWindow(window);
    RECT rect{0, 0, MulDiv(720, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI),
              MulDiv(440, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI)};
    AdjustWindowRectExForDpi(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);
    SetWindowPos(window, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    for (;;) {
        // 結果由 GUI 執行緒從行程內的 future 取得，閒置時只等視窗訊息。
        MSG message{};
        if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT)
                return static_cast<int>(message.wParam);
            if (!IsDialogMessageW(window, &message)) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            continue;
        }

        finishCompute();
        const DWORD timeout = g_app.pendingResult.valid() ? RESULT_POLL_MS : INFINITE;
        if (MsgWaitForMultipleObjectsEx(0, nullptr, timeout, QS_ALLINPUT, MWMO_INPUTAVAILABLE) == WAIT_FAILED)
            return 1;
    }
}
