# 費波那契數列計算器 WinUI 3 實驗版

這個實驗專案使用 C#、XAML、.NET 11 和 Windows App SDK 2.7.0。`FibonacciNative.dll` 使用 C++23 和 Boost.Multiprecision 1.92.0，直接編入根目錄的 `fibonacci_core.hpp`，沿用 Fast Doubling 演算法及輸入解析規則。

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

## 單檔 EXE

本機測試用的 Windows x64 單檔 EXE 包含 .NET 和 Windows App SDK，首次啟動會將執行階段解壓縮至本機暫存目錄。

若要從原始碼產生單檔 EXE，請在此目錄執行以下命令：

```powershell
dotnet publish .\FibonacciWinUI.csproj -c Release -r win-x64 -p:Platform=x64 -p:FibonacciPortable=true --artifacts-path .\obj\portable-artifacts -o .\obj\portable-publish
```

單檔執行檔位於 `obj\portable-publish\FibonacciWinUI.exe`。此命令產生本機建置輸出。

### 二進位散布

現有的 1.0.0 ZIP 供本機測試。內部的 EXE 含有額外的專案與建置檔案，ZIP 也缺少 WinUI 範本的著作權與授權聲明，以及 Windows SDK .NET 目標套件的授權說明。準備新版封裝時，請檢查 EXE 內的檔案，排除多餘的建置內容，依實際版本附上專案、範本、Boost 和各第三方元件的授權及聲明，並重建 SHA-256 檢查碼。

Windows App SDK 2.7.0 與 Windows ML 相關套件的授權第 3 節允許在符合條件下隨應用程式散布元件。散布者須要求下游散布者和外部使用者同意至少提供同等保護的條款。公開 ZIP 下載與首次啟動的同意方式仍待確認。Windows SDK .NET 目標套件則有自己的再散布條件。[Windows App SDK 2.7.0 授權](https://www.nuget.org/packages/Microsoft.WindowsAppSDK/2.7.0/License) · [Windows SDK 授權](https://learn.microsoft.com/en-us/legal/windows-sdk/license)
