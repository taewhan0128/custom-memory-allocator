#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("test main running\n");
    int *a = malloc(100);
    int *b = malloc(200);
    free(a);
    free(b);
    return 0;
}