#ifndef WUI_WUI_HPP
#define WUI_WUI_HPP

// wui - declarative WinUI 3 helpers with gpui-style builder chaining.
//
//     auto ui = wui::vbox()
//         .gap_3()
//         .bg(wui::rgb(0x505050))
//         .size(500)
//         .justify_center()
//         .items_center()
//         .border_1()
//         .border_color(wui::rgb(0x0000ff))
//         .text_xl()
//         .text_color(wui::rgb(0xffffff))
//         .child(L"Hello, World!")
//         .child(wui::hbox().gap_2()
//                 .child(wui::card().size_8().bg(wui::red())));
//     window.Content(ui.m_el);
//
// WinUI owns the window, rendering, and layout. wui only expresses sizing
// intent (fixed / content / stretch / star) so trees resize dynamically.
// Builders return element handles; copies of an element refer to the same
// control, so capture them in lambdas for later updates.
//
// Numeric ramps follow gpui's Tailwind scale (16px base): 1=4 2=8 3=12
// 4=16 5=20 6=24 7=28 8=32.

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string_view>

#include <windows.h>
#undef GetCurrentTime
#include <dwmapi.h>

#include "MddBootstrap.h"
#include "winrt/Microsoft.UI.Text.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "winrt/Microsoft.UI.Xaml.Controls.h"
#include "winrt/Microsoft.UI.Xaml.Input.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Media.h"
#include "winrt/Microsoft.UI.Xaml.XamlTypeInfo.h"
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Windowing.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Graphics.h"
#include "winrt/Windows.UI.h"
#include "winrt/base.h"

