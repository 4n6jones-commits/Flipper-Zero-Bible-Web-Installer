#include "bible_app.h"
#include "bible_storage.h"
#include "scenes/bible_scene_config.h"

// ── Scene handler tables ──────────────────────────────────────────────────────

static const AppSceneOnEnterCallback bible_on_enter_handlers[] = {
    [BibleSceneSplash]         = bible_scene_splash_on_enter,
    [BibleSceneMainMenu]       = bible_scene_main_menu_on_enter,
    [BibleSceneBookSelect]     = bible_scene_book_select_on_enter,
    [BibleSceneChapterSelect]  = bible_scene_chapter_select_on_enter,
    [BibleSceneVerseView]      = bible_scene_verse_view_on_enter,
};

static const AppSceneOnEventCallback bible_on_event_handlers[] = {
    [BibleSceneSplash]         = bible_scene_splash_on_event,
    [BibleSceneMainMenu]       = bible_scene_main_menu_on_event,
    [BibleSceneBookSelect]     = bible_scene_book_select_on_event,
    [BibleSceneChapterSelect]  = bible_scene_chapter_select_on_event,
    [BibleSceneVerseView]      = bible_scene_verse_view_on_event,
};

static const AppSceneOnExitCallback bible_on_exit_handlers[] = {
    [BibleSceneSplash]         = bible_scene_splash_on_exit,
    [BibleSceneMainMenu]       = bible_scene_main_menu_on_exit,
    [BibleSceneBookSelect]     = bible_scene_book_select_on_exit,
    [BibleSceneChapterSelect]  = bible_scene_chapter_select_on_exit,
    [BibleSceneVerseView]      = bible_scene_verse_view_on_exit,
};

static const SceneManagerHandlers bible_scene_handlers = {
    .on_enter_handlers = bible_on_enter_handlers,
    .on_event_handlers = bible_on_event_handlers,
    .on_exit_handlers  = bible_on_exit_handlers,
    .scene_num         = BibleSceneCount,
};

// ── Singleton ─────────────────────────────────────────────────────────────────
static BibleApp* s_app = NULL;
BibleApp* bible_app_get(void) { return s_app; }

// ── Back handler ──────────────────────────────────────────────────────────────
static bool bible_back_event_callback(void* ctx) {
    BibleApp* app = ctx;
    return scene_manager_handle_back_event(app->scene_manager);
}

// ── Forward declarations of view callbacks (defined in their scene files) ─────
void splash_draw_cb(Canvas* canvas, void* model);
bool splash_input_cb(InputEvent* event, void* ctx);

void main_menu_draw_cb(Canvas* canvas, void* model);
bool main_menu_input_cb(InputEvent* event, void* ctx);

void book_select_draw_cb(Canvas* canvas, void* model);
bool book_select_input_cb(InputEvent* event, void* ctx);

void chapter_select_draw_cb(Canvas* canvas, void* model);
bool chapter_select_input_cb(InputEvent* event, void* ctx);

void verse_view_draw_cb(Canvas* canvas, void* model);
bool verse_view_input_cb(InputEvent* event, void* ctx);

// ── Model sizes (opaque to main, sized in scene files via sizeof) ─────────────
// We declare them large enough here; actual structs defined in each scene file.
// Each scene allocates its own model via view_allocate_model.
#define SPLASH_MODEL_SIZE         4
#define MAIN_MENU_MODEL_SIZE      8
#define BOOK_SELECT_MODEL_SIZE    8
#define CHAPTER_SELECT_MODEL_SIZE 8
#define VERSE_MODEL_SIZE        512

// ── Alloc / Free ──────────────────────────────────────────────────────────────

