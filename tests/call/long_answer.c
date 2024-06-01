int first(int x) {
    return x;
}

int second(int x) {
    return x;
}

int third(int x) {
    return x;
}

int main() {
    return first(second(third(42)));
}
