#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "mm_rank1_and_blas.c"
@set kernelsizelist = read('kernel_sizes.txt')
@set m, k, n = read('matrix_size.txt')
@for i, j in kernelsizelist
@assert m % i == 0 and n % j == 0
#include "mm@[i]x@[j].c"
@endfor

#define NUM_OF_WARMUPS 1
#define NUM_OF_REPEATS 16

typedef void (*mm_t) (float const*, float const*, float*, int, int, int);

void _set_random(size_t m, size_t n, float* c) {
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            c[j+n*i] = ((float) rand()) / RAND_MAX;
        }
    }
}

int _are_the_same(size_t m, size_t n, float const* c1, float const* c2) {
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (fabs(c2[j+n*i] - c1[j+n*i]) > 0.0001 * c1[j+n*i]) {
                printf("%f %f \n", c1[j+n*i], c2[j+n*i]);
                return 0;
            }
        }
    }
    return 1;
}

double mm_gflops(size_t m, size_t k, size_t n,
                 float const* a, float const* b, float* c, float const* true_c,
                 mm_t mm_func) {
    struct timespec start, finish;
    mm_func(a, b, c, m, k, n);
    if (_are_the_same(m, n, c, true_c) == 0) {
        return -1.0;
    }
    for (int i = 0; i < NUM_OF_WARMUPS; ++i) {
        mm_func(a, b, c, m, k, n);
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_OF_REPEATS; ++i) {
        mm_func(a, b, c, m, k, n);
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    double tt = finish.tv_sec - start.tv_sec;
    tt += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    tt /= NUM_OF_REPEATS;
    double flops = (2*k*m*n) / tt;
    double gflops = flops / (1 << 30);
    return gflops;
}

int main() {
    double flops;
    float* a = calloc(@m*@k, sizeof(float));
    float* b = calloc(@k*@n, sizeof(float));
    float* c = calloc(@m*@n, sizeof(float));
    float* true_c = calloc(@m*@n, sizeof(float));
    _set_random(@m, @k, a);
    _set_random(@k, @n, b);
    mm_rank1(a, b, true_c, @m, @k, @n);
    // flops = mm_gflops(@m, @k, @n, a, b, c, true_c, mm_rank1);
    // printf("Naive, num of gflops: %lf\n", flops);
    flops = mm_gflops(@m, @k, @n, a, b, c, true_c, mm_blas);
    if (flops < -0.5) {
        printf("Problem with BLAS implementation, result not as expected!\n");
    } else {
        printf("BLAS implementation, num of gflops: %lf\n", flops);
    }
    @for i, j in kernelsizelist
    flops = mm_gflops(@m, @k, @n, a, b, c, true_c, mm_@[i]x@[j]);
    if (flops < -0.5) {
        printf("Problem with kernel @[i]x@[j], result not as expected!\n");
    } else {
        printf("Kernel @[i]x@[j], num of gflops: %lf\n", flops);
    }
    @endfor
    return 0;
}
