#include "include/std.h"

typedef union U {
    long x;
    float y;
    char arr[7];
} union_t;

int main() {
    union_t x;
    x.x = 1500000000;

    printf("%f\n", x.y);
    printf("%c%c%c%c\n", x.arr[0], x.arr[1], x.arr[2], x.arr[3]);

    return EXIT_SUCCESS;
}
