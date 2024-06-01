#include "include/std.h"

struct personal_info {
    int age;
    char *name;
    float height;
    char gender;
};

void print_info(struct personal_info* p){
	printf("%d, %s, %f\n", p->age, p->name, p->height);
}

int main() {
    struct personal_info person;

    person.age = 28;
    person.name = "abc";
    person.height = 181.8;

    print_info(&person);

    return EXIT_SUCCESS;
}
