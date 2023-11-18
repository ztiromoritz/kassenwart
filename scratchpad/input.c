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

#define KEY_TODO 0

#define KEY_CHAR 1
#define KEY_ESC 2
#define KEY_RETURN 3
#define KEY_TAB 4
#define KEY_BACKSPACE 5

#define KEY_ARROW_UP 10
#define KEY_ARROW_DOWN 11
#define KEY_ARROW_LEFT 12
#define KEY_ARROW_RIGHT 13

#define KEY_CTRL(c) ((unsigned int)c)
#define KEY_CTRL_FROM_RAW(n) (n + 97) // ??? TODO test
#define KEY_CTRL_A 97
#define KEY_CTRL_B 98
#define KEY_CTRL_C 99
#define KEY_CTRL_D 100
#define KEY_CTRL_E 101
#define KEY_CTRL_F 102
#define KEY_CTRL_G 103
#define KEY_CTRL_H 104
#define KEY_CTRL_I 105
#define KEY_CTRL_J 106
#define KEY_CTRL_K 107
#define KEY_CTRL_L 108
#define KEY_CTRL_M 109
#define KEY_CTRL_N 110
#define KEY_CTRL_O 111
#define KEY_CTRL_P 112
#define KEY_CTRL_Q 113
#define KEY_CTRL_R 114
#define KEY_CTRL_S 115
#define KEY_CTRL_T 116
#define KEY_CTRL_U 117
#define KEY_CTRL_V 118
#define KEY_CTRL_W 119
#define KEY_CTRL_X 120
#define KEY_CTRL_Y 121
#define KEY_CTRL_Z 122

// Internal Match
#define IS_ASCII(b) (((b)&0x80) == 0)
#define IS_ASCII_LETTER(b) (b > 31 && b < 127)
#define IS_CTRL_KEY(b) (b > 0 && b < 27)
#define IS_ESC(b) (b == 27)
#define IS_DEL(b) (b == 127)
#define IS_RET(b) (b == 13)
#define IS_TAB(b) (b == 9)

static uint8_t const _UTF8_LENGTH[16] = {
    // maps the leading 4 bits to length
    // 0 1 2 3  4 5 6 7  ascii
    1, 1, 1, 1, 1, 1, 1, 1,
    // 8 9 a b           invalid
    0, 0, 0, 0,
    // c d e f           starter
    2, 2, 3, 4};

#define UTF8_LENGTH(starter) _UTF8_LENGTH[starter >> 4]
#define IS_UTF8_START(b) (((b)&0xc0) == 0xc0)
#define IS_UTF8_PART(b) ((b & (1 << 7)) && !(b & (1 << 6)))

struct _key_event {
  uint8_t type;
  char *name; // Pointer to a constant name. Must not be freed.
  uint8_t len;
  char *raw; // The raw data of length len. Must be freed.
};

typedef struct _key_event *KeyEvent;

struct _key_events {
  int len;
  struct _key_event *events;
};
typedef struct _key_events *KeyEvents;

#define BUFFER_SIZE 32  //
#define TRAILING_SIZE 8 // There is a bit of wiggle room for UTF-8

struct _input_handler {
  unsigned char buffer[BUFFER_SIZE];
  ssize_t buffer_len;

  unsigned char trailing[TRAILING_SIZE];
  ssize_t trailing_len;

  unsigned char work[BUFFER_SIZE + TRAILING_SIZE];
  ssize_t work_len;

  struct _key_events events;
};

typedef struct _input_handler *InputHandler;

void _write_key_event( //
    InputHandler handler,
    unsigned char type, //
    char *name,         //
    uint8_t len,        //
    unsigned char *data) {
  int i = handler->events.len;
  KeyEvent event = &(handler->events.events[i]);
  event->type = type;
  event->name = name;
  event->len = len;
  event->raw = malloc(sizeof(unsigned char) * len);
  memcpy(event->raw, data, len);
  handler->events.len++;
}

void _free_key_events(InputHandler handler) {
  for (int i = 0; i < handler->events.len; i++) {
    free(handler->events.events[i].raw);
  }
  handler->events.len = 0;
}

InputHandler init_input_handler() {
  struct _input_handler *handler;
  handler = malloc(sizeof(*handler));
  handler->buffer_len = 0;
  handler->trailing_len = 0;
  handler->work_len = 0;

  handler->events.len = 0;
  handler->events.events = malloc(sizeof(struct _key_event)*BUFFER_SIZE); //

  return handler;
}

void free_input_handler(InputHandler handler) {
  _free_key_events(handler);
  free(handler->events.events);
  free(handler);
}

KeyEvents get_key_events(InputHandler handler) { return &(handler->events); }

