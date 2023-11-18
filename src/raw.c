#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "utils.h"
#include "raw.h"

/**
 * Raw input and output
 * https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html
 **/
struct termios orig_termios;

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
