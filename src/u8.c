#include "./u8.h"

static uint8_t const _UTF8_LENGTH[16] = {
    // maps the leading 4 bits to length
    // 0 1 2 3  4 5 6 7  ascii
    1, 1, 1, 1, 1, 1, 1, 1,
    // 8 9 a b           invalid
    0, 0, 0, 0,
    // c d e f           starter
    2, 2, 3, 4};

uint8_t u8_length(char c) { return _UTF8_LENGTH[c >> 4]; }
