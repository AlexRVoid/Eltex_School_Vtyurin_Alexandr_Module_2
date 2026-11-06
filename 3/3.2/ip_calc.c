#include "ip_calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t cidr_to_mask(int cidr)
{
    if (cidr <= 0) return 0x00000000U;
    if (cidr >= 32) return 0xFFFFFFFFU;
    // 
     * Сдвиг единицы на (32 - cidr) разрядов влево, вычитание 1 (дает инвертированные биты хоста)
     * и побитовое отрицание '~' формирует непрерывную цепочку единиц (Лекция 3, слайд 143, 151)
     
    return ~((1U << (32 - cidr)) - 1U);
}

static int mask_to_cidr(uint32_t mask)
{
    int count = 0;
    while (mask & 0x80000000U) {
        count++;
        mask <<= 1;
    }
    return count;
}

static int is_valid_mask(uint32_t mask)
{
    if (mask == 0) return 1;
    uint32_t inverted = ~mask;
    // У правильной маски инвертированные биты хоста имеют вид 00...0011...11 
    return ((inverted + 1U) & inverted) == 0;
}

int parse_ip(const char *str, uint32_t *out_ip)
{
    if (!str || !out_ip) return -1;

    unsigned int a, b, c, d;
    char tail;
    if (sscanf(str, "%u.%u.%u.%u%c", &a, &b, &c, &d, &tail) != 4) {
        return -1;
    }

    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return -1;
    }

    // Упаковка 4 октетов в один uint32_t через сдвиги и побитовое ИЛИ (Лекция 3, слайды 144, 148) 
    *out_ip = ((uint32_t)a << 24) |
              ((uint32_t)b << 16) |
              ((uint32_t)c << 8)  |
              (uint32_t)d;
    return 0;
}

int parse_mask(const char *str, uint32_t *out_mask, int *out_cidr)
{
    if (!str || !out_mask || !out_cidr) return -1;

    // Вариант 1: Формат CIDR с префиксом '/' (например, "/24") или просто число ("24") 
    const char *p = str;
    if (*p == '/') p++;

    char *endptr;
    long val = strtol(p, &endptr, 10);
    if (*p != '\0' && *endptr == '\0') {
        if (val < 0 || val > 32) return -1;
        *out_cidr = (int)val;
        *out_mask = cidr_to_mask(*out_cidr);
        return 0;
    }

    // Вариант 2: Формат "255.255.255.0" 
    uint32_t raw_mask;
    if (parse_ip(str, &raw_mask) == 0) {
        if (!is_valid_mask(raw_mask)) {
            return -1; // Невалидная маска (например, разрыв между единицами) 
        }
        *out_mask = raw_mask;
        *out_cidr = mask_to_cidr(raw_mask);
        return 0;
    }

    return -1;
}

int is_same_subnet(uint32_t gateway_ip, uint32_t target_ip, uint32_t mask)
{
    // 
     * Алгоритм из Лекции 3, слайды 155-157:
     * 1. Адрес своей сети: gateway_ip & mask
     * 2. Адрес сети назначения: target_ip & mask
     * 3. Сравнение: (Net_Src ^ Net_Dst) == 0 (слайд 157)
     
    uint32_t net_src = gateway_ip & mask;
    uint32_t net_dst = target_ip & mask;
    uint32_t result = net_src ^ net_dst;

    return (result == 0);
}

uint32_t generate_random_ip(void)
{
    // 
     * rand() возвращает значение до RAND_MAX (минимум 15-31 бит).
     * Собираем полное 32-битное число сдвигами и исключающим ИЛИ:
     
    uint32_t r1 = (uint32_t)rand();
    uint32_t r2 = (uint32_t)rand();
    return (r1 << 16) ^ r2;
}

int run_simulation(uint32_t gateway_ip, uint32_t mask, int cidr, size_t n, SimulationStats *stats)
{
    if (!stats || n == 0) return -1;

    stats->gateway_ip = gateway_ip;
    stats->mask = mask;
    stats->cidr_prefix = cidr;
    stats->net_address = gateway_ip & mask;
    stats->broadcast_address = stats->net_address | (~mask);
    stats->total_packets = n;
    stats->same_net_count = 0;
    stats->other_net_count = 0;

    for (size_t i = 0; i < n; i++) {
        uint32_t dest_ip = generate_random_ip();
        if (is_same_subnet(gateway_ip, dest_ip, mask)) {
            stats->same_net_count++;
        } else {
            stats->other_net_count++;
        }
    }

    stats->same_net_percent = ((double)stats->same_net_count / (double)n) * 100.0;
    stats->other_net_percent = ((double)stats->other_net_count / (double)n) * 100.0;

    // 
     * Теоретическая вероятность: (2^(32 - cidr)) / (2^32) = 1 / (2^cidr)
     * В процентах: 100.0 / (2^cidr)
     
    double denom = 1.0;
    for (int i = 0; i < cidr; i++) {
        denom *= 2.0;
    }
    stats->theoretical_percent = 100.0 / denom;

    return 0;
}