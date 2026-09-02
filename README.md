# hellowinui

A minimal C++23 WinUI 3 desktop application. The UI is built entirely in
C++, with no XAML files.

## Setup

Requirements:

- Windows 10 version 1809 or newer
- CMake 3.21 or newer
- Ninja
- Visual Studio C++ build tools and a Windows SDK
- Windows App Runtime 1.8

CMake downloads the pinned Windows App SDK, C++/WinRT, WebView2 metadata,
and `fmt` packages during the first configuration. Network access is
required.

## Build and Run

From a Developer PowerShell or a configured C++ build environment:

```powershell
cmake -S . -B build -GNinja
ninja -C build
build\hellowinui.exe
```

The app uses unpackaged deployment, so the matching Windows App Runtime must
be installed on the machine before running it.
