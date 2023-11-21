#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/input.h"
#include "src/raw.h"

int main() {
  char buffer[10];
  enable_raw_mode();
  InputHandler input_handler = init_input_handler();
  while (1) {
    KeyEvent e = next_key_event(input_handler);
    memcpy(&buffer, e->raw, e->len);
    buffer[e->len] = '\0';
    if (e->type == KEY_CHAR)
      printf("%s (type: %d) \t\t'%s'\r\n", e->name, e->type, buffer);
    else
      printf("%s (type: %d) \t\t '%08b'\r\n", e->name, e->type, buffer);
    if (e->type == KEY_CTRL('c'))
      return 0;
  }

  free_input_handler(input_handler); // onexit
  return 0;
}
