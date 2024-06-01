#include "include/std.h"

int main() {
    int array[50];

	for (int i = 0; i < 50; i++) {
		array[i] = i * 2;
	}
	for (int i = 49; i >= 0; i--) {
		printf("%d\n", array[i]);
	}

    return EXIT_SUCCESS;
}
