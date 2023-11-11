#include <math.h> 
#include <stdio.h> 
#include <string.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "src/ui.h"
#include <ncurses.h>

int main() {

  UiState uiState = {.cell_x = 0, .cell_y = 0};

  initscr();
  cbreak();
  keypad(stdscr, TRUE); 
  noecho();             
  curs_set(0);

  WINDOW *window = newwin(0, 0, 0, 0);

  uiInit();
  int ch;
  do {

    drawRowHeader(window, &uiState);
    drawHeader(window, &uiState);
    drawStatusLine(window, &uiState);
    drawCursor(window, &uiState);

    refresh();

    ch = getch();
    switch (ch) {
    case KEY_UP:
    case 'k':
      uiState.cell_y = fmax(uiState.cell_y - 1, 0);
      break;
    case KEY_DOWN:
    case 'j':
      uiState.cell_y = fmin(uiState.cell_y + 1, 256 /*TODO const*/);
      break;
    case KEY_LEFT:
    case 'h':
      uiState.cell_x = fmax(uiState.cell_x - 1, 0);
      break;
    case KEY_RIGHT:
    case 'l':
      uiState.cell_x = fmin(uiState.cell_x + 1, 16 /*TODO const*/);
      break;
    }
  } while (ch != 'q');

  endwin();
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
