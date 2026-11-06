#ifndef MATH_OPERATOR_H_
#define MATH_OPERATOR_H_

#include <stddef.h>

typedef double (*BinaryFunc)(double, double);
typedef double (*UnaryFunc)(double);

/* Структура для хранения информации об операции */
typedef struct MathOperation {
    char name[64];          /* Название операции для меню */
    int args_count;         /* Количество аргументов (1 или 2) */
    BinaryFunc bin_op;      /* Функция для 2 аргументов */
    UnaryFunc un_op;        /* Функция для 1 аргумента */
    void *handle;           /* Дескриптор dlopen для закрытия библиотеки */
} MathOperation;

/* Функция сканирования каталога и загрузки библиотек */
int load_plugins(const char *dir_path, MathOperation **ops, size_t *count);

/* Функция освобождения памяти и выгрузки библиотек */
void free_plugins(MathOperation *ops, size_t count);

#endif /* MATH_OPERATOR_H_ */