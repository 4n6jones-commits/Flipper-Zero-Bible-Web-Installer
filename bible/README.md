# Flipper Bible — Redesigned UI

BBE Bible reader for Flipper Zero with splash screen, inverted headers,
book abbreviation badges, OT/NT indicator, verse progress bar, chapter
skip (long press), and automatic bookmark saving.

---

## How to build the .fap (no local tools needed)

1. Create a free account at **github.com** if you don't have one.

2. Click **+** → **New repository** → name it `flipper-bible` → Create.

3. Upload this entire folder to the repo (drag the files onto the GitHub
   page, or use the GitHub Desktop app).

4. GitHub will automatically start building. Go to the **Actions** tab
   and wait ~2 minutes for the green checkmark.

5. Click the completed workflow run → scroll to the bottom →
   **Download `bible_app.fap`** from the Artifacts section.

6. Open the web installer → **Step 2** → drop in the downloaded `.fap`.

7. Install everything to your Flipper in one click.

---

## Controls

| Button | Action |
|--------|--------|
| Up / Down | Scroll menus, previous / next verse |
| OK | Select book or chapter |
| Long Left | Previous chapter (from verse view) |
| Long Right | Next chapter (from verse view) |
| Back | Go back / exit (saves reading position) |

---

## File layout on device

```
/ext/apps/Misc/bible_app.fap
/ext/apps_data/bible/books.txt
/ext/apps_data/bible/data/GEN.txt
/ext/apps_data/bible/data/EXO.txt
... (66 books)
/ext/apps_data/bible/bookmark.txt  ← created on first exit
```
