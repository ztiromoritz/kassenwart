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
    }
    // draw
    ui_draw_row_head(ui_state);
    ui_draw_col_head(ui_state);
    ui_draw_status_line(ui_state);
    ui_draw_cells(ui_state);
    ui_draw_cursor(ui_state);

    refresh();

    ch = getch();

  } while (ch != 'q');

  endwin();
  ui_destroy(ui_state);
  return 0;
}

void cursesTest() {
  initscr();
  raw();                /* Line buffering disabled	*/
  keypad(stdscr, TRUE); /* We get F1, F2 etc..		*/
  noecho();             /* Don't echo() while we do getch */
  refresh();

  WINDOW *win = newwin(0, 0, 0, 0);

  // making box border with default border styles
  box(win, 0, 0);

  // move and print in window
  mvwprintw(win, 0, 1, "Greeter");
  mvwprintw(win, 1, 1, "Hello");

  // refreshing the window
  wrefresh(win); // making box border with default border styles
  box(win, 0, 0);

  // move and print in window
  mvwprintw(win, 0, 1, "Greeter");
  mvwprintw(win, 1, 1, "Hello");

  // refreshing the window
  wrefresh(win);
  refresh();
  getch();
  endwin();
}

int luaTest() {

  char buff[256];
  int error;
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  error = luaL_loadfile(L, "../functions.lua") || lua_pcall(L, 0, 0, 0);
  if (error) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
  }
  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
  lua_close(L);
  return 0;
}
