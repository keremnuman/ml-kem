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

// algo 9
void ntt(polynom *f)
{
    polynom f_hat;
    int i = 1;
    for (int len = 128; len >= 2; len /= 2)
    {
        for (int j = 0; j < N; j += 2 * len)
        {
                }
    }
}