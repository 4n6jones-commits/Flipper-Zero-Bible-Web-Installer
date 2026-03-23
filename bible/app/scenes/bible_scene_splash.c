#include "../bible_app.h"
#include "bible_scene_config.h"
#include <gui/canvas.h>
#include <input/input.h>
#include <furi_hal.h>

// ── Draw ──────────────────────────────────────────────────────────────────────
// Layout (128x64):
//   Full black background
//   Inverted title bar at top
//   Large "BIBLE" text centered
//   Edition subtitle
//   Decorative cross icon (hand-drawn with primitives)
//   "Loading..." hint at bottom

void splash_draw_cb(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_clear(canvas);

    // ── Full black top bar (rows 0-13) ────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 14);

    // "BIBLE" in white on black bar
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 11, AlignCenter, AlignBottom, "B  I  B  L  E");

    // ── Decorative cross — centered, rows 18-46 ───────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    // Vertical beam
    canvas_draw_box(canvas, 61, 18, 6, 28);
    // Horizontal beam
    canvas_draw_box(canvas, 51, 25, 26, 6);
    // White cutout to make it outline style (inner cross)
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 63, 20, 2, 24);
    canvas_draw_box(canvas, 53, 27, 22, 2);

    // ── Subtitle ──────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 55, AlignCenter, AlignBottom, "B.B.E. Edition  ·  66 Books");

    // ── Bottom hint ───────────────────────────────────────────────────────────
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, "Press any key");
}

// ── Input: any key skips splash ───────────────────────────────────────────────
bool splash_input_cb(InputEvent* event, void* ctx) {
    BibleApp* app = ctx;
    if(event->type == InputTypeShort || event->type == InputTypeLong) {
        scene_manager_handle_custom_event(app->scene_manager, 0);
        return true;
    }
    return false;
}

// ── Scene lifecycle ───────────────────────────────────────────────────────────

// Timer callback — fires after 2s to auto-advance
static void splash_timer_callback(void* ctx) {
    BibleApp* app = ctx;
    scene_manager_handle_custom_event(app->scene_manager, 0);
}

static FuriTimer* splash_timer = NULL;

void bible_scene_splash_on_enter(void* ctx) {
    BibleApp* app = ctx;
    view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewSplash);

    // Auto-advance after 2000ms
    splash_timer = furi_timer_alloc(splash_timer_callback, FuriTimerTypeOnce, app);
    furi_timer_start(splash_timer, 2000);
}

bool bible_scene_splash_on_event(void* ctx, SceneManagerEvent event) {
    BibleApp* app = ctx;
    if(event.type == SceneManagerEventTypeCustom) {
        // Cancel timer if user pressed a key
        if(splash_timer) {
            furi_timer_stop(splash_timer);
            furi_timer_free(splash_timer);
            splash_timer = NULL;
        }
        scene_manager_next_scene(app->scene_manager, BibleSceneMainMenu);
        return true;
    }
    return false;
}

void bible_scene_splash_on_exit(void* ctx) {
    UNUSED(ctx);
    if(splash_timer) {
        furi_timer_stop(splash_timer);
        furi_timer_free(splash_timer);
        splash_timer = NULL;
    }
}
