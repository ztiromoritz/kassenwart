#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "src/raw.h"
#include "src/utils.h"
#include "src/input.h"



// high level key press handling
void editor_process_keypress() {


}

int main() {

  enable_raw_mode();

  InputHandler input_handler = init_input_handler();
  KeyEvents key_events = get_key_events(input_handler);
  while (1) {
    update_key_events(input_handler);
    for (int i = 0; i < key_events->len; i++) {
      KeyEvent e = &(key_events->events[i]);
      if(e->type == KEY_CTRL('q'))
	exit(0);
      printf("%s type: %d\r\n", e->name, e->type);
    }
  }
  free_input_handler(input_handler); // onexit

  return 0;
}
/* vim: set sw=2 : */
