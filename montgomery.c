#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define ITERATIONS 200000000
#define Q 3329

uint16_t classic_mod(uint32_t a)
{
    return a % Q;
}

uint16_t fast_reduce(uint32_t a)
{
    uint64_t v = 2874324;
    uint32_t t = (uint32_t)((v * a) >> 32);
    uint16_t res = a - t * Q;
    return res >= Q ? res - Q : res;
}

int main()
{
    clock_t start, end;
    double cpu_time_used;
    volatile uint16_t sink;
    start = clock();
    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        sink = classic_mod(i);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Klasik Mod (%%) suresi:   %f saniye\n", cpu_time_used);

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