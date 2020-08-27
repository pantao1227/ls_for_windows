# ls_for_windows

ls commad for Windows 

## 关于效率

目前 Windows 默认的命令行窗口的 C 程序还不支持ANSI控制，新的 Windows Terminal 支持。好在，现在 Windows 10 自带的命令支持 ANSI 控制字符。因此我先把需要显示的字符输出到文件，然后用 `type` 命令显示出来，因此从键入命令到显示结果略有延迟。