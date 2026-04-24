#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <stdint.h>

#define K 2
#define N 256
#define Q 3329
#define eta1 3
#define eta2 2
// #define d 12

// 256 için 8 bit aldım.
typedef struct
{
    int8_t coefficients[N];
} polynom;

typedef struct
{
    polynom vector[K];
} polynom_vector;

typedef struct
{
    polynom matrix[K][K];
} polynom_matrix;

void bits_to_bytes(const uint8_t *b, size_t bit_len, uint8_t *B);

void bytes_to_bits(const uint8_t *B, size_t byte_len, uint8_t *b);

void byte_encode(const polynom *f, int d, uint8_t *out);

void byte_decode(const uint8_t *in, int d, polynom *f);

void sample_ntt(const uint8_t rho[32], uint8_t i, uint8_t j, polynom *a_hat);

void sample_poly_cbd(const uint8_t *b, int eta, polynom *f);

void ntt(polynom *f);

#endif // ALGORITHMS_H