#include <cblas.h>

// matmul via rank 1 updates, no attempt at optimization
void mm_rank1(float const* a, float const* b, float* c, int m, int k, int n) {
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            c[j+n*i] = a[k*i]*b[j];
        }
    }
    for (int p = 1; p < k; ++p) {
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                c[j+n*i] += a[p+k*i]*b[j+n*p];
            }
        }
    }
}

// matmul via blas library
void mm_blas(float const* a, float const* b, float* c, int m, int k, int n) {
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1.0, a, m, b, k, 0.0, c, m);
}


