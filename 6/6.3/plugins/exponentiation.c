#include "../math_operator.h"

double exponentiation(double arg1, double exp)
{
    if (exp == 0.0) return 1.0;
    double res = 1.0;
    long n = (long)exp;

    if (n > 0) {
        for (long i = 0; i < n; i++) res *= arg1;
        return res;
    } else {
        for (long i = 0; i < -n; i++) res *= arg1;
        return 1.0 / res;
    }
}

MathOperation operation = {
    .name = "Возведение в степень (exponent)",
    .args_count = 2,
    .bin_op = exponentiation,
    .un_op = NULL,
    .handle = NULL
};