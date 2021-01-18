#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <mpi.h>
#include <sys/time.h>

#include "fractal.h"

#define UNKNOWN_MODE 0
#define JULIA_MODE 1
#define MANDELBROT_MODE 2

void draw_fractal(int **fractal, int width, int height){
    int i, j;
    FILE *f = fopen("mpi", "w");

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

    int row, col, i, j, ii, jj, k;

    int rank;
    int nProcesses;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Status status;
    struct timeval t1, t2;
    double elapsedTime;

    int height_start;
    int height_end;
    int height_mid;

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
    fscanf(file, "%d %d %d %d %d %d %d %d %lf %d %d", &mode_sel, &width, &height, &x_min, &x_max, &y_min, &y_max, &max_iterations, &color_multiplier, &tmp1, &tmp2, &d);


    if (rank == 0) {
        gettimeofday(&t1, NULL);
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [J/M] [options]\n", argv[0]);
        return 1;
    } else {
        int mode = UNKNOWN_MODE;

        if (mode_sel == 0) {
            mode = JULIA_MODE;
            height_start = 500;
            height_end = 1300;
        } else if (mode_sel == 1) {
            mode = MANDELBROT_MODE;
            height_start = 600;
            height_end = 1200;
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
        int start = 0;
        int end = 0;

        if (rank < nProcesses / 6) {
            start = rank * (height_start /(nProcesses / 6));
            end = (rank + 1) * (height_start /(nProcesses / 6));
            if (rank == nProcesses / 6 - 1)
                end = height_start;

        }


        if (rank >= 5 * nProcesses / 6) {
            start = height_end + (rank - 5 * nProcesses / 6)  * ((height -height_end) /(nProcesses / 6));
            end = height_end + (rank - 5 * nProcesses / 6 + 1) * ((height -height_end) /(nProcesses / 6));

            if (rank == nProcesses - 1)
                end = height;

        }


        if (rank >= nProcesses / 6 && rank < 5 * nProcesses / 6) {
            start = height_start + (rank - nProcesses / 6) * (height_mid / (4 * nProcesses / 6));

            end = height_start + (rank - nProcesses / 6 +1) * (height_mid / (4 * nProcesses / 6));

            if (rank == 5 * nProcesses / 6 - 1)
                end = height_end;

        }

        // start = rank *(height/nProcesses);
        // end = (rank+1) * (height/nProcesses);
        // if (rank == nProcesses-1)
        //  end = height;


        int* fractal = (int*)malloc(width * (end - start) * sizeof(int));

        int index = 0;

        for (row = start; row < end; row++) {
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

                fractal[index] = color;
                index++;
            }
        }

        if (rank != 0) {

            MPI_Send(&start, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

            MPI_Send(&end, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

            MPI_Send(fractal, (end - start) * width, MPI_INT, 0, 1, MPI_COMM_WORLD);

        }

        if (rank == 0) {

            int* fractal_as_vector = (int*)malloc(width * height * sizeof(int));

            for (i = start; i < end; i++)
                for (j = 0; j < width; j++)
                    fractal_as_vector[i * width + j] = fractal[i * width + j];

            for (k = 1; k < nProcesses; k++) {

                int i_start;
                MPI_Recv(&i_start, 1, MPI_INT, k, 1, MPI_COMM_WORLD, &status);

                int i_end;

                MPI_Recv(&i_end, 1, MPI_INT, k, 1, MPI_COMM_WORLD, &status);

                int *fr = (int*)malloc(width * (i_end - i_start) * sizeof(int));
                MPI_Recv(fr, (i_end - i_start) * width, MPI_INT, k, 1, MPI_COMM_WORLD, &status);

                int iterator = 0;

                for (ii = i_start; ii < i_end; ii++)
                    for (jj = 0; jj < width; jj++) {
                        fractal_as_vector[ii * width + jj] = fr[iterator];
                        iterator++;
                    }
                free(fr);
            }

            draw_fractal(*fractal_as_vector, width, height);

            gettimeofday(&t2, NULL);
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
            printf("%.3lf\n", elapsedTime);
            free(fractal_as_vector);
        }
        free(fractal);
        MPI_Finalize();
        return 0;
    }
}
