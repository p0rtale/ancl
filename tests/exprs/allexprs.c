#include "include/std.h"

int main() {
    long a = 15;
    int b = -23;
    short e = 14;
    float c = 234.78921;
    double d = -123123.435;

    float x = d * (a + b) / (a << 4) - c * b / d + (a ^ e) * (a | e);
    printf("%f\n", x);

    return EXIT_SUCCESS;
}
