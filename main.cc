#include <iostream>
#include <memory>
#include <chrono>
#include "mpi.h"

using namespace std;
using namespace chrono;


namespace {
  template<typename T> bool validate(const T&, const T&, const T&, size_t);

  //
  // Dead-Simple MPI wrapper
  //
  class mpi {
    int _rank, _size;
  public:
    mpi(int &argc, char **&argv) {
      MPI_Init(&argc, &argv);
      MPI_Comm_size(MPI_COMM_WORLD, &_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    }

    ~mpi() {
      MPI_Finalize();
    }

    int rank() const { return _rank; }
    int size() const { return _size; }

    bool root() const { return rank() == 0; }
  };
}


//
// Entry point
//
int main(int argc, char **argv) {
  const mpi mpi(argc, argv);

  size_t width = 8192;

  // Root-Only variables
  bool validation = false;
  if (mpi.root()) {
    //
    // Parse argc/argv
    //
    for (int opt; (opt = getopt(argc, argv, "n:vh")) != -1;) {
      switch (opt) {
      case 'n':
        width = stoul(string(optarg));
        if (width % mpi.size() != 0) { MPI_Abort(MPI_COMM_WORLD, 1); }
        break;

      case 'v':
        validation = true;
        break;

      case 'h':
      default:
        cout << R"(Parallel Matrix Multiplier

USAGE: )" << argv[0] << R"( [-pvh]

OPTIONS:
  -n <N>    Change width of a matrix (N*N). Default size is 2048.
  -v        Validate calculate results.
  -h        Print this page.
)";
        MPI_Abort(MPI_COMM_WORLD, opt != 'h');
      }
    }
  }

  MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Allocate buffers
  const size_t part = width*width/mpi.size();
  auto lhs    = unique_ptr<float[]>(new float[mpi.root() ? width*width : part]);
  auto rhs    = unique_ptr<float[]>(new float[width*width]);
  auto result = unique_ptr<float[]>(new float[mpi.root() ? width*width : part]());

  // Initialize buffers
  if (mpi.root()) {
    for (size_t i = 0; i < width*width; ++i) { lhs[i] = rhs[i] = float(i) + 1.0f; }
  }
  MPI_Scatter(&lhs[0], part, MPI_FLOAT, &lhs[0], part, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rhs[0], width*width, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // Start timer
  auto time = system_clock::now();
  MPI_Barrier(MPI_COMM_WORLD);

  // Calc
  for (size_t i = 0; i < width/mpi.size(); ++i) {
    for (size_t k = 0; k < width; ++k) {
      for (size_t j = 0; j < width; ++j) {
        result[i*width + j] += lhs[i*width + k] * rhs[k*width + j];
      }
    }
  }

  // Gather results
  MPI_Gather(&result[0], part, MPI_FLOAT, &result[0], part, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // Stop timer
  auto elapsed = duration<double>(system_clock::now() - time).count();
  if (mpi.root()) {
    cout << "\nTime elapsed: " << elapsed << " 초\n" << endl;
  }


  //
  // Validation
  //
  if (mpi.root()) {
    bool valid = true;
    if (validation) {
      cout << "Validating";
      valid = validate(lhs, rhs, result, width);
      cout << (valid ? "OK" : "Failed") << endl;
    }
    if (!valid) { MPI_Abort(MPI_COMM_WORLD, 1); }
  }

  return 0;
}


namespace {
  //
  // Error handling
  //
  template<typename T>
  bool validate(const T &lhs, const T &rhs, const T &result, size_t width) {
    cout << ' ';
    for (size_t i = 0; i < width; ++i) {
      if (i%64 == 63) { cout << "\033[1D.."; }
      if (i%4 == 3) {
        char rotate[] = {'/', '-', '\\', '|'};
        cout << "\033[1D" << rotate[i/4%4] << flush;
      }

      for (size_t j = 0; j < width; ++j) {
        float sum = 0;
        for (size_t k = 0; k < width; ++k) { sum += lhs[i*width + k]*rhs[k*width + j]; }

        if (result[i*width + j] != sum) { return false; }
      }
    }
    cout << "\033[1D. ";
    return true;
  }
}
