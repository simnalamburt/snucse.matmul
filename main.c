#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>

#define NDIM 2048
static const int THREAD_COUNT = 4;

static float a[NDIM][NDIM];
static float b[NDIM][NDIM];
static float c[NDIM][NDIM];

static bool print_matrix = false;
static bool validation = false;

typedef struct {
  int begin, end;
} data_t;

void* per_thread(void* param) {
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

void mat_mul() {
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


void check_mat_mul() {
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

  if (ok) { return; }
  puts("Test failed");
}

void print_mat(float mat[NDIM][NDIM]) {
  for (int i = 0; i < NDIM; ++i) {
    for (int j = 1; j < NDIM; ++j) {
      printf("%8.2lf ", mat[i][j]);
    }
    printf("\n");
  }
}

void parse_opt(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "pvhikjs:")) != -1) {
    switch (opt) {
    case 'p': print_matrix = true; break;
    case 'v': validation = true; break;

    case 'h':
    default:
      printf(
          "Usage: %s [-pvh]\n"
          "\n"
          "OPTIONS\n"
          "  -p : print matrix data.\n"
          "  -v : validate matrix multiplication.\n"
          "  -h : print this page.\n"
          , argv[0]);
      exit(opt != 'h');
    }
  }
}


typedef struct timespec timespec_t;

// call this function to start a nanosecond-resolution timer
timespec_t timer_start() {
  timespec_t begin;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
  return begin;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(timespec_t begin) {
  timespec_t end;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  return end.tv_nsec - begin.tv_nsec;
}

int main(int argc, char* argv[]) {
  parse_opt(argc, argv);

  int k = 1;
  for (int i = 0; i < NDIM; ++i) {
    for (int j = 0; j < NDIM; ++j) {
      a[i][j] = k;
      b[i][j] = k;
      k++;
    }
  }

  timespec_t begin = timer_start();
  mat_mul();
  long elapsed = timer_end(begin);

  printf("Time elapsed : %lf ms\n", elapsed / 1000000.0);

  if (validation) { check_mat_mul(); }

  if (print_matrix) {
    puts("MATRIX A:");
    print_mat(a);

    puts("MATRIX B:");
    print_mat(b);

    puts("MATRIX C:");
    print_mat(c);
  }

  return 0;
}
