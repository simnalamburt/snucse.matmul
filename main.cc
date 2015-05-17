#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>

#define NDIM 2048
static float a[NDIM][NDIM], b[NDIM][NDIM], c[NDIM][NDIM];
static bool validation = false;
static const int THREAD_COUNT = 4;

static void calculate(void);


//
// Entry point
//
int main(int argc, char* argv[]) {
  // Parse argv
  for (int opt; (opt = getopt(argc, argv, "vh")) != -1;) {
    switch (opt) {
    case 'v': validation = true; break;

    case 'h':
    default:
      auto format = R"(Parallel Matrix Multiplier

USAGE: %s [-pvh]

OPTIONS:
-v : validate matrix multiplication.
-h : print this page.
)";

      printf(format, argv[0]);
      exit(opt != 'h');
    }
  }

  // Initialize a[][], b[][]
  int k = 1;
  for (int i = 0; i < NDIM; ++i) {
    for (int j = 0; j < NDIM; ++j) {
      a[i][j] = k;
      b[i][j] = k;
      k++;
    }
  }

  // Start timer
  struct timespec begin;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);

  // Calculate
  calculate();

  // Stop tiemr
  struct timespec end;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  printf("Time elapsed : %lf ms\n", (end.tv_nsec - begin.tv_nsec)/1000000.0);


  if (!validation) { return 0; }

  // Validation
  bool ok = true;
  for (int i = 0; i < NDIM; ++i) {
    for (int j = 0; j < NDIM; ++j) {
      float sum = 0;
      for (int k = 0; k < NDIM; ++k) { sum += a[i][k] * b[k][j]; }

      if (c[i][j] == sum) { continue; }
      printf("In c[%d][%d], expected %lf but it was %lf\n", i, j, sum, c[i][j]);
      ok = false;
    }
  }
  puts(ok ? "Test passed" : "Test failed");
  return 0;
}


//
// Computation codes
//
typedef struct {
  int begin, end;
} data_t;

void *per_thread(void* param) {
  data_t* data = (data_t*)param;
  for (int i = data->begin; i < data->end; ++i) {
    for (int k = 0; k < NDIM; ++k) {
      for (int j = 0; j < NDIM; ++j) {
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  return NULL;
}

void calculate(void) {
  size_t number = THREAD_COUNT;
  pthread_t thread[number];
  data_t data[number];

  for (size_t i = 0; i < number; ++i) {
    data[i].begin = NDIM*i/number;
    data[i].end = NDIM*(i+1)/number;
  }
  for (size_t i = 0; i < number; ++i) {
    pthread_create(&thread[i], NULL, per_thread, &data[i]);
  }
  for (size_t i = 0; i < number; ++i) {
    pthread_join(thread[i], NULL);
  }
}
