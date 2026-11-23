using System.Runtime.InteropServices;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Windows.ApplicationModel.DataTransfer;
using Windows.System;

namespace FibonacciWinUI;

public sealed partial class MainPage : Page
{
    private const int MaxInputLength = 20;
    private const int DigitsPerPage = 4096;
    private string _digits = string.Empty;
    private int _page;
    private bool _isCalculating;

    public MainPage()
    {
        InitializeComponent();
    }

    private async void AboutButton_Click(object sender, RoutedEventArgs e)
    {
        Version? version = typeof(MainPage).Assembly.GetName().Version;
        var licenseText = new TextBlock
        {
            IsTextSelectionEnabled = true,
            TextWrapping = TextWrapping.Wrap,
            Text = $"費波那契計算器 {version?.ToString(3)}\n\n"
                + $"專案授權\n{ReadLicense("Fibonacci.License")}\n\n"
                + $"Boost.Multiprecision 1.92.0\n{ReadLicense("Fibonacci.BoostLicense")}",
        };
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "關於與授權",
            CloseButtonText = "關閉",
            DefaultButton = ContentDialogButton.Close,
            Content = new ScrollViewer
            {
                Content = licenseText,
                MaxHeight = 270,
                VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
            },
        };
        await dialog.ShowAsync();
    }

    private static string ReadLicense(string resourceName)
    {
        using Stream? stream = typeof(MainPage).Assembly.GetManifestResourceStream(resourceName);
        if (stream is null)
        {
            return "無法讀取授權條文。";
        }

        using var reader = new StreamReader(stream);
        return reader.ReadToEnd().TrimEnd();
    }

    private void IndexInput_Loaded(object sender, RoutedEventArgs e)
    {
        FocusIndexInput();
    }

    private async void CalculateButton_Click(object sender, RoutedEventArgs e)
    {
        await CalculateAsync();
    }

    private async void IndexInput_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (e.Key == VirtualKey.Enter)
        {
            e.Handled = true;
            await CalculateAsync();
        }
    }

    private async Task CalculateAsync()
    {
        if (_isCalculating)
        {
            return;
        }

        string input = IndexInput.Text;
        Feedback.IsOpen = false;
        _digits = string.Empty;
        _page = 0;
        CopyButton.IsEnabled = false;
        PageControls.Visibility = Visibility.Collapsed;
        ResultOutput.Text = string.Empty;
        ResultHeading.Text = "計算結果";
        ResultDetails.Text = "尚無計算結果。";

        if (input.Length > MaxInputLength)
        {
            ShowFeedback(InfoBarSeverity.Error, "輸入最多可包含 20 個字元。");
            FocusIndexInput();
            return;
        }

        SetCalculating(true);
        ResultDetails.Text = "正在計算…";
        try
        {
            NativeFibonacci.Result result = await Task.Run(() => NativeFibonacci.Compute(input));
            switch (result.Status)
            {
                case NativeFibonacci.Status.Success:
                    _digits = result.Digits;
                    ResultHeading.Text = $"F({result.Index:N0})";
                    ResultDetails.Text = $"{_digits.Length:N0} 位數，耗時 {result.Elapsed.TotalSeconds:F3} 秒";
                    ShowPage();
                    CopyButton.IsEnabled = true;
                    break;
                case NativeFibonacci.Status.Invalid:
                    ShowCalculationError("請輸入 0 到 1,000,000 之間的整數，只使用半形數字。");
                    break;
                case NativeFibonacci.Status.TooLarge:
                    ShowCalculationError("項次上限為 1,000,000。");
                    break;
                default:
                    ShowCalculationError("計算失敗，請再試一次。");
                    break;
            }
        }
        catch (Exception exception) when (exception is DllNotFoundException or EntryPointNotFoundException or BadImageFormatException)
        {
            ShowCalculationError("無法載入運算元件，請重新建置 WinUI 專案。");
        }
        finally
        {
            SetCalculating(false);
            FocusIndexInput();
        }
    }

    private void SetCalculating(bool calculating)
    {
        _isCalculating = calculating;
        IndexInput.IsEnabled = !calculating;
        CalculateButton.IsEnabled = !calculating;
        CalculationProgress.IsIndeterminate = calculating;
        CalculationProgress.Visibility = calculating ? Visibility.Visible : Visibility.Collapsed;
    }

    private void FocusIndexInput()
    {
        IndexInput.Focus(FocusState.Programmatic);
        IndexInput.SelectAll();
    }

    private void ShowCalculationError(string message)
    {
        ResultDetails.Text = "尚無計算結果。";
        ShowFeedback(InfoBarSeverity.Error, message);
    }

    private void ShowFeedback(InfoBarSeverity severity, string message)
    {
        Feedback.Severity = severity;
        Feedback.Message = message;
        Feedback.IsOpen = true;
    }

    private void ShowPage()
    {
        int start = _page * DigitsPerPage;
        int length = Math.Min(DigitsPerPage, _digits.Length - start);
        int pages = (_digits.Length + DigitsPerPage - 1) / DigitsPerPage;
        ResultOutput.Text = _digits.Substring(start, length);
        ResultOutput.Select(0, 0);
        PageControls.Visibility = pages > 1 ? Visibility.Visible : Visibility.Collapsed;
        PageLabel.Text = $"第 {_page + 1} 頁，共 {pages} 頁";
        PreviousPageButton.IsEnabled = _page > 0;
        NextPageButton.IsEnabled = _page + 1 < pages;
    }

    private void PreviousPageButton_Click(object sender, RoutedEventArgs e)
    {
        _page--;
        ShowPage();
        if (!PreviousPageButton.IsEnabled)
        {
            NextPageButton.Focus(FocusState.Programmatic);
        }
    }

    private void NextPageButton_Click(object sender, RoutedEventArgs e)
    {
        _page++;
        ShowPage();
        if (!NextPageButton.IsEnabled)
        {
            PreviousPageButton.Focus(FocusState.Programmatic);
        }
    }

    private void CopyButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            var data = new DataPackage();
            data.SetText(_digits);
            Clipboard.SetContent(data);
            Clipboard.Flush();
            ShowFeedback(InfoBarSeverity.Success, "已將完整數字複製到剪貼簿。");
        }
        catch (Exception exception) when (exception is COMException or UnauthorizedAccessException)
        {
            ShowFeedback(InfoBarSeverity.Error, "無法複製到剪貼簿，請再試一次。");
        }
    }
}
