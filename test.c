#include <stdlib.h>
#include <stdio.h>

int main(){
    // debug print line to see test file is ran
    printf("test main running\n");
    // allocated memory block
    int *a = malloc(100);
    int *b = malloc(200);
    int *c = malloc(300);

    free(a);
    free(b);

    return 0;
}