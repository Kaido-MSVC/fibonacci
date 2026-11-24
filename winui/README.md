# 費波那契數列計算器 WinUI 3 版

這個專案使用 C#、XAML、.NET 11 和 Windows App SDK 2.7.0。`FibonacciNative.dll` 使用 C++23 和 Boost.Multiprecision 1.92.0，直接編入根目錄的 `fibonacci_core.hpp`，沿用 Fast Doubling 演算法及輸入解析規則。

## 建置與啟動

- 目前支援 x64 建置。需要 .NET 11 SDK，開發套件的註冊與啟動另需 WinApp CLI 0.8.0 以上及 Windows 開發人員模式。
- 原生部分使用 Visual Studio 2026、MSVC x64 14.52、Windows SDK 10.0.28000.0 與 Boost 1.92.0。Boost 的下載、雜湊與標頭檔位置請參考[根目錄說明](../README.md)。

從專案根目錄執行以下 PowerShell 指令。每次建置都會使用目前的 Boost 標頭重新編譯原生 DLL。`winapp run` 會建置、註冊並啟動本機開發套件。

```powershell
Set-Location .\winui
winapp run . --arch x64 --devtools off --detach --output-appx-directory .\obj\AppX\Debug
```

只編譯時，可在 `winui` 目錄執行 `dotnet build .\FibonacciWinUI.csproj -c Debug -p:Platform=x64`。

## 操作方式

輸入 0 到 1,000,000 的項次後按 Enter 或「計算」。結果會列出位數及耗時。超過 4,096 位時可逐頁瀏覽，「複製結果」會將所有位數一起複製到剪貼簿。

主畫面右上角的「關於」會顯示版本、MIT 授權及 Boost Software License 1.0 的完整條文。

## ZIP 封裝

Windows x64 ZIP 採免安裝、自包含的資料夾部署。請完整解壓縮，再執行 `FibonacciWinUI.exe`。啟動程式需要同一資料夾中的 DLL、資源和設定檔。

從 `winui` 目錄執行下列命令，將 Release 建置輸出到新的空目錄。`FibonacciPortable` 啟用 ZIP 專用設定，包括 WinUI 及 DWrite 元件參照，以及 Windows SDK .NET 10.0.26100.86 的 `lib/net8.0` 投影套件。

```powershell
dotnet publish .\FibonacciWinUI.csproj -c Release -r win-x64 -p:Platform=x64 -p:FibonacciPortable=true --artifacts-path .\obj\zip-artifacts -o .\obj\zip-publish
```

從 `obj\zip-publish` 取用執行檔及同目錄的執行階段檔案。把 `portable/` 內的兩份文件及根目錄 `LICENSE` 放在 ZIP 內的執行檔旁，依第三方聲明的對照表附上 `licenses/` 內的授權與聲明。核對實際檔案、測試解壓後的程式，再產生 ZIP 與 SHA-256。執行階段升版後須重新核對授權及散布內容。

### 二進位散布

舊版 1.0.0 單檔 ZIP 供本機測試。1.0.1 是資料夾式 ZIP，移除 Windows ML、AI 等未使用的套件。含微軟再散布元件的公開下載仍需處理下游散布者與外部使用者的條款同意方式。請查閱[隨包第三方授權與散布條件](portable/THIRD-PARTY-NOTICES.md)。
