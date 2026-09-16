# hellowinui

A C++23 WinUI 3 desktop application with no XAML files. The UI is built
entirely in C++ on top of `wui`, a small header-only, gpui-style
declarative builder for the Windows App SDK.

## Demos

| Target       | What it shows                                          |
| ------------ | ------------------------------------------------------ |
| `hellowinui` | Minimal centered layout with colored dots              |
| `winuiex`    | Component showcase: cards, info bar, sliders, toggles  |
| `reactive`   | Responsive breakpoints; panels reflow on window resize |

## wui

`wui/wui.hpp` is a header-only C++23 helper library. It wraps WinUI 3
controls in a chainable builder with gpui-like syntax:

```cpp
auto root = vbox()
                .gap_3()
                .bg(rgb(0x505050))
                .justify_center()
                .child(L"Hello, World!")
                .child(hbox().gap_2()
                          .child(card().size_8().bg(red()))
                          .child(card().size_8().bg(green())));
win.Content(root.m_el);
```

- `wui::run(w, h, build)` hides the app bootstrap; demo files only
  describe the UI tree.
- Chained modifiers: spacing/size ramps (`gap_3`, `size_8`), colors
  (`rgb(0x...)`, named colors), font presets (`text_xs` to `text_3xl`).
- `.child(L"text")` wraps text; `.child(element)` nests containers.
- Grids take `rows()`/`cols()` of `fit()`, `star()`, or pixel lengths,
  with row/column placement per child.
- `on_resize(fn)` fires with the element's DIP size during the layout
  pass, so handlers should only change properties (text, colors, grid
  placement), never the visual tree.

## Setup

Requirements:

- Windows 10 version 1809 or newer
- CMake 3.21 or newer
- Ninja
- Visual Studio C++ build tools and a Windows SDK
- Windows App Runtime 2.4

CMake downloads the pinned Windows App SDK, C++/WinRT, WebView2 metadata,
and `fmt` packages during the first configuration. Network access is
required.

## Build and Run

```powershell
cmake -S . -B build -GNinja
ninja -C build
```

Run any demo from a Developer PowerShell or a configured C++ build
environment:

```powershell
build\hellowinui.exe
build\winuiex.exe
build\reactive.exe
```

The apps use unpackaged deployment, so the matching Windows App Runtime 2.4
must be installed on the machine before running them.
