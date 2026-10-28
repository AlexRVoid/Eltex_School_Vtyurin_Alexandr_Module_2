#include "user_interface.h"

void userScreen()
{
    printf ("Добро пожаловать в программу калькулятор\n");
    int choice;
    double arg1, arg2, result;

    while (1) {
        printf("\n=== Меню калькулятора ===\n");
        printf("1. Сложение (add)\n");
        printf("2. Вычитание (subtract)\n");
        printf("3. Умножение (multiply)\n");
        printf("4. Деление (divide)\n");
        printf("5. Возведение в степень (Exponentiation)\n");
        printf("6. Квадратный корень (sqrt)\n");
        printf("7. Остаток от деления (mod)\n");
        printf("0. Выход\n");
        printf("Выберите операцию: ");

        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Завершение работы.\n");
            break;
        }

        if (choice == 0) {
            printf("Выход из программы.\n");
            break;
        }

        switch (choice) {
            case 1:
                printf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = add(arg1, arg2);
                printf("Результат: %lf\n", result);
                break;

            case 2:
                printf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = subtract(arg1, arg2);
                printf("Результат: %lf\n", result);
                break;

            case 3:
                printf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = multiply(arg1, arg2);
                printf("Результат: %lf\n", result);
                break;

            case 4:
                printf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = divide(arg1, arg2);
                if (result != result) {
                   printf("Ошибка: деление на ноль!\n");
                } else {
                    printf("Результат: %lf\n", result);
                }
                break;

            case 5:
                pprintf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = Exponentiation(arg1, arg2);
                printf("Результат: %lf\n", result);
                break;

            case 6:
                printf("Введите первый аргумент: ");
                scanf("%lf", &arg1);
                printf("Введите второй аргумент аргумент: ");
                scanf("%lf", &arg2);
                result = sqrt(arg1);
                /* Проверка на NaN без использования math.h (NaN != NaN) */
                if (result != result) {
                    printf("Ошибка: корень из отрицательного числа!\n");
                } else {
                    printf("Результат: %lf\n", result);
                }
                break;

            /*case 7:
                printf("Введите два числа: ");
                scanf("%lf %lf", &arg1, &arg2);
                if (arg2 == 0.0) {
                    printf("Ошибка: деление на ноль!\n");
                } else {
                    result = mod(arg1, arg2);
                    printf("Результат: %lf\n", result);
                }
                break;*/

            default:
                printf("Неверный пункт меню. Попробуйте снова.\n");
                break;
        }
    }

}