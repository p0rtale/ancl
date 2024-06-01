#include "include/std.h"

int main() {
    for (int i = 1; i < 5; ++i) {
        for (int j = 1; j < 8; j++) {
            printf("%d\n", i * j / (i + j));
        }
    }

    return EXIT_SUCCESS;
}
