#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <stdint.h>

#define K 2
#define N 256
#define Q 3329
#define eta1 3
#define eta2 2
// #define d 12

// 256 için 8 bit aldım. ??
typedef struct
{
    int16_t coefficients[N];
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

void ntt_inverse(polynom *f_hat);

void multiply_ntts(polynom *f_hat, polynom *g_hat, polynom *h_hat);

void base_case_multiply(int16_t a0, int16_t a1, int16_t b0, int16_t b1, int16_t *c0, int16_t *c1, int16_t gamma);

void keygen(uint8_t d[32], uint8_t encryption_key[K * 384 + 32], uint8_t decryption_key[K * 384]);

void prf(uint8_t sigma[32], uint8_t b, int eta, uint8_t *out);

void print_hex(uint8_t *in, size_t len);

void print_A_matrix(const polynom_matrix *A_hat);

void encrypt(uint8_t encryption_key[K * 384 + 32], uint8_t m[32], uint8_t r[32]); // m: message, r: randomness

void compress(void);

void decompress(void);

#endif // ALGORITHMS_H