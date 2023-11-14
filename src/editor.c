#include <ncurses.h>
#include <stdlib.h>

#include "editor.h"
#include "modes.h"

#define ctrl(x) ((x)&0x1f)

typedef struct _editor_row {
  int size;
  char *chars;
} EditorRow;

struct _editor_state {

  int row_count;
  EditorRow *rows;

  int cursor_x;
  int cursor_y;
  int width;
  int height;
  WINDOW *window;
};

EditorState editor_init() {

  struct _editor_state *editor_state;
  editor_state = malloc(sizeof(*editor_state));
  editor_state->row_count = 0;
  editor_state->cursor_x = 0;
  editor_state->cursor_y = 0;

  editor_state->window = newwin(30, 80, 10, 10);
  notimeout(editor_state->window, FALSE); // ESC handling

  return editor_state;
};

void editor_destroy(EditorState editor_state) { // Todo free EditorRow s
  free(editor_state);
}

int editor_update(EditorState editor_state, int ch) {
  // update
  switch (ch) {
  case ctrl('x'):
  case 27:
    return MODE_SHEET;
  }
  return MODE_EDITOR;
}

// Draw
void editor_draw(EditorState editor_state) {
  curs_set(1);
  wattron(editor_state->window, A_REVERSE);
  box(editor_state->window, 0, 0);
  wattroff(editor_state->window, A_REVERSE);
  wmove(editor_state->window, 10, 10);
  waddch(editor_state->window, 'V');

  refresh();
  doupdate();
  wrefresh(editor_state->window);
}
