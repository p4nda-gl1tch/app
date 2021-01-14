CC = mpicc

CFLAGS = -g -Wall -O3
LDFLAGS = -lm

fractastic_mpi_omp: fractal.c fractastic.c
	 $(CC) $(CFLAGS) -fopenmp $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -f *.o *~ fractastic_mpi_omp