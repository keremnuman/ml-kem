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
        if (b[i] & 1)
            B[i / 8] |= (uint8_t)(b[i] << (i % 8));
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
            b[i * 8 + j] = (byte >> j) & 1; // 1 bit için 1 yapıyorum.
        }
    }
}

// algo 5
void byte_encode(const polynom *f, int d, uint8_t *out)
{
    uint8_t max_bits[N * 12]; // d=12 max

    for (int i = 0; i < N; i++)
    {
        uint16_t c = (uint16_t)(f->coefficients[i]);
        for (int j = 0; j < d; j++)
        {
            max_bits[i * d + j] = (c >> j) & 1;
        }
    }

    bits_to_bytes(max_bits, N * d, out);
}

// algo 6
void byte_decode(const uint8_t *in, int d, polynom *f)
{
    memset(f->coefficients, 0, 256 * sizeof(int16_t));
    int m;
    if (d == 12)
    {
        m = Q;
    }
    else
        m = 1 << d;

    uint8_t bits[N * 12]; // d=12 max
    bytes_to_bits(in, (N * d) / 8, bits);
    for (int i = 0; i < N; i++)
    {
        int32_t sum = 0;
        for (int j = 0; j < d; j++)
        {
            sum |= (bits[i * d + j] << j);
        }
        f->coefficients[i] = (int16_t)(sum % Q);
    }
}

// algo 7
void sample_ntt(const uint8_t rho[32], uint8_t i, uint8_t j, polynom *a_hat) // rho dediği p sembolü.
{
    memset(a_hat->coefficients, 0, sizeof(a_hat->coefficients));
    uint8_t temp[34];
    memcpy(temp, rho, 32);
    temp[32] = j;
    temp[33] = i;

    keccak_state ctx;
    shake128_init(&ctx);
    shake128_absorb(&ctx, temp, 34); // rho demişiz ama temp olacak. (34)
    shake128_finalize(&ctx);
    // printf("Keccak State after absorbing: \n");
    // printf("Position: %u\n", ctx.pos);
    // printf("State: \n"); // 34'ten sonra 0 olacak. kesin değru.
    //  print_hex((uint8_t *)ctx.s, 25 * sizeof(uint64_t));

    int counter = 0; // dokümanda j.
    uint8_t C[3];    // çıkış 3 byte
    while (counter < N)
    {
        shake128_squeeze(C, 3, &ctx);
        // int16_t d1 = (int16_t)(C[0] + 256 * ((C[1] & 0x0F) << 8)); //!!!
        uint16_t d1 = (uint16_t)(C[0] | (((uint16_t)C[1] & 0x0F) << 8)); //!!!
        uint16_t d2 = (uint16_t)((C[1] >> 4) | ((uint16_t)C[2] << 4));   // or
        if (d1 < Q)
        {
            a_hat->coefficients[counter] = d1;
            counter++;
        }
        if (d2 < Q && counter < N)
        {
            a_hat->coefficients[counter] = d2;
            counter++;
        }
        // printf("d1: %d\n, d2: %d\n", d1, d2);
        // printf("counter: %d\n", counter);
    }
}

// algo 8
void sample_poly_cbd(const uint8_t *b, int eta, polynom *f)
{
    uint8_t bits[eta1 * 64 * 8]; // size'ı max yapalım
    bytes_to_bits(b, eta * 64, bits);
    // int x = 0;
    // int y = 0; bunları içeri alıyorum.
    for (int i = 0; i < N; i++)
    {
        int x = 0;
        int y = 0;
        for (int j = 0; j < eta; j++)
        {
            x += bits[2 * i * eta + j];
        }
        for (int j = 0; j < eta; j++)
        {
            y += bits[2 * i * eta + j + eta];
        }
        f->coefficients[i] = (uint16_t)((int)x - y + Q) % Q;
    }
}

// algo 9
void ntt(polynom *f)
{
    // polynom f_hat;                                             // f'ten devam edicem.
    // memset(f_hat.coefficients, 0, sizeof(f_hat.coefficients)); // bunu sıfırladığımız için sorun var galiba.
    // uint32_t t;
    int32_t t;
    int i = 1;
    for (int len = 128; len >= 2; len /= 2) // bit revision kısmı zetalarda zaten hazırmış.
    {
        for (int start = 0; start < N; start += 2 * len)
        {
            uint32_t zeta = zetas[i++];
            for (int j = start; j < start + len; j++)
            {
                // butterfly
                t = (int32_t)((int64_t)zeta * f->coefficients[j + len] % Q);
                t = (t % Q + Q) % Q;
                f->coefficients[j + len] = (f->coefficients[j] - t + Q) % Q;
                f->coefficients[j] = (f->coefficients[j] + t + Q) % Q;
                // modulo için montgomery?
            }
        }
    }
}

