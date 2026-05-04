#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "algorithms.h"

/* ═══════════════════════════════════════════════════════
   Test Altyapısı
   ═══════════════════════════════════════════════════════ */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Tek bir koşul kontrolü.
   Başarısız olursa dosya ve satır numarasını yazdırır. */
#define CHECK(cond, msg)                                          \
    do {                                                          \
        tests_run++;                                              \
        if (cond) {                                               \
            tests_passed++;                                       \
            printf("  [PASS] %s\n", msg);                        \
        } else {                                                  \
            tests_failed++;                                       \
            printf("  [FAIL] %s  ← %s:%d\n", msg, __FILE__, __LINE__); \
        }                                                         \
    } while (0)

/* Byte dizisi karşılaştırması.
   Başarısız olursa ilk farklı byte'ı gösterir. */
#define CHECK_BYTES(a, b, len, msg)                               \
    do {                                                          \
        tests_run++;                                              \
        int _ok = (memcmp((a), (b), (len)) == 0);                \
        if (_ok) {                                                \
            tests_passed++;                                       \
            printf("  [PASS] %s\n", msg);                        \
        } else {                                                  \
            tests_failed++;                                       \
            printf("  [FAIL] %s  ← %s:%d\n", msg, __FILE__, __LINE__); \
            /* İlk farklı byte'ı bul ve göster */                \
            for (size_t _i = 0; _i < (len); _i++) {              \
                if (((uint8_t*)(a))[_i] != ((uint8_t*)(b))[_i]) {\
                    printf("         İlk fark: [%zu]  ", _i);    \
                    printf("beklenen=0x%02x  ", ((uint8_t*)(b))[_i]); \
                    printf("üretilen=0x%02x\n", ((uint8_t*)(a))[_i]); \
                    break;                                        \
                }                                                 \
            }                                                     \
        }                                                         \
    } while (0)

static void section(const char *name)
{
    printf("\n┌─────────────────────────────────────────┐\n");
    printf("│  %-40s│\n", name);
    printf("└─────────────────────────────────────────┘\n");
}

static void print_summary(void)
{
    printf("\n══════════════════════════════════════\n");
    printf("  Toplam : %d\n", tests_run);
    printf("  Geçti  : %d\n", tests_passed);
    printf("  Kaldı  : %d\n", tests_failed);
    printf("══════════════════════════════════════\n");
    if (tests_failed == 0)
        printf("  Tüm testler geçti ✓\n\n");
    else
        printf("  %d test BAŞARISIZ ✗\n\n", tests_failed);
}

/* ═══════════════════════════════════════════════════════
   KATMAN 0 — BitsToBytes (Alg 3) & BytesToBits (Alg 4)
   ═══════════════════════════════════════════════════════
   Bu ikisi birbirinin tersi. Üç şeyi test edeceğiz:
   1. Bilinen giriş → bilinen çıkış (manuel hesaplanan)
   2. Round-trip: bytes_to_bits(bits_to_bytes(x)) == x
   3. Edge case: hep sıfır, hep bir
*/

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
        CHECK(result[0] == 139, "11010001 (LE) → 139");
    }

    /* Test 2: Sıfır byte
       8 sıfır bit → 0x00 */
    {
        uint8_t bits[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x00, "00000000 → 0x00");
    }

    /* Test 3: Hep bir
       11111111 (LE) = 255 = 0xFF */
    {
        uint8_t bits[8] = {1, 1, 1, 1, 1, 1, 1, 1};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0xFF, "11111111 → 0xFF");
    }

    /* Test 4: İlk bit 1, geri kalan 0
       little-endian → 2^0 = 1 */
    {
        uint8_t bits[8] = {1, 0, 0, 0, 0, 0, 0, 0};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x01, "10000000 (LE) → 0x01");
    }

    /* Test 5: Son bit 1, geri kalan 0
       little-endian → 2^7 = 128 */
    {
        uint8_t bits[8] = {0, 0, 0, 0, 0, 0, 0, 1};
        uint8_t result[1];
        bits_to_bytes(bits, 8, result);
        CHECK(result[0] == 0x80, "00000001 (LE) → 0x80");
    }

    /* Test 6: 2 byte — little-endian sıralaması doğru mu?
       byte[0]: 00000001 = 1
       byte[1]: 10000000 = 128 */
    {
        uint8_t bits[16] = {
            1,0,0,0,0,0,0,0,   /* byte[0] = 1   */
            0,0,0,0,0,0,0,1    /* byte[1] = 128 */
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
        uint8_t input[1]  = {139};
        uint8_t expected[8] = {1, 1, 0, 1, 0, 0, 0, 1};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "139 → {1,1,0,1,0,0,0,1}");
    }

    /* Test 2: 0x00 → hep sıfır */
    {
        uint8_t input[1]    = {0x00};
        uint8_t expected[8] = {0,0,0,0,0,0,0,0};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "0x00 → {0,0,0,0,0,0,0,0}");
    }

    /* Test 3: 0xFF → hep bir */
    {
        uint8_t input[1]    = {0xFF};
        uint8_t expected[8] = {1,1,1,1,1,1,1,1};
        uint8_t result[8];
        bytes_to_bits(input, 1, result);
        CHECK_BYTES(result, expected, 8, "0xFF → {1,1,1,1,1,1,1,1}");
    }
}

