#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include "arr.h"
#include "mm.h"

float init_rand(float placeholder) { return (int)(10 * (float)rand() / RAND_MAX); }
float square(float x) { return x * x; }

double gettime() {
  struct timeval ret;
  gettimeofday(&ret, NULL);
  return (double)ret.tv_sec + (double)ret.tv_usec / 1000000.;
}

#ifdef HAVE_BLAS
char *mm_options = "<naive/tiled/blas/goto/vec_trans>";
#else
char *mm_options = "<naive/tiled/goto/vec_trans>";
#endif

// if (*mm_fn)(Matrix, Matrix) = Matrix, then (*mm_fn) = function of params (Matrix, Matrix) returning Matrix
// therefore, mm_fn = pointer to (function of params (Matrix, Matrix) returning Matrix)
typedef struct {
  Matrix (*fn)(Matrix, Matrix);
  char *name;
} mm_fn;
mm_fn str_to_matmul_fn(char* fn_str) {
  if (strcmp(fn_str, "naive") == 0)
    return (mm_fn) {p_matmul, "Naive"};
  if (strcmp(fn_str, "tiled") == 0)
    return (mm_fn) {cache_tiled_matmul, "Cache tiled"};
  if (strcmp(fn_str, "vec_trans") == 0)
    return (mm_fn) {vector_transpose_mm, "Vector-transposed"};
  if (strcmp(fn_str, "goto") == 0)
    return (mm_fn) {goto_mm, "Custom GotoBLAS"};
#ifdef HAVE_BLAS
  if (strcmp(fn_str, "blas") == 0)
    return (mm_fn) {blas_mm, "System BLAS"};
#endif

  fprintf(stderr, "\033[0;31mERROR:\033[0m function \"%s\" not in: %s\n", fn_str, mm_options);
  exit(-1);
}

int main(int argc, char *argv[])
{
  char *mm1_str = "goto";
  char *mm2_str = "goto";
  int M = 2048;
  int N = 2048;
  int P = 2048;
  int loop = 1;
  float cooldown_time = 1;
  int opt;
  while ((opt = getopt(argc, argv, "m:n:p:a:b:l:c:h")) != -1) {
    switch(opt) {
      case 'm': M = atoi(optarg); break;
      case 'n': N = atoi(optarg); break;
      case 'p': P = atoi(optarg); break;
      case 'a': mm1_str = optarg; break;
      case 'b': mm2_str = optarg; break;
      case 'l': loop = atoi(optarg); break;
      case 'c': cooldown_time = atof(optarg); break;
      case 'h': default:
        fprintf(stderr, "Usage: %s [FLAGS]\n", argv[0]);
        fprintf(stderr, "Flags:\n");
        fprintf(stderr, "  -m M\t\t\t\t\t\tNumber of rows for matrix A (default: 2048)\n");
        fprintf(stderr, "  -n N\t\t\t\t\t\tNumber of columns/rows for matrix A/B respectively(default: 2048)\n");
        fprintf(stderr, "  -p P\t\t\t\t\t\tNumber of columns for matrix B (default: 2048)\n");
        fprintf(stderr, "  -a %s\t\tMethod 1 to run matrix multiplication (default: goto)\n", mm_options);
        fprintf(stderr, "  -b %s\t\tMethod 2 to run matrix multiplication (default: goto)\n", mm_options);
        fprintf(stderr, "  -l LOOP\t\t\t\t\tNumber of times to rerun matmul (default: 1)\n");
        fprintf(stderr, "  -c COOLDOWN\t\t\t\t\tTime (in seconds) delay (to allow CPU to cool down)\n\t\t\t\t\t\tbetween matmuls (default: 1)\n");
        fprintf(stderr, "  -h\t\t\t\t\t\tDisplay this help description\n\n");
        fprintf(stderr, "\nMatrix Multiplaction Methods\n");
        fprintf(stderr, "	naive\t\tStandard (naive) matmul\n");
        fprintf(stderr, "	tiled\t\tCache-tiled matmul\n");
        fprintf(stderr, "	blas\t\tSystem BLAS implementation\n");
        fprintf(stderr, "	goto\t\tCustom GotoBLAS implementation written in C\n");
        fprintf(stderr, "	vec_trans\tVector-transposed (just a name I gave the method,\n\t\t\tnot really an actual technique) matmul\n\t\t\treference: https://www.youtube.com/watch?v=VgSQ1GOC86s\n");
        return -1;
    }
  }

#ifdef HAVE_BLAS
    printf("System BLAS implementation found\n");
#else
    printf("Program not compiled with system BLAS implementation\n");
#endif

#ifdef __AVX__
    printf("Using AVX instructions; ");
#elif __SSE__
    printf("Using SSE instructions; ");
#endif

#if defined(_OPENMP)
#include <omp.h>
  printf("using %d thread(s)\n\n", omp_get_max_threads());
#else
  printf("using 1 thread (not compiled with OpenMP)\n\n");
#endif

  printf("M: %d, N: %d, P: %d\n", M, N, P);
  mm_fn mm1 = str_to_matmul_fn(mm1_str);
  mm_fn mm2 = str_to_matmul_fn(mm2_str);
  printf("Method 1: %s, Method 2: %s\n", mm1.name, mm2.name);
  printf("Looping %d time(s)\n", loop);
  printf("\n");
  printf("======================================\n");

  srand((unsigned int)(gettime() * 1000000));
  for (int _loop_idx = 0; _loop_idx < loop; ++_loop_idx) { 
    Matrix a = pointwise_inplace(initMat(M, N), init_rand);
    Matrix b = pointwise_inplace(initMat(N, P), init_rand);
    double ss, duration, duration2, gflop;

    ss = gettime();
    Matrix c = (*mm1.fn)(a, b);
    duration = gettime() - ss;
    gflop = 2.0*M*N*P / 1e9;
    printf("%s matmul duration: %.5fs (%.3f GFLOP/s)\n", mm1.name, duration, gflop / duration);

    sleep(cooldown_time);

    ss = gettime();
    Matrix c2 = (*mm2.fn)(a, b);
    duration2 = gettime() - ss;
    printf("%s matmul duration: %.5fs (%.3f GFLOP/s)\n", mm2.name, duration2, gflop / duration2);

    printf("======================================\n");
    printf("%s speedup (over %s): %.3fx\n", mm2.name, mm1.name, duration / duration2);

    // check for correctness between methods
    //Matrix diff = sub_mm(c, c2);
    //float mse = mean(pointwise_inplace(diff, square));
    //if (mse < 1)
    //  printf("\033[92m"); // green font
    //else
    //  printf("\033[91m"); // red font
    //printf("Mean squared error: %f\n\033[0m", mse); // green font
    //free(diff.w);

    free(a.w);
    free(b.w);
    free(c.w);
    free(c2.w);
    printf("\n");

    if (loop != 1)
      sleep(cooldown_time);
  }
}