static BibleApp* bible_app_alloc(void) {
    BibleApp* app = malloc(sizeof(BibleApp));
    furi_assert(app);
    memset(app, 0, sizeof(BibleApp));

    app->verse_buffer = malloc(VERSE_BUF_SIZE);
    furi_assert(app->verse_buffer);

    app->gui             = furi_record_open(RECORD_GUI);
    app->scene_manager   = scene_manager_alloc(&bible_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_attach_to_gui(
        app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bible_back_event_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    // ── Splash view ───────────────────────────────────────────────────────────
    app->splash_view = view_alloc();
    view_set_draw_callback(app->splash_view, splash_draw_cb);
    view_set_input_callback(app->splash_view, splash_input_cb);
    view_set_context(app->splash_view, app);
    view_allocate_model(app->splash_view, ViewModelTypeLockFree, SPLASH_MODEL_SIZE);
    view_dispatcher_add_view(app->view_dispatcher, BibleViewSplash, app->splash_view);

    // ── Main menu view ────────────────────────────────────────────────────────
    app->main_menu_view = view_alloc();
    view_set_draw_callback(app->main_menu_view, main_menu_draw_cb);
    view_set_input_callback(app->main_menu_view, main_menu_input_cb);
    view_set_context(app->main_menu_view, app);
    view_allocate_model(app->main_menu_view, ViewModelTypeLockFree, MAIN_MENU_MODEL_SIZE);
    view_dispatcher_add_view(app->view_dispatcher, BibleViewMainMenu, app->main_menu_view);

    // ── Book select view ──────────────────────────────────────────────────────
    app->book_select_view = view_alloc();
    view_set_draw_callback(app->book_select_view, book_select_draw_cb);
    view_set_input_callback(app->book_select_view, book_select_input_cb);
    view_set_context(app->book_select_view, app);
    view_allocate_model(app->book_select_view, ViewModelTypeLockFree, BOOK_SELECT_MODEL_SIZE);
    view_dispatcher_add_view(app->view_dispatcher, BibleViewBookSelect, app->book_select_view);

    // ── Chapter select view ───────────────────────────────────────────────────
    app->chapter_select_view = view_alloc();
    view_set_draw_callback(app->chapter_select_view, chapter_select_draw_cb);
    view_set_input_callback(app->chapter_select_view, chapter_select_input_cb);
    view_set_context(app->chapter_select_view, app);
    view_allocate_model(app->chapter_select_view, ViewModelTypeLockFree, CHAPTER_SELECT_MODEL_SIZE);
    view_dispatcher_add_view(app->view_dispatcher, BibleViewChapterSelect, app->chapter_select_view);

    // ── Verse view ────────────────────────────────────────────────────────────
    app->verse_view = view_alloc();
    view_set_draw_callback(app->verse_view, verse_view_draw_cb);
    view_set_input_callback(app->verse_view, verse_view_input_cb);
    view_set_context(app->verse_view, app);
    view_allocate_model(app->verse_view, ViewModelTypeLockFree, VERSE_MODEL_SIZE);
    view_dispatcher_add_view(app->view_dispatcher, BibleViewVerseView, app->verse_view);

    return app;
}

static void bible_app_free(BibleApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, BibleViewSplash);
    view_dispatcher_remove_view(app->view_dispatcher, BibleViewMainMenu);
    view_dispatcher_remove_view(app->view_dispatcher, BibleViewBookSelect);
    view_dispatcher_remove_view(app->view_dispatcher, BibleViewChapterSelect);
    view_dispatcher_remove_view(app->view_dispatcher, BibleViewVerseView);

    view_free(app->splash_view);
    view_free(app->main_menu_view);
    view_free(app->book_select_view);
    view_free(app->chapter_select_view);
    view_free(app->verse_view);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    furi_record_close(RECORD_GUI);

    free(app->verse_buffer);
    free(app);
}

// ── Entry point ───────────────────────────────────────────────────────────────

int32_t bible_app_main(void* p) {
    UNUSED(p);

    BibleApp* app = bible_app_alloc();
    s_app = app;

    if(!bible_load_books(app)) {
        // Show a brief error — no data files found
        furi_record_close(RECORD_GUI);
        free(app->verse_buffer);
        free(app);
        return 1;
    }

    // Load last reading position if bookmark exists
    bible_load_bookmark(app);

    // Start at splash screen
    scene_manager_next_scene(app->scene_manager, BibleSceneSplash);
    view_dispatcher_run(app->view_dispatcher);

    // Save position on exit
    bible_save_bookmark(app);

    bible_app_free(app);
    s_app = NULL;
    return 0;
}
