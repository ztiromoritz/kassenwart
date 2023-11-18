#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disable_raw_mode() { //
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
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
