#include <form.h>
#include <math.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "modes.h"

#define HEADER 1
#define ODD 2
#define EVEN 3
#define STATUS 4

#define COL_WIDTH 9
#define ROW_HEADER_WIDTH 5
#define MAX_COLS 36
#define MAX_ROWS 256
// TODO: base26
#define COL_HEADER_CHAR "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ALTERNATE_COLOR_ON(win, n, odd, even)                                  \
  (n % 2 == 0) ? wattron(win, COLOR_PAIR(even)) : wattron(win, COLOR_PAIR(odd));
#define ALTERNATE_COLOR_OFF(win, n, odd, even)                                 \
  (n % 2 == 0) ? wattroff(win, COLOR_PAIR(even))                               \
               : wattroff(win, COLOR_PAIR(odd));

// Division of positive integers which rounds up
#define CEIL_DIV(a, b) a / b + (a % b != 0)

// General convention:
// * rows and columns mean the cell indices
// * row=0 and col=0 indicates the header.
//    So the real data cells start at index 1
// * x,y address the character position on screen.

int col_to_x(int n) { return n * COL_WIDTH + ROW_HEADER_WIDTH; }

struct _ui_state {
  int col_width[MAX_COLS];
  int col_offset[MAX_COLS];
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

bool ui_cursor_goto(UiState ui_state, UiCursor cursor, int row, int col) {
  if (row < 0 || row >= MAX_ROWS)
    return false;
  if (col < 0 || col >= MAX_COLS)
    return false;

  int max_y = getmaxy(ui_state->window) - 1;
  if (row >= max_y)
    return false;

  int max_x = getmaxx(ui_state->window);
  int target_x = ui_state->col_offset[col];

  if (target_x > max_x)
    return false;

  cursor->row = row;
  cursor->col = col;
  cursor->x = target_x;
  cursor->y = row;
  cursor->width = ui_state->col_width[col];
  return true;
}
// move the cursor
// returns true if actually moved
bool ui_cursor_left(UiState ui_state, UiCursor cursor) {
  int next_col = cursor->col - 1;
  if (next_col < 0)
    return false;
  int next_x = ui_state->col_offset[next_col];

  cursor->col = next_col;
  cursor->x = next_x;
  cursor->width = ui_state->col_width[next_col];
  return true;
}

bool ui_cursor_right(UiState ui_state, UiCursor cursor) {
  int next_col = cursor->col + 1;
  if (next_col >= MAX_COLS)
    return false;

  int next_width = ui_state->col_width[next_col];
  int next_x = ui_state->col_offset[next_col];
  int max_x = getmaxx(ui_state->window);
  if (next_x + next_width >= max_x)
    return false;

  cursor->col = next_col;
  cursor->x = next_x;
  cursor->width = next_width;
  return true;
}

bool ui_cursor_up(UiState ui_state, UiCursor cursor) {

  int next_row = cursor->row - 1;
  if (next_row < 0)
    return false;

  cursor->row = next_row;
  cursor->y = next_row;
  // TODO: might not be necessary
  cursor->width = ui_state->col_width[cursor->col];

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
  cursor->width = ui_state->col_width[cursor->col];
  return true;
}

// next element from left to right and then top to bottom
bool ui_cursor_next(UiState ui_state, UiCursor cursor) {
  // try goto right
  if (ui_cursor_right(ui_state, cursor))
    return true;
  // try to goto next row
  if (ui_cursor_goto(ui_state, cursor, cursor->row + 1, 0))
    return true;
  return false;
}

//
//
// UiState
//
//
void update_col_offset(UiState ui_state) {
  int curr = 0;
  for (int c = 0; c < MAX_COLS; c++) {
    ui_state->col_offset[c] = curr;
    curr += ui_state->col_width[c];
  }
  ui_state->main_cursor->width =
      ui_state->col_width[ui_state->main_cursor->col];
}

UiState ui_init() {
  start_color();
  init_pair(HEADER, COLOR_YELLOW, COLOR_GREEN);
  init_pair(ODD, COLOR_WHITE, COLOR_BLUE);
  init_pair(EVEN, COLOR_BLACK, COLOR_WHITE);
  init_pair(STATUS, COLOR_BLACK, COLOR_CYAN);

  struct _ui_state *ui_state;
  ui_state = malloc(sizeof(*ui_state));
  ui_state->col_width[0] = ROW_HEADER_WIDTH;
  for (int n = 1; n < MAX_COLS; n++) {
    ui_state->col_width[n] = COL_WIDTH;
  }
  ui_state->window = newwin(0, 0, 0, 0);
  ui_state->main_cursor = ui_cursor_create(ui_state);

  // box(ui_state->window, 0,0);
  update_col_offset(ui_state);

  return ui_state;
}

void ui_destroy(UiState ui_state) {
  delwin(ui_state->window);
  ui_cursor_detroy(ui_state->main_cursor);
  free(ui_state);
}

//
// Ui Operations
//
void ui_left(UiState ui_state) {
  ui_cursor_left(ui_state, ui_state->main_cursor);
}

void ui_right(UiState ui_state) {
  ui_cursor_right(ui_state, ui_state->main_cursor);
}

void ui_up(UiState ui_state) { ui_cursor_up(ui_state, ui_state->main_cursor); }

void ui_down(UiState ui_state) {
  ui_cursor_down(ui_state, ui_state->main_cursor);
}

void ui_inc_current_col(UiState ui_state) {
  int current_column = ui_state->main_cursor->col;

  int max_x = getmaxx(ui_state->window);

  if ((ui_state->main_cursor->x + ui_state->main_cursor->width) >= max_x)
    return;

  ui_state->col_width[current_column] =
      fmin(ui_state->col_width[current_column] + 1, 128);
  // TODO: check if column exceeds page width
  update_col_offset(ui_state);
}

void ui_dec_current_col(UiState ui_state) {
  int current_column = ui_state->main_cursor->col;
  ui_state->col_width[current_column] =
      fmax(ui_state->col_width[current_column] - 1, 1);
  update_col_offset(ui_state);
}

//
// DRAW
//

void ui_draw_col_head(UiState ui_state) {

  WINDOW *win = ui_state->window;
  UiCursor cursor = ui_cursor_create(ui_state);

  while (ui_cursor_right(ui_state, cursor)) {

    int pad_mid = 1;
    int pad_left = (CEIL_DIV((cursor->width - pad_mid), 2)) + pad_mid;
    int pad_right = (cursor->width - pad_mid) / 2;

    ALTERNATE_COLOR_ON(win, cursor->col, ODD, EVEN)

    wmove(win, cursor->y, cursor->x);
    wprintw(win, "%*c%*s",
            /** TODO: base-26 **/
            pad_left, COL_HEADER_CHAR[(cursor->col - 1) % 26], pad_right, "");

    ALTERNATE_COLOR_OFF(win, cursor->col, ODD, EVEN)
  }

  ui_cursor_detroy(cursor);
}

void ui_draw_row_head(UiState ui_state) {

  WINDOW *win = ui_state->window;
  UiCursor cursor = ui_cursor_create(ui_state);

  int pad_mid = 3;
  int pad_left = (CEIL_DIV((cursor->width - pad_mid), 2)) + pad_mid;
  int pad_right = (cursor->width - pad_mid) / 2;

  while (ui_cursor_down(ui_state, cursor)) {

    ALTERNATE_COLOR_ON(win, cursor->row, ODD, EVEN)

    wmove(win, cursor->y, cursor->x);
    wprintw(win, "%*d%*s", pad_left, cursor->y, pad_right, "");

    ALTERNATE_COLOR_OFF(win, cursor->row, ODD, EVEN)
  }

  ui_cursor_detroy(cursor);
}

void ui_draw_status_line(UiState ui_state) {
  WINDOW *win = ui_state->window;

  int max_y = getmaxy(win);
  int max_x = getmaxx(win);
  wmove(win, max_y - 1, 0);
  wattron(win, COLOR_PAIR(STATUS));
  for (int i = 0; i < max_x; i++) {
    waddstr(win, " ");
  }
  wmove(win, max_y - 1, max_x - 10);
  // TODO: base-26
  wprintw(win, "$%c%d     ", COL_HEADER_CHAR[ui_state->main_cursor->col],
          ui_state->main_cursor->row);
  wattroff(win, COLOR_PAIR(STATUS));
}

void ui_draw_cursor(UiState ui_state) {

  WINDOW *win = ui_state->window;

  int x = ui_state->main_cursor->x;
  int y = ui_state->main_cursor->y;
  int width = ui_state->main_cursor->width;

  wmove(win, y, x);
  wattron(win, COLOR_PAIR(STATUS));
  for (int i = 0; i < width; i++)
    waddstr(win, "x");
  wattroff(win, COLOR_PAIR(STATUS));
  wmove(win, 0, 0);
  wprintw(win, "%d:%d:%d", x, y, width);
}

void ui_draw_cells(UiState ui_state) {

  WINDOW *win = ui_state->window;
  UiCursor cursor = ui_cursor_create(ui_state);

  do {
    if (cursor->col == 0 || cursor->row == 0) {
      continue;
    }
    wmove(win, cursor->y, cursor->x);
    for (int i = 0; i < cursor->width; i++) {
      waddstr(win, "-");
    }
  } while (ui_cursor_next(ui_state, cursor));

  ui_cursor_detroy(cursor);
}

void ui_draw(UiState ui_state) {

  // ui draws its own cursor
  curs_set(0);
  ui_draw_row_head(ui_state);
  ui_draw_col_head(ui_state);
  ui_draw_status_line(ui_state);
  ui_draw_cells(ui_state);
  ui_draw_cursor(ui_state);

  // wrefresh(ui_state->window);
  // refresh(); // is this needed
  refresh();
  doupdate();
  wrefresh(ui_state->window);
}

int ui_update(UiState ui_state, int ch){
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
      // ui_open_editor(ui_state);
      //  TO: editor handling loop
      return MODE_EDITOR;
    case 'q':
      return MODE_EXIT;
    }
    return MODE_SHEET;
}