static void test_bits_bytes_roundtrip(void)
{
    section("Algoritma 3+4: Round-trip (bytes→bits→bytes)");

    /*
     * Round-trip testi: herhangi bir byte dizisi için
     * bytes_to_bits ardından bits_to_bytes orijinali vermeli.
     *
     * 32 byte (256 bit) ile test ediyoruz — polinom katsayı
     * sayısıyla aynı, kasıtlı.
     */
    uint8_t original[32];
    for (int i = 0; i < 32; i++) original[i] = (uint8_t)(i * 7 + 3);

    uint8_t bits[256];
    uint8_t recovered[32];

    bytes_to_bits(original, 32, bits);
    bits_to_bytes(bits, 256, recovered);

    CHECK_BYTES(recovered, original, 32,
                "bytes→bits→bytes round-trip (32 byte)");

    /* Ters yön: bits→bytes→bits */
    uint8_t original_bits[64];
    for (int i = 0; i < 64; i++) original_bits[i] = i % 2;

    uint8_t mid_bytes[8];
    uint8_t recovered_bits[64];

    bits_to_bytes(original_bits, 64, mid_bytes);
    bytes_to_bits(mid_bytes, 8, recovered_bits);

    CHECK_BYTES(recovered_bits, original_bits, 64,
                "bits→bytes→bits round-trip (64 bit)");
}

/* ═══════════════════════════════════════════════════════
   KATMAN 1 — ByteEncode (Alg 5) & ByteDecode (Alg 6)
   ═══════════════════════════════════════════════════════
   Test stratejisi:
   1. d=1: bit düzeyinde encode/decode (en basit durum)
   2. d=12: mod q davranışı — 3329'dan büyük değer encode
            edilip decode edilince farklı çıkmalı
   3. Round-trip: decode(encode(f)) == f (d<12 için)
*/

