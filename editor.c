#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>>

#include "src/input.h"
#include "src/raw.h"
#include "src/utils.h"

struct editor_config {
  int screen_rows;
  int screen_cols;
};

struct editor_config E;

// high level key press handling
void editor_process_keypress(KeyEvent e) {
  if (e->type == KEY_CTRL('q')) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
  }
  printf("Key event %s\r\n", e->name);
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void editor_draw_rows() {
  for (int y = 0; y < E.screen_rows; y++) {
    write(STDIN_FILENO, "~\r\n", 3);
  }
}

void editor_refresh_screen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editor_draw_rows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void init_editor() {
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
