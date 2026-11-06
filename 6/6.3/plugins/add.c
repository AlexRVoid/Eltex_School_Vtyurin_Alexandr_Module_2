#include "../math_operator.h"

double add(double arg1, double arg2)
{
    return arg1 + arg2;
}

MathOperation operation = {
    .name = "Сложение (add)",
    .args_count = 2,
    .bin_op = add,
    .un_op = NULL,
    .handle = NULL
};