static void test_byte_encode_decode(void)
{
    section("Algoritma 5+6: ByteEncode / ByteDecode");

    /* Test 1: d=1, tüm katsayılar 0
       Beklenen: 32 byte, hepsi 0x00 */
    {
        poly f;
        memset(&f, 0, sizeof(poly));
        uint8_t out[32];
        byte_encode(&f, 1, out);

        uint8_t expected[32];
        memset(expected, 0, 32);
        CHECK_BYTES(out, expected, 32, "d=1, hep-0 encode → 32 sıfır byte");
    }

    /* Test 2: d=1, tüm katsayılar 1
       256 bit → 32 byte, hepsi 0xFF
       little-endian: her byte'ın 8 biti = {1,1,1,1,1,1,1,1} → 0xFF */
    {
        poly f;
        for (int i = 0; i < MLKEM_N; i++) f.coeffs[i] = 1;
        uint8_t out[32];
        byte_encode(&f, 1, out);

        uint8_t expected[32];
        memset(expected, 0xFF, 32);
        CHECK_BYTES(out, expected, 32, "d=1, hep-1 encode → 32 × 0xFF");
    }

    /* Test 3: d=1 round-trip
       f encode et, sonra decode et, orijinal f gelmeli */
    {
        poly f, recovered;
        for (int i = 0; i < MLKEM_N; i++) f.coeffs[i] = i % 2;

        uint8_t buf[32];
        byte_encode(&f, 1, buf);
        byte_decode(buf, 1, &recovered);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (f.coeffs[i] != recovered.coeffs[i]) { ok = 0; break; }
        }
        CHECK(ok, "d=1 round-trip: encode → decode == orijinal");
    }

    /* Test 4: d=12 round-trip (geçerli değerler için)
       Katsayılar [0, q-1] aralığında → encode → decode aynı gelmeli */
    {
        poly f, recovered;
        for (int i = 0; i < MLKEM_N; i++)
            f.coeffs[i] = (int16_t)(i * 13 % MLKEM_Q);

        uint8_t buf[384];
        byte_encode(&f, 12, buf);
        byte_decode(buf, 12, &recovered);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (f.coeffs[i] != recovered.coeffs[i]) { ok = 0; break; }
        }
        CHECK(ok, "d=12 round-trip: [0,q-1] değerler");
    }

    /* Test 5: d=12 mod q davranışı
       Eğer bir katsayı q=3329 ise → encode → decode → 0 gelmeli
       Çünkü ByteDecode_12 mod q alıyor ve 3329 mod 3329 = 0.
       Bu aynı zamanda Encaps'taki "modulus check"in temelini oluşturuyor. */
    {
        poly f, recovered;
        memset(&f, 0, sizeof(poly));
        f.coeffs[0] = MLKEM_Q;   /* 3329 — geçersiz, ama bakalım ne olur */

        uint8_t buf[384];
        byte_encode(&f, 12, buf);
        byte_decode(buf, 12, &recovered);

        /*
         * 3329 mod 3329 = 0 — yani recovered.coeffs[0] = 0 olmalı.
         * Bu da encode(decode(x)) != x durumunu gösteriyor.
         * ML-KEM.Encaps'taki modulus check tam bunu yakalıyor.
         */
        CHECK(recovered.coeffs[0] == 0,
              "d=12: katsayı=q=3329 encode→decode → 0 (mod q davranışı)");
    }

    /* Test 6: d=4, d=10 round-trip
       Parametre setimizde dv=4 ve du=10 kullanılıyor */
    {
        poly f4, r4, f10, r10;
        for (int i = 0; i < MLKEM_N; i++) {
            f4.coeffs[i]  = (int16_t)(i % 16);      /* [0, 15]   = [0, 2^4-1] */
            f10.coeffs[i] = (int16_t)(i % 1024);    /* [0, 1023] = [0, 2^10-1] */
        }

        uint8_t buf4[128], buf10[320];
        byte_encode(&f4,  4,  buf4);
        byte_encode(&f10, 10, buf10);
        byte_decode(buf4,  4,  &r4);
        byte_decode(buf10, 10, &r10);

        int ok4 = 1, ok10 = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (f4.coeffs[i]  != r4.coeffs[i])  ok4  = 0;
            if (f10.coeffs[i] != r10.coeffs[i]) ok10 = 0;
        }
        CHECK(ok4,  "d=4  round-trip (dv parametresi)");
        CHECK(ok10, "d=10 round-trip (du parametresi)");
    }
}

/* ═══════════════════════════════════════════════════════
   KATMAN 2 — NTT (Alg 9), NTT^-1 (Alg 10)
   ═══════════════════════════════════════════════════════
   NTT için en kritik test: round-trip.
   NTT^-1(NTT(f)) == f olmazsa tüm çarpımlar yanlış gider.

   Ayrıca:
   - Lineerlik: NTT(f+g) == NTT(f) + NTT(g)
   - Sıfır polinom: NTT(0) == 0
   - Sabit polinom: özel bir durum
*/

/* İki poly'nin eşit olup olmadığını kontrol eder */
static int poly_equal(const poly *a, const poly *b)
{
    for (int i = 0; i < MLKEM_N; i++)
        if (a->coeffs[i] != b->coeffs[i]) return 0;
    return 1;
}

