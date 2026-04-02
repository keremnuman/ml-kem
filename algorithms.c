#include <stdio.h>
#include <stdlib.h>

#include "algorithms.h"

void bits_to_bytes(const uint8_t *b, size_t bit_len, uint8_t *B)
{
    size_t byte_len = bit_len / 8;
    memset(B, 0, byte_len);

    for (size_t i = 0; i < bit_len; i++) {
        /* i. bit → B[i/8] içindeki (i%8). pozisyon */
        B[i / 8] |= (uint8_t)(b[i] << (i % 8));
    }
}