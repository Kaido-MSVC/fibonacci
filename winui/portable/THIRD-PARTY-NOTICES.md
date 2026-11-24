# 第三方授權與聲明

費波那契數列計算器的自有原始碼採 MIT 授權，著作權為 2026 Kaido-MSVC。以下元件適用各自的授權。

| 元件與來源版本 | ZIP 內的授權與聲明 |
| --- | --- |
| Boost.Multiprecision 1.92.0。標頭檔編譯至 `FibonacciNative.dll`。 | `licenses/Boost-1.92.0.txt` |
| Microsoft WinUI 3 C# Blank App 範本 0.0.8-alpha。含衍生程式碼與應用程式圖示。 | `licenses/WinUI-Templates-MIT.txt` |
| .NET 11.0.0 x64 執行階段與應用程式啟動器。 | `licenses/DotNet-11.0.0-LICENSE.txt`、`licenses/DotNet-11.0.0-THIRD-PARTY-NOTICES.txt` |
| Windows App SDK 2.7.0，包括 WinUI、Foundation、InteractiveExperiences、DWrite 及 Base。各套件隨附的授權條文相同。 | `licenses/WindowsAppSDK-2.7.0-LICENSE.txt`、`licenses/WindowsAppSDK-2.7.0-NOTICE.txt`、`licenses/WinUI-2.7.0-NOTICE.txt` |
| Windows SDK .NET 10.0.26100.86。`Microsoft.Windows.SDK.NET.dll` 和 `WinRT.Runtime.dll` 來自 `lib/net8.0`，以原始二進位檔附上。 | 隨包提供 Windows SDK 的授權及第三方聲明副本 `licenses/WindowsSDK-26100-LICENSE.rtf`、`licenses/WindowsSDK-26100-THIRD-PARTY-NOTICES.rtf`。套件指定的條款與清單見下方。 |
| WebView2 SDK 1.0.3719.77。WinUI 依賴的 SDK DLL 與載入器。 | `licenses/WebView2-1.0.3719.77-LICENSE.txt`、`licenses/WebView2-1.0.3719.77-NOTICE.txt` |

`FibonacciNative.dll` 以 Visual Studio 2026 的 MSVC 14.52 編譯，使用 `/MT` 靜態連結 C/C++ 執行階段。相關條款見 [Visual Studio 2026 授權目錄](https://visualstudio.microsoft.com/license-terms/)與[再散布清單](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution)。

## 微軟元件的再散布條件

- Windows App SDK 授權第 3(a) 節涵蓋 NuGet 套件放入應用程式輸出目錄的檔案。第 3(b) 節要求應用程式提供實質主要功能、要求下游散布者與外部使用者同意具同等或更高保護程度的條款，並由散布者承擔所列賠償及抗辯責任。請見隨附的完整條文及第 3(c) 節散布限制。[Windows App SDK 2.7.0 套件](https://www.nuget.org/packages/Microsoft.WindowsAppSDK/2.7.0)
- Windows SDK 授權第 2(a) 節另列有再散布權、同意、著作權標示、賠償及散布限制。Windows SDK REDIST 的 `Microsoft.Windows.SDK.NET.Ref` 段落列出 `lib/net8.0` 內的兩個 DLL。[套件 10.0.26100.86](https://www.nuget.org/packages/Microsoft.Windows.SDK.NET.Ref/10.0.26100.86) · [套件指定的 Windows SDK 授權](https://learn.microsoft.com/en-us/legal/windows-sdk/license) · [Windows SDK REDIST](https://learn.microsoft.com/en-us/legal/windows-sdk/redist#microsoftwindowssdknetref)
- 隨附條文可供讀取。公開 ZIP 的散布者與外部使用者如何表示同意，目前仍待確認。[Windows App SDK 公開討論](https://github.com/microsoft/WindowsAppSDK/discussions/6847)
