#include "include/std.h"

int main() {
    int x = 0;
    int y = 1;
    int z = 1;

    int n = 30;

    printf("%d %d", x, y);
    for (int i = 2; i < n; ++i) {
        x = y;
        y = z;
        z = x + y;
        printf("%d", z);     
    }

    return EXIT_SUCCESS;
}
