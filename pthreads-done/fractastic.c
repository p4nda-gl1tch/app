#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "ppm.h"
#include "fractal.h"

#define UNKNOWN_MODE 0
#define JULIA_MODE 1
#define MANDELBROT_MODE 2

#define NUMBER_OF_THREADS 10

typedef struct {

    // ------------------------------------ const ------------------------------
    int mode;

    int width;
    int height;

    double x_min;
    double x_max;
    double y_min;
    double y_max;

    int max_iterations;
    int color_multiplier;

    double c_re;
    double c_im;
    double d;

    double x_step;
    double y_step;

    fractal_config fc;

    double complex c;

    unsigned short number_of_threads;
    unsigned short chunk_size;
    // ------------------------------------ const ------------------------------

    int thread_id;
    int start;
    int stop;

    int** fractal;
    int iterations, color;
} pthread_argv_t;


void draw_fractal(int **fractal, int width, int height){
    int i, j;
    printf("in draw");
    FILE *f = fopen("out", "w");

    fprintf(f, "P3 %d %d 255\n", width, height);

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            fprintf(f, "%d %d %d ", fractal[i][j], fractal[i][j], fractal[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void* pthread_func (void* void_argv) {

    pthread_argv_t* argv = malloc(sizeof(pthread_argv_t));
    memcpy(argv, void_argv, sizeof(pthread_argv_t));

    double x0, y0;
    double complex z0;
    double complex parameters[2] = { argv->c, argv->d };

    int col;
    int row;

    for (row = argv->start; row < argv->stop; row++) {
        for (col = 0; col < argv->width; col++) {

            x0 = argv->x_min + col * argv->x_step;
            y0 = argv->y_max - row * argv->y_step;
            z0 = x0 + y0 * I;

            if (argv->mode == JULIA_MODE) {
                argv->iterations = julia(complex_polynomial,
                                    z0,
                                    parameters,
                                    &argv->fc);
            } else if (argv->mode == MANDELBROT_MODE) {
                argv->iterations = generalized_mandelbrot(complex_polynomial,
                                                    z0,
                                                    parameters,
                                                    0,
                                                    &argv->fc);
            }

            if (argv->iterations == CONVERGE) {
                argv->color = 255;
            } else {
                argv->color = argv->color_multiplier * argv->iterations;
            }

            //output_color(color, color, color);
            argv->fractal[row][col] = argv->color;
        }
    }

    free(argv);

    return NULL;
}

// const int JULIA_SIZE = 24;
// const int JULIA_THRESHOLDS_START[] = {0,500,1300};
// const int JULIA_THRESHOLDS_STOP[] = {500,1300,1800};
// const int JULIA_THRESHOLDS_THREADS[] = {1,20,1};

// const int MANDELBROT_SIZE = 24;
// const int MANDELBROT_THRESHOLDS_START[] = {0,500,1300};
// const int MANDELBROT_THRESHOLDS_STOP[] = {500,1300,1800};
// const int MANDELBROT_THRESHOLDS_THREADS[] = {2,20,2};

const int JULIA_SIZE = 24;
const int JULIA_THRESHOLDS_START[] = {0,411,561,614,656,686,712,736,763,793,825,854,893,934,965,996,1028,1056,1080,1105,1135,1181,1241,1466};
const int JULIA_THRESHOLDS_STOP[] = {411,561,614,656,686,712,736,763,793,825,854,893,934,965,996,1028,1056,1080,1105,1135,1181,1241,1466,1800};
// const int JULIA_THRESHOLDS_THREADS[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

const int MANDELBROT_SIZE = 24;
const int MANDELBROT_THRESHOLDS_START[] = {0,561,633,668,703,735,764,792,815,837,858,879,900,921,942,963,985,1008,1036,1066,1098,1134,1172,1264};
const int MANDELBROT_THRESHOLDS_STOP[] = {561,633,668,703,735,764,792,815,837,858,879,900,921,942,963,985,1008,1036,1066,1098,1134,1172,1264,1800};
// const int MANDELBROT_THRESHOLDS_THREADS[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

int main(int argc, char **argv) {

    pthread_argv_t p_argv;
    pthread_t* threads;
    int ret;

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

        int arg = 2;

        p_argv.width = strtol(argv[arg++], NULL, 0);
        p_argv.height = strtol(argv[arg++], NULL, 0);
        p_argv.x_min = strtod(argv[arg++], NULL);
        p_argv.x_max = strtod(argv[arg++], NULL);
        p_argv.y_min = strtod(argv[arg++], NULL);
        p_argv.y_max = strtod(argv[arg++], NULL);
        p_argv.max_iterations = strtol(argv[arg++], NULL, 0);
        p_argv.color_multiplier = strtol(argv[arg++], NULL, 0);

        p_argv.c_re = p_argv.mode == JULIA_MODE ? strtod(argv[arg++], NULL) : 0;
        p_argv.c_im = p_argv.mode == JULIA_MODE ? strtod(argv[arg++], NULL) : 0;
        p_argv.d = strtod(argv[arg++], NULL);

        p_argv.x_step = (p_argv.x_max - p_argv.x_min) / p_argv.width;
        p_argv.y_step = (p_argv.y_max - p_argv.y_min) / p_argv.height;

        p_argv.fc.epsilon = EPSILON;
        p_argv.fc.infinity = INFINITY;
        p_argv.fc.max_iterations = p_argv.max_iterations;

        p_argv.c = p_argv.c_re + p_argv.c_im * I;

        p_argv.iterations = 0;
        p_argv.color = 0;

        p_argv.fractal = (int**)malloc(p_argv.height * sizeof(int*));

        int i;
        for (i=0; i<p_argv.height; i++) {
            p_argv.fractal[i] = (int*)malloc(p_argv.width * sizeof(int));
        }

        //output_color_header(width, height);

        p_argv.number_of_threads = strtod(argv[arg++], NULL);

        threads = malloc(sizeof(pthread_t) * p_argv.number_of_threads);

        /* Chunk-bazed smart implementation */
        if (!strcmp(argv[arg], "C")) {
            arg++;
            p_argv.chunk_size = strtod(argv[arg++], NULL);

            int level = 0;

            do {
                pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                p_argv.thread_id = level % p_argv.number_of_threads;
                memcpy(a, &p_argv, sizeof(pthread_argv_t));

                a->start = a->chunk_size * level;

                if (p_argv.chunk_size * (level + 1) > p_argv.height) {
                    a->stop = a->height;
                } else {
                    a->stop = a->chunk_size * (level + 1);
                }

                // printf("%d: building [%4d -> %4d]\n",
                //     level % p_argv.number_of_threads,
                //     a->start,
                //     a->stop);

                if (level >= p_argv.number_of_threads) {
                    pthread_join(threads[level % p_argv.number_of_threads], NULL);
                }

                ret = pthread_create(&threads[level % p_argv.number_of_threads], NULL, pthread_func, a);

                if (ret) {
                    exit(-1);
                }

                level++;
            }
            while(p_argv.chunk_size * level < p_argv.height);
        } /* Hard-coded for a smart implementation */
        else if (!strcmp(argv[arg], "H")) {
            arg++;
            p_argv.chunk_size = strtod(argv[arg++], NULL);

            if (p_argv.mode == JULIA_MODE) {
                for (i = 0; i < JULIA_SIZE; i++) {

                    pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                    memcpy(a, &p_argv, sizeof(pthread_argv_t));

                    a->thread_id = i;
                    a->start = JULIA_THRESHOLDS_START[i];
                    a->stop = JULIA_THRESHOLDS_STOP[i];

                    pthread_create(&threads[a->thread_id], NULL, pthread_func, a);

                    // for (j = 0; j < JULIA_THRESHOLDS_THREADS[i]; j++) {
                    //     pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                    //     memcpy(a, &p_argv, sizeof(pthread_argv_t));

                    //     int chunk_size = JULIA_THRESHOLDS_STOP[i] - JULIA_THRESHOLDS_START[i];

                    //     a->start = JULIA_THRESHOLDS_START[i] + chunk_size / JULIA_THRESHOLDS_THREADS[i] * j;
                    //     if (j == JULIA_THRESHOLDS_THREADS[i] - 1) {
                    //         a->stop = JULIA_THRESHOLDS_STOP[i];
                    //     } else {
                    //         a->stop = JULIA_THRESHOLDS_START[i] + chunk_size / JULIA_THRESHOLDS_THREADS[i] * (j + 1);
                    //     }
                    //     a->thread_id = thread_id;

                    //     // printf("{%2d}: [%4d->%4d]\n", a->thread_id, a->start, a->stop);

                    //     thread_id += 1;
                    //     pthread_create(&threads[a->thread_id], NULL, pthread_func, a);
                    // }
                }
            } else if (p_argv.mode == MANDELBROT_MODE) {
                for (i = 0; i < MANDELBROT_SIZE; i++) {
                    pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                    memcpy(a, &p_argv, sizeof(pthread_argv_t));

                    a->thread_id = i;
                    a->start = MANDELBROT_THRESHOLDS_START[i];
                    a->stop = MANDELBROT_THRESHOLDS_STOP[i];

                    pthread_create(&threads[i], NULL, pthread_func, a);
                    // for (j = 0; j < MANDELBROT_THRESHOLDS_THREADS[i]; j++) {
                    //     pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                    //     memcpy(a, &p_argv, sizeof(pthread_argv_t));

                    //     int chunk_size = MANDELBROT_THRESHOLDS_STOP[i] - MANDELBROT_THRESHOLDS_START[i];

                    //     a->start = MANDELBROT_THRESHOLDS_START[i] + chunk_size / MANDELBROT_THRESHOLDS_THREADS[i] * j;
                    //     if (j == MANDELBROT_THRESHOLDS_THREADS[i] - 1) {
                    //         a->stop = MANDELBROT_THRESHOLDS_STOP[i];
                    //     } else {
                    //         a->stop = MANDELBROT_THRESHOLDS_START[i] + chunk_size / MANDELBROT_THRESHOLDS_THREADS[i] * (j + 1);
                    //     }
                    //     a->thread_id = thread_id;
                    //     thread_id++;

                    //     pthread_create(&threads[a->thread_id], NULL, pthread_func, a);
                    // }
                }
            }
        } /* Default implementation of pthread (not smart) */
        else if (!strcmp(argv[arg], "D")) {
            arg++;

            /* DOES NOTHING */
            p_argv.chunk_size = strtod(argv[arg++], NULL);

            for (i = 0; i < p_argv.number_of_threads; i++) {
                pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                memcpy(a, &p_argv, sizeof(pthread_argv_t));

                a->thread_id = i;
                a->start = a->height / a->number_of_threads * a->thread_id;
                a->stop = a->height / a->number_of_threads * (a->thread_id + 1);
                if (i == p_argv.number_of_threads - 1) {
                    a->stop = a->height;
                }

                ret = pthread_create(&threads[i], NULL, pthread_func, a);

                if (ret) {
                    exit(-1);
                }
            }
        } /* Smart one */
        else if (!strcmp(argv[arg], "S")) {
            arg += 2;

            FILE *fin;

            if (p_argv.mode == JULIA_MODE) {
                fin = fopen("t_julia", "r");
            } else if (p_argv.mode == MANDELBROT_MODE) {
                fin = fopen("t_mandle", "r");
            }

            if (fin == NULL) {
                perror("File while opening the input file.\n");
                exit(-1);
            }

            double x[p_argv.height];
            /* Time to build all lines */
            double sum = 0;
            /* How much work should be on each thread */
            double avg = 0;

            int line = 0;

            for (i = 0; i < p_argv.height; i++) {
                ret = fscanf(fin, "%lf", &x[i]);
                if  (ret < 0) {
                    perror("Error while reading input file;\nMaybe size differs.\n");
                    exit(-1);
                }
                sum += x[i];
            }

            avg = sum / p_argv.number_of_threads * 1.0;
            printf("sum:%lf\navg: %lf\n", sum, avg);

            /* line = 0 */
            for (i = 0; i < p_argv.number_of_threads; i++) {

                pthread_argv_t *a = malloc(sizeof(pthread_argv_t));
                p_argv.thread_id = line % p_argv.number_of_threads;
                memcpy(a, &p_argv, sizeof(pthread_argv_t));

                a->start = line;

                /* reset sum for each thread */
                double thread_work = 0;

                if (i == p_argv.number_of_threads - 1) {
                    line = p_argv.height;
                } else {
                    do {
                        thread_work += x[line++];
                    } /* compute how much work a thread should do */
                    while(line < p_argv.height && thread_work + x[line] < avg);
                }

                a->stop = line;

                // printf("%d: building [%4d -> %4d] (%lf | %lf)\n",
                //     i,
                //     a->start,
                //     a->stop,
                //     thread_work,
                //     avg);

                if (i >= 0) {
                    pthread_join(threads[i], NULL);
                }

                ret = pthread_create(&threads[i], NULL, pthread_func, a);

                if (ret) {
                    exit(-1);
                }
            }
        }

        for (i = 0; i < p_argv.number_of_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        draw_fractal(p_argv.fractal, p_argv.width, p_argv.height);

        return 0;
    }
}
