#include "../bible_app.h"
#include "bible_scene_config.h"
#include <gui/canvas.h>
#include <input/input.h>

#define CH_LIST_Y_START  16
#define CH_ROW_HEIGHT    10
#define CH_VISIBLE_ROWS   4
#define CH_FOOTER_Y      57

void chapter_select_draw_cb(Canvas* canvas, void* model) {
    UNUSED(model);
    BibleApp* app = bible_app_get();
    if(!app) return;

    int sel    = app->selected_chapter;
    int offset = app->chapter_scroll_offset;
    int num_ch = app->books[app->selected_book].num_chapters;

    canvas_clear(canvas);

    // ── Header ────────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 14);

    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);

    // Book name truncated to fit
    char header[32];
    snprintf(header, sizeof(header), "%s", app->books[app->selected_book].name);
    canvas_draw_str(canvas, 4, 11, header);

    // Chapter count badge right side
    char ch_badge[8];
    snprintf(ch_badge, sizeof(ch_badge), "%d ch", num_ch);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 124, 11, AlignRight, AlignBottom, ch_badge);

    // ── List ──────────────────────────────────────────────────────────────────
    for(int row = 0; row < CH_VISIBLE_ROWS; row++) {
        int ch_idx = offset + row;
        if(ch_idx >= num_ch) break;

        int  y      = CH_LIST_Y_START + row * CH_ROW_HEIGHT;
        bool is_sel = (ch_idx == sel);

        if(is_sel) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, y - 7, 128, CH_ROW_HEIGHT);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_set_font(canvas, FontSecondary);
        char ch_label[16];
        snprintf(ch_label, sizeof(ch_label), "Chapter %d", ch_idx + 1);
        canvas_draw_str(canvas, 10, y, ch_label);

        // Selection arrow
        if(is_sel) {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_dot(canvas, 4, y - 3);
            canvas_draw_dot(canvas, 5, y - 2);
            canvas_draw_dot(canvas, 6, y - 1);
            canvas_draw_dot(canvas, 5, y);
            canvas_draw_dot(canvas, 4, y + 1);
        }
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 0, CH_FOOTER_Y, 128, CH_FOOTER_Y);

    int bar_w  = 80;
    int filled = (num_ch > 1)
        ? (int)((long)sel * bar_w / (num_ch - 1))
        : bar_w;
    canvas_draw_frame(canvas, 2, 59, bar_w + 2, 4);
    canvas_draw_box(canvas, 3, 60, filled, 2);

    char label[16];
    snprintf(label, sizeof(label), "%d of %d", sel + 1, num_ch);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 88, 63, label);
}

bool chapter_select_input_cb(InputEvent* event, void* ctx) {
    BibleApp* app = ctx;
    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    int num_ch = app->books[app->selected_book].num_chapters;

    if(event->key == InputKeyDown) {
        if(app->selected_chapter < num_ch - 1) {
            app->selected_chapter++;
            if(app->selected_chapter >= app->chapter_scroll_offset + CH_VISIBLE_ROWS) {
                app->chapter_scroll_offset++;
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewChapterSelect);
        }
        return true;
    }
    if(event->key == InputKeyUp) {
        if(app->selected_chapter > 0) {
            app->selected_chapter--;
            if(app->selected_chapter < app->chapter_scroll_offset) {
                app->chapter_scroll_offset--;
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewChapterSelect);
        }
        return true;
    }
    if(event->key == InputKeyOk) {
        scene_manager_handle_custom_event(app->scene_manager, app->selected_chapter);
        return true;
    }
    return false;
}

void bible_scene_chapter_select_on_enter(void* ctx) {
    BibleApp* app = ctx;
    if(app->selected_chapter < app->chapter_scroll_offset) {
        app->chapter_scroll_offset = app->selected_chapter;
    }
    if(app->selected_chapter >= app->chapter_scroll_offset + CH_VISIBLE_ROWS) {
        app->chapter_scroll_offset = app->selected_chapter - CH_VISIBLE_ROWS + 1;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewChapterSelect);
}

bool bible_scene_chapter_select_on_event(void* ctx, SceneManagerEvent event) {
    BibleApp* app = ctx;
    if(event.type == SceneManagerEventTypeCustom) {
        app->current_verse = 0;
        if(bible_load_chapter(app, app->selected_book, app->selected_chapter)) {
            scene_manager_next_scene(app->scene_manager, BibleSceneVerseView);
        }
        return true;
    }
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void bible_scene_chapter_select_on_exit(void* ctx) {
    UNUSED(ctx);
}
