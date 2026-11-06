#include "../math_operator.h"

double my_sqrt(double arg)
{
    if (arg < 0.0) return 0.0 / 0.0; /* NaN */
    if (arg == 0.0) return 0.0;

    double res = arg;
    double eps = 1e-7;

    while ((res - arg / res) > eps || (arg / res - res) > eps) {
        res = 0.5 * (res + arg / res);
    }
    return res;
}

MathOperation operation = {
    .name = "Квадратный корень (sqrt)",
    .args_count = 1,
    .bin_op = NULL,
    .un_op = my_sqrt,
    .handle = NULL
};