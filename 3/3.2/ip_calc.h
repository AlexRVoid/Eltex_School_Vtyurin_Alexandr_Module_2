#ifndef IP_CALC_H_
#define IP_CALC_H_

#include <stdint.h>
#include <stddef.h>

/* Структура результатов имитационного моделирования */
typedef struct SimulationStats{
    uint32_t gateway_ip;        /* IP шлюза */
    uint32_t mask;              /* Маска подсети */
    uint32_t net_address;       /* Адрес подсети */
    uint32_t broadcast_address; /* Широковещательный адрес */
    int cidr_prefix;            /* Длина префикса маски (0..32) */

    size_t total_packets;       /* Общее число сгенерированных пакетов N */
    size_t same_net_count;      /* Число пакетов своей сети */
    size_t other_net_count;     /* Число пакетов чужой сети */

    double same_net_percent;    /* Экспериментальный процент попаданий */
    double other_net_percent;   /* Процент чужих сетей */
    double theoretical_percent; /* Теоретическая вероятность попадания в % */
} SimulationStats;

/* Парсинг IP-адреса из строки "A.B.C.D" в uint32_t. Возврат: 0 - успех, -1 - ошибка */
int parse_ip(const char *str, uint32_t *out_ip);

/* 
 * Парсинг маски: поддерживает форматы "255.255.255.0", "/24" или "24".
 * Записывает маску в uint32_t и длину префикса в out_cidr.
 * Возврат: 0 - успех, -1 - ошибка.
 */
int parse_mask(const char *str, uint32_t *out_mask, int *out_cidr);

/* Перевод префикса CIDR (0..32) в маску uint32_t через побитовые сдвиги */
uint32_t cidr_to_mask(int cidr);

/* Проверка: является ли сгенерированный IP адресом "своей" подсети (Лекция 3, слайд 157) */
int is_same_subnet(uint32_t gateway_ip, uint32_t target_ip, uint32_t mask);

/* 
 * Генерация случайного 32-битного IPv4 адреса.
 */
uint32_t generate_random_ip(void);

/* 
 * Основная функция симуляции:
 * Генерирует N пакетов, проверяет каждый через побитовые операции и заполняет stats.
 */
int run_simulation(uint32_t gateway_ip, uint32_t mask, int cidr, size_t n, SimulationStats *stats);

#endif /* IP_CALC_H_ */