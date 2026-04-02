#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <stdint.h>

#define K 2
#define N 256
#define Q 3329

// 256 için 16 bit aldım.
typedef struct {
    int16_t coefficients[N];
} polynom;

typedef struct {
    polynom vector[K];
} polynom_vector;

typedef struct {
    polynom matrix[K][K];
} polynom_matrix;

void bits_to_bytes(const uint8_t *b, size_t bit_len, uint8_t *B);










    
#endif // ALGORITHMS_H