namespace wui {

// Short type aliases, mirroring rui::types.
namespace types {
using f64 = double;
using u8  = std::uint8_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
}

using namespace types;

namespace xaml = winrt::Microsoft::UI::Xaml;
namespace controls = xaml::Controls;
namespace media = xaml::Media;
namespace markup = xaml::Markup;
namespace xaml_type_info = xaml::XamlTypeInfo;
namespace interop = winrt::Windows::UI::Xaml::Interop;
namespace ui = winrt::Windows::UI;

// The window type, passed to run() callbacks.
using window = xaml::Window;

// --- Length spec for cols()/rows() definitions and size() ---

struct len {
    f64 m_value = 0.0;
    enum class unit : u8 { fit, star, px } m_unit = unit::fit;
};

inline auto fit() -> len { return {}; }
inline auto star(f64 value = 1.0) -> len { return {value, len::unit::star}; }
inline auto px(f64 value) -> len { return {value, len::unit::px}; }

// --- Colors ---

// Solid brush from a 0xRRGGBB hex value.
inline auto rgb(u32 hex) -> media::SolidColorBrush {
    ui::Color color{};
    color.R = static_cast<u8>((hex >> 16) & 0xFF);
    color.G = static_cast<u8>((hex >> 8) & 0xFF);
    color.B = static_cast<u8>(hex & 0xFF);
    color.A = 0xFF;
    return media::SolidColorBrush(color);
}

inline auto red() -> media::SolidColorBrush { return rgb(0xFF0000); }
inline auto green() -> media::SolidColorBrush { return rgb(0x00FF00); }
inline auto blue() -> media::SolidColorBrush { return rgb(0x0000FF); }
inline auto yellow() -> media::SolidColorBrush { return rgb(0xFFFF00); }
inline auto black() -> media::SolidColorBrush { return rgb(0x000000); }
inline auto white() -> media::SolidColorBrush { return rgb(0xFFFFFF); }

namespace detail {

auto to_grid_length(len const& spec) -> xaml::GridLength {
    switch (spec.m_unit) {
        case len::unit::star:
            return xaml::GridLengthHelper::FromValueAndType(
                spec.m_value, xaml::GridUnitType::Star);
        case len::unit::px:
            return xaml::GridLengthHelper::FromPixels(spec.m_value);
        default:
            return xaml::GridLengthHelper::Auto();
    }
}

auto to_column_definition(len const& spec) -> controls::ColumnDefinition {
    auto column = controls::ColumnDefinition();
    column.Width(to_grid_length(spec));
    return column;
}

auto to_row_definition(len const& spec) -> controls::RowDefinition {
    auto rowdef = controls::RowDefinition();
    rowdef.Height(to_grid_length(spec));
    return rowdef;
}

auto set_padding(xaml::FrameworkElement const& el, xaml::Thickness value)
    -> void {
    if (auto panel = el.try_as<controls::StackPanel>()) {
        panel.Padding(value);
    } else if (auto grid = el.try_as<controls::Grid>()) {
        grid.Padding(value);
    } else if (auto border = el.try_as<controls::Border>()) {
        border.Padding(value);
    }
}

auto set_background(xaml::FrameworkElement const& el, media::Brush const& value)
    -> void {
    if (auto panel = el.try_as<controls::Panel>()) {
        panel.Background(value);
    } else if (auto border = el.try_as<controls::Border>()) {
        border.Background(value);
    } else if (auto control = el.try_as<controls::Control>()) {
        control.Background(value);
    }
}

auto set_border_brush(xaml::FrameworkElement const& el, media::Brush const& value)
    -> void {
    if (auto border = el.try_as<controls::Border>()) {
        border.BorderBrush(value);
    } else if (auto grid = el.try_as<controls::Grid>()) {
        grid.BorderBrush(value);
    } else if (auto control = el.try_as<controls::Control>()) {
        control.BorderBrush(value);
    }
}

auto set_border_thickness(xaml::FrameworkElement const& el, xaml::Thickness value)
    -> void {
    if (auto border = el.try_as<controls::Border>()) {
        border.BorderThickness(value);
    } else if (auto grid = el.try_as<controls::Grid>()) {
        grid.BorderThickness(value);
    } else if (auto control = el.try_as<controls::Control>()) {
        control.BorderThickness(value);
    }
}

auto set_corner_radius(xaml::FrameworkElement const& el, xaml::CornerRadius value)
    -> void {
    if (auto border = el.try_as<controls::Border>()) {
        border.CornerRadius(value);
    } else if (auto grid = el.try_as<controls::Grid>()) {
        grid.CornerRadius(value);
    } else if (auto control = el.try_as<controls::Control>()) {
        control.CornerRadius(value);
    }
}

auto lookup_brush(winrt::hstring const& key) -> media::Brush {
    return xaml::Application::Current().Resources()
        .Lookup(winrt::box_value(key))
        .as<media::Brush>();
}

struct hwnd_search {
    DWORD m_pid;
    HWND m_hwnd;
};

auto WINAPI enum_windows_callback(HWND hwnd, LPARAM lparam) -> BOOL {
    auto* search = reinterpret_cast<hwnd_search*>(lparam);
    auto pid = DWORD{};
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == search->m_pid) {
        search->m_hwnd = hwnd;
        return FALSE;
    }
    return TRUE;
}

auto find_main_window() -> HWND {
    hwnd_search search{GetCurrentProcessId(), nullptr};
    EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&search));
    return search.m_hwnd;
}