void update_key_events(InputHandler h) {
  // Free possible events from last round
  _free_key_events(h);

  unsigned char *buffer = h->buffer;
  unsigned char *trailing = h->trailing;
  unsigned char *work = h->work;

  h->buffer_len =
      read(STDIN_FILENO, buffer, sizeof(unsigned char) * BUFFER_SIZE);

  h->work_len = h->trailing_len + h->buffer_len;

  printf("trailing len %ld\r\n", h->trailing_len);
  // 1. add trailing chars from last call
  memcpy(work, trailing, h->trailing_len);

  // 2. copy receive chars from this call
  memcpy(&(work[h->trailing_len]), buffer, h->buffer_len);

  char *debug = malloc(sizeof(unsigned char) * (h->work_len + 1));
  memcpy(debug, work, h->work_len);
  debug[h->work_len] = '\0';
  printf("work %ld %ld %s\r\n", h->work_len, h->trailing_len, debug);

  // 3. search for trailing UTF-8 bytes
  int n = 0;
  for (; n < TRAILING_SIZE; n++) {
    // TODO: real validation if this is a trailing char
    // This is just to support pasting in long strings in.
    unsigned char c = work[h->work_len - 1 - n];
    if (IS_ASCII(c)) {
      break;
    }
    if (IS_UTF8_START(c)) {
      if (UTF8_LENGTH(c) == n + 1) {
        // last char is complete utf8
        n = 0;
        break;
      } else if (UTF8_LENGTH(c) > n + 1) {
        // trailing utf8 bytes
        n++; // add the starter
        break;
      }
    }
  }
  // 4. "remove" trailing UTF-8 bytes from end of buffer
  h->work_len = h->trailing_len + h->buffer_len - n;

  // 5. copy trailing UTF-8 bytes from end of buffer
  h->trailing_len = n;
  printf("n %d\r\n", n);
  memcpy(trailing, &(work[h->work_len]), h->trailing_len);

  // 6. Iterate of work buffer and create tokens
  int i = 0;
  while (i < h->work_len) {

    // parse ESC Sequenze
    unsigned char first = work[i];
    if (IS_ESC(first)) {
      printf("ESC branch \r\n");
      if (i + 1 == h->work_len || i + 2 == h->work_len) {
        // Simple ESC near end of buffer
        // We assume non trailing in escape sequences
        _write_key_event(h, 0, "ESC", 1, &work[i]);
        i++;
        continue;
      } else if (i + 2 < h->work_len) {
        unsigned char second = work[i + 1];
        unsigned char third = work[i + 2];
        // May be an ESC sequence
        if (work[i + 1] == '[') {
          switch (third) {
          case 'A':
          case 'B':
          case 'C':
          case 'D':
            _write_key_event(h, KEY_TODO, "Arrow", 3, &work[i]);
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
        _write_key_event(h, KEY_CHAR, "Letter", 1, &work[i]);
      } else if (IS_RET(first)) {
        _write_key_event(h, KEY_RETURN, "Return", 1, &work[i]);
      } else if (IS_DEL(first)) {
        _write_key_event(h, KEY_BACKSPACE, "Backspace", 1, &work[i]);
      } else if (IS_TAB(first)) {
        _write_key_event(h, KEY_TAB, "Tab", 1, &work[i]);
      } else if (IS_CTRL_KEY(first)) {
        _write_key_event(h, KEY_CTRL_FROM_RAW(first), "CTRL+[]", 1, &work[i]);
      } else {
        _write_key_event(h, KEY_TODO, "ASCII FS,GS,RS,US", 1, &work[i]);
      }
      i++;
      continue;
    }

    if (IS_UTF8_START(first)) {
      int utf8_len = UTF8_LENGTH(first);
      _write_key_event(h, KEY_CHAR, "Letter", utf8_len, &work[i]);
      i += utf8_len;
      continue;
    }
    if (IS_UTF8_PART(first)) {
      // printf("UTF-8 part, parsing error :( \r\n");
      i++;
      continue;
    }
    // printf(" ¯\\_(ツ)_/¯ %08d \r\n", work[i]);
  }
}

int main() {
  enable_raw_mode();
  InputHandler input_handler = init_input_handler();
  KeyEvents key_events = get_key_events(input_handler);

  while (1) {
    //printf("while\r\n");
    update_key_events(input_handler);
    //printf("Len %d\r\n", key_events->len);
    for (int i = 0; i < key_events->len; i++) {
      KeyEvent e = &(key_events->events[i]);
      printf("%s type: %d\r\n", e->name, e->type);
      //printf("here\r\n");
    }
  }

  free_input_handler(input_handler); // onexit
  return 0;
}
/* vim: set sw=2  */
