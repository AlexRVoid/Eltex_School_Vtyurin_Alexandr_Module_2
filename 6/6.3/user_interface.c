#include "user_interface.h"
#include "math_operator.h"
#include <stdio.h>
#include <stdlib.h>

static void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void userScreen(void)
{
    printf("Добро пожаловать в модульный калькулятор (Задание 6.3)\n");

    MathOperation *operations = NULL;
    size_t total_ops = 0;

    // Загрузка всех плагинов из папки ./plugins 
    if (load_plugins("./plugins", &operations, &total_ops) != 0 || total_ops == 0) {
        printf("Ошибка: не найдено ни одной математической библиотеки в каталоге ./plugins!\n");
        printf("Скомпилируйте библиотеки и поместите их в папку ./plugins.\n");
        return;
    }

    printf("Успешно загружено операций из библиотек: %zu\n", total_ops);

    int choice;
    double arg1, arg2, result;

    while (1) {
        printf("\n=== Меню калькулятора ===\n");
        for (size_t i = 0; i < total_ops; i++) {
            printf("%zu. %s\n", i + 1, operations[i].name);
        }
        printf("0. Выход\n");
        printf("Выберите операцию: ");

        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Попробуйте снова.\n");
            clear_input();
            continue;
        }

        if (choice == 0) {
            printf("Выгрузка библиотек и завершение работы...\n");
            break;
        }

        if (choice < 1 || (size_t)choice > total_ops) {
            printf("Неверный номер команды!\n");
            continue;
        }

        size_t idx = (size_t)(choice - 1);

        if (operations[idx].args_count == 1) {
            printf("Введите аргумент: ");
            if (scanf("%lf", &arg1) != 1) {
                printf("Ошибка ввода числа!\n");
                clear_input();
                continue;
            }
            result = operations[idx].un_op(arg1);
        } else {
            printf("Введите первый аргумент: ");
            if (scanf("%lf", &arg1) != 1) {
                printf("Ошибка ввода числа!\n");
                clear_input();
                continue;
            }
            printf("Введите второй аргумент: ");
            if (scanf("%lf", &arg2) != 1) {
                printf("Ошибка ввода числа!\n");
                clear_input();
                continue;
            }
            result = operations[idx].bin_op(arg1, arg2);
        }

        // Проверка на NaN (result != result) 
        if (result != result) {
            printf("Ошибка: недопустимая математическая операция (деление на ноль или корень из отрицательного числа)!\n");
        } else {
            printf("Результат: %lf\n", result);
        }
    }

    // Выгрузка библиотек через dlclose и освобождение массива 
    free_plugins(operations, total_ops);
}