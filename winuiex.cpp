#include "wui/wui.hpp"

using namespace wui;

auto WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) -> int {
    return run(940, 608, [](window& win) {
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
                             .style(resources()
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

        win.Content(root.m_el);
        win.Title(L"hellowinui");

        auto const card_background = resources()
                                          .Lookup(winrt::box_value(
                                              L"CardBackgroundFillColorDefaultBrush"))
                                          .as<media::Brush>();
        auto const card_stroke = resources()
                                     .Lookup(winrt::box_value(
                                         L"CardStrokeColorDefaultBrush"))
                                     .as<media::Brush>();
        main_card.bg(card_background).border_color(card_stroke);
        media_bar.bg(card_background).border_color(card_stroke);

        root.m_el.ActualThemeChanged([main_card, media_bar](
                                        xaml::FrameworkElement const&,
                                        winrt::Windows::Foundation::IInspectable const&) {
            // Non-const copies so the handle modifiers can be chained.
            auto card = main_card;
            auto bar = media_bar;

            auto const theme = resources();
            auto const background = theme
                                       .Lookup(winrt::box_value(
                                           L"CardBackgroundFillColorDefaultBrush"))
                                       .as<media::Brush>();
            auto const stroke = theme
                                    .Lookup(winrt::box_value(
                                        L"CardStrokeColorDefaultBrush"))
                                    .as<media::Brush>();
            card.bg(background).border_color(stroke);
            bar.bg(background).border_color(stroke);
        });
    });
}
