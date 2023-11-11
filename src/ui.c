#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ui.h"

#define HEADER 1
#define ODD 2
#define EVEN 3
#define STATUS 4

#define COL_WIDTH 9
#define ROW_HEADER_WIDTH 5
#define MAX_COLS 16
#define MAX_ROWS 256
#define COL_HEADER_CHAR " ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ALTERNATE_COLOR_ON(n, odd, even)                                       \
  (n % 2 == 0) ? attron(COLOR_PAIR(even)) : attron(COLOR_PAIR(odd));
#define ALTERNATE_COLOR_OFF(n, odd, even)                                      \
  (n % 2 == 0) ? attroff(COLOR_PAIR(even)) : attroff(COLOR_PAIR(odd));

// General convention:
// * rows and columns mean the cell indices
// * row=0 and col=0 indicates the header.
//    So the real data cells start at index 1
// * x,y address the character position on screen.

int col_to_x(int n) { return n * COL_WIDTH + ROW_HEADER_WIDTH; }

struct _ui_state {
  // TODO: make cel_x/y to a UiCursor
  int cell_x;
  int cell_y;

  int col_width[MAX_COLS];
  WINDOW *window;
  UiCursor main_cursor;
};

//
//
// UiCursor
//
// Iterate and access the different rows and columns
//
struct _ui_cursor {
  // TODO: this might be shorts
  int col;
  int row;

  int x;
  int y;

  // width of the current cell
  int width;
};

UiCursor ui_cursor_create(UiState ui_state) {
  struct _ui_cursor *cursor;
  cursor = malloc(sizeof(*cursor));
  cursor->col = 0;
  cursor->row = 0;
  cursor->x = 0;
  cursor->y = 0;
  cursor->width = ui_state->col_width[0];
  return cursor;
}

void ui_cursor_detroy(UiCursor cursor) { free(cursor); }

bool ui_cursor_goto(UiState ui_state, int row, int col) {}
// move the cursor
// returns true if actually moved
bool ui_cursor_left(UiState ui_state, UiCursor cursor) {
  int next_col = cursor->col - 1;
  if (next_col < 0)
    return false;

  int next_x = 0;
  for (int c = 0; c < next_col; c++) {
    next_x += ui_state->col_width[c];
  }

  cursor->col = next_col;
  cursor->x = next_x;
  cursor->width = ui_state->col_width[next_col];
  return true;
}

bool ui_cursor_right(UiState ui_state, UiCursor cursor) {
  int next_col = cursor->col + 1;
  if (next_col >= MAX_COLS)
    return false;

  int next_x = 0;
  for (int c = 0; c < next_col; c++) {
    next_x += ui_state->col_width[c];
  }
  int max_x = getmaxx(ui_state->window);
  if (next_x >= max_x)
    return false;

  cursor->col = next_col;
  cursor->x = next_x;
  cursor->width = ui_state->col_width[next_col];
  return true;
}

bool ui_cursor_up(UiState ui_state, UiCursor cursor) {

  int next_row = cursor->row - 1;
  if (next_row < 0)
    return false;

  cursor->row = next_row;
  cursor->y = next_row;
  // TODO: might not be necessary
  cursor->width = ui_state->col_width[cursor->x];

  return true;
}

bool ui_cursor_down(UiState ui_state, UiCursor cursor) {
  int next_row = cursor->row + 1;
  int max_y = getmaxy(ui_state->window) - 1; /*minus status row*/
  if (next_row >= max_y)
    return false;

  cursor->row = next_row;
  cursor->y = next_row;
  // TODO: might not be necessary
  cursor->width = ui_state->col_width[cursor->x];
  return true;
}

//
//
// UiState
//
//
UiState ui_init() {
  start_color();
  init_pair(HEADER, COLOR_YELLOW, COLOR_GREEN);
  init_pair(ODD, COLOR_WHITE, COLOR_BLUE);
  init_pair(EVEN, COLOR_BLACK, COLOR_WHITE);
  init_pair(STATUS, COLOR_BLACK, COLOR_CYAN);

  struct _ui_state *ui_state;
  ui_state = malloc(sizeof(*ui_state));
  ui_state->col_width[0] = 5;
  for (int n = 1; n < MAX_COLS; n++) {
    ui_state->col_width[n] = 8;
  }

  ui_state->window = newwin(0, 0, 0, 0);
  ui_state->main_cursor = ui_cursor_create(ui_state);

  return ui_state;
}

void ui_destroy(UiState ui_state) {
  delwin(ui_state->window); //??
  ui_cursor_detroy(ui_state->main_cursor);
  free(ui_state);
}

//
// Ui Operations
//
void ui_left(UiState ui_state) { ui_cursor_left(ui_state, ui_state->main_cursor);}
void ui_right(UiState ui_state) { ui_cursor_right(ui_state, ui_state->main_cursor);}
void ui_up(UiState ui_state) { ui_cursor_up(ui_state, ui_state->main_cursor);}
void ui_down(UiState ui_state) { ui_cursor_down(ui_state, ui_state->main_cursor);}



void ui_draw_col_head(UiState ui_state) {
  int max_x = getmaxx(ui_state->window);
  char head[] = "         ";
  for (int n = 0; col_to_x(n + 1) < max_x; n++) {

    ALTERNATE_COLOR_ON(n, ODD, EVEN)

    move(0, ROW_HEADER_WIDTH + n * COL_WIDTH);
    head[4] = COL_HEADER_CHAR[n];
    addstr(head);

    ALTERNATE_COLOR_OFF(n, ODD, EVEN)
  }
}

void ui_draw_status_line(UiState ui_state) {
  int max_y = getmaxy(ui_state->window);
  int max_x = getmaxx(ui_state->window);
  move(max_y - 1, 0);
  attron(COLOR_PAIR(STATUS));
  for (int i = 0; i < max_x; i++) {
    addstr(" ");
  }
  move(max_y - 1, max_x - 10);
  printw("$%c%d     ", COL_HEADER_CHAR[ui_state->cell_x], ui_state->cell_y);
  attroff(COLOR_PAIR(STATUS));
}

void ui_draw_row_head(UiState ui_state) {
  int max_y = getmaxy(ui_state->window);
  for (int y = 1; y < max_y; y++) {

    ALTERNATE_COLOR_ON(y, ODD, EVEN)

    move(y, 0);
    printw(" %3d ", y);

    ALTERNATE_COLOR_OFF(y, ODD, EVEN)
  }
}

void ui_draw_cursor(UiState ui_state) {
  int x = ui_state->main_cursor->x;
  int y = ui_state->main_cursor->y;
  int width = ui_state->main_cursor->width;
  move(y, x);
  // printw("%d:%d:%d",x,y, width);
  attron(COLOR_PAIR(STATUS));
  for (int i = 0; i < width; i++)
    addstr("x");
  attroff(COLOR_PAIR(STATUS));
  move(0, 0);
}
