#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "src/input.h"
#include "src/raw.h"
#include "src/utils.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/*** editor state ***/
struct editor_config {
  int cx;
  int cy;
  int screen_rows;
  int screen_cols;
};

struct editor_config E;

/*** high level key press handling ***/
void editor_process_keypress(KeyEvent e) {
  switch (e->type) {
  case KEY_CTRL('q'):
    // clear screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
  case KEY_ARROW_UP:
    E.cy = MAX(E.cy - 1, 0);
    break;
  case KEY_ARROW_DOWN:
    E.cy = MIN(E.cy + 1, E.screen_rows - 1);
    break;
  case KEY_ARROW_LEFT:
    E.cx = MAX(E.cx - 1, 0);
    break;
  case KEY_ARROW_RIGHT:
    E.cx = MIN(E.cx + 1, E.screen_cols - 1);
    break;
  }
}

/*** screen ***/
int get_cursor_position(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    return -1;
  }

  while (i < sizeof(buf) - 1) {
    while (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;

  return 0;
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return get_cursor_position(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** append buffer ***/
struct abuf {
  char *b;
  int len;
};
#define ABUF_INIT                                                              \
  { NULL, 0 }
void abuf_append(struct abuf *ab, const char *s, int len) {
  int new_len = ab->len + len;
  char *new = realloc(ab->b, new_len);
  if (new == NULL)
    return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len = new_len;
}

void abuf_free(struct abuf *ab) { free(ab->b); }

/*** update screen ***/
void editor_draw_rows(struct abuf *ab) {
  for (int y = 0; y < E.screen_rows; y++) {
    abuf_append(ab, "~", 1);
    // clear line
    abuf_append(ab, "\x1b[K", 4);
    if (y < E.screen_rows - 1) {
      abuf_append(ab, "\r\n", 2);
    }
  }
}

void position_cursor(struct abuf *ab) {
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abuf_append(ab, buf, strlen(buf));
}

void editor_refresh_screen() {
  struct abuf ab = ABUF_INIT;
  // hide cursor
  abuf_append(&ab, "\x1b[?25l", 6);

  abuf_append(&ab, "\x1b[H", 3);

  editor_draw_rows(&ab);

  position_cursor(&ab);

  // show cursor again
  abuf_append(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);

  abuf_free(&ab);
}

void init_editor() {
  // init cursor position
  E.cx = 0;
  E.cy = 0;

  if (get_window_size(&E.screen_rows, &E.screen_cols) == -1)
    die("get_window_size");
}

int main() {

  enable_raw_mode();
  init_editor();

  InputHandler input_handler = init_input_handler();
  do {
    editor_refresh_screen();
    KeyEvent e = next_key_event(input_handler);
    editor_process_keypress(e);
  } while (1);

  free_input_handler(input_handler); // onexit

  return 0;
}
/* vim: set sw=2 : */
