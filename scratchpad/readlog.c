#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
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

  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Assumptions:
//  1. Typing:
//   * We read a full keystroke within one read call.
//   * If someone types fast, we maybe see two or more full keystrokes in one
//   read call.
//  2. Pasting/Piping:
//   * We assume, that we do not paste ESC sequenzes. (this might be wrong)
//   * We assume, that we paste valid utf-8  (this might be wrong ->
//   validation)
//
//  3. We assume to receive the following ESC Sequenze to read as keystroke:
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
#define KEY_CHAR 1 << 4
#define KEY_CTRL 2 << 4
#define KEY_ESC 3 << 4
// usw.
//
//

// Internal Match
#define IS_ASCII(b) (((b)&0x80) == 0)
#define IS_ASCII_LETTER(b) (b > 31 && b < 127)
#define IS_CTRL_KEY(b) (b > 0 && b < 27)
#define IS_ESC(b) (b == 27)
#define IS_DEL(b) (b == 127)
#define IS_RET(b) (b == 13)
#define IS_TAB(b) (b == 9)

static uint8_t const _UTF8_LENGTH[16] =
    {
	// maps the leading 4 bits to length
        // 0 1 2 3  4 5 6 7  ascii
        1, 1, 1, 1, 1, 1, 1, 1,  
	// 8 9 a b           invalid 
	0, 0, 0, 0, 
	// c d e f           starter
	2, 2, 3, 4};

#define UTF8_LENGTH(starter) _UTF8_LENGTH[starter>>4]
#define IS_UTF8_START(b) (((b)&0xc0) == 0xc0)
#define IS_UTF8_PART(b) ((b & (1 << 7)) && !(b & (1 << 6)))


#define BUFFER_SIZE 32 //
#define TRAILING_SIZE 8 // There is a bit of wiggle room for UTF-8
int main() {

  enable_raw_mode();

  const int buffer_size = 0;
  unsigned char buffer[BUFFER_SIZE];
  ssize_t len = 0;

  unsigned char trailing[TRAILING_SIZE];
  ssize_t trailing_len = 0;

  unsigned char work[BUFFER_SIZE + TRAILING_SIZE];
  ssize_t work_len = 0;

  do {
    len = read(STDIN_FILENO, &buffer, sizeof(buffer));

    if (false) {
      printf("Raw read buffer");
      printf("len: %ld ", len);
      for (int i = 0; i < len; i++) {
        printf("%x ", buffer[i]);
      }
      for (int i = 0; i < len; i++) {
        printf("%08b ", buffer[i]);
      }
      printf("\r\n");
    }

    if (work[0] == 'q')
      break;

    work_len = trailing_len + len;

    // 1. add trailing chars from last call
    memcpy(&work, &trailing, trailing_len);

    // 2. copy receive chars from this call
    memcpy(&work[trailing_len], &buffer, len);

    // 3. search for trailing UTF-8 bytes
    int n = 0;
    for (; n < TRAILING_SIZE; n++) {
      // TODO: real validation if this is a trailing char
      // This is just to support pasting in long strings in.
      unsigned char c = work[work_len - 1 - n];
      if (IS_ASCII(c)) {
        break;
      }
      if (IS_UTF8_START(c)) {
	if(UTF8_LENGTH(c) == n + 1) {
	  // last char is complete utf8
	  n=0;
	  break;
	}else if(UTF8_LENGTH(c) > n+1){
	  // trailing utf8 bytes
	  n++; // add the starter
	  break;
	}
      }
    }
    // 4. "remove" trailing UTF-8 bytes from end of buffer
    work_len = trailing_len + len - n;

    // 5. copy trailing UTF-8 bytes from end of buffer
    trailing_len = n;
    memcpy(&trailing, &work[work_len], trailing_len);

    // 6. Iterate of work buffer and create tokens
    int i = 0;
    while (i < work_len) {

      // parse ESC Sequenze
      unsigned char first = work[i];
      if (IS_ESC(first)) {
	printf("ESC branch \r\n");
        if (i + 1 == work_len || i + 2 == work_len) {
          // Simple ESC near end of buffer
          // We assume non trailing in escape sequences
          printf("ESC\r\n");
          i++;
          continue;
        } else if (i + 2 < work_len) {
          unsigned char second = work[i + 1];
          unsigned char third = work[i + 2];
          // May be an ESC sequence
          if (work[i + 1] == '[') {
            switch (third) {
            case 'A':
            case 'B':
            case 'C':
            case 'D':
              printf("Arrow Key %c\r\n", third);
              i += 3;
              continue;
            }
          } else if (work[i + 1] == 'O') {
          }
        }
      }

      // parse UTF-8
      if (IS_ASCII(first)) {
        if (IS_ASCII_LETTER(first)) {
          printf("Letter '%c'\r\n", first);
        } else if (IS_RET(first)) {
          printf("Return\r\n");
        } else if (IS_DEL(first)) {
          printf("Backspace\r\n");
        } else if (IS_TAB(first)) {
          printf("Tab\r\n");
        } else if (IS_CTRL_KEY(first)) {
          printf("Ctrl + [ ]\r\n");
        } else {
          printf("ASCII FS,GS,RS,US\r\n");
        }
        i++;
        continue;
      }
      // if (IS_CTRL_KEY(c))
      //   printf("simple char %c", )

      if (IS_UTF8_START(first)) {
        // TODO
	char utf8_out[5];
	int utf8_len = UTF8_LENGTH(first);
	if(utf8_len < 1){
	  printf("utf8_len %d parsing error :( \r\n", utf8_len);
	  exit(1);
	}
	memcpy(&utf8_out, &work[i], utf8_len);
	utf8_out[utf8_len]= '\0';
        printf("Letter '%s'\r\n", utf8_out);
        i+=utf8_len;
        continue;
      }
      if (IS_UTF8_PART(first)) {
        printf("UTF-8 part, parsing error :( \r\n");
        i++;
        continue;
      }

      printf(" ¯\\_(ツ)_/¯ %08d \r\n", work[i]);
    }
  } while (work[0] != 'q');

  return 0;
}
/* vim: set sw=2 : */
