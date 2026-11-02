#include "user_interface.h"
#include "math_operator.h"

void userScreen()
{
    printf ("Добро пожаловать в программу калькулятор\n");
    
    MathOperation operations[] = { //Массив с мат операциями
        {"Сложение (add)",                   2, add,            NULL},
        {"Вычитание (subtract)",             2, subtract,       NULL},
        {"Умножение (multiply)",             2, multiply,       NULL},
        {"Деление (divide)",                 2, divide,         NULL},
        {"Возведение в степень (exponent)", 2, exponentiation, NULL},
        {"Квадратный корень (sqrt)",         1, NULL,           sqrt}
    };

    int total_ops = sizeof(operations) / sizeof(operations[0]); //Размер массива(колличество достуапных операций операций)
    int choice;
    double arg1, arg2, result;

    while (1) {
        printf("\n=== Меню калькулятора ===\n");
        
        for (int i = 0; i < total_ops; i++) {
            printf("%d. %s\n", i + 1, operations[i].name);
        }
        printf("0. Выход\n");
        printf("Выберите операцию: ");
        
        if (scanf("%d", &choice) != 1) { //Проверка что было только 1 считывание пользовательского ввода
            printf("Ошибка ввода! Завершение работы.\n");
            break;
        }

        if (choice == 0) { //Выход из программы
            printf("Выход из программы. До свидания!\n");
            break;
        }

        if (choice < 1 || choice > total_ops) { //Проверка на валидность ввода от пользователя
            printf("Неверный номер команды! Попробуйте снова.\n");
            continue;
        }

        if (operations[choice - 1].args_count == 1) {
            printf("Введите аргумент: ");
            scanf("%lf", &arg1);

            /* Вызов унарной функции через указатель */
            result = operations[choice - 1].un_op(arg1);
        } else {
            printf("Введите первый аргумент: ");
            scanf("%lf", &arg1);
            printf("Введите второй аргумент: ");
            scanf("%lf", &arg2);

            /* Вызов бинарной функции через указатель */
            result = operations[choice - 1].bin_op(arg1, arg2);
        }

        /* Проверка на ошибку вычисления (NaN != NaN) */
        if (result != result) {
            printf("Ошибка: недопустимая математическая операция (деление на ноль или корень из отрицательного числа)!\n");
        } else {
            printf("Результат: %lf\n", result);
        }
        
    }
        
}
