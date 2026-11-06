#include "user_interface.h"
#include "ip_calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Вспомогательная функция вывода 32-битного числа в двоичном виде по октетам */
static void print_ip_binary(uint32_t val)
{
    for (int i = 31; i >= 0; i--) {
        /* Проверка i-го бита*/
        printf("%c", (val & (1U << i)) ? '1' : '0');
        if (i % 8 == 0 && i != 0) {
            printf("."); /* Разделитель октетов */
        } else if (i % 4 == 0 && i % 8 != 0) {
            printf(" "); /* Пробел между полубайтами для наглядности */
        }
    }
}

static void print_ip_decimal(uint32_t ip)
{
    printf("%u.%u.%u.%u",
           (ip >> 24) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >> 8)  & 0xFF,
           ip & 0xFF);
}

static void read_string(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) != NULL) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
    } else {
        buf[0] = '\0';
    }
}

void run_ipv4_simulator_ui()
{
    char ip_str[64];
    char mask_str[64];
    char n_str[64];

    uint32_t gateway_ip;
    uint32_t mask;
    int cidr;
    size_t n_packets;

    srand((unsigned int)time(NULL));

    printf("========================================================\n");
    printf("   ИМИТАТОР ПРИНЯТИЯ РЕШЕНИЙ IPV4-МАРШРУТИЗАЦИИ    \n");
    printf("========================================================\n");

    /* Ввод IP шлюза */
    while (1) {
        printf("Введите IP адрес шлюза (например, 192.168.1.10): ");
        read_string(ip_str, sizeof(ip_str));
        if (parse_ip(ip_str, &gateway_ip) == 0) {
            break;
        }
        printf("Некорректный IP адрес! Попробуйте снова.\n");
    }

    /* Ввод маски подсети */
    while (1) {
        printf("Введите маску подсети (например, 255.255.255.0 или /24): ");
        read_string(mask_str, sizeof(mask_str));
        if (parse_mask(mask_str, &mask, &cidr) == 0) {
            break;
        }
        printf("Некорректная маска подсети! Попробуйте снова.\n");
    }

    /* Ввод количества пакетов */
    while (1) {
        printf("Введите количество пакетов для имитации N (например, 100000): ");
        read_string(n_str, sizeof(n_str));
        long val = atol(n_str);
        if (val > 0) {
            n_packets = (size_t)val;
            break;
        }
        printf("Количество пакетов должно быть положительным числом!\n");
    }

    /* Запуск вычислений */
    SimulationStats stats;
    run_simulation(gateway_ip, mask, cidr, n_packets, &stats);

    /* Вывод структуры подсети с битовыми представлениями (как в слайде 155-156) */
    printf("\n--- Параметры сети ---\n");
    printf("IP шлюза:   "); print_ip_decimal(stats.gateway_ip);
    printf("  \tBIN: "); print_ip_binary(stats.gateway_ip); printf("\n");

    printf("Маска (/%d): ", stats.cidr_prefix); print_ip_decimal(stats.mask);
    printf("  \tBIN: "); print_ip_binary(stats.mask); printf("\n");

    printf("Сеть:       "); print_ip_decimal(stats.net_address);
    printf("  \tBIN: "); print_ip_binary(stats.net_address); printf("\n");

    printf("Broadcast:  "); print_ip_decimal(stats.broadcast_address);
    printf("  \tBIN: "); print_ip_binary(stats.broadcast_address); printf("\n");

    /* Вывод результатов имитации */
    printf("\n--- Результаты имитации обработки %zu пакетов ---\n", stats.total_packets);
    printf("1. Пакетов в «свою» подсеть: %zu шт. (%.4f %%)\n",
           stats.same_net_count, stats.same_net_percent);
    printf("2. Пакетов в «чужие» сети:  %zu шт. (%.4f %%)\n",
           stats.other_net_count, stats.other_net_percent);

    printf("\n--- Теоретический расчет ---\n");
    printf("Размер подсети: 2^(32 - %d) = %u адресов\n",
           stats.cidr_prefix,
           stats.cidr_prefix == 0 ? 0xFFFFFFFFU : (1U << (32 - stats.cidr_prefix)));
    printf("Теоретическая вероятность случайного попадания: %.8f %%\n",
           stats.theoretical_percent);
}