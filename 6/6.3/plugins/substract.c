#include "../math_operator.h"

double subtract(double arg1, double arg2)
{
    return arg1 - arg2;
}

MathOperation operation = {
    .name = "Вычитание (subtract)",
    .args_count = 2,
    .bin_op = subtract,
    .un_op = NULL,
    .handle = NULL
};