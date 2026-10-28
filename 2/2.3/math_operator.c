#include "math_operator.h"


double add (double arg1, double arg2)
{
    return arg1 + arg2;
}

double subtract (double arg1, double arg2)
{
    return arg1 - arg2;
}

double multiply (double arg1, double arg2)
{
    return arg1 * arg2;
}

double divide (double arg1, double arg2)
{
    return arg1 / arg2;
}

double exponentiation (double arg1, double exp)
{
    if(exp == 0){
        return 0;
    }
    double expResult = arg1;
    for (int i = 1; i <= exp; i++){
        expResult *= arg1;
    }
    return expResult;
}

double sqrt (double arg)
{
    if (arg < 0) return 0.0 / 0.0; // Возврат ошибки для отрицательных чисел
    if (arg == 0) return 0;

    double sqrtResult = arg;       // Начальное приближение
    double eps = 1e-7;  // Точность вычислений

    while ((sqrtResult - arg / sqrtResult) > eps || (arg / sqrtResult - sqrtResult) > eps) {
        sqrtResult = 0.5 * (sqrtResult + arg / sqrtResult);
    }

    return sqrtResult;
}

/* double mod (double arg1, double arg2)
{
    return arg1 % arg2;
}*/