/* Katsayı katsayı toplama, mod q */
static void poly_add(poly *out, const poly *a, const poly *b)
{
    for (int i = 0; i < MLKEM_N; i++) {
        int32_t s = (int32_t)a->coeffs[i] + b->coeffs[i];
        out->coeffs[i] = (int16_t)(s % MLKEM_Q);
    }
}

static void test_ntt(void)
{
    section("Algoritma 9+10: NTT / NTT^-1");

    /* Test 1: Sıfır polinom
       NTT(0) = 0 ve NTT^-1(0) = 0 olmalı */
    {
        poly zero, result;
        memset(&zero, 0, sizeof(poly));
        ntt_forward(&result, &zero);
        CHECK(poly_equal(&result, &zero), "NTT(0) = 0");

        ntt_inverse(&result, &zero);
        CHECK(poly_equal(&result, &zero), "NTT^-1(0) = 0");
    }

    /* Test 2: Round-trip — en kritik test
       Rastgele bir polinom için NTT^-1(NTT(f)) == f
       Birden fazla farklı polinom ile dene */
    {
        /* Polinom 1: katsayılar 0, 1, 2, ... 255 */
        {
            poly f, f_hat, recovered;
            for (int i = 0; i < MLKEM_N; i++)
                f.coeffs[i] = (int16_t)(i % MLKEM_Q);

            ntt_forward(&f_hat, &f);
            ntt_inverse(&recovered, &f_hat);

            CHECK(poly_equal(&f, &recovered),
                  "Round-trip: f[i]=i — NTT^-1(NTT(f)) == f");
        }

        /* Polinom 2: sadece sabit terim 1, geri kalan 0
           f = 1 (sabit polinom) */
        {
            poly f, f_hat, recovered;
            memset(&f, 0, sizeof(poly));
            f.coeffs[0] = 1;

            ntt_forward(&f_hat, &f);
            ntt_inverse(&recovered, &f_hat);

            CHECK(poly_equal(&f, &recovered),
                  "Round-trip: f=1 (sabit) — NTT^-1(NTT(f)) == f");
        }

        /* Polinom 3: rastgele gibi — asal sayı modulo ile oluştur */
        {
            poly f, f_hat, recovered;
            for (int i = 0; i < MLKEM_N; i++)
                f.coeffs[i] = (int16_t)((i * 1337 + 42) % MLKEM_Q);

            ntt_forward(&f_hat, &f);
            ntt_inverse(&recovered, &f_hat);

            CHECK(poly_equal(&f, &recovered),
                  "Round-trip: f[i]=(i*1337+42) mod q");
        }
    }

    /* Test 3: Lineerlik — NTT(f + g) == NTT(f) + NTT(g)
       NTT bir lineer dönüşüm, bu özelliği sağlamak zorunda. */
    {
        poly f, g, f_plus_g;
        poly ntt_f, ntt_g, ntt_sum, ntt_of_sum;

        for (int i = 0; i < MLKEM_N; i++) {
            f.coeffs[i] = (int16_t)(i % MLKEM_Q);
            g.coeffs[i] = (int16_t)((MLKEM_N - i) % MLKEM_Q);
        }

        poly_add(&f_plus_g, &f, &g);

        ntt_forward(&ntt_f, &f);
        ntt_forward(&ntt_g, &g);
        ntt_forward(&ntt_of_sum, &f_plus_g);

        poly_add(&ntt_sum, &ntt_f, &ntt_g);

        CHECK(poly_equal(&ntt_of_sum, &ntt_sum),
              "Lineerlik: NTT(f+g) == NTT(f) + NTT(g)");
    }

    /* Test 4: NTT çıktısı q'dan küçük mü?
       Her katsayı [0, q-1] aralığında olmalı */
    {
        poly f, f_hat;
        for (int i = 0; i < MLKEM_N; i++)
            f.coeffs[i] = (int16_t)(i * 97 % MLKEM_Q);

        ntt_forward(&f_hat, &f);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (f_hat.coeffs[i] < 0 || f_hat.coeffs[i] >= MLKEM_Q) {
                ok = 0;
                printf("         Aralık dışı: coeffs[%d] = %d\n",
                       i, f_hat.coeffs[i]);
                break;
            }
        }
        CHECK(ok, "NTT çıktısı: tüm katsayılar [0, q-1] aralığında");
    }
}

