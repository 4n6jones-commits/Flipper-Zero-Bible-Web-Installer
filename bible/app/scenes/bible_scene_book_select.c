#include "../bible_app.h"
#include "bible_scene_config.h"
#include <gui/canvas.h>
#include <input/input.h>

// ── Layout constants ──────────────────────────────────────────────────────────
#define LIST_Y_START    16   // first row y-baseline
#define ROW_HEIGHT      10   // pixels per row
#define VISIBLE_ROWS     4   // rows that fit between header and footer
#define FOOTER_Y        57   // footer divider y

// ── Draw ──────────────────────────────────────────────────────────────────────
// Header: "Select Book" + OT/NT badge (right)
// 4 visible rows, current selection highlighted
// Footer: progress bar + "N of 66"

void book_select_draw_cb(Canvas* canvas, void* model) {
    UNUSED(model);
    BibleApp* app = bible_app_get();
    if(!app) return;

    int sel    = app->selected_book;
    int offset = app->book_scroll_offset;

    canvas_clear(canvas);

    // ── Header bar ────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 14);

    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 11, "Select Book");

    // OT/NT badge (right side of header)
    const char* testament = (sel < NT_START_BOOK) ? "OT" : "NT";
    canvas_set_color(canvas, ColorBlack);
    // Badge background (white rounded rect)
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_rbox(canvas, 104, 2, 20, 10, 2);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 114, 10, AlignCenter, AlignBottom, testament);

    // ── List rows ─────────────────────────────────────────────────────────────
    for(int row = 0; row < VISIBLE_ROWS; row++) {
        int book_idx = offset + row;
        if(book_idx >= app->num_books) break;

        int y = LIST_Y_START + row * ROW_HEIGHT;
        bool is_sel = (book_idx == sel);

        if(is_sel) {
            // Selection: full-width filled bar
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, y - 7, 128, ROW_HEIGHT);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        // Abbreviation badge (small rounded rect left)
        if(!is_sel) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rframe(canvas, 2, y - 7, 22, 9, 2);
        } else {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_rframe(canvas, 2, y - 7, 22, 9, 2);
        }

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 13, y, AlignCenter, AlignBottom,
                                 app->books[book_idx].abbrev);

        // Book full name
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 27, y, app->books[book_idx].name);

        // Right arrow if selected
        if(is_sel) {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_dot(canvas, 122, y - 4);
            canvas_draw_dot(canvas, 123, y - 3);
            canvas_draw_dot(canvas, 124, y - 2);
            canvas_draw_dot(canvas, 123, y - 1);
            canvas_draw_dot(canvas, 122, y);
        }
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 0, FOOTER_Y, 128, FOOTER_Y);

    // Progress bar
    int bar_w = 80;
    int filled = (app->num_books > 1)
        ? (int)((long)sel * bar_w / (app->num_books - 1))
        : bar_w;
    canvas_draw_frame(canvas, 2, 59, bar_w + 2, 4);
    canvas_draw_box(canvas, 3, 60, filled, 2);

    // "N of 66" label
    char label[16];
    snprintf(label, sizeof(label), "%d of %d", sel + 1, app->num_books);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 88, 63, label);
}

// ── Input ─────────────────────────────────────────────────────────────────────
bool book_select_input_cb(InputEvent* event, void* ctx) {
    BibleApp* app = ctx;
    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    if(event->key == InputKeyDown) {
        if(app->selected_book < app->num_books - 1) {
            app->selected_book++;
            // Scroll down if selection goes below visible window
            if(app->selected_book >= app->book_scroll_offset + VISIBLE_ROWS) {
                app->book_scroll_offset++;
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewBookSelect);
        }
        return true;
    }
    if(event->key == InputKeyUp) {
        if(app->selected_book > 0) {
            app->selected_book--;
            if(app->selected_book < app->book_scroll_offset) {
                app->book_scroll_offset--;
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewBookSelect);
        }
        return true;
    }
    if(event->key == InputKeyOk) {
        scene_manager_handle_custom_event(app->scene_manager, app->selected_book);
        return true;
    }
    return false;
}

// ── Scene lifecycle ───────────────────────────────────────────────────────────

void bible_scene_book_select_on_enter(void* ctx) {
    BibleApp* app = ctx;
    // Restore scroll so selection is visible
    if(app->selected_book < app->book_scroll_offset) {
        app->book_scroll_offset = app->selected_book;
    }
    if(app->selected_book >= app->book_scroll_offset + VISIBLE_ROWS) {
        app->book_scroll_offset = app->selected_book - VISIBLE_ROWS + 1;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewBookSelect);
}

bool bible_scene_book_select_on_event(void* ctx, SceneManagerEvent event) {
    BibleApp* app = ctx;
    if(event.type == SceneManagerEventTypeCustom) {
        app->selected_chapter     = 0;
        app->chapter_scroll_offset = 0;
        scene_manager_next_scene(app->scene_manager, BibleSceneChapterSelect);
        return true;
    }
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void bible_scene_book_select_on_exit(void* ctx) {
    UNUSED(ctx);
}
