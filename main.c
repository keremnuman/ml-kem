#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "algorithms.h"

int main(int argc, char *argv[])
{
    uint8_t d[32] = {
        0x89, 0x4c, 0x5d, 0xe8, 0xeb, 0x12, 0xe3, 0x04,
        0xeb, 0x26, 0x81, 0x48, 0xf8, 0xc2, 0x86, 0xe3,
        0xf6, 0x35, 0xdc, 0x01, 0x47, 0x24, 0x46, 0x0c,
        0xbb, 0xb1, 0x3b, 0xe7, 0x15, 0x02, 0x14, 0x25};

    uint8_t encryption_key[K * 384 + 32];
    uint8_t decryption_key[K * 384];

    print_hex(d, 32);
    keygen(d, encryption_key, decryption_key);
    printf("Public Key: \n");
    print_hex(encryption_key, K * 384 + 32);
    printf("Private Key: \n");
    print_hex(decryption_key, K * 384);
    return 0;
}