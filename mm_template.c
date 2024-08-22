#include <immintrin.h>

@comment should set mr and nr

@assert nr % 8 == 0
@set mm, nn = mr, nr//8

// @[mr]x@[nr] kernel for matrix multiplication, utilizing the avx registers
void avx_kernel_@[mr]x@[nr](float const* a, float const* b, float* c, int m, int k, int n) {
    __m256 ar[@mm];
    __m256 br[@nn];
    __m256 cr[@mm][@nn];
    @for i in range(mm)
    ar[@i] = _mm256_broadcast_ss(a+k*@i);
    @endfor
    @for j in range(nn)
    br[@j] = _mm256_loadu_ps(b + 8*@j);
    @endfor
    @for i, j in product(range(mm), range(nn))
    cr[@i][@j] = _mm256_mul_ps(ar[@i], br[@j]);
    @endfor
    for (int h = 1; h < k; ++h) {
        @for i in range(mm)
        ar[@i] = _mm256_broadcast_ss(a + k*@i + h);
        @endfor
        @for j in range(nn)
        br[@j] = _mm256_loadu_ps(b + n*h + @[8*j]);
        @endfor
        @for i,j in product(range(mm), range(nn))
        cr[@i][@j] = _mm256_fmadd_ps(ar[@i], br[@j], cr[@i][@j]);
        @endfor
    }
    @for i,j in product(range(mm), range(nn))
    _mm256_storeu_ps(c+n*@i+@[8*j], cr[@i][@j]);
    @endfor
}

// matmul utilizing the @[mr]x@[nr] avx kernel
void mm_@[mr]x@[nr](float const* a, float const* b, float* c, int m, int k, int n) {
#pragma omp parallel for num_threads(16) schedule(static)
    for (int i = 0; i < m; i += @mr) {
        for (int j = 0; j < n; j += @nr) {
            avx_kernel_@[mr]x@[nr](a + k*i, b + j, c + n*i + j, m, k, n);
        }
    }
}




