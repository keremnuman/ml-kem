#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "algorithms.h"

int main(int argc, char *argv[])
{
    uint8_t d[32] = {
        0x8c, 0x72, 0x38, 0xe1, 0x96, 0x5d, 0xdd, 0x73, 0xb1, 0x11, 0x4b, 0x89, 0x7e, 0x1b, 0xf4, 0xb3,
        0x08, 0xc0, 0xd9, 0xcc, 0x71, 0x0d, 0x04, 0x82, 0xab, 0x8b, 0x9e, 0x73, 0x74, 0x05, 0x35, 0x4a};

    uint8_t z[32] = {
        0x84, 0x76, 0x01, 0x35, 0x60, 0x15, 0x1d, 0x98, 0x6d, 0xc7, 0x83, 0x4d, 0xcb, 0x57, 0xc7, 0x5f,
        0x84, 0x5f, 0x8d, 0x7e, 0xe7, 0x15, 0x58, 0xd0, 0x95, 0x5f, 0x3f, 0x4f, 0xeb, 0x72, 0x3c, 0xf2};

    uint8_t m[32] = {
        0xe4, 0x8f, 0x74, 0xad, 0x41, 0x6e, 0x99, 0x63, 0x10, 0x03, 0xde, 0xd1, 0x47, 0x8a, 0xec, 0x62,
        0xa0, 0x2c, 0x24, 0x5d, 0x88, 0xed, 0x7f, 0x2f, 0xae, 0x92, 0xae, 0xdf, 0x13, 0xca, 0x03, 0xb3};

    uint8_t r_test[32] = {
        0xBF, 0x79, 0xBD, 0x35, 0x17, 0xEB, 0xFC, 0x80, 0xEC, 0x52, 0x98, 0x12, 0x41,
        0xFA, 0x5E, 0x67, 0xF5, 0xCC, 0xE2, 0xA5, 0x3A, 0x81, 0x74, 0x6D, 0xA2, 0xCE,
        0xE4, 0x5D, 0x6C, 0x13, 0xB4, 0x68};

    uint8_t encryption_key[K * 384 + 32];
    uint8_t decryption_key[K * 384];
    uint8_t ciphertext_pke[32 * (du * K + dv)];
    uint8_t decrypted_m[32];

    uint8_t encapsulation_key[K * 384 + 32];
    uint8_t decapsulation_key[K * 768 + 96];
    uint8_t ciphertext_kem[32 * (du * K + dv)];
    uint8_t shared_key_A[32];
    uint8_t shared_key_B[32];

    keygen(d, encryption_key, decryption_key);
    printf("Encryption Key: \n");
    print_hex(encryption_key, K * 384 + 32);
    printf("Decryption Key: \n");
    print_hex(decryption_key, K * 384);
    encrypt(encryption_key, m, r_test, ciphertext_pke);
    printf("Ciphertext: \n");
    print_hex(ciphertext_pke, 32 * (du * K + dv));
    decrypt(decryption_key, ciphertext_pke, decrypted_m);
    printf("Decrypted Message: \n");
    print_hex(decrypted_m, 32);

    if (memcmp(m, decrypted_m, 32) != 0)
    {
        printf("m is not equal to decrypted_m\n");
        return 1;
    }
    // Key Generation (Internal)
    keygen_internal(d, z, encapsulation_key, decapsulation_key);
    printf("Encapsulation Key: \n");
    print_hex(encapsulation_key, K * 384 + 32);
    printf("Decapsulation Key: \n");
    print_hex(decapsulation_key, K * 768 + 96);

    // Encapsulation
    encaps_internal(encapsulation_key, m, shared_key_A, ciphertext_kem);
    printf("Ciphertext KEM: \n");
    print_hex(ciphertext_kem, 32 * (du * K + dv));

    decaps_internal(decapsulation_key, ciphertext_kem, shared_key_B);
    printf("Shared Key: \n");
    print_hex(shared_key_B, 32);

    if (memcmp(shared_key_A, shared_key_B, 32) != 0)
    {
        printf("A: ");
        print_hex(shared_key_A, 32);
        printf("B: ");
        print_hex(shared_key_B, 32);
        return 1;
    }
    else
    {
        printf("Matched shared key\n");
        print_hex(shared_key_A, 32);
    }
    return 0;
}