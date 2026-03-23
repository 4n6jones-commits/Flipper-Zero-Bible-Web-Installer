#include "../bible_app.h"
#include "bible_scene_config.h"
#include <gui/canvas.h>
#include <input/input.h>

// ── Layout ────────────────────────────────────────────────────────────────────
// Row 0-13:  Inverted header — "Book Ch:V"
// Row 14:    Thin divider line
// Row 16-50: Word-wrapped verse text (FontSecondary, 5 lines max)
// Row 51:    Divider
// Row 52-63: Footer — progress bar left, "V of N" right

#define TEXT_Y_START   23    // first text line baseline
#define LINE_HEIGHT     9
#define MAX_TEXT_WIDTH 124
#define MAX_LINES       4    // lines of verse text that fit

// ── Word-wrap renderer ────────────────────────────────────────────────────────
static void draw_wrapped(Canvas* canvas, const char* text) {
    canvas_set_font(canvas, FontSecondary);

    char word[64];
    char line[128] = {0};
    int  line_w    = 0;
    int  y         = TEXT_Y_START;
    int  lines     = 0;

    const char* p = text;

#define FLUSH()                                      \
    do {                                             \
        if(line[0] && lines < MAX_LINES) {           \
            canvas_draw_str(canvas, 2, y, line);     \
            y += LINE_HEIGHT;                        \
            lines++;                                 \
            line[0] = '\0';                          \
            line_w  = 0;                             \
        }                                            \
    } while(0)

    while(*p && lines < MAX_LINES) {
        int wlen = 0;
        while(*p && *p != ' ' && wlen < (int)sizeof(word) - 1)
            word[wlen++] = *p++;
        word[wlen] = '\0';
        if(*p == ' ') p++;
        if(wlen == 0) continue;

        int word_w  = canvas_string_width(canvas, word);
        int space_w = line[0] ? canvas_string_width(canvas, " ") : 0;

        if(line[0] && line_w + space_w + word_w > MAX_TEXT_WIDTH) FLUSH();
        if(line[0]) { strlcat(line, " ", sizeof(line)); line_w += space_w; }
        strlcat(line, word, sizeof(line));
        line_w += word_w;
    }
    FLUSH();

    // If text was truncated, show ellipsis on last line
    if(*p) {
        // replace last 3 chars of last drawn line with "..."
        // (just redraw a "..." indicator at bottom right of text area)
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_str(canvas, 116, TEXT_Y_START + (MAX_LINES - 1) * LINE_HEIGHT, "...");
    }

#undef FLUSH
}

// ── Draw ──────────────────────────────────────────────────────────────────────
void verse_view_draw_cb(Canvas* canvas, void* model) {
    UNUSED(model);
    BibleApp* app = bible_app_get();
    if(!app) return;

    canvas_clear(canvas);

    // ── Header bar (inverted) ─────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 14);

    // Reference: "Genesis 1:3"
    char ref[40];
    snprintf(ref, sizeof(ref), "%s %d:%d",
             app->books[app->selected_book].name,
             app->selected_chapter + 1,
             app->current_verse + 1);

    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 11, ref);

    // ── Verse text ────────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    const char* verse_text = (app->current_verse < app->verse_count)
        ? app->verse_ptrs[app->current_verse]
        : "";
    draw_wrapped(canvas, verse_text);

    // ── Footer divider ────────────────────────────────────────────────────────
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 0, 52, 128, 52);

    // ── Footer: progress bar ──────────────────────────────────────────────────
    int bar_w  = 76;
    int filled = (app->verse_count > 1)
        ? (int)((long)app->current_verse * bar_w / (app->verse_count - 1))
        : bar_w;
    canvas_draw_frame(canvas, 2, 55, bar_w + 2, 5);
    canvas_draw_box(canvas, 3, 56, filled, 3);

    // ── Footer: "V of N" label ────────────────────────────────────────────────
    char label[16];
    snprintf(label, sizeof(label), "%d/%d",
             app->current_verse + 1, app->verse_count);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 84, 62, label);

    // ── Nav arrows at footer right ────────────────────────────────────────────
    // Left arrow if not at first verse
    if(app->current_verse > 0) {
        canvas_draw_str(canvas, 82, 62, "<");
    }
    // Right arrow if not at last verse
    if(app->current_verse < app->verse_count - 1) {
        int label_w = canvas_string_width(canvas, label);
        canvas_draw_str(canvas, 84 + label_w + 2, 62, ">");
    }
}

// ── Input ─────────────────────────────────────────────────────────────────────
// Short press Up/Down or Left/Right: previous/next verse
// Long press Left/Right: previous/next chapter
bool verse_view_input_cb(InputEvent* event, void* ctx) {
    BibleApp* app = ctx;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        if(event->key == InputKeyRight || event->key == InputKeyDown) {
            if(app->current_verse < app->verse_count - 1) {
                app->current_verse++;
                scene_manager_handle_custom_event(app->scene_manager, 0);
            }
            return true;
        }
        if(event->key == InputKeyLeft || event->key == InputKeyUp) {
            if(app->current_verse > 0) {
                app->current_verse--;
                scene_manager_handle_custom_event(app->scene_manager, 0);
            }
            return true;
        }
    }

    // Long press: chapter navigation
    if(event->type == InputTypeLong) {
        if(event->key == InputKeyRight) {
            // Next chapter (or next book ch 0)
            int num_ch = app->books[app->selected_book].num_chapters;
            if(app->selected_chapter < num_ch - 1) {
                app->selected_chapter++;
            } else if(app->selected_book < app->num_books - 1) {
                app->selected_book++;
                app->selected_chapter = 0;
            }
            app->current_verse = 0;
            if(bible_load_chapter(app, app->selected_book, app->selected_chapter)) {
                scene_manager_handle_custom_event(app->scene_manager, 0);
            }
            return true;
        }
        if(event->key == InputKeyLeft) {
            // Previous chapter
            if(app->selected_chapter > 0) {
                app->selected_chapter--;
            } else if(app->selected_book > 0) {
                app->selected_book--;
                app->selected_chapter = app->books[app->selected_book].num_chapters - 1;
            }
            app->current_verse = 0;
            if(bible_load_chapter(app, app->selected_book, app->selected_chapter)) {
                scene_manager_handle_custom_event(app->scene_manager, 0);
            }
            return true;
        }
    }

    return false;
}

// ── Scene lifecycle ───────────────────────────────────────────────────────────

void bible_scene_verse_view_on_enter(void* ctx) {
    BibleApp* app = ctx;
    view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewVerseView);
}

bool bible_scene_verse_view_on_event(void* ctx, SceneManagerEvent event) {
    BibleApp* app = ctx;
    if(event.type == SceneManagerEventTypeCustom) {
        // Trigger a redraw by forcing view switch (view_commit_model alternative)
        view_dispatcher_switch_to_view(app->view_dispatcher, BibleViewVerseView);
        return true;
    }
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void bible_scene_verse_view_on_exit(void* ctx) {
    UNUSED(ctx);
}
