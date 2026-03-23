#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include <storage/storage.h>
#include <furi_hal_random.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ── SD card paths ─────────────────────────────────────────────────────────────
#define BIBLE_DIR          "/ext/apps_data/bible"
#define BIBLE_BOOKS_FILE   BIBLE_DIR "/books.txt"
#define BIBLE_DATA_DIR     BIBLE_DIR "/data"
#define BIBLE_BOOKMARK     BIBLE_DIR "/bookmark.txt"

// ── Limits ────────────────────────────────────────────────────────────────────
#define MAX_BOOKS         66
#define MAX_BOOK_NAME     32
#define MAX_ABBREV         5
#define MAX_CHAPTERS     155
#define MAX_VERSES       200
#define VERSE_BUF_SIZE  (16 * 1024)

// First book of the New Testament (0-based index = 39 = Matthew)
#define NT_START_BOOK     39

// ── View IDs ──────────────────────────────────────────────────────────────────
typedef enum {
    BibleViewSplash,
    BibleViewMainMenu,
    BibleViewBookSelect,
    BibleViewChapterSelect,
    BibleViewVerseView,
} BibleViewId;

// ── Book index entry ──────────────────────────────────────────────────────────
typedef struct {
    char name[MAX_BOOK_NAME];
    char abbrev[MAX_ABBREV];
    int  num_chapters;
} BookInfo;

// ── App state ─────────────────────────────────────────────────────────────────
typedef struct BibleApp {
    Gui*             gui;
    ViewDispatcher*  view_dispatcher;
    SceneManager*    scene_manager;

    // Views (all custom-drawn)
    View* splash_view;
    View* main_menu_view;
    View* book_select_view;
    View* chapter_select_view;
    View* verse_view;

    // Book index
    BookInfo  books[MAX_BOOKS];
    int       num_books;

    // Navigation state
    int selected_book;
    int selected_chapter;
    int current_verse;

    // Scroll state for list views
    int book_scroll_offset;
    int chapter_scroll_offset;

    // Current chapter verse data
    char* verse_buffer;
    char* verse_ptrs[MAX_VERSES];
    int   verse_count;

    // Scratch buffer
    char display_text[512];
} BibleApp;

BibleApp* bible_app_get(void);
