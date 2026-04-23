#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_basic(){
    void *p1 = malloc(100);
    void *p2 = malloc(200);
    free(p1);
    void *p3 = malloc(50);  // should reuse p1's block
    printf("p1=%p, p3=%p\n", p1, p3);  // should be same address
}

void test_coalesce(){
    void *p1 = malloc(100);
    void *p2 = malloc(100);
    free(p1);
    free(p2);  // should merge with p1
    void *p3 = malloc(180);  // should fit in merged block
}

void test_realloc(){
    char *p = malloc(10);
    strcpy(p, "hello");
    p = realloc(p, 100);
    printf("%s\n", p);  // should still print "hello"
}

int main(){
    test_basic();
    test_coalesce();
    test_realloc();
    return 0;
}