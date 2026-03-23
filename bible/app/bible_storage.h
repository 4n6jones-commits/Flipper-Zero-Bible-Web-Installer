#pragma once
#include "bible_app.h"

bool bible_load_books(BibleApp* app);
bool bible_load_chapter(BibleApp* app, int book_idx, int chapter_idx);
void bible_save_bookmark(BibleApp* app);
void bible_load_bookmark(BibleApp* app);
