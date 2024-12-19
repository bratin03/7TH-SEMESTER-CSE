#include <stdio.h>
#include <math.h>

int main()
{
    double a = 2.5;
    double b = 3.0;
    double c = 1.2;

    // Using FMA: result = (a * b) + c
    double result = fma(a, b, c);

    printf("Result of FMA (%.2f * %.2f + %.2f): %.5f\n", a, b, c, result);

    return 0;
}
