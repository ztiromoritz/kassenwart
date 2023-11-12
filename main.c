#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "src/ui.h"
#include <ncurses.h>

int main() {

  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);

  UiState ui_state = ui_init();
  int ch;
  do {
    // update
    switch (ch) {
    case KEY_UP:
    case 'k':
      ui_up(ui_state);
      break;
    case KEY_DOWN:
    case 'j':
      ui_down(ui_state);
      break;
    case KEY_LEFT:
    case 'h':
      ui_left(ui_state);
      break;
    case KEY_RIGHT:
    case 'l':
      ui_right(ui_state);
      break;
    case '+':
      ui_inc_current_col(ui_state);
      break;
    case '-':
      ui_dec_current_col(ui_state);
      break;
    case 'i':
      ui_open_editor(ui_state);
      // TODO: editor handling loop
      break;
    }
    // draw
    ui_draw(ui_state);

    ch = getch();

  } while (ch != 'q');

  endwin();
  ui_destroy(ui_state);
  return 0;
}

