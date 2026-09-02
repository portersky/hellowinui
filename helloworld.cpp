#include <windows.h>

#undef GetCurrentTime

#include "MddBootstrap.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "winrt/Microsoft.UI.Xaml.Controls.h"
#include "winrt/Microsoft.UI.Xaml.Media.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.XamlTypeInfo.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/base.h"
#include "fmt/format.h"

namespace xaml = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;
namespace markup = winrt::Microsoft::UI::Xaml::Markup;
namespace xaml_type_info = winrt::Microsoft::UI::Xaml::XamlTypeInfo;
namespace interop = winrt::Windows::UI::Xaml::Interop;

class app : public xaml::ApplicationT<app, markup::IXamlMetadataProvider> {
public:
    auto OnLaunched(xaml::LaunchActivatedEventArgs const&) -> void {
        m_window = xaml::Window();

        // RequestedTheme is intentionally unset so WinUI follows Windows.
        Resources().MergedDictionaries().Append(controls::XamlControlsResources{});
        m_window.SystemBackdrop(xaml::Media::MicaBackdrop{});

        auto root = controls::Grid();
        root.Padding(xaml::Thickness{24.0, 24.0, 24.0, 24.0});
        root.RowSpacing(20.0);

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
        content.ColumnSpacing(24.0);
        content.RowSpacing(18.0);

        auto left_column = controls::ColumnDefinition();
        left_column.Width(xaml::GridLengthHelper::FromValueAndType(
            1.2, xaml::GridUnitType::Star));
        content.ColumnDefinitions().Append(left_column);

        auto middle_column = controls::ColumnDefinition();
        middle_column.Width(xaml::GridLengthHelper::FromValueAndType(
            1.0, xaml::GridUnitType::Star));
        content.ColumnDefinitions().Append(middle_column);

        auto right_column = controls::ColumnDefinition();
        right_column.Width(xaml::GridLengthHelper::FromValueAndType(
            1.0, xaml::GridUnitType::Star));
        content.ColumnDefinitions().Append(right_column);

        auto left = controls::StackPanel();
        left.Spacing(16.0);

        auto description = controls::TextBlock();
        description.Text(
            L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
            L"eiusmod tempor.");
        description.TextWrapping(xaml::TextWrapping::Wrap);

        auto buttons = controls::StackPanel();
        buttons.Orientation(xaml::Controls::Orientation::Horizontal);
        buttons.Spacing(8.0);

        auto primary_button = controls::Button();
        primary_button.Content(winrt::box_value(L"Text"));
        primary_button.Click([info_bar](
            winrt::Windows::Foundation::IInspectable const&,
            xaml::RoutedEventArgs const&) {
            info_bar.Message(L"The button was pressed.");
        });
        buttons.Children().Append(primary_button);

        auto secondary_button = controls::Button();
        secondary_button.Content(winrt::box_value(L"Text"));
        buttons.Children().Append(secondary_button);

        auto editor = controls::TextBox();
        editor.Text(
            L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam "
            L"sollicitudin.");
        editor.AcceptsReturn(true);
        editor.TextWrapping(xaml::TextWrapping::Wrap);
        editor.Height(112.0);

        auto command_bar = controls::CommandBar();
        auto play_button = controls::AppBarButton();
        play_button.Label(L"Play");
        play_button.Icon(controls::SymbolIcon{controls::Symbol::Play});
        command_bar.PrimaryCommands().Append(play_button);

        auto edit_button = controls::AppBarButton();
        edit_button.Label(L"Edit");
        edit_button.Icon(controls::SymbolIcon{controls::Symbol::Edit});
        command_bar.PrimaryCommands().Append(edit_button);

        left.Children().Append(description);
        left.Children().Append(buttons);
        left.Children().Append(editor);
        left.Children().Append(command_bar);
        controls::Grid::SetColumn(left, 0);
        content.Children().Append(left);

        auto middle = controls::StackPanel();
        middle.Spacing(14.0);

        auto password = controls::PasswordBox();
        password.PlaceholderText(L"Password");

        auto text_box = controls::TextBox();
        text_box.PlaceholderText(L"2 + 2");

        auto number_box = controls::NumberBox();
        number_box.Value(10.0);
        number_box.Minimum(0.0);
        number_box.Maximum(100.0);
        number_box.SmallChange(1.0);

        auto search = controls::AutoSuggestBox();
        search.PlaceholderText(L"Text");
        search.Items().Append(winrt::box_value(L"Text"));
        search.Items().Append(winrt::box_value(L"Lorem ipsum"));
        search.Items().Append(winrt::box_value(L"Another result"));

        middle.Children().Append(password);
        middle.Children().Append(text_box);
        middle.Children().Append(number_box);
        middle.Children().Append(search);
        controls::Grid::SetColumn(middle, 1);
        content.Children().Append(middle);

        auto right = controls::StackPanel();
        right.Spacing(16.0);

        auto toggle = controls::ToggleSwitch();
        toggle.Header(winrt::box_value(L"Header"));
        toggle.OnContent(winrt::box_value(L"On"));
        toggle.OffContent(winrt::box_value(L"Off"));

        auto slider = controls::Slider();
        slider.Header(winrt::box_value(L"Header"));
        slider.Minimum(0.0);
        slider.Maximum(100.0);
        slider.Value(50.0);
        slider.StepFrequency(1.0);

        auto checks = controls::StackPanel();
        checks.Spacing(8.0);

        auto check_one = controls::CheckBox();
        check_one.Content(winrt::box_value(L"Text"));
        checks.Children().Append(check_one);

        auto check_two = controls::CheckBox();
        check_two.Content(winrt::box_value(L"Text"));
        check_two.IsChecked(true);
        checks.Children().Append(check_two);

        auto radios = controls::StackPanel();
        radios.Spacing(8.0);

        auto radio_one = controls::RadioButton();
        radio_one.Content(winrt::box_value(L"Text"));
        radio_one.GroupName(L"Options");
        radio_one.IsChecked(true);
        radios.Children().Append(radio_one);

        auto radio_two = controls::RadioButton();
        radio_two.Content(winrt::box_value(L"Text"));
        radio_two.GroupName(L"Options");
        radios.Children().Append(radio_two);

        right.Children().Append(toggle);
        right.Children().Append(slider);
        right.Children().Append(checks);
        right.Children().Append(radios);
        controls::Grid::SetColumn(right, 2);
        content.Children().Append(right);

        controls::Grid::SetRow(content, 1);
        root.Children().Append(content);
        m_window.Content(root);
        m_window.Title(L"hellowinui");
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