auto update_title_bar_theme(xaml::Window const& win) -> void {
    auto const hwnd = find_main_window();
    if (!hwnd) {
        return;
    }
    auto content = win.Content().try_as<xaml::FrameworkElement>();
    auto const dark = static_cast<bool>(content) &&
        content.ActualTheme() == xaml::ElementTheme::Dark;
    auto const value = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(
        hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

}  // namespace detail

struct element;

// Forward declaration; defined below with the leaf builders.
auto txt(std::wstring_view s = {}) -> element;

// --- Element handle over a FrameworkElement ---

struct element {
    xaml::FrameworkElement m_el{nullptr};
    // Where child() adds elements: the inner panel for vbox/hbox, the
    // element itself otherwise.
    xaml::FrameworkElement m_host{nullptr};

    // Grid placement, applied by child() when the parent is a grid.
    i32 m_row = -1;
    i32 m_col = -1;
    i32 m_row_span = 0;
    i32 m_col_span = 0;

    // Font styling inherited by children, applied when they are added.
    f64 m_font_size = 0.0;
    bool m_has_foreground = false;
    media::Brush m_foreground{nullptr};

    explicit element(xaml::FrameworkElement el) : m_el(el), m_host(el) {}
    element(xaml::FrameworkElement el, xaml::FrameworkElement host)
        : m_el(el), m_host(host) {}

    operator xaml::FrameworkElement() const { return m_el; }

    // Escape hatch for anything wui does not model yet.
    template <typename T> auto as() const -> T { return m_el.as<T>(); }

    // -- sizing and alignment (FrameworkElement) --

    auto w(f64 v) -> element& { m_el.Width(v); return *this; }
    auto h(f64 v) -> element& { m_el.Height(v); return *this; }
    auto min_w(f64 v) -> element& { m_el.MinWidth(v); return *this; }
    auto min_h(f64 v) -> element& { m_el.MinHeight(v); return *this; }

    // Square sizing: sets both width and height.
    auto size(f64 v) -> element& {
        m_el.Width(v);
        m_el.Height(v);
        return *this;
    }
    auto size(len const& s) -> element& {
        if (s.m_unit == len::unit::px) {
            m_el.Width(s.m_value);
            m_el.Height(s.m_value);
        }
        return *this;
    }

    // Numeric size ramp, gpui scale: 0=0 1=4 2=8 3=12 4=16 5=20 6=24 7=28
    // 8=32.
    auto size_0() -> element& { return size(0.0); }
    auto size_1() -> element& { return size(4.0); }
    auto size_2() -> element& { return size(8.0); }
    auto size_3() -> element& { return size(12.0); }
    auto size_4() -> element& { return size(16.0); }
    auto size_5() -> element& { return size(20.0); }
    auto size_6() -> element& { return size(24.0); }
    auto size_7() -> element& { return size(28.0); }
    auto size_8() -> element& { return size(32.0); }

    auto fill() -> element& {
        m_el.HorizontalAlignment(xaml::HorizontalAlignment::Stretch);
        m_el.VerticalAlignment(xaml::VerticalAlignment::Stretch);
        return *this;
    }
    auto center() -> element& {
        m_el.HorizontalAlignment(xaml::HorizontalAlignment::Center);
        m_el.VerticalAlignment(xaml::VerticalAlignment::Center);
        return *this;
    }
    auto h_center() -> element& {
        m_el.HorizontalAlignment(xaml::HorizontalAlignment::Center);
        return *this;
    }
    auto v_center() -> element& {
        m_el.VerticalAlignment(xaml::VerticalAlignment::Center);
        return *this;
    }

    // Main-axis centering of the content within this container, like
    // gpui's justify-center. vbox/hbox wrap their stack panel in a grid
    // cell, so the panel can be centered while the container still fills
    // its allocation.
    auto justify_center() -> element& {
        if (auto panel = m_host.try_as<controls::StackPanel>()) {
            if (panel.Orientation() == controls::Orientation::Vertical) {
                m_host.VerticalAlignment(xaml::VerticalAlignment::Center);
            } else {
                m_host.HorizontalAlignment(xaml::HorizontalAlignment::Center);
            }
        }
        return *this;
    }

    // Cross-axis centering of the content within this container.
    auto items_center() -> element& {
        if (auto panel = m_host.try_as<controls::StackPanel>()) {
            if (panel.Orientation() == controls::Orientation::Vertical) {
                m_host.HorizontalAlignment(xaml::HorizontalAlignment::Center);
            } else {
                m_host.VerticalAlignment(xaml::VerticalAlignment::Center);
            }
        }
        return *this;
    }

    // -- margin, padding, spacing --

    auto margin(f64 v) -> element& {
        m_el.Margin(xaml::Thickness{v, v, v, v});
        return *this;
    }
    auto margin(f64 l, f64 t, f64 r, f64 b) -> element& {
        m_el.Margin(xaml::Thickness{l, t, r, b});
        return *this;
    }
    auto padding(f64 v) -> element& {
        detail::set_padding(m_el, xaml::Thickness{v, v, v, v});
        return *this;
    }
    auto padding(f64 l, f64 t, f64 r, f64 b) -> element& {
        detail::set_padding(m_el, xaml::Thickness{l, t, r, b});
        return *this;
    }

    // StackPanel spacing, or both grid spacings on a grid.
    auto gap(f64 v) -> element& {
        if (auto panel = m_host.try_as<controls::StackPanel>()) {
            panel.Spacing(v);
        } else if (auto grid = m_el.try_as<controls::Grid>()) {
            grid.RowSpacing(v);
            grid.ColumnSpacing(v);
        }
        return *this;
    }

    // Numeric gap ramp, gpui scale: 0=0 1=4 2=8 3=12 4=16 5=20 6=24 7=28
    // 8=32.
    auto gap_0() -> element& { return gap(0.0); }
    auto gap_1() -> element& { return gap(4.0); }
    auto gap_2() -> element& { return gap(8.0); }
    auto gap_3() -> element& { return gap(12.0); }
    auto gap_4() -> element& { return gap(16.0); }
    auto gap_5() -> element& { return gap(20.0); }
    auto gap_6() -> element& { return gap(24.0); }
    auto gap_7() -> element& { return gap(28.0); }
    auto gap_8() -> element& { return gap(32.0); }

    auto row_gap(f64 v) -> element& {
        m_el.as<controls::Grid>().RowSpacing(v);
        return *this;
    }
    auto col_gap(f64 v) -> element& {
        m_el.as<controls::Grid>().ColumnSpacing(v);
        return *this;
    }

    // -- appearance --

    auto bg(media::Brush const& brush) -> element& {
        detail::set_background(m_el, brush);
        return *this;
    }
    auto border(f64 v) -> element& {
        detail::set_border_thickness(m_el, xaml::Thickness{v, v, v, v});
        return *this;
    }
    auto border_0() -> element& { return border(0.0); }
    auto border_1() -> element& { return border(1.0); }
    auto border_2() -> element& { return border(2.0); }
    auto border_color(media::Brush const& brush) -> element& {
        detail::set_border_brush(m_el, brush);
        return *this;
    }
    auto rounded_px(f64 v) -> element& {
        detail::set_corner_radius(m_el, xaml::CornerRadius{v, v, v, v});
        return *this;
    }
    auto style(xaml::Style const& value) -> element& {
        m_el.as<controls::Control>().Style(value);
        return *this;
    }

    // -- content and text --

    auto content(std::wstring_view s) -> element& {
        m_el.as<controls::ContentControl>()
            .Content(winrt::box_value(std::wstring{s}));
        return *this;
    }
    auto text(std::wstring_view s) -> element& {
        if (auto block = m_el.try_as<controls::TextBlock>()) {
            block.Text(s);
        } else if (auto box = m_el.try_as<controls::TextBox>()) {
            box.Text(s);
        }
        return *this;
    }
    auto wrap(bool value = true) -> element& {
        auto const mode =
            value ? xaml::TextWrapping::Wrap : xaml::TextWrapping::NoWrap;
        if (auto block = m_el.try_as<controls::TextBlock>()) {
            block.TextWrapping(mode);
        } else if (auto box = m_el.try_as<controls::RichEditBox>()) {
            box.TextWrapping(mode);
        }
        return *this;
    }
    auto password(std::wstring_view s) -> element& {
        m_el.as<controls::PasswordBox>().Password(s);
        return *this;
    }
    auto header(std::wstring_view s) -> element& {
        m_el.as<controls::Slider>()
            .Header(winrt::box_value(std::wstring{s}));
        return *this;
    }
    auto message(std::wstring_view s) -> element& {
        m_el.as<controls::InfoBar>().Message(s);
        return *this;
    }

    // Font size, applied to text blocks and controls, and inherited by
    // children added after this call.
    auto text_size(f64 v) -> element& {
        if (auto block = m_el.try_as<controls::TextBlock>()) {
            block.FontSize(v);
        } else if (auto control = m_el.try_as<controls::Control>()) {
            control.FontSize(v);
        }
        m_font_size = v;
        return *this;
    }

    // Tailwind font sizes on a 16px base.
    auto text_xs() -> element& { return text_size(12.0); }
    auto text_sm() -> element& { return text_size(14.0); }
    auto text_base() -> element& { return text_size(16.0); }
    auto text_lg() -> element& { return text_size(18.0); }
    auto text_xl() -> element& { return text_size(20.0); }
    auto text_2xl() -> element& { return text_size(24.0); }
    auto text_3xl() -> element& { return text_size(30.0); }

    // Foreground color for text blocks and controls, inherited by children
    // added after this call.
    auto text_color(media::Brush const& brush) -> element& {
        if (auto block = m_el.try_as<controls::TextBlock>()) {
            block.Foreground(brush);
        } else if (auto control = m_el.try_as<controls::Control>()) {
            control.Foreground(brush);
        }
        m_foreground = brush;
        m_has_foreground = true;
        return *this;
    }

    // -- values --

    auto value(f64 v) -> element& {
        if (auto slider = m_el.try_as<controls::Slider>()) {
            slider.Value(v);
        } else if (auto num = m_el.try_as<controls::NumberBox>()) {
            num.Value(v);
        }
        return *this;
    }
    auto min(f64 v) -> element& {
        if (auto slider = m_el.try_as<controls::Slider>()) {
            slider.Minimum(v);
        } else if (auto num = m_el.try_as<controls::NumberBox>()) {
            num.Minimum(v);
        }
        return *this;
    }
    auto max(f64 v) -> element& {
        if (auto slider = m_el.try_as<controls::Slider>()) {
            slider.Maximum(v);
        } else if (auto num = m_el.try_as<controls::NumberBox>()) {
            num.Maximum(v);
        }
        return *this;
    }
    auto step(f64 v) -> element& {
        if (auto slider = m_el.try_as<controls::Slider>()) {
            slider.StepFrequency(v);
        } else if (auto num = m_el.try_as<controls::NumberBox>()) {
            num.SmallChange(v);
        }
        return *this;
    }
    auto checked(bool v) -> element& {
        if (auto check = m_el.try_as<controls::CheckBox>()) {
            check.IsChecked(v);
        } else if (auto radio = m_el.try_as<controls::RadioButton>()) {
            radio.IsChecked(v);
        }
        return *this;
    }
    auto group(std::wstring_view name) -> element& {
        m_el.as<controls::RadioButton>().GroupName(name);
        return *this;
    }
    auto on(bool v) -> element& {
        m_el.as<controls::ToggleSwitch>().IsOn(v);
        return *this;
    }
    auto off_content(std::wstring_view s) -> element& {
        m_el.as<controls::ToggleSwitch>()
            .OffContent(winrt::box_value(std::wstring{s}));
        return *this;
    }
    auto on_content(std::wstring_view s) -> element& {
        m_el.as<controls::ToggleSwitch>()
            .OnContent(winrt::box_value(std::wstring{s}));
        return *this;
    }
    auto items(std::initializer_list<std::wstring_view> values) -> element& {
        auto box = m_el.as<controls::AutoSuggestBox>();
        for (auto const& v : values) {
            box.Items().Append(winrt::box_value(std::wstring{v}));
        }
        return *this;
    }
    auto query_icon(controls::Symbol symbol) -> element& {
        m_el.as<controls::AutoSuggestBox>().QueryIcon(controls::SymbolIcon{symbol});
        return *this;
    }

    // -- grid placement and definitions --

    auto row(i32 v) -> element& { m_row = v; return *this; }
    auto col(i32 v) -> element& { m_col = v; return *this; }
    auto row_span(i32 v) -> element& { m_row_span = v; return *this; }
    auto col_span(i32 v) -> element& { m_col_span = v; return *this; }

    // Column and row definitions; brace or comma form:
    //     .cols({fit(), star()})  or  .cols(fit(), star())

    auto cols(std::initializer_list<len> const& defs) -> element& {
        auto grid = m_el.as<controls::Grid>();
        for (auto const& def : defs) {
            grid.ColumnDefinitions()
                .Append(detail::to_column_definition(def));
        }
        return *this;
    }
    template <typename... Specs> auto cols(Specs&&... specs) -> element& {
        auto grid = m_el.as<controls::Grid>();
        (grid.ColumnDefinitions().Append(
             detail::to_column_definition(static_cast<len const&>(specs))),
         ...);
        return *this;
    }
    auto rows(std::initializer_list<len> const& defs) -> element& {
        auto grid = m_el.as<controls::Grid>();
        for (auto const& def : defs) {
            grid.RowDefinitions().Append(detail::to_row_definition(def));
        }
        return *this;
    }
    template <typename... Specs> auto rows(Specs&&... specs) -> element& {
        auto grid = m_el.as<controls::Grid>();
        (grid.RowDefinitions().Append(
             detail::to_row_definition(static_cast<len const&>(specs))),
         ...);
        return *this;
    }

    // -- children --

    auto child(element const& kid) -> element& {
        if (auto grid = m_host.try_as<controls::Grid>()) {
            grid.Children().Append(kid.m_el);
            if (kid.m_row >= 0) {
                controls::Grid::SetRow(kid.m_el, kid.m_row);
            }
            if (kid.m_col >= 0) {
                controls::Grid::SetColumn(kid.m_el, kid.m_col);
            }
            if (kid.m_row_span > 0) {
                controls::Grid::SetRowSpan(kid.m_el, kid.m_row_span);
            }
            if (kid.m_col_span > 0) {
                controls::Grid::SetColumnSpan(kid.m_el, kid.m_col_span);
            }
        } else if (auto panel = m_host.try_as<controls::Panel>()) {
            panel.Children().Append(kid.m_el);
        } else if (auto border = m_host.try_as<controls::Border>()) {
            border.Child(kid.m_el);
        }
        // Handles are shareable, so styling a const child is safe.
        auto& self_kid = const_cast<element&>(kid);
        if (m_font_size > 0.0) {
            self_kid.text_size(m_font_size);
        }
        if (m_has_foreground) {
            self_kid.text_color(m_foreground);
        }
        return *this;
    }

    // Plain text child, like gpui's child(format!(...)).
    auto child(std::wstring_view text) -> element& { return child(txt(text)); }
};

// --- Container builders ---

// A stack panel wrapped in a single-cell star grid. The wrapper fills its
// allocation (so bg/border/padding cover the whole container) while the
// inner panel can be centered within it, matching gpui's flex semantics.
namespace detail {
auto make_stack(controls::Orientation orientation) -> element {
    auto panel = controls::StackPanel();
    panel.Orientation(orientation);
    auto outer = controls::Grid();
    auto rowdef = controls::RowDefinition();
    rowdef.Height(detail::to_grid_length({1.0, len::unit::star}));
    outer.RowDefinitions().Append(rowdef);
    auto colddef = controls::ColumnDefinition();
    colddef.Width(detail::to_grid_length({1.0, len::unit::star}));
    outer.ColumnDefinitions().Append(colddef);
    outer.Children().Append(panel);
    return element(outer, panel);
}
}  // namespace detail

auto vbox() -> element {
    return detail::make_stack(controls::Orientation::Vertical);
}

auto hbox() -> element {
    return detail::make_stack(controls::Orientation::Horizontal);
}

auto grid() -> element { return element(controls::Grid()); }

// Bordered box for card-like surfaces; takes a single child.
auto card() -> element { return element(controls::Border()); }

// --- Leaf builders ---

auto txt(std::wstring_view s) -> element {
    auto block = controls::TextBlock();
    if (!s.empty()) {
        block.Text(s);
    }
    return element(block);
}

auto btn(std::wstring_view s = {}, std::function<void()> const& handler = {})
    -> element {
    auto button = controls::Button();
    button.Content(winrt::box_value(std::wstring{s}));
    if (handler) {
        auto fn = handler;
        button.Click([fn](winrt::Windows::Foundation::IInspectable const&,
                          xaml::RoutedEventArgs const&) { fn(); });
    }
    return element(button);
}

// Icon-only button with the standard transparent hover idiom. Requires
// XamlControlsResources to be loaded.
auto sym_btn(controls::Symbol symbol) -> element {
    auto button = controls::Button();
    auto const transparent =
        detail::lookup_brush(L"ControlFillColorTransparentBrush");
    auto const hover =
        detail::lookup_brush(L"ControlFillColorSecondaryBrush");
    button.Content(controls::SymbolIcon{symbol});
    button.Width(40.0);
    button.Height(32.0);
    button.MinWidth(0.0);
    button.MinHeight(0.0);
    button.Padding(xaml::Thickness{});
    button.CornerRadius(xaml::CornerRadius{4.0, 4.0, 4.0, 4.0});
    button.HorizontalContentAlignment(xaml::HorizontalAlignment::Center);
    button.VerticalContentAlignment(xaml::VerticalAlignment::Center);
    button.Background(transparent);
    button.BorderBrush(transparent);
    button.BorderThickness(xaml::Thickness{});
    button.PointerEntered([hover](
        winrt::Windows::Foundation::IInspectable const& sender,
        xaml::Input::PointerRoutedEventArgs const&) {
        sender.as<controls::Button>().Background(hover);
    });
    button.PointerExited([transparent](
        winrt::Windows::Foundation::IInspectable const& sender,
        xaml::Input::PointerRoutedEventArgs const&) {
        sender.as<controls::Button>().Background(transparent);
    });
    return element(button);
}

auto input() -> element { return element(controls::TextBox()); }
auto pwd() -> element { return element(controls::PasswordBox()); }
auto num() -> element { return element(controls::NumberBox()); }
auto suggest() -> element { return element(controls::AutoSuggestBox()); }

auto editor(std::wstring_view s = {}) -> element {
    auto box = controls::RichEditBox();
    box.AcceptsReturn(true);
    box.TextWrapping(xaml::TextWrapping::Wrap);
    if (!s.empty()) {
        box.Document().SetText(
            winrt::Microsoft::UI::Text::TextSetOptions::None, s);
    }
    return element(box);
}

auto slider() -> element { return element(controls::Slider()); }
auto toggle() -> element { return element(controls::ToggleSwitch()); }
auto check() -> element { return element(controls::CheckBox()); }
auto radio() -> element { return element(controls::RadioButton()); }

auto info(std::wstring_view title, std::wstring_view message) -> element {
    auto bar = controls::InfoBar();
    bar.IsOpen(true);
    bar.IsClosable(true);
    bar.Title(title);
    bar.Message(message);
    return element(bar);
}

// --- Window helpers ---

// Application resources, for theme lookups like CardBackground...Brush.
inline auto resources() -> xaml::ResourceDictionary {
    return xaml::Application::Current().Resources();
}

// Keep the DWM title bar in sync with the app theme. Call once after
// window.Content is set; the HWND may not exist until Activate().
inline auto sync_title_bar(xaml::Window const& win) -> void {
    detail::update_title_bar_theme(win);
    if (auto content = win.Content().try_as<xaml::FrameworkElement>()) {
        content.ActualThemeChanged(
            [win](xaml::FrameworkElement const&,
                  winrt::Windows::Foundation::IInspectable const&) {
                detail::update_title_bar_theme(win);
            });
    }
    win.Activated([win](
        winrt::Windows::Foundation::IInspectable const&,
        xaml::WindowActivatedEventArgs const&) {
        detail::update_title_bar_theme(win);
    });
}

// --- App entry point ---

namespace detail {

using ui_fn = std::function<void(xaml::Window&)>;
inline ui_fn s_ui{};

struct app : xaml::ApplicationT<app, markup::IXamlMetadataProvider> {
    auto OnLaunched(xaml::LaunchActivatedEventArgs const&) -> void {
        m_window = xaml::Window();
        Resources().MergedDictionaries()
            .Append(controls::XamlControlsResources{});
        m_window.SystemBackdrop(xaml::Media::MicaBackdrop{});
        s_ui(m_window);
        sync_title_bar(m_window);
        m_window.Activate();
    }

    auto GetXamlType(interop::TypeName const& type) -> markup::IXamlType {
        return m_provider.GetXamlType(type);
    }

    auto GetXamlType(winrt::hstring const& full_name) -> markup::IXamlType {
        return m_provider.GetXamlType(full_name);
    }

    auto GetXmlnsDefinitions() -> winrt::com_array<markup::XmlnsDefinition> {
        return m_provider.GetXmlnsDefinitions();
    }

private:
    xaml::Window m_window{nullptr};
    xaml_type_info::XamlControlsXamlMetaDataProvider m_provider;
};

}  // namespace detail

// Open a window, call build with it, and activate. The title bar follows
// the app theme automatically.
template <typename Fn> auto run(Fn&& build) -> int {
    return run(940.0, 608.0, std::forward<Fn>(build));
}
template <typename Fn> auto run(f64 width, f64 height, Fn&& build) -> int {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto const result = MddBootstrapInitialize2(
        WINDOWSAPPSDK_RELEASE_MAJORMINOR,
        nullptr,
        PACKAGE_VERSION{},
        MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
    if (FAILED(result)) {
        return result;
    }

    detail::s_ui = [width, height, build](xaml::Window& win) mutable {
        win.AppWindow().Resize(winrt::Windows::Graphics::SizeInt32{
            static_cast<i32>(width), static_cast<i32>(height)});
        build(win);
    };
    xaml::Application::Start([](auto&&) { winrt::make<detail::app>(); });
    MddBootstrapShutdown();
    return 0;
}

}  // namespace wui

#endif  // !WUI_WUI_HPP
