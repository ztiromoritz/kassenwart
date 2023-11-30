#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

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
#include "src/u8.h"
#include "src/utils.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/*** data ***/
typedef struct erow {
  int size;
  char *chars;
  // wcwidth()
  // https://stackoverflow.com/questions/3634627/how-to-know-the-preferred-display-width-in-columns-of-unicode-characters
  // TODO: int display_width; //rendered size respecting unicode chars
  // T H I S   I S   T H E   N E X T   S T E P ! ! !

} erow;

/*** editor state ***/
struct editor_config {
  int cx;
  int cy;
  int screen_rows;
  int screen_cols;
  int num_rows;
  erow *row;
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

void abuf_append(struct abuf *ab, const char *s, int len, int offset) {
  int new_len = ab->len + len - offset;
  char *new = realloc(ab->b, new_len);
  if (new == NULL)
    return;
  // TODO offset
  memcpy(&new[ab->len], s + offset, len);
  ab->b = new;
  ab->len = new_len;
}

void abuf_free(struct abuf *ab) { free(ab->b); }
/*
TODO:

void next_char(char *text, int *i, int) {
  char c = text[*i];
  //u8_length(c)
}

*/

void append_row(struct abuf *ab, int row, int offset_cols) {
  int cols = 0;
  int i = 0;

  erow r = E.row[row];

  while (cols < E.screen_cols + offset_cols && i < r.size) {
    uint8_t len;
    uint8_t col_size;
    u8_next(&r.chars[i], &len, &col_size);
    i = i + len;
    cols = cols + col_size;
  }
  abuf_append(ab, r.chars, i, 0);
}

/*** update screen ***/
void editor_draw_rows(struct abuf *ab) {
  for (int y = 0; y < E.screen_rows; y++) {
    if (y >= E.num_rows) {
      abuf_append(ab, "~", 1, 0);
    } else {
      /*
      // TODO E.row.display_cols
      int len = MIN(E.row[y].size, E.screen_cols);
      abuf_append(ab, E.row[y].chars, len, 0);
      */
      // TODO: quick hack, re calculates the col_width
      // of a char again and again.
      append_row(ab, y, 0);
    }
    // clear line
    abuf_append(ab, "\x1b[K", 4, 0);
    if (y < E.screen_rows - 1) {
      abuf_append(ab, "\r\n", 2, 0);
    }
  }
}

void position_cursor(struct abuf *ab) {
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abuf_append(ab, buf, strlen(buf), 0);
}

void editor_refresh_screen() {
  struct abuf ab = ABUF_INIT;
  // hide cursor
  abuf_append(&ab, "\x1b[?25l", 6, 0);

  abuf_append(&ab, "\x1b[H", 3, 0);

  editor_draw_rows(&ab);

  position_cursor(&ab);

  // show cursor again
  abuf_append(&ab, "\x1b[?25h", 6, 0);

  write(STDOUT_FILENO, ab.b, ab.len);

  abuf_free(&ab);
}

/*** file i/o ***/

void editor_append_row(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.num_rows + 1));

  int at = E.num_rows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  E.row[at].chars[len] = '\0';
  memcpy(E.row[at].chars, s, len);
  E.num_rows++;
}

void editor_open(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    die("fopen");
  }
  char *line = NULL;
  size_t line_cap = 0;
  ssize_t line_len;
  while ((line_len = getline(&line, &line_cap, fp)) != -1) {
    while (line_len > 0 &&
           (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
      line_len--;
    }
    editor_append_row(line, line_len);
  }
  free(line);
  fclose(fp);
}

/*** init ***/

void init_editor() {
  // init cursor position
  E.cx = 0;
  E.cy = 0;
  E.num_rows = 0;
  E.row = NULL;

  if (get_window_size(&E.screen_rows, &E.screen_cols) == -1)
    die("get_window_size");
}

int main(int argc, char *argv[]) {

  enable_raw_mode();
  init_editor();
  if (argc >= 2) {
    editor_open(argv[1]);
  }

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
