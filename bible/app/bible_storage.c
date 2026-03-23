#include "bible_storage.h"

// ── Internal helpers ──────────────────────────────────────────────────────────

static int read_line(File* file, char* buf, int max_len) {
    int  len = 0;
    char c;
    while(len < max_len - 1) {
        if(storage_file_read(file, &c, 1) != 1) break;
        if(c == '\n') break;
        if(c != '\r') buf[len++] = c;
    }
    buf[len] = '\0';
    return len;
}

// ── Book index ────────────────────────────────────────────────────────────────

bool bible_load_books(BibleApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);

    bool ok = false;
    app->num_books = 0;

    if(!storage_file_open(file, BIBLE_BOOKS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        goto done;
    }

    char line[64];
    while(app->num_books < MAX_BOOKS) {
        int len = read_line(file, line, sizeof(line));
        if(len == 0 && storage_file_eof(file)) break;
        if(len == 0) continue;

        char* p1 = strchr(line, '|');
        if(!p1) continue;
        char* p2 = strchr(p1 + 1, '|');
        if(!p2) continue;

        BookInfo* b = &app->books[app->num_books];

        int name_len = (int)(p1 - line);
        if(name_len >= MAX_BOOK_NAME) name_len = MAX_BOOK_NAME - 1;
        memcpy(b->name, line, name_len);
        b->name[name_len] = '\0';

        int abbr_len = (int)(p2 - p1 - 1);
        if(abbr_len >= MAX_ABBREV) abbr_len = MAX_ABBREV - 1;
        memcpy(b->abbrev, p1 + 1, abbr_len);
        b->abbrev[abbr_len] = '\0';

        b->num_chapters = atoi(p2 + 1);
        app->num_books++;
    }

    ok = (app->num_books > 0);

done:
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

// ── Chapter loader ────────────────────────────────────────────────────────────

bool bible_load_chapter(BibleApp* app, int book_idx, int chapter_idx) {
    if(book_idx < 0 || book_idx >= app->num_books) return false;
    if(chapter_idx < 0 || chapter_idx >= app->books[book_idx].num_chapters) return false;

    char path[128];
    snprintf(path, sizeof(path), "%s/%s.txt", BIBLE_DATA_DIR,
             app->books[book_idx].abbrev);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);

    bool ok = false;
    app->verse_count = 0;

    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        goto done;
    }

    // Read index header
    uint32_t chapter_offsets[MAX_CHAPTERS];
    memset(chapter_offsets, 0, sizeof(chapter_offsets));

    char line[64];
    while(true) {
        int len = read_line(file, line, sizeof(line));
        if(len == 0) break;
        if(line[0] != '#') break;

        if(strncmp(line, "#IDX:", 5) != 0) {
            char* colon = strchr(line + 1, ':');
            if(colon) {
                int      idx    = atoi(line + 1);
                uint32_t offset = (uint32_t)strtoul(colon + 1, NULL, 10);
                if(idx >= 0 && idx < MAX_CHAPTERS) {
                    chapter_offsets[idx] = offset;
                }
            }
        }

        if(storage_file_eof(file)) break;
    }

    if(!storage_file_seek(file, chapter_offsets[chapter_idx], true)) goto done;

    // Read verses
    char* buf     = app->verse_buffer;
    int   buf_pos = 0;
    int   expected_chapter = chapter_idx + 1;

    app->verse_count = 0;
    char verse_line[512];

    while(app->verse_count < MAX_VERSES) {
        int len = read_line(file, verse_line, sizeof(verse_line));
        if(len == 0) {
            if(storage_file_eof(file)) break;
            continue;
        }

        char* first_colon = strchr(verse_line, ':');
        if(!first_colon) continue;

        int chap_num = atoi(verse_line);
        if(chap_num != expected_chapter) break;

        char* pipe = strchr(first_colon + 1, '|');
        if(!pipe) continue;

        const char* text = pipe + 1;
        int         tlen = (int)strlen(text);

        if(buf_pos + tlen + 1 > VERSE_BUF_SIZE) break;

        app->verse_ptrs[app->verse_count] = buf + buf_pos;
        memcpy(buf + buf_pos, text, tlen);
        buf[buf_pos + tlen] = '\0';
        buf_pos += tlen + 1;
        app->verse_count++;
    }

    ok = (app->verse_count > 0);

done:
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

// ── Bookmark ──────────────────────────────────────────────────────────────────

void bible_save_bookmark(BibleApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);

    if(storage_file_open(file, BIBLE_BOOKMARK, FSAM_WRITE,
                          FSOM_CREATE_ALWAYS)) {
        char buf[32];
        int  len = snprintf(buf, sizeof(buf), "%d %d %d\n",
                            app->selected_book,
                            app->selected_chapter,
                            app->current_verse);
        storage_file_write(file, buf, len);
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void bible_load_bookmark(BibleApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);

    if(storage_file_open(file, BIBLE_BOOKMARK, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buf[32];
        int  len = (int)storage_file_read(file, buf, sizeof(buf) - 1);
        buf[len] = '\0';
        int b = 0, c = 0, v = 0;
        if(sscanf(buf, "%d %d %d", &b, &c, &v) == 3) {
            if(b >= 0 && b < app->num_books) {
                app->selected_book    = b;
                app->selected_chapter = c;
                app->current_verse    = v;
            }
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}