//
// Editor
//
void ui_open_editor(UiState ui_state) {
  FORM *editor_form;
  FIELD *editor_field[2];
  int ch;
  /*
    WINDOW *my_form_win;
    int rows, cols;
    */

  editor_field[0] = new_field(10, 40, 1, 1, 0, 0);
  editor_field[1] = NULL;

  set_field_back(editor_field[0], A_UNDERLINE);
  set_field_fore(editor_field[0], STATUS);
  field_opts_off(editor_field[0], O_AUTOSKIP);
  field_opts_off(editor_field[0], O_AUTOSKIP);

  editor_form = new_form(editor_field);
  post_form(editor_form);
  set_current_field(editor_form, editor_field[0]);

  /*
    scale_form(editor_form, &rows, &cols);
    my_form_win = newwin(rows + 4, cols + 4, 4, 4);
    keypad(my_form_win, TRUE);
    set_form_win(editor_form, my_form_win);
    set_form_sub(editor_form, derwin(my_form_win, rows, cols, 2, 2));
    box(my_form_win, 0, 0);

    wrefresh(my_form_win);
  */

  refresh();

  while ((ch = getch()) != 'q') {
    form_driver(editor_form, ch);

    ui_draw_row_head(ui_state);
    refresh();
  }
  unpost_form(editor_form);
  free_form(editor_form);
  free_field(editor_field[0]);
}
