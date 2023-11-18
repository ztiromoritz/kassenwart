#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "src/input.h"
#include "src/raw.h"
#include "src/utils.h"

// high level key press handling
void editor_process_keypress(KeyEvent e) {
  if (e->type == KEY_CTRL('q')) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
  }
  printf("Key event %s\r\n", e->name);
}

void edior_draw_rows() {
  for (int y = 0; y < 24; y++) {
    write(STDIN_FILENO, "~\r\n", 3);
  }
}

void editor_refresh_screen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  edior_draw_rows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

int main() {

  enable_raw_mode();

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
