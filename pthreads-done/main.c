
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define THRESHOLD 440
#define COUNT 1800
#define FILENAME "t_julia"

// #define THRESHOLD 801
// #define COUNT 1800
// #define FILENAME "t_mandle"


void main(void) {

    double sum;
    double x;
    int i;

    FILE *in = fopen(FILENAME, "r+");
    if (in == NULL) {
        perror("");
        exit(-1);
    }

    sum = 0;

    for (i = 0; i < COUNT; i++) {
        fscanf(in, "%lf", &x);
        sum += x;
        // printf("[%d - %d + %d]\n", i, sum, x);
        if (sum >= THRESHOLD) {
            sum = 0;
            printf("%d,", i);
        }
    }
}