// algo 10
void ntt_inverse(polynom *f_hat)
{
    // polynom f; // yine aynı mevzu
    // uint32_t t;
    int32_t t;
    int i = 127;
    for (int len = 2; len <= 128; len *= 2)
    {
        for (int start = 0; start < N; start += 2 * len)
        {
            uint32_t zeta = zetas[i--];
            for (int j = start; j < start + len; j++)
            {
                t = f_hat->coefficients[j];
                f_hat->coefficients[j] = (t + f_hat->coefficients[j + len] + Q) % Q;
                int32_t diff = (int32_t)f_hat->coefficients[j + len] - t;
                f_hat->coefficients[j + len] = (int16_t)(((int64_t)zeta * diff % Q + Q) % Q);
            }
        }
    }
    for (int i = 0; i < N; i++)
    {
        f_hat->coefficients[i] = (f_hat->coefficients[i] * 3303 + Q) % Q;
    }
}

// algo 11
void multiply_ntts(polynom *f_hat, polynom *g_hat, polynom *h_hat)
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

// algo 12
void base_case_multiply(int16_t a0, int16_t a1, int16_t b0, int16_t b1, int16_t *c0, int16_t *c1, int16_t gamma)
{
    // P1 = a0 + a1*x, P2 = b0 + b1*x
    // X^2 = gamma (sabit terim) olsun
    int32_t temp1 = ((int32_t)a0 * b0) % Q;
    int32_t temp2 = ((int32_t)a1 * b1) % Q;
    int32_t temp3 = ((int32_t)a0 * b1) % Q;
    int32_t temp4 = ((int32_t)a1 * b0) % Q;

    int32_t c0_val = temp1 + temp2 * gamma;
    *c0 = (int16_t)((c0_val % Q + Q) % Q);

    int32_t c1_val = temp3 + temp4;
    *c1 = (int16_t)((c1_val % Q + Q) % Q);
    // *c0 = (a0 * b0 + a1 * b1 * gamma + Q) % Q; // çarpmaları ayrı ayrı 32 bitte yazmak gerekebilir bakıcaz.
    // bu gerekliymiş bu arada. düzelttim.
    // *c1 = (a0 * b1 + a1 * b0 + Q) % Q;
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
    sha3_512(output, input, 33); // bu kesin doğru.
    uint8_t rho[32];
    memcpy(rho, output, 32);
    printf("Rho: \n");
    print_hex(rho, 32);
    uint8_t sigma[32];
    memcpy(sigma, output + 32, 32);
    printf("Sigma: \n");
    print_hex(sigma, 32);
    int n = 0;
    polynom_matrix A_hat;
    memset(&A_hat, 0, sizeof(A_hat)); // algo 7'de de var ama orada sadece tek polinomdu.
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
    print_A_matrix(&A_hat);

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
                int32_t sum = (int32_t)t_hat.vector[i].coefficients[k] + (int32_t)temp.coefficients[k];
                t_hat.vector[i].coefficients[k] = (sum + Q) % Q;
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

void print_A_matrix(const polynom_matrix *A_hat)
{
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            printf("A_hat[%d][%d] = ", i, j);
            for (int k = 0; k < N; k++)
            {
                printf("%d", (uint16_t)A_hat->matrix[i][j].coefficients[k]);
                if (k < N - 1)
                    printf(", ");
            }
            printf("\n");
        }
    }
}