/* ═══════════════════════════════════════════════════════
   KATMAN 2B — MultiplyNTTs (Alg 11) & BaseCaseMultiply (Alg 12)
   ═══════════════════════════════════════════════════════
   Test stratejisi:
   1. f * 0 = 0
   2. f * 1 = f  (birim eleman)
   3. Değişme yasası: f*g == g*f
   4. Dağılma yasası: f*(g+h) == f*g + f*h
   5. BaseCaseMultiply doğrudan: bilinen çarpım
*/

static void test_multiply_ntts(void)
{
    section("Algoritma 11+12: MultiplyNTTs / BaseCaseMultiply");

    /* Test 1: BaseCaseMultiply — elle hesaplanmış örnek
       (1 + 0·X)(2 + 0·X) mod (X² - γ) = 2 + 0·X
       a0=1, a1=0, b0=2, b1=0, gamma=17 (GAMMAS[0])
       c0 = 1*2 + 0*0*17 = 2
       c1 = 1*0 + 0*2   = 0 */
    {
        int16_t c0, c1;
        base_case_multiply(&c0, &c1, 1, 0, 2, 0, 17);
        CHECK(c0 == 2 && c1 == 0,
              "BaseCaseMultiply: (1)(2) mod (X²-17) = 2");
    }

    /* Test 2: BaseCaseMultiply — X * X = γ (mod X² - γ)
       X·X = X² = γ  →  c0 = γ, c1 = 0
       a0=0, a1=1, b0=0, b1=1, gamma=17
       c0 = 0*0 + 1*1*17 = 17
       c1 = 0*1 + 1*0   = 0 */
    {
        int16_t c0, c1;
        base_case_multiply(&c0, &c1, 0, 1, 0, 1, 17);
        CHECK(c0 == 17 && c1 == 0,
              "BaseCaseMultiply: X * X = 17 (mod X²-17)");
    }

    /* Test 3: f * 0 = 0  (NTT alanında) */
    {
        poly f, zero, result;
        memset(&zero, 0, sizeof(poly));
        for (int i = 0; i < MLKEM_N; i++)
            f.coeffs[i] = (int16_t)(i % MLKEM_Q);

        multiply_ntts(&result, &f, &zero);
        CHECK(poly_equal(&result, &zero), "f * 0 = 0");
    }

    /* Test 4: Değişme yasası f*g == g*f
       NTT çarpımı değişmeli olmalı. */
    {
        poly f, g, f_hat, g_hat, fg, gf;

        for (int i = 0; i < MLKEM_N; i++) {
            f.coeffs[i] = (int16_t)(i * 3 % MLKEM_Q);
            g.coeffs[i] = (int16_t)(i * 7 % MLKEM_Q);
        }

        ntt_forward(&f_hat, &f);
        ntt_forward(&g_hat, &g);

        multiply_ntts(&fg, &f_hat, &g_hat);
        multiply_ntts(&gf, &g_hat, &f_hat);

        CHECK(poly_equal(&fg, &gf), "Değişme: f*g == g*f");
    }

    /* Test 5: Dağılma yasası f*(g+h) == f*g + f*h */
    {
        poly f, g, h;
        poly f_hat, g_hat, h_hat;
        poly g_plus_h_hat, fg_hat, fh_hat;
        poly f_times_sum, fg_plus_fh;

        for (int i = 0; i < MLKEM_N; i++) {
            f.coeffs[i] = (int16_t)(i * 5 % MLKEM_Q);
            g.coeffs[i] = (int16_t)(i * 3 % MLKEM_Q);
            h.coeffs[i] = (int16_t)(i * 2 % MLKEM_Q);
        }

        ntt_forward(&f_hat, &f);
        ntt_forward(&g_hat, &g);
        ntt_forward(&h_hat, &h);

        poly_add(&g_plus_h_hat, &g_hat, &h_hat);

        multiply_ntts(&f_times_sum, &f_hat, &g_plus_h_hat);
        multiply_ntts(&fg_hat, &f_hat, &g_hat);
        multiply_ntts(&fh_hat, &f_hat, &h_hat);
        poly_add(&fg_plus_fh, &fg_hat, &fh_hat);

        CHECK(poly_equal(&f_times_sum, &fg_plus_fh),
              "Dağılma: f*(g+h) == f*g + f*h");
    }

    /* Test 6: Çarpım çıktısı q'dan küçük mü? */
    {
        poly f, g, f_hat, g_hat, result;
        for (int i = 0; i < MLKEM_N; i++) {
            f.coeffs[i] = (int16_t)(i * 13 % MLKEM_Q);
            g.coeffs[i] = (int16_t)(i * 17 % MLKEM_Q);
        }
        ntt_forward(&f_hat, &f);
        ntt_forward(&g_hat, &g);
        multiply_ntts(&result, &f_hat, &g_hat);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (result.coeffs[i] < 0 || result.coeffs[i] >= MLKEM_Q) {
                ok = 0; break;
            }
        }
        CHECK(ok, "MultiplyNTTs çıktısı: tüm katsayılar [0, q-1]");
    }
}

