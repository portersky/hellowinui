#include "wui/wui.hpp"

using namespace wui;

auto WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) -> int {
    return run(500, 500, [](window& win) {
        auto root = vbox()
                        .gap_3()
                        .bg(rgb(0x505050))
                        .justify_center()
                        .items_center()
                        .border_1()
                        .border_color(rgb(0x0000ff))
                        .text_xl()
                        .text_color(rgb(0xffffff))
                        .child(L"Hello, World!")
                        .child(hbox().gap_2()
                                  .child(card().size_8().bg(red()))
                                  .child(card().size_8().bg(green()))
                                  .child(card().size_8().bg(blue()))
                                  .child(card().size_8().bg(yellow()))
                                  .child(card().size_8().bg(black()))
                                  .child(card().size_8().bg(white())));
        win.Content(root.m_el);
        win.Title(L"hellowinui");
    });
}
