#include "../math_operator.h"

double multiply(double arg1, double arg2)
{
    return arg1 * arg2;
}

MathOperation operation = {
    .name = "Умножение (multiply)",
    .args_count = 2,
    .bin_op = multiply,
    .un_op = NULL,
    .handle = NULL
};