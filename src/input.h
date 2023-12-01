#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#define KEY_TODO 0

#define KEY_CHAR 1
#define KEY_ESC 2
#define KEY_RETURN 3
#define KEY_TAB 4
#define KEY_BACKSPACE 5
#define KEY_SHIFT_TAB 6

#define KEY_ARROW_UP 10
#define KEY_ARROW_DOWN 11
#define KEY_ARROW_LEFT 12
#define KEY_ARROW_RIGHT 13
#define KEY_HOME 14
#define KEY_END 15
#define KEY_PAGE_DOWN 16
#define KEY_PAGE_UP 17
#define KEY_DELETE 18
#define KEY_INSERT 19

#define KEY_F1 21
#define KEY_F2 22
#define KEY_F3 23
#define KEY_F4 24
#define KEY_F5 25
#define KEY_F6 26
#define KEY_F7 27
#define KEY_F8 28
#define KEY_F9 29
#define KEY_F10 30
#define KEY_F11 31
#define KEY_F12 32

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


struct _key_event {
  uint8_t type; 
  char *name; 
  uint8_t len;
  char *raw; 
  uint8_t display_width;
};

typedef struct _key_event *KeyEvent;

typedef struct _input_handler *InputHandler;

InputHandler init_input_handler();

KeyEvent next_key_event(InputHandler);

void free_input_handler(InputHandler);

#endif
