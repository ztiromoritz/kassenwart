#ifndef U8_U_
#define U8_U_ 
#include <stdint.h>

#define IS_UTF8_START(b) (((b)&0xc0) == 0xc0)

#define IS_UTF8_PART(b) ((b & (1 << 7)) && !(b & (1 << 6)))

uint8_t u8_length(char c);

#endif
