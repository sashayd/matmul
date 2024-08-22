I started writing some matrix multiplication tests after reading https://salykova.github.io/matmul-cpu.

I implemented a simple Python-backed "preprocessor macro" language in order to automate the construction of matrix multiplication kernels with different kernel sizes.

TODO: I didn't add yet cache utilization and I did not add the option of matrix sizes not divisible by kernel sizes. Also, probably a lot of bugs etc. The macro language can be polished and documented a bit more.

To run the test as it is now (on Linux):

1. Run the Python script `generate_c_files_from_templates.py`, which generates files such as `mm3x24.c` using the template file `mm_template.c` and `mm_time.c` using the template file `mm_time_template.c`.
   It uses `precopmiler.py` to process the `@...` instructions and it uses settings from the config files `kernel_sizes.txt` and `matrix_size.txt`.
2. Run the shell script `mm_time.sh` which compiles `mm_time.c` (it uses `-lblas` (for comparing with BLAS) and `-fopenmp` (for multithreading) flags) and then runs `mm_time.out`.
   For example, on my laptop I got the following output (this shows the number of giga FLOP/s obtained when multiplying 1440x1440 matrices):
   ```
   BLAS implementation, num of gflops: 661.409937
   Kernel 1x8, num of gflops: 61.230235
   Kernel 1x16, num of gflops: 101.330824
   Kernel 1x24, num of gflops: 112.155428
   Kernel 1x32, num of gflops: 120.382050
   Kernel 2x8, num of gflops: 100.950594
   Kernel 2x16, num of gflops: 170.828675
   Kernel 2x24, num of gflops: 193.705840
   Kernel 2x32, num of gflops: 212.986526
   Kernel 3x8, num of gflops: 151.810526
   Kernel 3x16, num of gflops: 237.845258
   Kernel 3x24, num of gflops: 267.749480
   Kernel 4x8, num of gflops: 197.239656
   Kernel 4x16, num of gflops: 288.317405
   Kernel 5x8, num of gflops: 218.152263
   Kernel 6x8, num of gflops: 233.878741
   ```
