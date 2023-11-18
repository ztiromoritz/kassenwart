#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
void die(const char *s) {
  perror(s); 
  exit(1);
}

