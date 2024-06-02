#include "include/std.h"

int main() {
    short a = 2;
    int x = a + 123;
    float y = x * x;
    double z = y / x;

    printf("%d\n", x);
    printf("%f\n", y);
    printf("%f\n", z);

    return EXIT_SUCCESS;
}