/* ═══════════════════════════════════════════════════════
   KATMAN 3 — SampleNTT (Alg 7) & SamplePolyCBD (Alg 8)
   ═══════════════════════════════════════════════════════
   KAT vektörü olmadan doğruluğu tam bilemeyiz (o aşamaya
   geçince ekleyeceğiz). Şimdilik yapısal testler:
   1. Çıktı her zaman [0, q-1] aralığında mı?
   2. Determinizm: aynı girdi → aynı çıktı
   3. Bağımsızlık: farklı (i,j) → farklı sonuç
   4. CBD: katsayılar [-η, η] aralığında mı?
*/

static void test_sample_ntt(void)
{
    section("Algoritma 7: SampleNTT");

    uint8_t rho[32];
    for (int i = 0; i < 32; i++) rho[i] = (uint8_t)(i + 1);

    /* Test 1: Çıktı katsayıları [0, q-1] aralığında mı? */
    {
        poly a_hat;
        sample_ntt(rho, 0, 0, &a_hat);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            if (a_hat.coeffs[i] < 0 || a_hat.coeffs[i] >= MLKEM_Q) {
                ok = 0;
                printf("         Aralık dışı: coeffs[%d] = %d\n",
                       i, a_hat.coeffs[i]);
                break;
            }
        }
        CHECK(ok, "SampleNTT çıktısı: tüm katsayılar [0, q-1]");
    }

    /* Test 2: Determinizm — aynı (rho, i, j) → aynı polinom */
    {
        poly a1, a2;
        sample_ntt(rho, 1, 0, &a1);
        sample_ntt(rho, 1, 0, &a2);
        CHECK(poly_equal(&a1, &a2),
              "Determinizm: aynı (rho,j,i) → aynı polinom");
    }

    /* Test 3: Farklı (j, i) → farklı sonuç
       Â[0][1] ≠ Â[1][0] olmalı (overwhelmingly probable) */
    {
        poly a00, a01, a10, a11;
        sample_ntt(rho, 0, 0, &a00);
        sample_ntt(rho, 1, 0, &a01);   /* col=1, row=0 */
        sample_ntt(rho, 0, 1, &a10);   /* col=0, row=1 */
        sample_ntt(rho, 1, 1, &a11);

        /* Hepsi birbirinden farklı olmalı */
        CHECK(!poly_equal(&a00, &a01), "Â[0][0] ≠ Â[0][1]");
        CHECK(!poly_equal(&a00, &a10), "Â[0][0] ≠ Â[1][0]");
        CHECK(!poly_equal(&a01, &a10), "Â[0][1] ≠ Â[1][0] (transpoz farklı)");
        CHECK(!poly_equal(&a00, &a11), "Â[0][0] ≠ Â[1][1]");
    }

    /* Test 4: Farklı rho → farklı sonuç */
    {
        uint8_t rho2[32];
        for (int i = 0; i < 32; i++) rho2[i] = (uint8_t)(255 - i);

        poly a1, a2;
        sample_ntt(rho,  0, 0, &a1);
        sample_ntt(rho2, 0, 0, &a2);

        CHECK(!poly_equal(&a1, &a2), "Farklı ρ → farklı Â[0][0]");
    }
}

