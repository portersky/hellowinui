#include "wui/wui.hpp"

using namespace wui;

namespace {
enum class bp { narrow, medium, wide };

auto bp_of(f64 width) -> bp {
    if (width < 640.0) {
        return bp::narrow;
    }
    if (width < 900.0) {
        return bp::medium;
    }
    return bp::wide;
}

auto bp_name(bp b) -> std::wstring_view {
    if (b == bp::medium) {
        return L"medium";
    }
    if (b == bp::wide) {
        return L"wide";
    }
    return L"narrow";
}

auto bp_color(bp b) -> media::Brush {
    if (b == bp::medium) {
        return rgb(0xffb454);
    }
    if (b == bp::wide) {
        return rgb(0x3ddc84);
    }
    return rgb(0xff5d5d);
}

// Exact grid placement; re-applied on every breakpoint switch so stale
// spans from a previous arrangement never linger.
auto place(element const& e, i32 row, i32 col, i32 col_span) -> void {
    controls::Grid::SetRow(e.m_el, row);
    controls::Grid::SetColumn(e.m_el, col);
    controls::Grid::SetColumnSpan(e.m_el, col_span);
}
}  // namespace

auto WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) -> int {
    return run(1000, 640, [](window& win) {
        auto label = txt(L"").text_sm().text_color(rgb(0x8a8a8a));
        label.m_el.HorizontalAlignment(xaml::HorizontalAlignment::Right);

        auto header = grid()
                          .cols({fit(), star()})
                          .child(txt(L"responsive demo").text_lg())
                          .child(label.col(1))
                      .row(0);

        // The three panels that get rearranged. They live in one grid at
        // all times; reflow only changes their Grid attached properties,
        // never the visual tree, so it is safe to run from a size event.
        auto panel_a = card()
                           .padding(16)
                           .border_1()
                           .rounded_px(8)
                           .child(vbox()
                                     .gap_3()
                                     .child(hbox()
                                               .gap_2()
                                               .items_center()
                                               .child(card()
                                                         .size_3()
                                                         .rounded_px(6)
                                                         .bg(rgb(0x4f8cff)))
                                               .child(txt(L"A").text_lg()))
                                     .child(slider().min(0).max(100).value(30)));

        auto panel_b = card()
                           .padding(16)
                           .border_1()
                           .rounded_px(8)
                           .child(vbox()
                                     .gap_3()
                                     .child(hbox()
                                               .gap_2()
                                               .items_center()
                                               .child(card()
                                                         .size_3()
                                                         .rounded_px(6)
                                                         .bg(rgb(0x3ddc84)))
                                               .child(txt(L"B").text_lg()))
                                     .child(toggle().on(true)));

        auto panel_c = card()
                           .padding(16)
                           .border_1()
                           .rounded_px(8)
                           .child(vbox()
                                     .gap_3()
                                     .child(hbox()
                                               .gap_2()
                                               .items_center()
                                               .child(card()
                                                         .size_3()
                                                         .rounded_px(6)
                                                         .bg(rgb(0xffb454)))
                                               .child(txt(L"C").text_lg()))
                                     .child(check().content(L"Done").checked(true)));

        // Six equal columns and three auto rows cover every arrangement:
        // wide uses 3x2-column spans, medium a full row plus two halves,
        // narrow full-width rows. Unused rows collapse to zero height.
        auto layout = grid()
                          .cols({star(), star(), star(), star(), star(), star()})
                          .rows({fit(), fit(), fit()})
                          .gap(12)
                          .child(panel_a)
                          .child(panel_b)
                          .child(panel_c);

        auto apply_bp = [panel_a, panel_b, panel_c](bp b) {
            if (b == bp::wide) {
                place(panel_a, 0, 0, 2);
                place(panel_b, 0, 2, 2);
                place(panel_c, 0, 4, 2);
            } else if (b == bp::medium) {
                place(panel_a, 0, 0, 6);
                place(panel_b, 1, 0, 3);
                place(panel_c, 1, 3, 3);
            } else {
                place(panel_a, 0, 0, 6);
                place(panel_b, 1, 0, 6);
                place(panel_c, 2, 0, 6);
            }
        };

        auto root = grid()
                        .rows({fit(), star()})
                        .row_gap(16)
                        .padding(24)
                        .child(header)
                        .child(layout.row(1));

        // App-lifetime state shared by the handler copies; starts at the
        // pre-applied breakpoint so the first size event does not re-apply.
        auto* current = new int(static_cast<int>(bp::wide));
        root.on_resize([=](f64 w, f64) {
            auto lab = label;  // non-const copy for chaining
            auto const b = bp_of(w);
            lab.text(std::to_wstring(static_cast<int>(w)) + L" px · "
                     + std::wstring{bp_name(b)});
            if (b != static_cast<bp>(*current)) {
                *current = static_cast<int>(b);
                apply_bp(b);
                lab.text_color(bp_color(b));
            }
        });

        win.Content(root.m_el);
        win.Title(L"hellowinui");
        apply_bp(bp::wide);
    });
}
