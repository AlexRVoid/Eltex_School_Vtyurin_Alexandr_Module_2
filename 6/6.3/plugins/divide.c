#include "../math_operator.h"

double divide(double arg1, double arg2)
{
    if (arg2 == 0.0) return 0.0 / 0.0; /* NaN */
    return arg1 / arg2;
}

MathOperation operation = {
    .name = "Деление (divide)",
    .args_count = 2,
    .bin_op = divide,
    .un_op = NULL,
    .handle = NULL
};