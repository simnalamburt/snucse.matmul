#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include "timers.h"

static int number, NDIM;

float **a;
float **b;
float **c;

int print_matrix = 0;
int validation = 0;

typedef struct {
  int begin, end;
} data_t;

void per_thread(void* param) {
  data_t* data = (data_t*)param;
  for (int i = data->begin; i < data->end; ++i) {
    for (int j = 0; j < NDIM; ++j) {
      for (int k = 0; k < NDIM; ++k) {
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

void mat_mul(float **c, float **a, float **b) {
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

/************************** DO NOT TOUCH BELOW HERE ******************************/

void check_mat_mul(float **c, float **a, float **b)
{
  int i, j, k;
  float sum;
  int validated = 1;

  printf("Validating the result..\n");

  // C = AB
  for( i = 0; i < NDIM; i++ )
  {
    for( j = 0; j < NDIM; j++ )
    {
      sum = 0;
      for( k = 0; k < NDIM; k++ )
      {
        sum += a[i][k] * b[k][j];
      }

      if( c[i][j] != sum )
      {
        printf("c[%d][%d] is differ(value=%lf correct_value=%lf)!!\n", i, j, c[i][j], sum );
        validated = 0;
      }
    }
  }

  printf("Validation : ");
  if( validated )
    printf("SUCCESSFUL.\n");
  else
    printf("FAILED.\n");
}

void print_mat( float **mat )
{
  int i, j;

  for( i = 0; i < NDIM; i++ )
  {
    for( j = 0; j < NDIM; j++ )
    {
      printf("%8.2lf ", mat[i][j]);
    }
    printf("\n");
  }
}

void print_help(const char* prog_name)
{
  printf("Usage: %s [-pvh]\n", prog_name );
  printf("\n");
  printf("OPTIONS\n");
  printf("  -p : print matrix data.\n");
  printf("  -v : validate matrix multiplication.\n");
  printf("  -h : print this page.\n");
}

void parse_opt(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "%s <thread count> <matrix dim>\n", argv[0]);
    exit(1);
  }
  number = atoi(argv[1]);
  NDIM = atoi(argv[2]);

  a = malloc(NDIM * sizeof *a);
  b = malloc(NDIM * sizeof *b);
  c = malloc(NDIM * sizeof *c);
  for (int i = 0; i < NDIM; ++i) {
    a[i] = malloc(NDIM * sizeof **a);
    b[i] = malloc(NDIM * sizeof **b);
    c[i] = malloc(NDIM * sizeof **c);
  }
}

int main(int argc, char** argv)
{
  int i, j, k = 1;

  parse_opt( argc, argv );

  for( i = 0; i < NDIM; i++ )
  {
    for( j = 0; j < NDIM; j++ )
    {
      a[i][j] = k;
      b[i][j] = k;
      k++;
    }
  }

  timer_start(1);
  mat_mul( c, a, b );
  timer_stop(1);

  printf("%d,%lf\n", number, timer_read(1));


  if( validation )
    check_mat_mul( c, a, b );

  if( print_matrix )
  {
    printf("MATRIX A: \n");
    print_mat(a);

    printf("MATRIX B: \n");
    print_mat(b);

    printf("MATRIX C: \n");
    print_mat(c);
  }

  return 0;
}
