#pragma once

typedef enum {
    BibleSceneSplash,
    BibleSceneMainMenu,
    BibleSceneBookSelect,
    BibleSceneChapterSelect,
    BibleSceneVerseView,
    BibleSceneCount,
} BibleScene;

// Splash
void bible_scene_splash_on_enter(void* ctx);
bool bible_scene_splash_on_event(void* ctx, SceneManagerEvent event);
void bible_scene_splash_on_exit(void* ctx);

// Main menu
void bible_scene_main_menu_on_enter(void* ctx);
bool bible_scene_main_menu_on_event(void* ctx, SceneManagerEvent event);
void bible_scene_main_menu_on_exit(void* ctx);

// Book select
void bible_scene_book_select_on_enter(void* ctx);
bool bible_scene_book_select_on_event(void* ctx, SceneManagerEvent event);
void bible_scene_book_select_on_exit(void* ctx);

// Chapter select
void bible_scene_chapter_select_on_enter(void* ctx);
bool bible_scene_chapter_select_on_event(void* ctx, SceneManagerEvent event);
void bible_scene_chapter_select_on_exit(void* ctx);

// Verse view
void bible_scene_verse_view_on_enter(void* ctx);
bool bible_scene_verse_view_on_event(void* ctx, SceneManagerEvent event);
void bible_scene_verse_view_on_exit(void* ctx);
