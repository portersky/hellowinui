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

        auto root = controls::Grid();
        root.Padding(xaml::Thickness{54.0, 54.0, 54.0, 24.0});
        root.RowSpacing(28.0);

        auto info_row = controls::RowDefinition();
        info_row.Height(xaml::GridLengthHelper::Auto());
        root.RowDefinitions().Append(info_row);

        auto content_row = controls::RowDefinition();
        content_row.Height(xaml::GridLengthHelper::FromValueAndType(
            1.0, xaml::GridUnitType::Star));
        root.RowDefinitions().Append(content_row);

        auto info_bar = controls::InfoBar();
        info_bar.IsOpen(true);
        info_bar.Title(L"Title");
        info_bar.Message(L"Lorem ipsum dolor sit amet");
        info_bar.IsClosable(true);

        auto info_action = controls::Button();
        info_action.Content(winrt::box_value(L"Text"));
        info_bar.ActionButton(info_action);
        controls::Grid::SetRow(info_bar, 0);
        root.Children().Append(info_bar);

        auto content = controls::Grid();
        content.ColumnSpacing(44.0);
        content.RowSpacing(16.0);

        for (auto const weight : {1.3, 0.85, 0.85}) {
            auto column = controls::ColumnDefinition();
            column.Width(xaml::GridLengthHelper::FromValueAndType(
                weight, xaml::GridUnitType::Star));
            content.ColumnDefinitions().Append(column);
        }

        auto content_row0 = controls::RowDefinition();
        content_row0.Height(xaml::GridLengthHelper::Auto());
        content.RowDefinitions().Append(content_row0);
        auto content_row1 = controls::RowDefinition();
        content_row1.Height(xaml::GridLengthHelper::FromValueAndType(
            1.0, xaml::GridUnitType::Star));
        content.RowDefinitions().Append(content_row1);

        auto left = controls::StackPanel();
        left.Spacing(16.0);

        auto card = controls::Border();
        card.BorderThickness(xaml::Thickness{1.0, 1.0, 1.0, 1.0});
        card.CornerRadius(xaml::CornerRadius{8.0, 8.0, 8.0, 8.0});
        card.Padding(xaml::Thickness{16.0, 16.0, 16.0, 16.0});

        auto card_inner = controls::StackPanel();
        card_inner.Spacing(12.0);

        auto description = controls::TextBlock();
        description.Text(
            L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
            L"eiusmod tempor.");
        description.TextWrapping(xaml::TextWrapping::Wrap);
        card_inner.Children().Append(description);

        auto buttons = controls::StackPanel();
        buttons.Orientation(xaml::Controls::Orientation::Horizontal);
        buttons.Spacing(8.0);

        auto primary_button = controls::Button();
        primary_button.Content(winrt::box_value(L"Text"));
        primary_button.Style(Resources()
                                 .Lookup(winrt::box_value(L"AccentButtonStyle"))
                                 .as<xaml::Style>());
        primary_button.Click([info_bar](
            winrt::Windows::Foundation::IInspectable const&,
            xaml::RoutedEventArgs const&) {
            info_bar.Message(L"The button was pressed.");
        });
        buttons.Children().Append(primary_button);

        auto secondary_button = controls::Button();
        secondary_button.Content(winrt::box_value(L"Text"));
        buttons.Children().Append(secondary_button);
        card_inner.Children().Append(buttons);
        card.Child(card_inner);
        left.Children().Append(card);

        auto editor = controls::RichEditBox();
        editor.AcceptsReturn(true);
        editor.TextWrapping(xaml::TextWrapping::Wrap);
        editor.Height(120.0);
        editor.Document().SetText(
            winrt::Microsoft::UI::Text::TextSetOptions::None,
            L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam "
            L"sodales, felis id scelerisque.");
        left.Children().Append(editor);

        auto media_bar = controls::Border();
        media_bar.BorderThickness(xaml::Thickness{1.0, 1.0, 1.0, 1.0});
        media_bar.CornerRadius(xaml::CornerRadius{6.0, 6.0, 6.0, 6.0});
        media_bar.Height(48.0);
        media_bar.Padding(xaml::Thickness{8.0, 0.0, 8.0, 0.0});
        media_bar.VerticalAlignment(xaml::VerticalAlignment::Center);

        auto media_grid = controls::Grid();
        for (auto const width : {xaml::GridLengthHelper::Auto(),
                                 xaml::GridLengthHelper::FromValueAndType(
                                     1.0, xaml::GridUnitType::Star),
                                 xaml::GridLengthHelper::Auto(),
                                 xaml::GridLengthHelper::Auto(),
                                 xaml::GridLengthHelper::Auto()}) {
            auto column = controls::ColumnDefinition();
            column.Width(width);
            media_grid.ColumnDefinitions().Append(column);
        }

        auto make_media_button = [](controls::Symbol const symbol)
            -> controls::Button {
            auto button = controls::Button();
            auto const transparent = xaml::Application::Current()
                                         .Resources()
                                         .Lookup(winrt::box_value(
                                             L"ControlFillColorTransparentBrush"))
                                         .as<xaml::Media::Brush>();
            button.Content(controls::SymbolIcon{symbol});
            button.Width(40.0);
            button.Height(32.0);
            button.MinWidth(0.0);
            button.MinHeight(0.0);
            button.Padding(xaml::Thickness{0.0, 0.0, 0.0, 0.0});
            button.CornerRadius(xaml::CornerRadius{4.0, 4.0, 4.0, 4.0});
            button.HorizontalContentAlignment(
                xaml::HorizontalAlignment::Center);
            button.VerticalContentAlignment(xaml::VerticalAlignment::Center);
            button.Background(transparent);
            button.BorderBrush(transparent);
            button.BorderThickness(xaml::Thickness{0.0, 0.0, 0.0, 0.0});
            button.PointerEntered([](
                winrt::Windows::Foundation::IInspectable const& sender,
                xaml::Input::PointerRoutedEventArgs const&) {
                auto const brush = xaml::Application::Current()
                                       .Resources()
                                       .Lookup(winrt::box_value(
                                           L"ControlFillColorSecondaryBrush"))
                                       .as<xaml::Media::Brush>();
                sender.as<controls::Button>().Background(brush);
            });
            button.PointerExited([](
                winrt::Windows::Foundation::IInspectable const& sender,
                xaml::Input::PointerRoutedEventArgs const&) {
                auto const brush = xaml::Application::Current()
                                       .Resources()
                                       .Lookup(winrt::box_value(
                                           L"ControlFillColorTransparentBrush"))
                                       .as<xaml::Media::Brush>();
                sender.as<controls::Button>().Background(brush);
            });
            return button;
        };

        auto media_play = make_media_button(controls::Symbol::Play);
        controls::Grid::SetColumn(media_play, 0);
        media_grid.Children().Append(media_play);

        auto media_seek = controls::Slider();
        media_seek.Minimum(0.0);
        media_seek.Maximum(100.0);
        media_seek.Value(45.0);
        media_seek.Height(32.0);
        media_seek.VerticalAlignment(xaml::VerticalAlignment::Center);
        media_seek.Margin(xaml::Thickness{8.0, 0.0, 8.0, 0.0});
        controls::Grid::SetColumn(media_seek, 1);
        media_grid.Children().Append(media_seek);

        auto media_volume = make_media_button(controls::Symbol::Volume);
        controls::Grid::SetColumn(media_volume, 2);
        media_grid.Children().Append(media_volume);

        auto media_keyboard = make_media_button(controls::Symbol::Keyboard);
        controls::Grid::SetColumn(media_keyboard, 3);
        media_grid.Children().Append(media_keyboard);

        auto media_fullscreen = make_media_button(controls::Symbol::FullScreen);
        controls::Grid::SetColumn(media_fullscreen, 4);
        media_grid.Children().Append(media_fullscreen);

        media_bar.Child(media_grid);
        controls::Grid::SetColumn(left, 0);
        controls::Grid::SetRow(left, 0);
        content.Children().Append(left);

        controls::Grid::SetColumn(media_bar, 0);
        controls::Grid::SetColumnSpan(media_bar, 2);
        controls::Grid::SetRow(media_bar, 1);
        content.Children().Append(media_bar);

        auto middle = controls::StackPanel();
        middle.Spacing(14.0);

        auto password = controls::PasswordBox();
        password.Password(L"secret");
        middle.Children().Append(password);

        auto text_box = controls::TextBox();
        text_box.Text(L"2 + 2");
        middle.Children().Append(text_box);

        auto number_box = controls::NumberBox();
        number_box.Value(10.0);
        number_box.Minimum(0.0);
        number_box.Maximum(100.0);
        number_box.SmallChange(1.0);
        middle.Children().Append(number_box);

        auto search = controls::AutoSuggestBox();
        search.QueryIcon(controls::SymbolIcon{controls::Symbol::Find});
        search.Text(L"Text");
        search.Items().Append(winrt::box_value(L"Text"));
        search.Items().Append(winrt::box_value(L"Text"));
        search.Items().Append(winrt::box_value(L"Text"));
        search.Loaded([](winrt::Windows::Foundation::IInspectable const& sender,
                        xaml::RoutedEventArgs const&) {
            sender.as<controls::AutoSuggestBox>().IsSuggestionListOpen(true);
        });
        middle.Children().Append(search);
        controls::Grid::SetColumn(middle, 1);
        controls::Grid::SetRow(middle, 0);
        content.Children().Append(middle);

        auto right = controls::StackPanel();
        right.Spacing(18.0);

        auto toggle_section = controls::StackPanel();
        toggle_section.Spacing(8.0);
        auto toggle_header = controls::TextBlock();
        toggle_header.Text(L"Header");
        toggle_section.Children().Append(toggle_header);

        auto toggle_off = controls::ToggleSwitch();
        toggle_off.Width(96.0);
        toggle_off.MinWidth(0.0);
        toggle_off.OffContent(winrt::box_value(L"Off"));
        auto toggle_on = controls::ToggleSwitch();
        toggle_on.Width(96.0);
        toggle_on.MinWidth(0.0);
        toggle_on.IsOn(true);
        toggle_on.OnContent(winrt::box_value(L"On"));
        auto toggle_row = controls::StackPanel();
        toggle_row.Orientation(xaml::Controls::Orientation::Horizontal);
        toggle_row.Spacing(16.0);
        toggle_row.Children().Append(toggle_off);
        toggle_row.Children().Append(toggle_on);
        toggle_section.Children().Append(toggle_row);
        right.Children().Append(toggle_section);

        auto slider = controls::Slider();
        slider.Header(winrt::box_value(L"Header"));
        slider.Minimum(0.0);
        slider.Maximum(100.0);
        slider.Value(50.0);
        slider.StepFrequency(1.0);
        right.Children().Append(slider);

        auto selection_grid = controls::Grid();
        selection_grid.ColumnSpacing(16.0);

        auto checks_column = controls::ColumnDefinition();
        checks_column.Width(xaml::GridLengthHelper::Auto());
        selection_grid.ColumnDefinitions().Append(checks_column);
        auto radios_column = controls::ColumnDefinition();
        radios_column.Width(xaml::GridLengthHelper::Auto());
        selection_grid.ColumnDefinitions().Append(radios_column);

        auto checks = controls::StackPanel();
        checks.Spacing(8.0);
        auto checks_header = controls::TextBlock();
        checks_header.Text(L"Header");
        checks.Children().Append(checks_header);
        for (int i = 0; i < 4; ++i) {
            auto check = controls::CheckBox();
            check.Content(winrt::box_value(L"Text"));
            if (i == 1) {
                check.IsChecked(true);
            }
            checks.Children().Append(check);
        }
        controls::Grid::SetColumn(checks, 0);
        selection_grid.Children().Append(checks);

        auto radios = controls::StackPanel();
        radios.Spacing(8.0);
        auto radios_header = controls::TextBlock();
        radios_header.Text(L"Header");
        radios.Children().Append(radios_header);
        for (int i = 0; i < 4; ++i) {
            auto radio = controls::RadioButton();
            radio.Content(winrt::box_value(L"Text"));
            radio.GroupName(L"Options");
            if (i == 1) {
                radio.IsChecked(true);
            }
            radios.Children().Append(radio);
        }
        controls::Grid::SetColumn(radios, 1);
        selection_grid.Children().Append(radios);

        right.Children().Append(selection_grid);
        controls::Grid::SetColumn(right, 2);
        controls::Grid::SetRow(right, 0);
        controls::Grid::SetRowSpan(right, 2);
        content.Children().Append(right);

        controls::Grid::SetRow(content, 1);
        root.Children().Append(content);
        m_window.Content(root);
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
        card.Background(card_background);
        card.BorderBrush(card_stroke);
        media_bar.Background(card_background);
        media_bar.BorderBrush(card_stroke);

        root.ActualThemeChanged([card, media_bar](
                                    xaml::FrameworkElement const& sender,
                                    winrt::Windows::Foundation::IInspectable const&) {
            auto const resources = xaml::Application::Current().Resources();
            auto const background = resources
                                        .Lookup(winrt::box_value(
                                            L"CardBackgroundFillColorDefaultBrush"))
                                        .as<xaml::Media::Brush>();
            auto const stroke = resources
                                    .Lookup(winrt::box_value(
                                        L"CardStrokeColorDefaultBrush"))
                                    .as<xaml::Media::Brush>();
            card.Background(background);
            card.BorderBrush(stroke);
            media_bar.Background(background);
            media_bar.BorderBrush(stroke);

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
                    hwnd, root.ActualTheme() == xaml::ElementTheme::Dark);
            }
        });

        auto const hwnd = find_main_window();
        if (hwnd) {
            apply_title_bar_theme(
                hwnd, root.ActualTheme() == xaml::ElementTheme::Dark);
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
