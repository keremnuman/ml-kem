
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include "algorithms.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Tek bir koşul kontrolü.
   Başarısız olursa dosya ve satır numarasını yazdırır. */
#define CHECK(cond, msg)                                               \
    do                                                                 \
    {                                                                  \
        tests_run++;                                                   \
        if (cond)                                                      \
        {                                                              \
            tests_passed++;                                            \
            printf("  [PASS] %s\n", msg);                              \
        }                                                              \
        else                                                           \
        {                                                              \
            tests_failed++;                                            \
            printf("  [FAIL] %s  ← %s:%d\n", msg, __FILE__, __LINE__); \
        }                                                              \
    } while (0)

/* Byte dizisi karşılaştırması.
   Başarısız olursa ilk farklı byte'ı gösterir. */
#define CHECK_BYTES(a, b, len, msg)                                     \
    do                                                                  \
    {                                                                   \
        tests_run++;                                                    \
        int _ok = (memcmp((a), (b), (len)) == 0);                       \
        if (_ok)                                                        \
        {                                                               \
            tests_passed++;                                             \
            printf("  [PASS] %s\n", msg);                               \
        }                                                               \
        else                                                            \
        {                                                               \
            tests_failed++;                                             \
            printf("  [FAIL] %s  <- %s:%d\n", msg, __FILE__, __LINE__); \
            /* İlk farklı byte'ı bul ve göster */                       \
            for (size_t _i = 0; _i < (len); _i++)                       \
            {                                                           \
                if (((uint8_t *)(a))[_i] != ((uint8_t *)(b))[_i])       \
                {                                                       \
                    printf("  Ilk fark: [%zu]  ", _i);                  \
                    printf("beklenen=0x%02x  ", ((uint8_t *)(b))[_i]);  \
                    printf("uretilen=0x%02x\n", ((uint8_t *)(a))[_i]);  \
                    break;                                              \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0)

static void section(const char *name)
{
    printf("\n%s\n", name);
}

static void test_bits_to_bytes(void)
{
    section("Algoritma 3: BitsToBytes");

    /* Test 1: Bilinen dönüşüm
       Standart Bölüm 4.2.1'den örnek:
       bit dizisi 11010001 (little-endian)
       = 2^0 + 2^1 + 2^3 + 2^7 = 1+2+8+128 = 139 */
    {
        uint8_t bits[8] = {1, 1, 0, 1, 0, 0, 0, 1};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 139, "11010001 (LE) -> 139");
    }

    /* Test 2: Sıfır byte
       8 sıfır bit → 0x00 */
    {
        uint8_t bits[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x00, "00000000 -> 0x00");
    }

    /* Test 3: Hep bir
       11111111 (LE) = 255 = 0xFF */
    {
        uint8_t bits[8] = {1, 1, 1, 1, 1, 1, 1, 1};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0xFF, "11111111 -> 0xFF");
    }

    /* Test 4: İlk bit 1, geri kalan 0
       little-endian → 2^0 = 1 */
    {
        uint8_t bits[8] = {1, 0, 0, 0, 0, 0, 0, 0};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x01, "10000000 (LE) -> 0x01");
    }

    /* Test 5: Son bit 1, geri kalan 0
       little-endian → 2^7 = 128 */
    {
        uint8_t bits[8] = {0, 0, 0, 0, 0, 0, 0, 1};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x80, "00000001 (LE) -> 0x80");
    }

    /* Test 6: 2 byte — little-endian sıralaması doğru mu?
       byte[0]: 00000001 = 1
       byte[1]: 10000000 = 128 */
    {
        uint8_t bits[16] = {
            1, 0, 0, 0, 0, 0, 0, 0, /* byte[0] = 1   */
            0, 0, 0, 0, 0, 0, 0, 1  /* byte[1] = 128 */
        };
        uint8_t result[2];
        bits_to_bytes(bits, 16, result);
        CHECK(result[0] == 1 && result[1] == 128,
              "2-byte: [1,128]");
    }
}

static void test_bytes_to_bits(void)
{
    section("Algoritma 4: BytesToBits");

    /* Test 1: Bilinen byte → bit dizisi
       139 = 10001011 (big-endian) = 11010001 (little-endian)
       yani bit[0]=1, bit[1]=1, bit[2]=0, bit[3]=1, ... */
    {
        uint8_t input[1] = {139};
        uint8_t expected[8] = {1, 1, 0, 1, 0, 0, 0, 1};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "139 -> {1,1,0,1,0,0,0,1}");
    }

    /* Test 2: 0x00 → hep sıfır */
    {
        uint8_t input[1] = {0x00};
        uint8_t expected[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "0x00 -> {0,0,0,0,0,0,0,0}");
    }

    /* Test 3: 0xFF → hep bir */
    {
        uint8_t input[1] = {0xFF};
        uint8_t expected[8] = {1, 1, 1, 1, 1, 1, 1, 1};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "0xFF -> {1,1,1,1,1,1,1,1}");
    }
}

int main(void)
{

    setlocale(LC_ALL, "Turkish");
    test_bits_to_bytes();
    test_bytes_to_bits();

    return 0;
}