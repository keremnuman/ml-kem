#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <stdint.h>

#define K 2
#define N 256
#define Q 3329
#define eta1 3
#define eta2 2
#define du 10
#define dv 4
// #define d 12

static const int16_t zetas[128] = {
    1, 1729, 2580, 3289, 2642, 630, 1897, 848,
    1062, 1919, 193, 797, 2786, 3260, 569, 1746,
    296, 2447, 1339, 1476, 3046, 56, 2240, 1333,
    1426, 2094, 535, 2882, 2393, 2879, 1974, 821,
    289, 331, 3253, 1756, 1197, 2304, 2277, 2055,
    650, 1977, 2513, 632, 2865, 33, 1320, 1915,
    2319, 1435, 807, 452, 1438, 2868, 1534, 2402,
    2647, 2617, 1481, 648, 2474, 3110, 1227, 910,
    17, 2761, 583, 2649, 1637, 723, 2288, 1100,
    1409, 2662, 3281, 233, 756, 2156, 3015, 3050,
    1703, 1651, 2789, 1789, 1847, 952, 1461, 2687,
    939, 2308, 2437, 2388, 733, 2337, 268, 641,
    1584, 2298, 2037, 3220, 375, 2549, 2090, 1645,
    1063, 319, 2773, 757, 2099, 561, 2466, 2594,
    2804, 1092, 403, 1026, 1143, 2150, 2775, 886,
    1722, 1212, 1874, 1029, 2110, 2935, 885, 2154};

static const int16_t gammas[128] = {
    17, 3312, 2761, 568, 583, 2746, 2649, 680,
    1637, 1692, 723, 2606, 2288, 1041, 1100, 2229,
    1409, 1920, 2662, 667, 3281, 48, 233, 3096,
    756, 2573, 2156, 1173, 3015, 314, 3050, 279,
    1703, 1626, 1651, 1678, 2789, 540, 1789, 1540,
    1847, 1482, 952, 2377, 1461, 1868, 2687, 642,
    939, 2390, 2308, 1021, 2437, 892, 2388, 941,
    733, 2596, 2337, 992, 268, 3061, 641, 2688,
    1584, 1745, 2298, 1031, 2037, 1292, 3220, 109,
    375, 2954, 2549, 780, 2090, 1239, 1645, 1684,
    1063, 2266, 319, 3010, 2773, 556, 757, 2572,
    2099, 1230, 561, 2768, 2466, 863, 2594, 735,
    2804, 525, 1092, 2237, 403, 2926, 1026, 2303,
    1143, 2186, 2150, 1179, 2775, 554, 886, 2443,
    1722, 1607, 1212, 2117, 1874, 1455, 1029, 2300,
    2110, 1219, 2935, 394, 885, 2444, 2154, 1175};

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

void encrypt(uint8_t encryption_key[K * 384 + 32], uint8_t m[32], uint8_t r[32], uint8_t ciphertext[32 * (du * K + dv)]); // m: message, r: randomness

void compress(int16_t x, int d, uint16_t *compressed);

void decompress(uint16_t compressed, int d, int16_t *decompressed);

#endif // ALGORITHMS_H