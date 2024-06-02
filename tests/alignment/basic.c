#include "include/std.h"

struct A {
    char x;
    long y;
};

struct B {
    char arr[7];
    struct A x;
    double q;
    char rra[4];
};

union C {
    char arr[3];
    struct A x;
    double q;
    char rra[7];
};

int main() {
    printf("%lu\n", sizeof(struct A));
    printf("%lu\n", sizeof(struct B));
    printf("%lu\n", sizeof(union C));

    return EXIT_SUCCESS;
}
