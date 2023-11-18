#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <utils.h>

struct termios orig_termios;


// high level key press handling
void editor_process_keypress() {


}

int main() {

  enable_raw_mode();

  unsigned char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    printf("%4d 0x%2x %08b ('%c')\r\n", c, c, c, c);
  }
  while (1) {
    unsigned char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");
    if (iscntrl(c)) {
      printf("%4d 0x%2x %08b \r\n", c, c, c);
    } else {
      printf("%4d 0x%2x %08b ('%c')\r\n", c, c, c, c);
    }
    if (c == 'q')
      break;
  }

  return 0;
}
/* vim: set sw=2 : */
