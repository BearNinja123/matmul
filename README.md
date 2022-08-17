# matmul
A collection of matrix multiplication methods in C, from naive to GotoBLAS-style GEMM

The program runs two methods (specified in CLI args) and multiplies a pair of matrices
(sizes also specified by CLI args). It displays the duration of completion as well as GFLOP/s
for both methods, and compares their speeds against each other.

To run the program, just run ```make```, then ```./mm -h``` in a terminal.

## Make flags

By default the program is compiled with OpenMP and BLAS. To disable OpenMP,
run ```make NO_OMP=1```.

To disable BLAS, run ```make NO_BLAS=1```.

## Speed comparison

(Tests were run on an Intel i5-6300HQ with 4 cores running on Debian 11)

Method | Duration (seconds, 2048x2048) | GFLOP/s (2048x2048) | Relative speed over system BLAS (2048x2048) | Duration (seconds, 4096x4096) | GFLOP/s (4096x4096) | Relative speed over system BLAS (4096x4096)
---|---:|---:|---:|---:|---:|---:
Naive  | 0.578 | 29.71 | 0.102 | 4.84 | 28.34 | 0.092
Cache-tiled | 0.287 | 59.90 | 0.206 | 2.54 | 54.04 | 0.175
Vector-transposed* | 0.090 | 190.66 | 0.655 | 0.693 | 192.41 | 0.622
My implementation of GotoBLAS GEMM (written in C) | 0.073 | 234.62 | 0.806 | 0.546 | 251.33 | 0.813
BLAS (OpenBLAS 0.3.13) | 0.059 | 291.14 | 1.000 | 0.444 | 309.27 | 1.000

*Vector-transposed matrix multiplication isn't an actual method I found, it was just the implementation that I saw on a video.

## Useful references

[MIT lecture on optimizing matrix multiplication](https://www.youtube.com/watch?v=o7h_sYMk_oc)

[Cache blocking (cache-tiled) matrix multiplication](https://www.youtube.com/watch?v=G92BCtfTwOE)

[OpenBLAS website](https://www.openblas.net/)

[GotoBLAS paper](https://www.cs.utexas.edu/users/flame/pubs/GotoTOMS_final.pdf)

[GotoBLAS diagram and pseudocode](https://www.researchgate.net/publication/307564216_BLISlab_A_Sandbox_for_Optimizing_GEMM)

[High-level GotoBLAS video explanation](https://www.youtube.com/watch?v=07SMaudtH6k)

[Reference code for vector-transposed matmul](https://github.com/geohot/tinygrad/blob/gemm/extra/gemm/gemm.c)

[Livestream of George Hotz implementing matmul](https://www.youtube.com/watch?v=VgSQ1GOC86s)