// algo 14
void encrypt(uint8_t encryption_key[K * 384 + 32], uint8_t m[32], uint8_t r[32], uint8_t ciphertext[32 * (du * K + dv)])
{
    int n = 0;
    polynom_vector t_hat;
    memset(&t_hat, 0, sizeof(polynom_vector));
    for (int i = 0; i < K; i++)
    {
        byte_decode(&encryption_key[i * 384], 12, &t_hat.vector[i]);
    }
    uint8_t *rho = &encryption_key[K * 384]; // seed for matrix (32B).
    polynom_matrix A_hat;
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            sample_ntt(rho, j, i, &A_hat.matrix[i][j]); // i-j
        }
    }
    uint8_t prf_buffer[eta1 * 64];
    polynom_vector y, e1, u;
    for (int i = 0; i < K; i++)
    {
        prf(r, n, eta1, prf_buffer);
        sample_poly_cbd(prf_buffer, eta1, &y.vector[i]);
        n++;
    }
    for (int i = 0; i < K; i++)
    {
        prf(r, n, eta2, prf_buffer);
        sample_poly_cbd(prf_buffer, eta2, &e1.vector[i]);
        n++;
    }
    polynom e2;
    prf(r, n, eta2, prf_buffer); // n++?
    n++;
    sample_poly_cbd(prf_buffer, eta2, &e2);
    for (int i = 0; i < K; i++)
    {
        ntt(&y.vector[i]);
    }
    polynom temp, v;
    for (int i = 0; i < K; i++)
    {
        memset(&u.vector[i], 0, sizeof(polynom));
        for (int j = 0; j < K; j++)
        {
            multiply_ntts(&A_hat.matrix[i][j], &y.vector[j], &temp);
            for (int k = 0; k < N; k++)
            {
                int32_t acc = (int32_t)u.vector[i].coefficients[k] + temp.coefficients[k];
                u.vector[i].coefficients[k] = (int16_t)((acc % Q + Q) % Q);
            }
        }
        ntt_inverse(&u.vector[i]);
        for (int j = 0; j < N; j++)
        {
            int32_t sum = (int32_t)u.vector[i].coefficients[j] + e1.vector[i].coefficients[j];
            u.vector[i].coefficients[j] = (int16_t)((sum % Q + Q) % Q);
        }
    }

    polynom mü;
    memset(&mü, 0, sizeof(polynom));
    // byte_decode(m, 1, &mü);
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            uint8_t bit = (m[i] >> j) & 1;
            mü.coefficients[i * 8 + j] = bit ? 1665 : 0; // Decompress_1(bit)
        }
    }
    memset(&v, 0, sizeof(polynom));
    for (int i = 0; i < K; i++)
    {
        multiply_ntts(&t_hat.vector[i], &y.vector[i], &temp);
        for (int k = 0; k < N; k++)
        {
            int32_t acc = (int32_t)v.coefficients[k] + temp.coefficients[k];
            v.coefficients[k] = (int16_t)((acc % Q + Q) % Q);
        }
    }
    ntt_inverse(&v);
    for (int i = 0; i < N; i++) // K?
    {
        int32_t sum = (int32_t)v.coefficients[i] + e2.coefficients[i] + mü.coefficients[i];
        v.coefficients[i] = (int16_t)((sum % Q + Q) % Q);
    }
    int u_bytes = 256 * du / 8; // 320B
    for (int i = 0; i < K; i++) // 320x2 = 640B, 10bit
    {
        polynom u_compressed;
        memset(&u_compressed, 0, sizeof(polynom));
        for (int j = 0; j < N; j++)
        {
            uint16_t temp_val;
            compress(u.vector[i].coefficients[j], du, &temp_val);
            u_compressed.coefficients[j] = (int16_t)temp_val;
        }
        // printf("Encrypt u[0][0]: %d\n", u.vector[0].coefficients[0]);
        byte_encode(&u_compressed, du, &ciphertext[i * u_bytes]);
    }
    polynom v_compressed;
    memset(&v_compressed, 0, sizeof(polynom));
    for (int j = 0; j < N; j++)
    {
        uint16_t temp_val;
        compress(v.coefficients[j], dv, &temp_val);
        v_compressed.coefficients[j] = (int16_t)temp_val;
    }
    byte_encode(&v_compressed, dv, &ciphertext[K * u_bytes]); // 128B, 4bit
    printf("Ciphertext: \n");
    printf("c1-u[0]\n");
    print_hex(&ciphertext[0], u_bytes); // 320
    printf("c1-u[1]\n");
    print_hex(&ciphertext[320], u_bytes); // 320
    printf("c2-v\n");
    print_hex(&ciphertext[640], 128); // 128
    printf("Full Ciphertext: \n");
    print_hex(ciphertext, 32 * (du * K + dv)); // 768
    fflush(stdout);
}

void compress(int16_t x, int d, uint16_t *compressed)
{
    uint32_t power_of_two = 1 << d;
    uint32_t x_positive = (uint32_t)(x % Q + Q) % Q;
    uint32_t result = (x_positive * power_of_two + (Q / 2)) / Q;
    *compressed = (uint16_t)(result & (power_of_two - 1));
}

void decompress(uint16_t compressed, int d, int16_t *decompressed)
{
    uint32_t power_of_two = 1 << d;
    uint32_t result = (uint32_t)compressed * Q;
    result = (result + (power_of_two / 2)) / power_of_two;
    *decompressed = (int16_t)(result);
}

