#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
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
#define KEY_CTRL_FROM_RAW(n) (n + 96) 
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

struct _key_events {
  int len;
  struct _key_event *events;
};

typedef struct _key_events *KeyEvents;

typedef struct _input_handler *InputHandler;

InputHandler init_input_handler();

struct _key_event {
  uint8_t type; // KEY_* 
  char *name; 
  uint8_t len;
  char *raw; 
};

typedef struct _key_event *KeyEvent;

KeyEvents get_key_events(InputHandler handler);

void update_key_events(InputHandler h);

void free_input_handler(InputHandler handler);

#endif
