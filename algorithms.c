#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "algorithms.h"
#include "fips202.h"

// little-endian olacak.

// algo 3
void bits_to_bytes(const uint8_t *b, size_t bit_len, uint8_t *B)
{
    size_t byte_len = (bit_len + 7) / 8;
    memset(B, 0, byte_len);

    for (size_t i = 0; i < bit_len; i++)
    {
        // i. bit → B[i/8] içindeki (i%8). pozisyon
        B[i / 8] |= (uint8_t)(b[i] << (i % 8)); // En son toplarım.
    }
}

// algo 4
void bytes_to_bits(const uint8_t *B, size_t byte_len, uint8_t *b)
{
    size_t bit_len = byte_len * 8;
    memset(b, 0, bit_len);
    for (size_t i = 0; i < byte_len; i++)
    {
        // i. bit → B[i/8] içindeki (i%8). pozisyon
        uint8_t byte = B[i];
        for (size_t j = 0; j < 8; j++)
        {
            b[i * 8 + j] = (byte >> j);
        }
    }
}

// algo 5
void byte_encode(const polynom *f, int d, uint8_t *out)
{
    uint8_t max_bits[N * 12]; // d=12 max

    for (int i = 0; i < N; i++)
    {
        int8_t c = f->coefficients[i];
        for (int j = 0; j < d; j++)
        {
            max_bits[i * d + j] = c % 2;
            c = (c - (c % 2)) / 2; // >> 1 de olabilir.
        }
    }

    bits_to_bytes(max_bits, N * d, out);
}

// algo 6
void byte_decode(const uint8_t *in, int d, polynom *f)
{
    int m;
    if (d == 12)
    {
        m = Q;
    }
    else
        m = 1 << d;

    uint8_t bits[N * 12]; // d=12 max
    bytes_to_bits(in, N * d, bits);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < d; j++)
        {
            f->coefficients[i] += (bits[i * d + j] * (1 << j)) % m;
        }
    }
}

// algo 7
void sample_ntt(const uint8_t rho[32], uint8_t i, uint8_t j, polynom *a_hat) // rho dediği p sembolü.
{
    uint8_t temp[34];
    memcpy(temp, rho, 32);
    temp[32] = i;
    temp[33] = j;

    keccak_state ctx;
    shake128_init(&ctx);
    shake128_absorb(&ctx, rho, 34);

    int counter = 0; // dokümanda j.
    uint8_t C[3];
    while (counter < N)
    {
        shake128_squeeze(C, 3, &ctx);
        int16_t d1 = (int16_t)(C[0] + 256 * ((C[1] & 0x0F) << 8));
        int16_t d2 = (int16_t)((C[1] >> 4) + 16 * C[2]);
        if (d1 < Q)
        {
            a_hat->coefficients[counter] = d1;
        }
        if (d2 < Q && counter < 256)
        {
            a_hat->coefficients[counter] = d2;
        }
        counter++;
    }
}

// algo 8
void sample_poly_cbd(const uint8_t *b, int eta, polynom *f)
{
    uint8_t bits[eta1 * 64 * 8]; // size'ı max yapalım
    bytes_to_bits(b, eta * 64, bits);
    int x = 0;
    int y = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            x += bits[2 * i * eta + j];
        }
        for (int j = 0; j < N; j++)
        {
            y += bits[2 * i * eta + j + eta];
        }
        f->coefficients[i] = (uint8_t)(x - y + Q) % Q;
    }
}

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

// algo 9
void ntt(polynom *f)
{
    polynom f_hat;
    uint32_t t;
    int i = 1;
    for (int len = 128; len >= 2; len /= 2) // bit revision kısmı zetalarda zaten hazırmış.
    {
        for (int start = 0; start < N; start += 2 * len)
        {
            uint32_t zeta = zetas[i++];
            for (int j = start; j < start + len; j++)
            {
                // butterfly
                t = ((uint32_t)zeta * f_hat.coefficients[j + len] + Q) % Q;
                f_hat.coefficients[j + len] = (f_hat.coefficients[j] - t + Q) % Q;
                f_hat.coefficients[j] = (f_hat.coefficients[j] + t + Q) % Q;
                // modulo için montgomery?
            }
        }
    }
}

