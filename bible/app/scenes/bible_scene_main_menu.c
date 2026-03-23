#include "../bible_app.h"
#include "bible_scene_config.h"
#include <gui/canvas.h>
#include <input/input.h>

// ── Model ─────────────────────────────────────────────────────────────────────
typedef struct {
    int selected; // 0 = Read Bible, 1 = Random Verse
} MainMenuModel;

// ── Draw ──────────────────────────────────────────────────────────────────────
// 128x64 layout:
//   [0-13]  Inverted header bar — "BIBLE"
//   [15-24] Menu item 0
//   [26-35] Menu item 1
//   [50-63] Bottom hint bar

void main_menu_draw_cb(Canvas* canvas, void* model) {
    MainMenuModel* m = (MainMenuModel*)model;

    canvas_clear(canvas);

    // ── Header bar ────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 14);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 11, AlignCenter, AlignBottom, "BIBLE");

    // Small cross icon in header (right side, white on black)
    // Vertical
    canvas_draw_box(canvas, 119, 2, 2, 10);
    // Horizontal
    canvas_draw_box(canvas, 115, 5, 10, 2);
    // Cutout
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 120, 3, 0, 8); // just a dot-thin line

    // ── Menu items ────────────────────────────────────────────────────────────
    const char* items[] = {"Read Bible", "Random Verse"};
    const char* icons[] = {"\x10 ", "\x07 "}; // arrow, bullet — ASCII fallback

    for(int i = 0; i < 2; i++) {
        int y = 24 + i * 14;

        if(m->selected == i) {
            // Selection highlight: filled rounded rect
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_rbox(canvas, 4, y - 9, 120, 12, 3);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        // Selection arrow
        if(m->selected == i) {
            canvas_set_color(canvas, ColorWhite);
            // Draw right-pointing triangle as selection marker
            canvas_draw_dot(canvas, 10, y - 3);
            canvas_draw_dot(canvas, 11, y - 2);
            canvas_draw_dot(canvas, 12, y - 1);
            canvas_draw_dot(canvas, 11, y);
            canvas_draw_dot(canvas, 10, y + 1);
        }

        canvas_set_font(canvas, FontPrimary);
        if(m->selected == i) {
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_str(canvas, 18, y, items[i]);
        (void)icons;
    }

    // ── Divider ───────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 0, 51, 128, 51);

    // ── Bottom hint bar ───────────────────────────────────────────────────────
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 62, "OK:Select");
    canvas_draw_str(canvas, 80, 62, "Back:Exit");
}

// ── Input ─────────────────────────────────────────────────────────────────────
bool main_menu_input_cb(InputEvent* event, void* ctx) {
    BibleApp* app = ctx;
    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    if(event->key == InputKeyDown || event->key == InputKeyRight) {
        with_view_model(app->main_menu_view, MainMenuModel* m, {
            if(m->selected < 1) m->selected++;
        }, true);
        return true;
    }
    if(event->key == InputKeyUp || event->key == InputKeyLeft) {
        with_view_model(app->main_menu_view, MainMenuModel* m, {
            if(m->selected > 0) m->selected--;
        }, true);
        return true;
    }
    if(event->key == InputKeyOk) {
        int sel = 0;
        with_view_model(app->main_menu_view, MainMenuModel* m, {
            sel = m->selected;
        }, false);
        scene_manager_handle_custom_event(app->scene_manager, sel);
        return true;
    }
    return false;
}

// ── Scene lifecycle ───────────────────────────────────────────────────────────

void bible_scene_main_menu_on_enter(void* ctx) {
    BibleApp* app = ctx;
    with_view_model(app->main_menu_view, MainMenuModel* m, {
        m->selected = 0;
    }, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewMainMenu);
}

bool bible_scene_main_menu_on_event(void* ctx, SceneManagerEvent event) {
    BibleApp* app = ctx;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == 0) {
            // Read Bible → book select
            scene_manager_next_scene(app->scene_manager, BibleSceneBookSelect);
            return true;
        }
        if(event.event == 1) {
            // Random Verse
            if(app->num_books == 0) return true;
            static bool seeded = false;
            if(!seeded) { srand(furi_hal_random_get()); seeded = true; }

            app->selected_book    = rand() % app->num_books;
            app->selected_chapter = rand() % app->books[app->selected_book].num_chapters;

            if(bible_load_chapter(app, app->selected_book, app->selected_chapter)) {
                app->current_verse = rand() % app->verse_count;
                scene_manager_next_scene(app->scene_manager, BibleSceneVerseView);
            }
            return true;
        }
    }
    return false;
}

void bible_scene_main_menu_on_exit(void* ctx) {
    UNUSED(ctx);
}