static void test_sample_poly_cbd(void)
{
    section("Algoritma 8: SamplePolyCBD");

    /* Test 1: Katsayılar [-η, η] aralığında mı?
       Mod q alındıktan sonra: [0, η] veya [q-η, q-1] */
    {
        uint8_t sigma[32] = {0};
        uint8_t prf_out[MLKEM_PRF_ETA1_BYTES];
        prf_eta1(sigma, 0, prf_out);

        poly f;
        sample_poly_cbd(prf_out, MLKEM_ETA1, &f);

        int ok = 1;
        for (int i = 0; i < MLKEM_N; i++) {
            int16_t c = f.coeffs[i];
            /* c ∈ [0, η] ∪ [q-η, q-1]  yani küçük veya q'ya yakın */
            int small    = (c >= 0 && c <= MLKEM_ETA1);
            int near_q   = (c >= MLKEM_Q - MLKEM_ETA1 && c < MLKEM_Q);
            if (!small && !near_q) {
                ok = 0;
                printf("         Beklenen küçük katsayı, "
                       "coeffs[%d] = %d\n", i, c);
                break;
            }
        }
        CHECK(ok, "CBD katsayıları küçük: [0,η] ∪ [q-η, q-1]");
    }

    /* Test 2: Determinizm */
    {
        uint8_t sigma[32] = {42};
        uint8_t prf1[MLKEM_PRF_ETA1_BYTES];
        uint8_t prf2[MLKEM_PRF_ETA1_BYTES];
        prf_eta1(sigma, 5, prf1);
        prf_eta1(sigma, 5, prf2);

        poly f1, f2;
        sample_poly_cbd(prf1, MLKEM_ETA1, &f1);
        sample_poly_cbd(prf2, MLKEM_ETA1, &f2);

        CHECK(poly_equal(&f1, &f2),
              "Determinizm: aynı PRF girdi → aynı polinom");
    }

    /* Test 3: Farklı N → farklı polinom */
    {
        uint8_t sigma[32] = {0};
        uint8_t prf0[MLKEM_PRF_ETA1_BYTES];
        uint8_t prf1[MLKEM_PRF_ETA1_BYTES];
        prf_eta1(sigma, 0, prf0);
        prf_eta1(sigma, 1, prf1);

        poly f0, f1;
        sample_poly_cbd(prf0, MLKEM_ETA1, &f0);
        sample_poly_cbd(prf1, MLKEM_ETA1, &f1);

        CHECK(!poly_equal(&f0, &f1),
              "Farklı N sayacı → farklı gürültü polinomu");
    }
}

/* ═══════════════════════════════════════════════════════
   KATMAN 4 — K-PKE.KeyGen (Alg 13)
   ═══════════════════════════════════════════════════════
   KAT olmadan şu testleri yapabiliriz:
   1. Çıktı boyutları doğru mu?
   2. Deterministik mi? (aynı d → aynı ek, dk)
   3. Farklı d → farklı anahtar
   4. ek içindeki ρ tutarlı mı?
      (ek'in son 32 byte'ı, G(d||k)'nın ilk 32 byte'ına == ρ)
   5. ek decode edilebiliyor mu? (modulus check)
*/

