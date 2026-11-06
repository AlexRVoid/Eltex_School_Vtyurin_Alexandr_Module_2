#include "math_operator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

int load_plugins(const char *dir_path, MathOperation **ops, size_t *count)
{
    if (!dir_path || !ops || !count) return -1;

    *ops = NULL;
    *count = 0;

    /* 1. Открытие каталога с библиотеками */
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Не удалось открыть каталог плагинов");
        return -1;
    }

    struct dirent *entry;
    size_t capacity = 0;

    /* 2. Итерация по всем файлам каталога */
    while ((entry = readdir(dir)) != NULL) {
        size_t len = strlen(entry->d_name);
        /* Проверяем расширение: файл должен оканчиваться строго на ".so" */
        if (len < 4 || strcmp(entry->d_name + len - 3, ".so") != 0) {
            continue;
        }

        /* Формирование полного пути: "plugins/libadd.so" */
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        /* 3. Загрузка динамической библиотеки в адресное пространство процесса */
        void *handle = dlopen(full_path, RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Ошибка загрузки %s: %s\n", full_path, dlerror());
            continue;
        }

        dlerror(); /* Сброс предыдущих ошибок */

        /* 4. Поиск экспортируемой структуры операции */
        MathOperation *plugin_op = (MathOperation *)dlsym(handle, "operation");
        char *err = dlerror();
        if (err != NULL || !plugin_op) {
            fprintf(stderr, "В библиотеке %s не найден символ 'operation'\n", entry->d_name);
            dlclose(handle);
            continue;
        }

        /* 5. Валидация сигнатуры */
        if ((plugin_op->args_count == 2 && !plugin_op->bin_op) ||
            (plugin_op->args_count == 1 && !plugin_op->un_op)) {
            fprintf(stderr, "Некорректная операция в %s\n", entry->d_name);
            dlclose(handle);
            continue;
        }

        /* 6. Добавление операции в динамический массив */
        if (*count >= capacity) {
            size_t new_cap = (capacity == 0) ? 4 : capacity * 2;
            MathOperation *temp = (MathOperation *)realloc(*ops, new_cap * sizeof(MathOperation));
            if (!temp) {
                perror("Ошибка realloc при загрузке плагинов");
                dlclose(handle);
                closedir(dir);
                return -1;
            }
            *ops = temp;
            capacity = new_cap;
        }

        (*ops)[*count] = *plugin_op;
        (*ops)[*count].handle = handle; /* Сохраняем handle для последующего dlclose */
        (*count)++;
    }

    closedir(dir);
    return 0;
}

void free_plugins(MathOperation *ops, size_t count)
{
    if (!ops) return;

    for (size_t i = 0; i < count; i++) {
        if (ops[i].handle) {
            dlclose(ops[i].handle); /* Выгрузка библиотеки из памяти */
        }
    }
    free(ops);
}