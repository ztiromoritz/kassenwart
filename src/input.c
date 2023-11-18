#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./input.h"
#include "./utils.h"

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

#define BUFFER_SIZE 32  //
#define TRAILING_SIZE 8 // There is a bit of wiggle room for UTF-8

struct _key_events {
  int current;
  int len;
  struct _key_event *events;
};

struct _input_handler {
  unsigned char buffer[BUFFER_SIZE];
  ssize_t buffer_len;

  unsigned char trailing[TRAILING_SIZE];
  ssize_t trailing_len;

  unsigned char work[BUFFER_SIZE + TRAILING_SIZE];
  ssize_t work_len;

  // key events buffer
  int current_event;
  int event_buffer_len;
  struct _key_event *events;
  // struct _key_events events;
};

void _write_key_event( //
    InputHandler handler,
    unsigned char type, //
    char *name,         //
    uint8_t len,        //
    unsigned char *data) {
  int i = handler->event_buffer_len;
  KeyEvent event = &(handler->events[i]);
  handler->event_buffer_len++;

  event->type = type;
  event->name = name;
  event->len = len;
  event->raw = malloc(sizeof(unsigned char) * len);
  memcpy(event->raw, data, len);

}

void _free_key_events(InputHandler handler) {
  for (int i = 0; i < handler->event_buffer_len; i++) {
    free(handler->events[i].raw);
  }
  handler->event_buffer_len = 0;
}

InputHandler init_input_handler() {
  struct _input_handler *handler;
  handler = malloc(sizeof(*handler));
  handler->buffer_len = 0;
  handler->trailing_len = 0;
  handler->work_len = 0;

  handler->current_event = 0;
  handler->event_buffer_len = 0;
  handler->events = malloc(sizeof(struct _key_event) * BUFFER_SIZE); //

  return handler;
}

void _update_key_events(InputHandler);

KeyEvent next_key_event(InputHandler handler) {
  if (handler->current_event >= handler->event_buffer_len) {
    _update_key_events(handler);
    handler->current_event = 0;
  }
  return &(handler->events[handler->current_event++]);
}

void free_input_handler(InputHandler handler) {
  _free_key_events(handler);
  free(handler->events);
  free(handler);
}

void _update_key_events(InputHandler h) {
  // Free possible events from last round
  _free_key_events(h);

  unsigned char *buffer = h->buffer;
  unsigned char *trailing = h->trailing;
  unsigned char *work = h->work;

  ssize_t r = read(STDIN_FILENO, buffer, sizeof(unsigned char) * BUFFER_SIZE);
  if (r == -1 && errno != EAGAIN)
    die("input handler: read");

  h->buffer_len = r;
  h->work_len = h->trailing_len + h->buffer_len;

  // 1. add trailing chars from last call
  memcpy(work, trailing, h->trailing_len);

  // 2. copy receive chars from this call
  memcpy(&(work[h->trailing_len]), buffer, h->buffer_len);

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
  memcpy(trailing, &(work[h->work_len]), h->trailing_len);

  // 6. Iterate of work buffer and create KeyEvents
  int i = 0;
  while (i < h->work_len) {
    unsigned char first = work[i];

    //
    // parse ESC Sequenze
    //
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
        if (second == '[') {
          switch (third) {
          case 'A':
            _write_key_event(h, KEY_ARROW_UP, "Arrow Up", 3, &work[i]);
            i += 3;
            continue;
          case 'B':
            _write_key_event(h, KEY_ARROW_DOWN, "Arrow Down", 3, &work[i]);
            i += 3;
            continue;
          case 'C':
            _write_key_event(h, KEY_ARROW_RIGHT, "Arrow Right", 3, &work[i]);
            i += 3;
            continue;
          case 'D':
            _write_key_event(h, KEY_ARROW_LEFT, "Arrow Left", 3, &work[i]);
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
        _write_key_event(h, KEY_CTRL_FROM_RAW(first), "CTRL+<>", 1, &work[i]);
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
  }
}

int example_main() {
  InputHandler input_handler = init_input_handler();

  while (1) {
    KeyEvent e = next_key_event(input_handler);
  }

  free_input_handler(input_handler); // onexit
  return 0;
}
/* vim: set sw=2  */
