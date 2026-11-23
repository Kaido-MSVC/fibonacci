using System.Diagnostics;
using System.Runtime.InteropServices;

namespace FibonacciWinUI;

internal static partial class NativeFibonacci
{
    internal enum Status : int { Success, Invalid, TooLarge, Failed }

    internal readonly record struct Result(Status Status, ulong Index, string Digits, TimeSpan Elapsed);

    internal static Result Compute(string input)
    {
        long start = Stopwatch.GetTimestamp();
        Status status = ComputeNative(input, input.Length, out ulong index, out nint buffer);

        try
        {
            string digits = status == Status.Success ? Marshal.PtrToStringUTF8(buffer) ?? string.Empty : string.Empty;
            if (status == Status.Success && digits.Length == 0)
            {
                status = Status.Failed;
            }

            return new(status, index, digits, Stopwatch.GetElapsedTime(start));
        }
        finally
        {
            Marshal.FreeCoTaskMem(buffer);
        }
    }

    [LibraryImport("FibonacciNative.dll", EntryPoint = "fibonacci_compute", StringMarshalling = StringMarshalling.Utf16)]
    [DefaultDllImportSearchPaths(DllImportSearchPath.AssemblyDirectory | DllImportSearchPath.SafeDirectories)]
    private static partial Status ComputeNative(string input, int length, out ulong index, out nint digits);
}