// algo 15
void decrypt(uint8_t decryption_key[K * 384], uint8_t ciphertext[32 * (du * K + dv)], uint8_t m[32])
{
    uint8_t c1[640];
    uint8_t c2[128];
    memcpy(c1, &ciphertext[0], 640);
    memcpy(c2, &ciphertext[640], 128);
    polynom_vector u_hat, s_hat;
    polynom v_hat;
    memset(&u_hat, 0, sizeof(polynom_vector));
    memset(&s_hat, 0, sizeof(polynom_vector));
    memset(&v_hat, 0, sizeof(polynom));
    int u_bytes = 256 * du / 8;
    polynom u_compressed;
    memset(&u_compressed, 0, sizeof(polynom));
    for (int i = 0; i < K; i++)
    {
        byte_decode(&c1[i * u_bytes], du, &u_compressed);
        for (int j = 0; j < N; j++)
        {
            int16_t decompressed_val;
            decompress((uint16_t)u_compressed.coefficients[j], du, &decompressed_val);
            u_hat.vector[i].coefficients[j] = decompressed_val;
            // printf("u_hat[%d][%d]: %d\n", i, j, u_hat.vector[i].coefficients[j]);
        }
    }
    polynom v_compressed;
    memset(&v_compressed, 0, sizeof(polynom));
    byte_decode(&ciphertext[K * u_bytes], dv, &v_compressed);
    for (int i = 0; i < N; i++)
    {
        int16_t decompressed_val;
        decompress((uint16_t)v_compressed.coefficients[i], dv, &decompressed_val);
        v_hat.coefficients[i] = decompressed_val;
    }
    for (int i = 0; i < K; i++)
    {
        byte_decode(&decryption_key[i * 384], 12, &s_hat.vector[i]);
    }
    polynom temp, s_hat_temp;
    memset(&temp, 0, sizeof(polynom));
    memset(&s_hat_temp, 0, sizeof(polynom));
    for (int i = 0; i < K; i++)
    {
        ntt(&u_hat.vector[i]);
        multiply_ntts(&s_hat.vector[i], &u_hat.vector[i], &temp); // NTT mult. initializes every round.
        for (int j = 0; j < N; j++)
        {
            s_hat_temp.coefficients[j] = (int16_t)(((s_hat_temp.coefficients[j] + temp.coefficients[j]) % Q + Q) % Q);
        }
    }
    ntt_inverse(&s_hat_temp);
    polynom w, w_bit;
    memset(&w, 0, sizeof(polynom));
    memset(&w_bit, 0, sizeof(polynom));
    for (int i = 0; i < N; i++)
    {
        int32_t sum = (int32_t)v_hat.coefficients[i] - (int32_t)s_hat_temp.coefficients[i];
        w.coefficients[i] = (int16_t)((sum % Q + Q) % Q);
    }
    for (int i = 0; i < N; i++)
    {
        uint16_t bit_val;
        compress(w.coefficients[i], 1, &bit_val);
        w_bit.coefficients[i] = (int16_t)bit_val;
    }
    byte_encode(&w_bit, 1, m);
    printf("m:\n");
    print_hex(m, 32);
}

// algo 16
void keygen_internal(uint8_t d[32], uint8_t z[32], uint8_t encapsulation_key[K * 384 + 32], uint8_t decapsulation_key[K * 768 + 96])
{
    uint8_t temp_dk[K * 384];
    uint8_t hash[32];
    memset(temp_dk, 0, K * 384);
    keygen(d, encapsulation_key, temp_dk);
    memcpy(decapsulation_key, temp_dk, K * 384);
    memcpy(&decapsulation_key[K * 384], encapsulation_key, K * 384 + 32);
    sha3_256(hash, encapsulation_key, K * 384 + 32); //
    memcpy(&decapsulation_key[2 * 384 * K + 32], hash, 32);
    memcpy(&decapsulation_key[2 * 384 * K + 64], z, 32);
    printf("Decapsulation Key:\n");
    print_hex(decapsulation_key, 2 * 384 * K + 96);
    printf("Encapsulation Key:\n");
    print_hex(encapsulation_key, 384 * K + 32);
}

