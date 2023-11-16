#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disable_raw_mode() { //
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

#define BUFFER_SIZE 64

#define KEY_EVENT_STRING 1
#define KEY_EVENT_ARROW_UP 2 
// usw.

struct key_event {
  ssize_t len;
  char *raw;
  int type;
};

int main() {

  enable_raw_mode();

  const int buffer_size = 0;
  unsigned char buffer[BUFFER_SIZE];
  unsigned char out[BUFFER_SIZE + 1];
  ssize_t len;

  // Assumption:
  //  Typing:
  //   * We read a full keystroke within one read call.
  //   * If someone types fast, we maybe see two or more full keystrokes in one
  //   read call.
  //  Pasting/Piping:
  //   * We assume, that we do not paste ESC sequenzes. (this might be wrong)
  //   * We assume, that we paste valid utf-8  (this might be wrong ->
  //   validation)
  //
  //  We assume to receive the following ESC Sequenze to read as keystroke:
  //   * [CTRL]+[a-z] 		-> read as 0x01-0x1a
  //   	 * What about TAB vs CTRL+i? ...
  //   	 * What about RET cs CTRL+j?
  //   * Arrow Keys   		-> ^[A ^[B ^[C ^[D
  //   * Tab                    -> 0x09
  //   * Return 	        -> 0x0a
  //   * There seem to be no clear standard here:
  //   * [POS1,INS,DEL,END] 	-> ^[1~ ^[2~ ^[3~ ^[4~
  //   	 * Are there other ~ ESC sequenzes? ...
  //   * F1,F2,F3,F4            -> ^OP ^OQ ^OR ^OS
  //   * F5-F12			->
  //
  // possible problems:
  //  * the buffer might be to small
  //    * hitting ä and ö the same time could produce 4 bytes
  //    * hitting -> and ö the same time can produce 5 bytes.
  //       * With a limit of 4 i just saw the keystrokes split in two read calls
  //       * Is this specified?
  //    * Copy paste to stdin "⼕ä" = "E2 BC 95"+"C3 A4" (5 bytes)
  //       -> leads to two chunks "E2 B2 95 C3" and "A4"  :(
  //
  //    * Copy paste polar bear: "🐻‍❄️>" ... my current console can
  //    handle this right now :)
  //    * Copy paste pride flag "🏳️‍🌈" ... hm same.
  //
  //    * What about combining characters ...
  //
  // For "fast typing" I think we can work with a reasonable buffer size (64,
  // 128 .. etc.) but we need to keep trailing bytes for the next iteration, if
  // we want to support copy paste.
  //  * Trailing byte detection for utf-8 should be possible
  //  * Trailing byte detection for ESC-Sequenzes?
  //       * -> We assume that we do not paste ESC Sequenzes
  //  * How to detect end of ESC Sequenz?
  //
  do {
    len = read(STDIN_FILENO, &buffer, sizeof(buffer));

    // 1. add trailing chars from last call
    // TODO

    // 2. copy receive chars from this call
    memcpy(&out, &buffer, len);
    out[len] = '\0';

    // 3. remove trailing UTF-8 bytes from end of the buffer
    // TODO

    // 4. scan current buffer and produce list of events
    // TODO

    printf("len: %ld %s ", len, out);
    for (int i = 0; i < len; i++) {
      printf("%x ", out[i]);
    }
    for (int i = 0; i < len; i++) {
      printf("%08b ", out[i]);
    }
    printf("\n");

  } while (len > 0);
  return 0;
}
/* vim: set sw=2 : */