// algo 10
void ntt_inverse(polynom *f_hat)
{
    polynom f;
    uint32_t t;
    int i = 127;
    for (int len = 2; len <= 128; len *= 2)
    {
        for (int start = 0; start < N; start += 2 * len)
        {
            uint32_t zeta = zetas[i--];
            for (int j = start; j < start + len; j++)
            {
                t = f.coefficients[j];
                f.coefficients[j] = (t + f.coefficients[j + len] + Q) % Q;
                f.coefficients[j + len] = ((uint32_t)zeta * (f.coefficients[j + len] - t) + Q) % Q;
            }
        }
    }
    for (int i = 0; i < N; i++)
    {
        f.coefficients[i] = (f.coefficients[i] * 3303 + Q) % Q;
    }
}

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

// algo 11
void multiply_ntts(polynom *f_hat, polynom *g_hat, polynom *h_hat)
{
    for (int i = 0; i < N; i++)
    {
        for (int i = 0; i < 128; i++)
        {
            int16_t a0 = f_hat->coefficients[2 * i];
            int16_t a1 = f_hat->coefficients[2 * i + 1];
            int16_t b0 = g_hat->coefficients[2 * i];
            int16_t b1 = g_hat->coefficients[2 * i + 1];
            int gamma = gammas[i];
            int16_t *c0 = &h_hat->coefficients[2 * i];
            int16_t *c1 = &h_hat->coefficients[2 * i + 1];
            base_case_multiply(a0, a1, b0, b1, c0, c1, gamma);
        }
    }
}

// algo 12
void base_case_multiply(int16_t a0, int16_t a1, int16_t b0, int16_t b1, int16_t *c0, int16_t *c1, int16_t gamma)
{
    // P1 = a0 + a1*x, P2 = b0 + b1*x
    // X^2 = gamma (sabit terim) olsun
    *c0 = (a0 * b0 + a1 * b1 * gamma + Q) % Q; // çarpmaları ayrı ayrı 32 bitte yazmak gerekebilir bakıcaz.
    *c1 = (a0 * b1 + a1 * b0 + Q) % Q;
    // bu ikisi h'nin katsayıları olacak (algo 11).
}

void prf(uint8_t sigma[32], uint8_t b, int eta, uint8_t *out)
{
    size_t outlen_bytes = eta * 64;
    uint8_t input[33];
    memcpy(input, sigma, 32);
    input[32] = b;
    shake256(out, outlen_bytes, input, 33);
}

// algo 13
void keygen(uint8_t d[32], uint8_t encryption_key[K * 384 + 32], uint8_t decryption_key[K * 384]) // 256*d(12)/8 = 384 byte
{
    uint8_t input[33];
    memcpy(input, d, 32);
    input[32] = (uint8_t)K;
    uint8_t output[64];
    sha3_512(output, input, 33);
    uint8_t rho[32];
    memcpy(rho, output, 32);
    uint8_t sigma[32];
    memcpy(sigma, output + 32, 32);
    int n = 0;
    polynom_matrix A_hat;
    uint8_t prf_buffer[eta1 * 64];
    polynom_vector s_hat, e_hat, t_hat;
    polynom temp;
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            sample_ntt(rho, i, j, &A_hat.matrix[i][j]);
        }
    }
    for (int i = 0; i < K; i++)
    {
        prf(sigma, n, eta1, prf_buffer);
        sample_poly_cbd(prf_buffer, eta1, &s_hat.vector[i]);
        n++;
    }
    for (int i = 0; i < K; i++)
    {
        prf(sigma, n, eta1, prf_buffer);
        sample_poly_cbd(prf_buffer, eta1, &e_hat.vector[i]);
        n++;
    }
    for (int i = 0; i < K; i++)
    {
        ntt(&s_hat.vector[i]);
        ntt(&e_hat.vector[i]);
    }
    for (int i = 0; i < K; i++) // 2 polinom var. Matriste de 2x2.
    {
        t_hat.vector[i] = e_hat.vector[i];
        for (int j = 0; j < K; j++)
        {
            multiply_ntts(&A_hat.matrix[i][j], &s_hat.vector[j], &temp);
            for (int k = 0; k < N; k++)
            {
                t_hat.vector[i].coefficients[k] += temp.coefficients[k];
                t_hat.vector[i].coefficients[k] = (t_hat.vector[i].coefficients[k] + Q);
            }
        }
    }

    for (int i = 0; i < K; i++)
    {
        byte_encode(&t_hat.vector[i], 12, &encryption_key[i * 384]); // d=12
    }
    memcpy(&encryption_key[K * 384], rho, 32);

    for (int i = 0; i < K; i++)
    {
        byte_encode(&s_hat.vector[i], 12, &decryption_key[i * 384]); // d=12
    }
}

void print_hex(uint8_t *in, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", in[i]);
    }
    printf("\n");
}