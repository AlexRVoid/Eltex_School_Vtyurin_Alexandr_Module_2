#ifndef MATH_OPERATOR_H_
#define MATH_OPERATOR_H_

#include <stdio.h>
double add (double arg1, double arg2);
double subtract (double arg1, double arg2);
double multiply (double arg1, double arg2);
double divide (double arg1, double arg2);
double exponentiation (double arg1, double exp);
double sqrt (double arg);
double mod (double arg1, double arg2);

typedef double (*BinaryFunc)(double, double);
typedef double (*UnaryFunc)(double);

/* Структура для хранения информации об операции */
typedef struct MathOperation{
    const char *name;       //Название операции
    int args_count;         //Колличество аргументов
    BinaryFunc bin_op;      //Функция для 2 аргументов (add, subtract...)
    UnaryFunc un_op;        //Функция для 1 аргумента (sqrt)
} MathOperation;

#endif /* MATH_OPERATOR_H_ */