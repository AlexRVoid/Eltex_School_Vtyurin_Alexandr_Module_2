#include "perm_calc.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void mode_to_symbolic(mode_t mode, char *out_str)
{
    const char chars[] = "rwxrwxrwx";
    for (int i = 8; i >= 0; i--) {
        /* Проверка i-го бита (Лекция 3, слайд 82) */
        if (mode & (1U << i)) {
            out_str[8 - i] = chars[8 - i];
        } else {
            out_str[8 - i] = '-';
        }
    }
    out_str[9] = '\0';
}

void mode_to_octal(mode_t mode, char *out_str)
{
    mode &= PERM_MASK;
    out_str[0] = '0';
    out_str[1] = (char)('0' + ((mode >> 6) & 07));
    out_str[2] = (char)('0' + ((mode >> 3) & 07));
    out_str[3] = (char)('0' + (mode & 07));
    out_str[4] = '\0';
}

void mode_to_binary(mode_t mode, char *out_str)
{
    /* Формат: "111 101 100" (9 бит с пробелами по триадам u/g/o) */
    int idx = 0;
    for (int i = 8; i >= 0; i--) {
        out_str[idx++] = (mode & (1U << i)) ? '1' : '0';
        if (i == 6 || i == 3) {
            out_str[idx++] = ' '; /* Разделитель триад */
        }
    }
    out_str[idx] = '\0';
}

static int parse_octal(const char *str, mode_t *out_mode)
{
    const char *p = str;
    if (*p == '0') p++; /* Пропуск ведущего нуля (например, в 0755) */

    if (strlen(p) != 3) return -1;

    mode_t res = 0;
    for (int i = 0; i < 3; i++) {
        if (p[i] < '0' || p[i] > '7') return -1;
        res = (res << 3) | (mode_t)(p[i] - '0');
    }

    *out_mode = res & PERM_MASK;
    return 0;
}

static int parse_symbolic(const char *str, mode_t *out_mode)
{
    if (strlen(str) != 9) return -1;

    mode_t res = 0;
    const char expected[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++) {
        if (str[i] == expected[i]) {
            res |= (1U << (8 - i)); /* Установка бита (Лекция 3, слайд 80) */
        } else if (str[i] == '-') {
            /* Бит остаётся нулевым */
        } else {
            return -1; /* Недопустимый символ */
        }
    }

    *out_mode = res;
    return 0;
}

int parse_initial_mode(const char *str, mode_t *out_mode)
{
    if (!str || !out_mode) return -1;

    /* Сначала пробуем разобрать как восьмеричное число */
    if (parse_octal(str, out_mode) == 0) {
        return 0;
    }

    /* Иначе пробуем разобрать как 9-символьную строку rwxr-xr-- */
    if (parse_symbolic(str, out_mode) == 0) {
        return 0;
    }

    return -1;
}

int get_file_mode(const char *filename, mode_t *out_mode)
{
    if (!filename || !out_mode) return -1;

    struct stat st;
    if (stat(filename, &st) != 0) {
        return -1; /* Ошибка: файл не найден или нет доступа */
    }

    /* Выделяем ровно 9 нижних бит прав доступа ugo (Лекция 3, слайд 88) */
    *out_mode = (mode_t)(st.st_mode & PERM_MASK);
    return 0;
}

/* Применение одного символьного блока chmod, например "g+w", "u-x", "a=rw" */
static int apply_single_clause(mode_t *mode, const char *clause)
{
    const char *p = clause;
    mode_t who_mask = 0;

    /* 1. Считываем категорию: u, g, o, a (Лекция 3, слайд 89) */
    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
        if (*p == 'u') who_mask |= 0700;
        if (*p == 'g') who_mask |= 0070;
        if (*p == 'o') who_mask |= 0007;
        if (*p == 'a') who_mask |= 0777;
        p++;
    }

    /* Если цель не указана явно (например, "+x"), по стандарту chmod применяется к 'a' */
    if (who_mask == 0) {
        who_mask = 0777;
    }

    /* 2. Считываем оператор: +, -, = (Лекция 3, слайд 89) */
    char op = *p++;
    if (op != '+' && op != '-' && op != '=') {
        return -1;
    }

    /* 3. Считываем права: комбинация r, w, x */
    mode_t perm_bits_base = 0;
    while (*p != '\0') {
        if (*p == 'r') perm_bits_base |= 0444;      /* чтение для всех */
        else if (*p == 'w') perm_bits_base |= 0222; /* запись для всех */
        else if (*p == 'x') perm_bits_base |= 0111; /* исполнение для всех */
        else return -1; /* Неизвестный флаг права */
        p++;
    }

    /* Маска прав, ограниченная выбранными категориями u/g/o */
    mode_t affected_perms = perm_bits_base & who_mask;

    /* 4. Применяем побитовую операцию (Лекция 3, слайды 80, 81, 89) */
    if (op == '+') {
        *mode |= affected_perms; /* Побитовое ИЛИ */
    } else if (op == '-') {
        *mode &= ~affected_perms; /* Побитовое И с инверсией */
    } else if (op == '=') {
        *mode &= ~who_mask;       /* Сбрасываем все права в категории */
        *mode |= affected_perms;  /* Устанавливаем новые права */
    }

    return 0;
}

int apply_chmod_command(mode_t current_mode, const char *cmd, mode_t *out_mode)
{
    if (!cmd || !out_mode) return -1;

    /* Вариант 1: Команда в числовом виде (например, "755" или "0644") */
    mode_t numeric_mode;
    if (parse_octal(cmd, &numeric_mode) == 0) {
        *out_mode = numeric_mode;
        return 0;
    }

    /* Вариант 2: Символьный синтаксис (поддерживает составные через запятую: "u+rw,g-w") */
    char buf[128];
    strncpy(buf, cmd, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    mode_t working_mode = current_mode;
    char *token = strtok(buf, ",");
    if (!token) return -1;

    while (token != NULL) {
        if (apply_single_clause(&working_mode, token) != 0) {
            return -1;
        }
        token = strtok(NULL, ",");
    }

    *out_mode = working_mode & PERM_MASK;
    return 0;
}