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

    int counter = 0;
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
    for (int len = 128; len >= 2; len /= 2)
    {
        for (int start = 0; start < N; start += 2 * len)
        {
            uint32_t zeta = zetas[i++];
            for (int j = start; j < start + len; j++)
            {
                t = ((uint32_t)zeta * f_hat.coefficients[j + len] + Q) % Q;
                f_hat.coefficients[j + len] = (f_hat.coefficients[j] - t + Q) % Q;
                f_hat.coefficients[j] = (f_hat.coefficients[j] + t + Q) % Q;
            }
        }
    }
}