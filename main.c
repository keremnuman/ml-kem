#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "algorithms.h"

int main(int argc, char *argv[])
{
    uint8_t d[32] = {
        0x47, 0xB8, 0x93, 0x47, 0x46, 0x72, 0xBA, 0x92, 0xE4, 0xB1, 0x2E, 0xE4, 0x4F, 0xB3, 0x29, 0x53,
        0xAF, 0x8E, 0x85, 0x03, 0xB5, 0xFB, 0x47, 0x1D, 0x16, 0x14, 0xFB, 0x8A, 0x02, 0x1A, 0x66, 0x0A};

    uint8_t z[32] = {
        0x1F, 0x8C, 0xB3, 0x9E, 0x9E, 0x30, 0xBC, 0x45, 0x8A, 0x0D, 0xC5, 0x40, 0x88, 0x84, 0xB1, 0x18,
        0x7F, 0xB2, 0x17, 0x01, 0x8D, 0xF7, 0x60, 0xFA, 0x57, 0x31, 0x77, 0x03, 0xB8, 0x44, 0xA0, 0xA9};

    uint8_t m[32] = {
        0x19, 0xC4, 0x4D, 0x35, 0xAB, 0x9E, 0xF3, 0x1B, 0x13, 0x60, 0xF0, 0xBF, 0x33, 0xCF, 0x63, 0xD8,
        0x0E, 0x40, 0x59, 0x62, 0xD6, 0x98, 0x41, 0x5C, 0x58, 0x88, 0xF0, 0xAF, 0x38, 0x5D, 0xCF, 0xF4};

    uint8_t r_test[32] = {
        0xBF, 0x79, 0xBD, 0x35, 0x17, 0xEB, 0xFC, 0x80, 0xEC, 0x52, 0x98, 0x12, 0x41, 0xFA, 0x5E, 0x67,
        0xF5, 0xCC, 0xE2, 0xA5, 0x3A, 0x81, 0x74, 0x6D, 0xA2, 0xCE, 0xE4, 0x5D, 0x6C, 0x13, 0xB4, 0x68};

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
    printf("Message: \n");
    print_hex(m, 32);

    keygen_internal(d, z, encapsulation_key, decapsulation_key);
    printf("Encapsulation Key: \n");
    print_hex(encapsulation_key, K * 384 + 32);
    printf("Decapsulation Key: \n");
    print_hex(decapsulation_key, K * 768 + 96);

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
        printf("Matched shared key:\n");
        print_hex(shared_key_A, 32);
    }
    return 0;
}