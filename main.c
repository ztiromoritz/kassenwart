#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "src/editor.h"
#include "src/modes.h"
#include "src/ui.h"

int main() {
  FILE *log;
  log = fopen("out.txt", "a");
  fprintf(log, "\nlog:\n");
  // fflush(log);

  initscr();
  cbreak();
  // timeout(0);
  // nocbreak();
  // /notimeout(stdscr,TRUE);
  keypad(stdscr, FALSE);
  fprintf(log,"notimeout %d\n",notimeout(stdscr, FALSE));
  raw();
  noecho();
  curs_set(0);

  timeout(5);

  UiState ui_state = ui_init();
  EditorState editor_state = editor_init();
  int ch;
  int mode = MODE_INIT;
  do {
    // update
    // draw
    switch (mode) {
    case MODE_INIT:
      // TODO:
      mode = MODE_SHEET;
      break;
    case MODE_SHEET:
      ui_draw(ui_state);
      mode = ui_update(ui_state, ch);
      break;
    case MODE_EDITOR:
      editor_draw(editor_state);
      mode = editor_update(editor_state, ch);
      break;
    }

    // refresh();
    doupdate();
    ch = getch();
    if (ch != -1) {
      fprintf(log, "%s - 0x%02x\n", keyname(ch), ch);
      fflush(log);
    }
  } while (mode != MODE_EXIT);

  endwin();
  ui_destroy(ui_state);
  editor_destroy(editor_state);

  fclose(log);
  return 0;
}
