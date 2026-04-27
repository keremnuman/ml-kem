#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define ITERATIONS 200000000 // 100 Milyon işlem
#define Q 4353               // ML-KEM Modülü

// Geleneksel yöntem: Bölme/Kalan operatörü
// Bu işlem sayıların büyüklüğüne göre (işlemci mimarisine bağlı olarak)
// değişkenlik gösterebilir veya sabit olsa bile yavaştır.
uint16_t classic_mod(uint32_t a)
{
    return a % Q;
}

// Basitleştirilmiş Montgomery/Barrett tarzı "Hızlı" ve "Sabit Zamanlı" mantık
// Not: Gerçek Montgomery tam sayı dönüşümü gerektirir ama hız ve sabit zaman
// prensibini görmek için bu "bölmesiz" yaklaşım yeterlidir.
uint16_t fast_reduce(uint32_t a)
{
    // 3329 için yaklaşık bir sabit çarpan (v = 2^32 / Q)
    uint64_t v = 2874324;
    uint32_t t = (uint32_t)((v * a) >> 32);
    uint16_t res = a - t * Q;
    return res >= Q ? res - Q : res;
}

int main()
{
    clock_t start, end;
    double cpu_time_used;
    volatile uint16_t sink; // Derleyici optimizasyonunu engellemek için

    printf("Deney basliyor (%d iterasyon)...\n\n", ITERATIONS);

    // --- 1. TEST: KLASIK MOD (%) ---
    start = clock();
    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        sink = classic_mod(i);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Klasik Mod (%%) suresi:   %f saniye\n", cpu_time_used);

    // --- 2. TEST: HIZLI REDUCE (SABIT ZAMANLI) ---
    start = clock();
    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        sink = fast_reduce(i);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Hizli/Sabit Zaman suresi: %f saniye\n", cpu_time_used);

    return 0;
}