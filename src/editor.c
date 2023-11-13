#include <ncurses.h>
#include <stdlib.h>

#include "editor.h"

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


  return editor_state;
};

void editor_destroy(EditorState editor_state) { // Todo free EditorRow s
  free(editor_state);
}

// Draw
void editor_draw(EditorState editor_state) {
  // Todo
  box(editor_state->window, 0, 0);
  wrefresh(editor_state->window);
}
