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
#include "./u8.h"

// Internal Match
#define IS_ASCII(b) (((b)&0x80) == 0)
#define IS_ASCII_LETTER(b) (b > 31 && b < 127)
#define IS_CTRL_KEY(b) (b > 0 && b < 27)
#define IS_ESC(b) (b == 27)
#define IS_DEL(b) (b == 127)
#define IS_RET(b) (b == 13)
#define IS_TAB(b) (b == 9)

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
    {"[A", KEY_ARROW_UP, "Arrow Up"},
    {"[B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1;2A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;2B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;2C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;2D", KEY_ARROW_LEFT, "Arrow Left"},

    // I'm so close to implement my own regexp :D
    // Alt+Arrow
    {"[1;3A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;3B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;3C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;3D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1;4A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;4B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;4C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;4D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1;5A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;5B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;5C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;5D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1;6A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;6B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;6C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;6D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1;2F", KEY_END, "End"},   // +Ctrl
    {"[1;3F", KEY_END, "End"},   // +Ctrl
    {"[1;4F", KEY_END, "End"},   // +Ctrl
    {"[1;5F", KEY_END, "End"},   // +Ctrl
    {"[1;6F", KEY_END, "End"},   // +Ctrl
                                 //
    {"[1;2H", KEY_HOME, "Home"}, // +Ctrl
    {"[1;3H", KEY_HOME, "Home"}, // +Ctrl
    {"[1;4H", KEY_HOME, "Home"}, // +Ctrl
    {"[1;5H", KEY_HOME, "Home"}, // +Ctrl
    {"[1;6H", KEY_HOME, "Home"}, // +Ctrl

    // Str+Alt+Arrow
    {"[1;7A", KEY_ARROW_UP, "Arrow Up"},
    {"[1;7B", KEY_ARROW_DOWN, "Arrow Down"},
    {"[1;7C", KEY_ARROW_RIGHT, "Arrow Right"},
    {"[1;7D", KEY_ARROW_LEFT, "Arrow Left"},

    {"[1~", KEY_HOME, "Home"},

    {"[2~", KEY_INSERT, "Insert"},
    {"[2;2~", KEY_INSERT, "Insert"},
    {"[2;3~", KEY_INSERT, "Insert"},
    {"[2;4~", KEY_INSERT, "Insert"},
    {"[2;5~", KEY_INSERT, "Insert"},
    {"[2;6~", KEY_INSERT, "Insert"},

    {"[3~", KEY_DELETE, "Delete"},
    {"[3;2~", KEY_DELETE, "Delete"}, // + Ctrl
    {"[3;3~", KEY_DELETE, "Delete"},
    {"[3;4~", KEY_DELETE, "Delete"},
    {"[3;5~", KEY_DELETE, "Delete"},
    {"[3;6~", KEY_DELETE, "Delete"},

    {"[4~", KEY_END, "End"},
    {"[5~", KEY_PAGE_UP, "PgUp"},
    {"[6~", KEY_PAGE_DOWN, "PgDown"},
    {"[7~", KEY_HOME, "Home"}, // home again
    {"[8~", KEY_END, "End"},   // End again
                               // [9~ ??
    {"[Z", KEY_SHIFT_TAB, "Shift Tab"},

    {"OP", KEY_F1, "F1"},
    {"OQ", KEY_F2, "F2"},
    {"OR", KEY_F3, "F3"},
    {"OS", KEY_F4, "F4"},

    {"[15~", KEY_F5, "F5"},
    {"[17~", KEY_F6, "F6"},
    {"[18~", KEY_F7, "F7"},
    {"[19~", KEY_F8, "F8"},
    {"[20~", KEY_F9, "F9"},
    {"[21~", KEY_F10, "F10"},
    {"[24~", KEY_F12, "F12"},
};

void fill_esc_trie(Trie trie) {
  int array_len = sizeof(esc_entries) / sizeof(_trie_entry);
  for (int i = 0; i < array_len; i++) {
    const _trie_entry *esc_entry = &esc_entries[i];
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
      if (u8_length(c) == n + 1) {
        // last char is complete utf8
        n = 0;
        break;
      } else if (u8_length(c) > n + 1) {
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
    int suffix_len = h->work_len - i;

    if (IS_ESC(first)) {
      if (suffix_len > 1) {
        void *result;
        int match_len;
        if (trie_query_prefix(h->esc_trie, &work[i + 1], &match_len, &result)) {
          _trie_entry *e = (_trie_entry *)(result);
          i = _write_event(h, e->type, e->name, match_len + 1, &work[i], i);
          continue;
        }
      }
      // No known ESC Sequence or single ESC
      i = _write_event(h, KEY_ESC, "ESC", 1, &work[i], i);
      continue;
    }

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
      int utf8_len = u8_length(first);
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