static void test_kpke_keygen(void)
{
    section("Algoritma 13: K-PKE.KeyGen");

    uint8_t d[32];
    for (int i = 0; i < 32; i++) d[i] = (uint8_t)(i + 1);

    uint8_t ek[MLKEM_EKPKE_BYTES];
    uint8_t dk[MLKEM_DKPKE_BYTES];

    /* Test 1: Boyutlar doğru mu?
       Bu derleme zamanında kontrol edilebilir, ama yine de gösterelim. */
    {
        kpke_keygen(d, ek, dk);
        CHECK(MLKEM_EKPKE_BYTES == 800,
              "ek_pke boyutu: 384*k+32 = 800 byte");
        CHECK(MLKEM_DKPKE_BYTES == 768,
              "dk_pke boyutu: 384*k = 768 byte");
    }

    /* Test 2: Determinizm — aynı d → aynı anahtar çifti */
    {
        uint8_t ek2[MLKEM_EKPKE_BYTES];
        uint8_t dk2[MLKEM_DKPKE_BYTES];
        kpke_keygen(d, ek2, dk2);

        CHECK_BYTES(ek, ek2, MLKEM_EKPKE_BYTES,
                    "Determinizm: aynı d → aynı ek_pke");
        CHECK_BYTES(dk, dk2, MLKEM_DKPKE_BYTES,
                    "Determinizm: aynı d → aynı dk_pke");
    }

    /* Test 3: Farklı d → farklı anahtar */
    {
        uint8_t d2[32];
        memcpy(d2, d, 32);
        d2[0] ^= 0xFF;   /* bir bit farklı */

        uint8_t ek3[MLKEM_EKPKE_BYTES];
        uint8_t dk3[MLKEM_DKPKE_BYTES];
        kpke_keygen(d2, ek3, dk3);

        CHECK(memcmp(ek, ek3, MLKEM_EKPKE_BYTES) != 0,
              "Farklı d → farklı ek_pke");
        CHECK(memcmp(dk, dk3, MLKEM_DKPKE_BYTES) != 0,
              "Farklı d → farklı dk_pke");
    }

    /* Test 4: ek içindeki ρ doğru mu?
       ek'in son 32 byte'ı = ρ = G(d||k) çıktısının ilk 32 byte'ı */
    {
        uint8_t G_in[33];
        memcpy(G_in, d, 32);
        G_in[32] = (uint8_t)MLKEM_K;

        uint8_t G_out[64];
        hash_G(G_in, 33, G_out);

        /* G_out[0..31] = ρ, G_out[32..63] = σ */
        uint8_t *rho_expected = G_out;
        uint8_t *rho_in_ek   = ek + MLKEM_POLYVECBYTES;   /* son 32 byte */

        CHECK_BYTES(rho_in_ek, rho_expected, 32,
                    "ek içindeki ρ, G(d||k) ile tutarlı");
    }

    /* Test 5: ek decode edilebiliyor mu? (modulus check)
       ByteDecode12(ByteEncode12(x)) == x olmalı — yani ek içindeki
       katsayılar zaten [0, q-1] aralığında.
       Bu, ML-KEM.Encaps'ın yapacağı kontrolün simülasyonu. */
    {
        uint8_t ek_copy[MLKEM_EKPKE_BYTES];
        memcpy(ek_copy, ek, MLKEM_EKPKE_BYTES);

        /* test = ByteEncode12(ByteDecode12(ek[0:384k])) */
        polyvec t_hat;
        polyvec_decode12(ek_copy, &t_hat);

        uint8_t reencoded[MLKEM_POLYVECBYTES];
        polyvec_encode12(&t_hat, reencoded);

        CHECK_BYTES(reencoded, ek, MLKEM_POLYVECBYTES,
                    "Modulus check: encode(decode(ek[0:768])) == ek[0:768]");
    }
}

/* ═══════════════════════════════════════════════════════
   main — Tüm testleri sırasıyla çalıştır
   ═══════════════════════════════════════════════════════ */
int main(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   ML-KEM-512  —  Test Paketi             ║\n");
    printf("║   Katman 0 → 4  (Alg 3-13)               ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    /* Katman 0: Temel bit/byte dönüşümleri */
    test_bits_to_bytes();
    test_bytes_to_bits();
    test_bits_bytes_roundtrip();

    /* Katman 1: Encode / Decode */
    test_byte_encode_decode();

    /* Katman 2: NTT */
    test_ntt();

    /* Katman 2B: NTT alanında çarpım */
    test_multiply_ntts();

    /* Katman 3: Örnekleme */
    test_sample_ntt();
    test_sample_poly_cbd();

    /* Katman 4: K-PKE.KeyGen */
    test_kpke_keygen();

    /* Özet */
    print_summary();

    return (tests_failed == 0) ? 0 : 1;
}
