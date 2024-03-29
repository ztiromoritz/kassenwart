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
  // The size in byte
  int size;
  // The chars string without '\0' termination
  char *chars;

  // A cache for the characters col size in monospace font
  int8_t *col_sizes_cache;

  // The col_sizes_cache should be keept in sync with the chars stream and thus
  // have the same size It should be inititalized with all values -1 not
  // Calculated
  //
  // Its should be filled like this:
  // Text:              a    ö         🐻
  // chars:             0x61 0xC3 0xB6 0xF0 0x9F 0x90 0xBB
  // col_sizes_cache:   1    1    -1   2    -1   -1   -1    // we could maybe
  // skip the -1 filler and do more index magic
  //
  // Note: its not the length of the encode char in bytes, this will be
  // calculated on the fly!

} erow;

/*** editor state ***/
struct editor_config {
  // Cursor position within the
  // visual rows/columns of the text data
  int cx;
  int cy;
  // Offset between the screen origin and
  // the origin of the text data
  int row_off;
  int col_off;

  // screen size
  int screen_rows;
  int screen_cols;

  // text data
  int file_rows;
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
    E.cy = MIN(E.cy + 1, E.file_rows - 1);
    break;
  case KEY_ARROW_LEFT:
    E.cx = MAX(E.cx - 1, 0);
    break;
  case KEY_ARROW_RIGHT:
    // E.cx = MIN(E.cx + 1, E.screen_cols - 1);
    E.cx++; // Boundary ??
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


void append_row(struct abuf *ab, int row) {
  int cols = 0;
  int len = 0;
  int char_offset = 0;
  int pad_left = 0;  
  int pad_right = 0; 
		     
  uint8_t letter_len;
  uint8_t letter_cols;

  erow r = E.row[row];

  while (cols < E.screen_cols + E.col_off && len < r.size) {
    // TODO: use cached values
    u8_next(&r.chars[len], &letter_len, &letter_cols);
    len = len + letter_len;
    if (cols < E.col_off) {
      // As long as we handle letters below the column offset
      // we update the char offset. 
      char_offset = len;
      if (cols + letter_cols > E.col_off) {
	// The current letter exceeds the offset
	// 
	// It starts offscreen 
	// and is only partially visible
	// So its visible cols will replaced
	// by a padding character.
	//
	// In reality this only handles 
	// the two coulmn double wide unicod letters.
	// So pad_left is either be 0 or 1
        pad_left = cols + letter_cols - E.col_off;
      }
    }
    cols = cols + letter_cols;
  }

  // Check how many cols of the last char are offscreen.
  uint8_t margin_right = cols - (E.screen_cols + E.col_off);
  if (margin_right > 0) {
    // Last character should not be printed
    len-=letter_len;
    // Calculate how wide the visible part of the letter is
    pad_right = letter_cols - margin_right;
  }

  for (int s = 0; s < pad_left; s++) {
    abuf_append(ab, "<", 1);
  }
  abuf_append(ab, &(r.chars[char_offset]), len - char_offset);
  for (int s = 0; s < pad_right; s++) {
    abuf_append(ab, "<", 1);
  }
}

/*** update screen ***/
void editor_draw_rows(struct abuf *ab) {
  int file_row;
  for (int y = 0; y < E.screen_rows; y++) {
    file_row = y + E.row_off;
    if (file_row >= E.file_rows) {
      abuf_append(ab, "~", 1);
    } else {
      append_row(ab, file_row);
    }
    // clear line
    abuf_append(ab, "\x1b[K", 4);
    if (y < E.screen_rows - 1) {
      abuf_append(ab, "\r\n", 2);
    }
  }
}

void editor_scroll() {
  if (E.cy < E.row_off) {
    E.row_off = E.cy;
  }
  if (E.cy >= E.row_off + E.screen_rows) {
    E.row_off = E.cy - E.screen_rows + 1;
  }
  if (E.cx < E.col_off) {
    E.col_off = E.cx;
  }
  if (E.cx >= E.col_off + E.screen_cols) {
    E.col_off = E.cx - E.screen_cols + 1;
  }
}

void position_cursor(struct abuf *ab) {
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.row_off + 1,
           E.cx - E.col_off + 1);
  abuf_append(ab, buf, strlen(buf));
}

void editor_refresh_screen() {
  struct abuf ab = ABUF_INIT;

  editor_scroll();
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

/*** file i/o ***/

void editor_append_row(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.file_rows + 1));

  int at = E.file_rows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  E.row[at].chars[len] = '\0';
  memcpy(E.row[at].chars, s, len);
  E.file_rows++;
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
  E.row_off = 0;
  E.col_off = 0;
  E.file_rows = 0;
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