// algo 17
void encaps_internal(uint8_t encapsulation_key[K * 384 + 32], uint8_t randomness_m[32], uint8_t shared_key[32], uint8_t ciphertext[(du * K + dv) * 32])
{
    uint8_t temp[64];
    memcpy(temp, randomness_m, 32);
    uint8_t hash[32];
    sha3_256(hash, encapsulation_key, K * 384 + 32);
    memcpy(&temp[32], hash, 32);
    uint8_t G_temp[64];
    sha3_512(G_temp, temp, 64);
    memcpy(shared_key, G_temp, 32);
    printf("Shared Key: \n");
    print_hex(shared_key, 32);
    uint8_t randomness_r[32];
    memcpy(randomness_r, &G_temp[32], 32);
    encrypt(encapsulation_key, randomness_m, randomness_r, ciphertext);
    printf("Ciphertext: \n");
    print_hex(ciphertext, 32 * (du * K + dv));
}

// algo 18
void decaps_internal(uint8_t decapsulation_key[K * 768 + 96], uint8_t ciphertext[32 * (du * K + dv)], uint8_t shared_key[32])
{
    int c_len = 32 * (du * K + dv); // 768
    uint8_t decryption_key[K * 384];
    memcpy(decryption_key, decapsulation_key, K * 384);
    uint8_t encryption_key[K * 384 + 32];
    memcpy(encryption_key, &decapsulation_key[K * 384], K * 384 + 32);
    uint8_t hash[32];
    memcpy(hash, &decapsulation_key[2 * 384 * K + 32], 32);
    uint8_t z[32];
    memcpy(z, &decapsulation_key[2 * 384 * K + 64], 32);
    uint8_t m_prime[32];
    decrypt(decryption_key, ciphertext, m_prime);
    printf("m_prime:\n");
    print_hex(m_prime, 32);
    uint8_t G_in[64];
    memcpy(G_in, m_prime, 32);
    memcpy(&G_in[32], hash, 32);
    uint8_t hash_result[64];
    sha3_512(hash_result, G_in, 64);
    printf("hash_result:\n");
    print_hex(hash_result, 64);
    uint8_t K_prime[32];
    uint8_t r_prime[32];
    memcpy(K_prime, &hash_result[0], 32);
    memcpy(r_prime, &hash_result[32], 32);
    printf("K_prime:\n");
    print_hex(K_prime, 32);
    printf("r_prime:\n");
    print_hex(r_prime, 32);
    uint8_t J_in[800];
    memcpy(J_in, z, 32);
    memcpy(J_in + 32, ciphertext, c_len);
    uint8_t K_bar[32];
    shake256(K_bar, 32, J_in, 800);
    printf("K_bar:\n");
    print_hex(K_bar, 32);
    uint8_t c_prime[768];
    memset(c_prime, 0, 768);
    encrypt(encryption_key, m_prime, r_prime, c_prime);
    printf("c_prime:\n");
    print_hex(c_prime, 768);
    if (memcmp(ciphertext, c_prime, 768) != 0)
    {
        printf("c_prime is not equal to ciphertext\n");
        printf(strerror(errno));
        printf("Implicit rejection\n");
        memcpy(shared_key, K_bar, 32);
    }
    else
        memcpy(shared_key, K_prime, 32);
    printf("Shared Key: \n");
    print_hex(shared_key, 32);
}

// algo 19
void mlkem_keygen(uint8_t encapsulation_key[384 * K + 32], uint8_t decapsulation_key[768 * K + 96], uint8_t d[32], uint8_t z[32])
{
    if (d == NULL || z == NULL)
    {
        printf("d or z is NULL\n");
        printf(strerror(errno));
        return;
    }
    keygen_internal(d, z, encapsulation_key, decapsulation_key);
}

// algo 20
void encaps(uint8_t encapsulation_key[384 * K + 32], uint8_t shared_key[32], uint8_t ciphertext[(du * K + dv) * 32], uint8_t m[32])
{
    if (m == NULL)
    {
        printf("m is NULL\n");
        printf(strerror(errno));
        return;
    }
    encaps_internal(encapsulation_key, m, shared_key, ciphertext);
}

// algo 21
void decaps(uint8_t decapsulation_key[768 * K + 96], uint8_t ciphertext[32 * (du * K + dv)], uint8_t shared_key[32])
{
    decaps_internal(decapsulation_key, ciphertext, shared_key);
    printf("Shared Key: \n");
    print_hex(shared_key, 32);
}

void hex_to_bytes(char *hex_str, uint8_t *byte_array)
{
    for (size_t i = 0; i < strlen(hex_str); i += 2)
    {
        sscanf(&hex_str[i], "%2hhx", &byte_array[i / 2]);
    }
}