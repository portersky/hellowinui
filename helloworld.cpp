#include <windows.h>

#undef GetCurrentTime

#include "MddBootstrap.h"
#include "winrt/Microsoft.UI.Xaml.Controls.h"
#include "winrt/Microsoft.UI.Xaml.Media.h"
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.UI.h"
#include "winrt/base.h"

#include "fmt/format.h"

class app : public winrt::Microsoft::UI::Xaml::ApplicationT<app> {
public:
    auto OnLaunched(
        winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const&) -> void {
        m_window = winrt::Microsoft::UI::Xaml::Window();

        auto root = winrt::Microsoft::UI::Xaml::Controls::Grid();
        root.Background(
            winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{
                winrt::Windows::UI::Color{255, 243, 243, 243}});

        auto card = winrt::Microsoft::UI::Xaml::Controls::Border();
        card.Background(
            winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{
                winrt::Windows::UI::Color{255, 255, 255, 255}});
        card.BorderBrush(
            winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{
                winrt::Windows::UI::Color{255, 225, 225, 225}});
        card.BorderThickness(
            winrt::Microsoft::UI::Xaml::Thickness{1.0, 1.0, 1.0, 1.0});
        card.CornerRadius(
            winrt::Microsoft::UI::Xaml::CornerRadius{16.0, 16.0, 16.0, 16.0});
        card.Padding(
            winrt::Microsoft::UI::Xaml::Thickness{32.0, 28.0, 32.0, 28.0});
        card.Width(440.0);
        card.HorizontalAlignment(
            winrt::Microsoft::UI::Xaml::HorizontalAlignment::Center);
        card.VerticalAlignment(
            winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);

        auto content = winrt::Microsoft::UI::Xaml::Controls::StackPanel();
        content.Spacing(10.0);

        auto title = winrt::Microsoft::UI::Xaml::Controls::TextBlock();
        title.Text(winrt::to_hstring(fmt::format("Hello from C++")));
        title.Foreground(
            winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{
                winrt::Windows::UI::Color{255, 26, 26, 26}});
        title.FontSize(32.0);
        title.TextAlignment(winrt::Microsoft::UI::Xaml::TextAlignment::Center);

        auto subtitle = winrt::Microsoft::UI::Xaml::Controls::TextBlock();
        subtitle.Text(L"A WinUI 3 window built without XAML.");
        subtitle.Foreground(
            winrt::Microsoft::UI::Xaml::Media::SolidColorBrush{
                winrt::Windows::UI::Color{255, 93, 93, 93}});
        subtitle.FontSize(16.0);
        subtitle.TextAlignment(
            winrt::Microsoft::UI::Xaml::TextAlignment::Center);

        content.Children().Append(title);
        content.Children().Append(subtitle);
        card.Child(content);
        root.Children().Append(card);

        m_window.Content(root);
        m_window.Title(L"hellowinui");
        m_window.Activate();
    }

private:
    winrt::Microsoft::UI::Xaml::Window m_window{nullptr};
};

auto WINAPI wWinMain(
    HINSTANCE,
    HINSTANCE,
    PWSTR,
    int) -> int {
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto const result = MddBootstrapInitialize2(
        WINDOWSAPPSDK_RELEASE_MAJORMINOR,
        nullptr,
        PACKAGE_VERSION{},
        MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
    if (FAILED(result)) {
        return result;
    }

    ::winrt::Microsoft::UI::Xaml::Application::Start(
        [](auto&&) { winrt::make<app>(); });
    MddBootstrapShutdown();
    return 0;
}
