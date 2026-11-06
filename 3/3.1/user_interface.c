#include "user_interface.h"
#include "perm_calc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void read_input(char *buf, size_t size)
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

static void print_representation_table(mode_t mode, const char *title)
{
    char sym[16];
    char oct[16];
    char bin[32];

    mode_to_symbolic(mode, sym);
    mode_to_octal(mode, oct);
    mode_to_binary(mode, bin);

    printf("\n>>> %s <<<\n", title);
    printf("  Буквенный вид:      %s\n", sym);
    printf("  Восьмеричный вид:   %s\n", oct);
    printf("  Битовый вид:        %s  (User: %c%c%c | Group: %c%c%c | Other: %c%c%c)\n",
           bin,
           bin[0], bin[1], bin[2],
           bin[4], bin[5], bin[6],
           bin[8], bin[9], bin[10]);
}

static void handle_input_mode(mode_t *current_mode, int *has_mode)
{
    char input[64];
    printf("\nВведите права (восьмеричные '754'/'0754' или буквенные 'rwxr-xr--'): ");
    read_input(input, sizeof(input));

    mode_t mode;
    if (parse_initial_mode(input, &mode) == 0) {
        *current_mode = mode;
        *has_mode = 1;
        print_representation_table(*current_mode, "Введенные права доступа");
    } else {
        printf("Ошибка: неверный формат прав доступа!\n");
    }
}

static void handle_file_stat(mode_t *current_mode, int *has_mode)
{
    char filename[256];
    printf("\nВведите путь к файлу: ");
    read_input(filename, sizeof(filename));

    mode_t mode;
    if (get_file_mode(filename, &mode) == 0) {
        *current_mode = mode;
        *has_mode = 1;
        print_representation_table(*current_mode, "Права из метаданных файла (stat)");

        /* Демонстрация сравнения с выводом системной ls -l (требование задания) */
        printf("\nСравнение с системным вызовом 'ls -ld %s':\n", filename);
        char cmd[300];
        snprintf(cmd, sizeof(cmd), "ls -ld --color=never '%s' 2>/dev/null", filename);
        if (system(cmd) != 0) {
            printf("  (Не удалось выполнить команду ls)\n");
        }
    } else {
        printf("Ошибка: файл '%s' не найден или недоступен для чтения атрибутов!\n", filename);
    }
}

static void handle_chmod_simulation(mode_t *current_mode, int has_mode)
{
    if (!has_mode) {
        printf("\nСначала задайте исходные права (пункт 1 или 2)!\n");
        return;
    }

    print_representation_table(*current_mode, "Текущее состояние прав");

    char cmd[128];
    printf("\nВведите команду изменения (например: 'g+w', 'u-x', 'a=rw', 'go+r', '755'): ");
    read_input(cmd, sizeof(cmd));

    mode_t new_mode;
    if (apply_chmod_command(*current_mode, cmd, &new_mode) == 0) {
        print_representation_table(new_mode, "Результат имитации chmod (файл на диске НЕ изменен)");
        *current_mode = new_mode; /* Сохраняем новое состояние для последующих модификаций */
    } else {
        printf("Ошибка: некорректная команда модификации!\n");
    }
}

void run_permissions_ui()
{
    mode_t current_mode = 0;
    int has_mode = 0;
    char choice_buf[16];
    int choice = -1;

    while (1) {
        printf("\n=======================================================\n");
        printf("        РАСЧЕТ МАСКИ ПРАВ ДОСТУПА К ФАЙЛУ        \n");
        printf("=======================================================\n");
        printf("1. Ввести права вручную (число / буквы)\n");
        printf("2. Прочитать права существующего файла (stat) и сравнить с ls -l\n");
        printf("3. Смоделировать команду изменения прав (chmod)\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");

        read_input(choice_buf, sizeof(choice_buf));
        if (sscanf(choice_buf, "%d", &choice) != 1) {
            printf("Некорректный ввод!\n");
            continue;
        }

        if (choice == 0) {
            printf("Выход из программы.\n");
            break;
        }

        switch (choice) {
            case 1: handle_input_mode(&current_mode, &has_mode); break;
            case 2: handle_file_stat(&current_mode, &has_mode); break;
            case 3: handle_chmod_simulation(&current_mode, has_mode); break;
            default:
                printf("Неверный пункт меню!\n");
                break;
        }
    }
}