int printf(const char *format, ...);

int doFirst() {
    volatile int x = 0;
    printf("First()");
    return x;
}

void undoFirst() {
    printf("~First()");
}

int doSecond() {
    volatile int y = 2;
    printf("Second()");
    return y;
}

void undoSecond() {
    printf("~Second()");
}

int doThird() {
    volatile int z = 3;
    printf("Third");
    return z;
}

int main() {
    if (!doFirst()) {
        goto exit;
    }
    if (!doSecond()) {
        goto cleanupFirst;
    }
    if (!doThird()) {
        goto cleanupSecond;
    }

    return 0;

cleanupFirst:
    undoFirst();
cleanupSecond:
    undoSecond();
exit:
    return 0;
}
