# TextEditor 截图辅助脚本
# 用法: pwsh -File cap.ps1 <输出名>
# 作用: 激活TextEditor主窗口并PrintWindow截图到当前目录
param([string]$name = "shot")

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class CapH {
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
}
"@

$p = Get-Process -Name "TextEditor" -ErrorAction SilentlyContinue
if (-not $p) { Write-Output "ERR:TextEditor 进程不存在"; exit 1 }
$h = $p.MainWindowHandle
[CapH]::ShowWindowAsync($h, 9) | Out-Null
[CapH]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 400
$r = New-Object CapH+RECT
[CapH]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $ht = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap($w, $ht)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[CapH]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc)
$out = Join-Path (Get-Location) ($name + ".png")
$bmp.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output ("OK:" + $out + " " + $w + "x" + $ht)
