#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./input.h"
#include "./trie.h"
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

  // Match for ESC sequences
  Trie esc_trie;

  // key events buffer
  int current_event;
  int event_buffer_len;
  struct _key_event *events;
  // struct _key_events events;
};

int _write_event( //
    InputHandler handler,
    unsigned char type, //
    char *name,         //
    uint8_t len,        //
    unsigned char *data, int work_offset) {
  int i = handler->event_buffer_len;
  KeyEvent event = &(handler->events[i]);
  handler->event_buffer_len++;

  event->type = type;
  event->name = name;
  event->len = len;
  event->raw = malloc(sizeof(unsigned char) * len);
  memcpy(event->raw, data, len);
  return work_offset + len;
}

void _free_key_events(InputHandler handler) {
  for (int i = 0; i < handler->event_buffer_len; i++) {
    free(handler->events[i].raw);
  }
  handler->event_buffer_len = 0;
}

struct _trie_entry {
  char *pattern;
  int type;
  char *name;
};

typedef struct _trie_entry _trie_entry;

static const _trie_entry esc_entries[] = {
    {"[A", KEY_ARROW_UP, "Arrow Up new"},
    {"[B", KEY_ARROW_DOWN, "Arrow Down new"},
    {"[C", KEY_ARROW_RIGHT, "Arrow Right new"},
    {"[D", KEY_ARROW_LEFT, "Arrow Left new"}};

void fill_esc_trie(Trie trie) {
  // static struct _trie_value x = {"sdf", , "Hello"};
  int array_len = sizeof(esc_entries) / sizeof(_trie_entry);
  for (int i = 0; i < array_len; i++) {
    const _trie_entry *esc_entry = &esc_entries[i];
    //printf("pattern %s \r\n", esc_entry->pattern);
    trie_add_entry(trie, esc_entry->pattern, (void *)esc_entry);
  }
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

  handler->esc_trie = trie_init();
  fill_esc_trie(handler->esc_trie);

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
  free(handler->esc_trie);
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
      int esc_len = h->work_len - i;

      // use the trie
      if (esc_len > 1) {

        void *result;
        int match_len; 
	if(trie_query_prefix(h->esc_trie, &work[i+1], &match_len, &result)){
	   _trie_entry* entry = (_trie_entry *)(result);
	   i = _write_event(h, entry->type, entry->name, match_len, &work[i], i); 
	   continue;
	}
      }

      // legacy methode
      if (esc_len > 2) {
        unsigned char second = work[i + 1];
        unsigned char third = work[i + 2];
        // May be an ESC sequence
        if (second == '[') {
          switch (third) {
          case 'A':
            i = _write_event(h, KEY_ARROW_UP, "Arrow Up", 3, &work[i], i);
            continue;
          case 'B':
            i = _write_event(h, KEY_ARROW_DOWN, "Arrow Down", 3, &work[i], i);
            continue;
          case 'C':
            i = _write_event(h, KEY_ARROW_RIGHT, "Arrow Right", 3, &work[i], i);
            continue;
          case 'D':
            i = _write_event(h, KEY_ARROW_LEFT, "Arrow Left", 3, &work[i], i);
            continue;
          }
          if (third >= '0' && third <= '9' && esc_len >= 4) {
            unsigned char fourth = work[i + 3];
            if (fourth == '~') {
              switch (third) {
              case '1':
              case '7':
                i = _write_event(h, KEY_HOME, "Home", 4, &work[i], i);
                continue;
              case '4':
              case '8':
                i = _write_event(h, KEY_END, "End", 4, &work[i], i);
                continue;
              case '5':
                i = _write_event(h, KEY_PAGE_UP, "PgUp", 4, &work[i], i);
                continue;
              case '6':
                i = _write_event(h, KEY_PAGE_DOWN, "PgDown", 4, &work[i], i);
                continue;
              case '3':
                i = _write_event(h, KEY_DELETE, "Delete", 4, &work[i], i);
                continue;
              }
            }
          }
        } else if (work[i + 1] == 'O') {
          i = _write_event(h, KEY_TODO, "<esc>O sequence", 2, &work[i], i);
          continue;
        }
      }
      // No known ESC Sequence or single ESC
      i = _write_event(h, KEY_ESC, "ESC", 1, &work[i], i);
      continue;
    }

    // parse UTF-8
    if (IS_ASCII(first)) {
      if (IS_ASCII_LETTER(first)) {
        i = _write_event(h, KEY_CHAR, "Letter", 1, &work[i], i);
      } else if (IS_RET(first)) {
        i = _write_event(h, KEY_RETURN, "Return", 1, &work[i], i);
      } else if (IS_DEL(first)) {
        i = _write_event(h, KEY_BACKSPACE, "Backspace", 1, &work[i], i);
      } else if (IS_TAB(first)) {
        i = _write_event(h, KEY_TAB, "Tab", 1, &work[i], i);
      } else if (IS_CTRL_KEY(first)) {
        i = _write_event(h, KEY_CTRL_FROM_RAW(first), "CTRL+<>", 1, &work[i],
                         i);
      } else {
        i = _write_event(h, KEY_TODO, "ASCII FS,GS,RS,US", 1, &work[i], i);
      }
    }

    if (IS_UTF8_START(first)) {
      int utf8_len = UTF8_LENGTH(first);
      i = _write_event(h, KEY_CHAR, "Letter", utf8_len, &work[i], i);
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
