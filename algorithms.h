#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <stdint.h>

#define K 2
#define N 256
#define Q 3329

// 256 için 8 bit aldım.
typedef struct {
    int8_t coefficients[N];
} polynom;

typedef struct {
    polynom vector[K];
} polynom_vector;

typedef struct {
    polynom matrix[K][K];
} polynom_matrix;

void bits_to_bytes(const uint8_t *b, size_t bit_len, uint8_t *B);

void bytes_to_bits(const uint8_t *B, size_t byte_len, uint8_t *b);










    
#endif // ALGORITHMS_H