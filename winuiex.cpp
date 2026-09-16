#include <windows.h>
#include <dwmapi.h>

#undef GetCurrentTime

#include "MddBootstrap.h"
#include "winrt/Microsoft.UI.Text.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "winrt/Microsoft.UI.Xaml.Controls.h"
#include "winrt/Microsoft.UI.Xaml.Input.h"
#include "winrt/Microsoft.UI.Xaml.Media.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.XamlTypeInfo.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Graphics.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Microsoft.UI.Windowing.h"
#include "winrt/base.h"
#include "fmt/format.h"

#include "wui/wui.hpp"

namespace xaml = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;
namespace markup = winrt::Microsoft::UI::Xaml::Markup;
namespace xaml_type_info = winrt::Microsoft::UI::Xaml::XamlTypeInfo;
namespace interop = winrt::Windows::UI::Xaml::Interop;

namespace {
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

auto apply_title_bar_theme(HWND hwnd, bool const dark) -> void {
    auto const value = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(
        hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}
}

class app : public xaml::ApplicationT<app, markup::IXamlMetadataProvider> {
public:
    auto OnLaunched(xaml::LaunchActivatedEventArgs const&) -> void {
        m_window = xaml::Window();

        Resources().MergedDictionaries().Append(controls::XamlControlsResources{});
        m_window.SystemBackdrop(xaml::Media::MicaBackdrop{});

        using namespace wui;

        auto info_bar = info(L"Title", L"Lorem ipsum dolor sit amet").row(0);
        auto info_action = btn(L"Text");
        info_bar.as<controls::InfoBar>().ActionButton(
            info_action.as<controls::Button>());

        auto primary_button = btn(
                                 L"Text",
                                 [info_bar] {
                                     info_bar.as<controls::InfoBar>()
                                         .Message(L"The button was pressed.");
                                 })
                             .style(Resources()
                                        .Lookup(winrt::box_value(
                                            L"AccentButtonStyle"))
                                        .as<xaml::Style>());

        auto card_inner = vbox()
                              .gap_3()
                              .child(
                                  txt(L"Lorem ipsum dolor sit amet, "
                                      L"consectetur adipiscing elit, sed do "
                                      L"eiusmod tempor.")
                                      .wrap())
                              .child(hbox()
                                        .gap_2()
                                        .child(primary_button)
                                        .child(btn(L"Text")));

        auto main_card = card()
                             .padding(16)
                             .border_1()
                             .rounded_px(8)
                             .child(card_inner);

        auto rich_editor =
            editor(L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                   L"Nam sodales, felis id scelerisque.")
                .h(120);

        auto left = vbox()
                        .gap_4()
                        .child(main_card)
                        .child(rich_editor)
                    .col(0)
                    .row(0);

        auto media_grid = grid()
                             .cols({fit(), star(), fit(), fit(), fit()})
                             .child(sym_btn(controls::Symbol::Play).col(0))
                             .child(slider()
                                       .min(0)
                                       .max(100)
                                       .value(45)
                                       .h(32)
                                       .v_center()
                                       .margin(8, 0, 8, 0)
                                       .col(1))
                             .child(sym_btn(controls::Symbol::Volume).col(2))
                             .child(
                                 sym_btn(controls::Symbol::Keyboard).col(3))
                             .child(
                                 sym_btn(controls::Symbol::FullScreen).col(4));

        auto media_bar = card()
                             .h(48)
                             .v_center()
                             .padding(8, 0, 8, 0)
                             .rounded_px(6)
                             .border_1()
                             .child(media_grid)
                         .col(0)
                         .col_span(2)
                         .row(1);

        auto search_box = suggest()
                              .text(L"Text")
                              .query_icon(controls::Symbol::Find)
                              .items({L"Text", L"Text", L"Text"});
        search_box.as<controls::AutoSuggestBox>().Loaded([](
            winrt::Windows::Foundation::IInspectable const& sender,
            xaml::RoutedEventArgs const&) {
            sender.as<controls::AutoSuggestBox>().IsSuggestionListOpen(true);
        });

        auto middle = vbox()
                          .gap(14)
                          .child(pwd().password(L"secret"))
                          .child(input().text(L"2 + 2"))
                          .child(num().value(10).min(0).max(100))
                          .child(search_box)
                      .col(1)
                      .row(0);

        auto toggles = vbox()
                           .gap_2()
                           .child(txt(L"Header"))
                           .child(hbox()
                                     .gap_4()
                                     .child(
                                         toggle()
                                             .w(96)
                                             .min_w(0)
                                             .off_content(L"Off"))
                                     .child(toggle()
                                               .on(true)
                                               .w(96)
                                               .min_w(0)
                                               .on_content(L"On")));

        auto checks = vbox().gap_2().child(txt(L"Header"));
        for (auto i = 0; i < 4; ++i) {
            auto check_box = check().content(L"Text");
            if (i == 1) {
                check_box.checked(true);
            }
            checks.child(check_box);
        }
        checks.col(0);

        auto radios = vbox().gap_2().child(txt(L"Header"));
        for (auto i = 0; i < 4; ++i) {
            auto radio_box = radio().content(L"Text").group(L"Options");
            if (i == 1) {
                radio_box.checked(true);
            }
            radios.child(radio_box);
        }
        radios.col(1);

        auto selections = grid()
                              .col_gap(16)
                              .cols({fit(), fit()})
                              .child(checks)
                              .child(radios);

        auto right = vbox()
                         .gap(18)
                         .child(toggles)
                         .child(
                             slider().header(L"Header").min(0).max(100)
                                 .value(50)
                                 .step(1))
                         .child(selections)
                     .col(2)
                     .row(0)
                     .row_span(2);

        auto content = grid()
                           .cols({star(1.3), star(0.85), star(0.85)})
                           .rows({fit(), star()})
                           .row_gap(16)
                           .col_gap(44)
                           .child(left)
                           .child(media_bar)
                           .child(middle)
                           .child(right)
                       .row(1);

        auto root = grid()
                        .rows({fit(), star()})
                        .row_gap(28)
                        .padding(54, 54, 54, 24)
                        .child(info_bar)
                        .child(content);

        m_window.Content(root.m_el);
        m_window.Title(L"hellowinui");
        m_window.AppWindow().Resize(
            winrt::Windows::Graphics::SizeInt32{940, 608});

        auto const card_background = Resources()
                                          .Lookup(winrt::box_value(
                                              L"CardBackgroundFillColorDefaultBrush"))
                                          .as<xaml::Media::Brush>();
        auto const card_stroke = Resources()
                                     .Lookup(winrt::box_value(
                                         L"CardStrokeColorDefaultBrush"))
                                     .as<xaml::Media::Brush>();
        main_card.bg(card_background).border_color(card_stroke);
        media_bar.bg(card_background).border_color(card_stroke);

        root.m_el.ActualThemeChanged([main_card, media_bar](
                                        xaml::FrameworkElement const& sender,
                                        winrt::Windows::Foundation::IInspectable const&) {
            // Non-const copies so the handle modifiers can be chained.
            auto card = main_card;
            auto bar = media_bar;

            auto const resources = xaml::Application::Current().Resources();
            auto const background = resources
                                        .Lookup(winrt::box_value(
                                            L"CardBackgroundFillColorDefaultBrush"))
                                        .as<xaml::Media::Brush>();
            auto const stroke = resources
                                    .Lookup(winrt::box_value(
                                        L"CardStrokeColorDefaultBrush"))
                                    .as<xaml::Media::Brush>();
            card.bg(background).border_color(stroke);
            bar.bg(background).border_color(stroke);

            auto const hwnd = find_main_window();
            if (hwnd) {
                apply_title_bar_theme(
                    hwnd, sender.ActualTheme() == xaml::ElementTheme::Dark);
            }
        });

        m_window.Activated([root](
            winrt::Windows::Foundation::IInspectable const&,
            xaml::WindowActivatedEventArgs const&) {
            auto const hwnd = find_main_window();
            if (hwnd) {
                apply_title_bar_theme(
                    hwnd,
                    root.m_el.ActualTheme() == xaml::ElementTheme::Dark);
            }
        });

        auto const hwnd = find_main_window();
        if (hwnd) {
            apply_title_bar_theme(
                hwnd, root.m_el.ActualTheme() == xaml::ElementTheme::Dark);
        }

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

auto WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) -> int {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto const result = MddBootstrapInitialize2(
        WINDOWSAPPSDK_RELEASE_MAJORMINOR,
        nullptr,
        PACKAGE_VERSION{},
        MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
    if (FAILED(result)) {
        return result;
    }

    xaml::Application::Start([](auto&&) { winrt::make<app>(); });
    MddBootstrapShutdown();
    return 0;
}
