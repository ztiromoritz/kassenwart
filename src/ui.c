#include "ui.h"
#include <ncurses.h>

#define HEADER 1
#define ODD 2
#define EVEN 3
#define STATUS 4

#define COL_WIDTH 9
#define ROW_HEADER_WIDTH 5
#define MAX_COLS 16
#define MAX_ROWS 256
#define COL_HEADER_CHAR "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ALTERNATE_COLOR_ON(n, odd, even)                                       \
  (n % 2 == 0) ? attron(COLOR_PAIR(even)) : attron(COLOR_PAIR(odd));
#define ALTERNATE_COLOR_OFF(n, odd, even)                                      \
  (n % 2 == 0) ? attroff(COLOR_PAIR(even)) : attroff(COLOR_PAIR(odd));

void uiInit() {
  start_color();
  init_pair(HEADER, COLOR_YELLOW, COLOR_GREEN);
  init_pair(ODD, COLOR_WHITE, COLOR_BLUE);
  init_pair(EVEN, COLOR_BLACK, COLOR_WHITE);
  init_pair(STATUS, COLOR_BLACK, COLOR_CYAN);
}

int colToX(int n) { return n * COL_WIDTH + ROW_HEADER_WIDTH; }

void drawHeader(WINDOW *window, UiState *uiState) {
  int max_x = getmaxx(window);
  char head[] = "         ";
  for (int n = 0; colToX(n + 1) < max_x; n++) {

    ALTERNATE_COLOR_ON(n, ODD, EVEN)

    move(0, ROW_HEADER_WIDTH + n * COL_WIDTH);
    head[4] = COL_HEADER_CHAR[n];
    addstr(head);

    ALTERNATE_COLOR_OFF(n, ODD, EVEN)
  }
}

void drawStatusLine(WINDOW *window, UiState *uiState) {
  int max_y = getmaxy(window);
  int max_x = getmaxx(window);
  move(max_y - 1, 0);
  attron(COLOR_PAIR(STATUS));
  for (int i = 0; i < max_x; i++) {
    addstr(" ");
  }
  move(max_y - 1, max_x - 10);
  printw("$%c%d     ", COL_HEADER_CHAR[uiState->cell_x], uiState->cell_y);
  attroff(COLOR_PAIR(STATUS));
}

void drawRowHeader(WINDOW *window, UiState *uiState) {
  int max_y = getmaxy(window);
  for (int y = 1; y < max_y; y++) {

    ALTERNATE_COLOR_ON(y, ODD, EVEN)

    move(y, 0);
    printw(" %3d ", y);

    ALTERNATE_COLOR_OFF(y, ODD, EVEN)
  }
}

void drawCursor(WINDOW *window, UiState *uiState) {
  int x = colToX(uiState->cell_x);
  int y = uiState->cell_y;
  move(y,x);
  attron(COLOR_PAIR(STATUS));
  addstr("x");
  attroff(COLOR_PAIR(STATUS));
  move(0,0);
}
