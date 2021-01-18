#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <sys/time.h>
#include <omp.h>

#include "ppm.h"
#include "fractal.h"

#define UNKNOWN_MODE 0
#define JULIA_MODE 1
#define MANDELBROT_MODE 2


void draw_fractal(int **fractal, int width, int height){
    int i, j;

    FILE *f = fopen("opemmp-out", "w");

    fprintf(f, "P3 %d %d 255\n", width, height);

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            fprintf(f, "%d %d %d ", fractal[i][j], fractal[i][j], fractal[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

char *in_filename;

int main(int argc, char **argv) {

    int row, col, i;

    int width, height; 

    int mode_sel;
    int max_iterations, color_multiplier; 
    
    // double x_min;
    // double x_max;
    // double y_min;
    // double y_max;
    // double d;
    // double tmp1, tmp2;


    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int d;
    int tmp2;
    double tmp1;

    in_filename = argv[1];
    FILE *file = fopen(in_filename, "r");
    printf("%s \n", argv[1]);

    // fscanf(file, "%d %d %d %lf %lf %lf %lf %lf %d %d %lf %lf %lf ", &mode_sel, &width, &height, &x_min, &x_max, &y_min, &y_max, &max_iterations, &color_multiplier, &tmp1, &tmp2, &d);
    fscanf(file, "%d %d %d %d %d %d %d %d %ld %le %d %d", &mode_sel, &width, &height, &x_min, &x_max, &y_min, &y_max, &max_iterations, &color_multiplier, &tmp1, &tmp2, &d);

    printf("%s \n", argv[1]);
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [J/M] [options]\n", argv[0]);
        return 1;
    } else {
        int mode = UNKNOWN_MODE;
        
        
        printf("%s \n", argv[1]);
        if (mode_sel == 0) {
            mode = JULIA_MODE;
            printf("julia");
        } else if (mode_sel == 1) {
            mode = MANDELBROT_MODE;
        }

        if (mode == UNKNOWN_MODE) {
            fprintf(stderr, "Unrecognized mode from config file");
            return 2;
        } else if (mode == JULIA_MODE && argc != 2) {
            fprintf(stderr, "Usage: %s config file\n", argv[0]);
            return 3;
        } else if (mode == MANDELBROT_MODE && argc != 2) {
            fprintf(stderr, "Usage: %s config file\n", argv[0]);
            return 4;
        }


        const double c_re = mode == JULIA_MODE ? tmp1 : 0;
        const double c_im = mode == JULIA_MODE ? tmp2 : 0;


        const double x_step = (x_max - x_min) / width;
        const double y_step = (y_max - y_min) / height;

        const fractal_config fc = {EPSILON, INFINITY, max_iterations};

        const double complex c = c_re + c_im * I;

        double complex parameters[2] = {c, d};

        double x0, y0;
        double complex z0;

        int iterations = 0, color = 0;

        int** fractal = (int**)malloc(height * sizeof(int*));

        for (i=0; i<height; i++) {
            fractal[i] = (int*)malloc(width * sizeof(int));
        }
        for (row = 0; row < height; row++) {
            for (col = 0; col < width; col++) {
                x0 = x_min + col * x_step;
                y0 = y_max - row * y_step;
                z0 = x0 + y0 * I;

                if (mode == JULIA_MODE) {
                    iterations = julia(complex_polynomial,
                                       z0,
                                       parameters,
                                       &fc);
                } else if (mode == MANDELBROT_MODE) {
                    iterations = generalized_mandelbrot(complex_polynomial,
                                                        z0,
                                                        parameters,
                                                        0,
                                                        &fc);
                }
                if (iterations == CONVERGE) {
                    color = 255;
                } else {
                    color = color_multiplier * iterations;
                }

                fractal[row][col] = color;
            }
        }
        draw_fractal(fractal, width, height);

        return 0;
    }
}
