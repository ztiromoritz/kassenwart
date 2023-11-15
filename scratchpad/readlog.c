#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

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

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(char byte)
{
    printf("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

int main() {

  enable_raw_mode();

  unsigned char buffer[4];
  unsigned char out[5];
  ssize_t len;


  do {
    len = read(STDIN_FILENO, &buffer, sizeof(buffer));
    
    memcpy(&out,&buffer, len);
    out[len]='\0';
     

    printf("len: %ld %s ", len, out);
    for(int i=0;i<len;i++){
      //print_byte(out[i]);
      printf("%x ", out[i]);
      

    }
    printf("\n");
   // printf("%4d 0x%2x %b ('%c')\n", c, c, c, c);

  } while (len > 0);
  return 0;
}
/* vim: set sw=2 : */
