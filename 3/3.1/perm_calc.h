#ifndef PERM_CALC_H_
#define PERM_CALC_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#define PERM_MASK 0777 /* Маска 9 бит прав доступа ugo */

/* Форматирование режимов в строки */
void mode_to_symbolic(mode_t mode, char *out_str); /* "rwxr-xr--" (9 символов + '\0') */
void mode_to_octal(mode_t mode, char *out_str);    /* "0754" или "754" */
void mode_to_binary(mode_t mode, char *out_str);   /* "111 101 100" */

/* Разбор исходных прав: поддерживает "754", "0754" или "rwxr-xr--" */
int parse_initial_mode(const char *str, mode_t *out_mode);

/* Получение прав реального файла через системный вызов stat */
int get_file_mode(const char *filename, mode_t *out_mode);

/* 
 * Модификация прав по синтаксису chmod:
 * - Числовой: "755", "644"
 * - Символьный: "g+w", "u-x", "a=rw", "o+r", "+x", "u+r,g-w"
 * Возвращает 0 при успехе, -1 при некорректной команде.
 */
int apply_chmod_command(mode_t current_mode, const char *cmd, mode_t *out_mode);

#endif /* PERM_CALC_H_ */