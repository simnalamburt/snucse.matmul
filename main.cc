#include <iostream>
#include <memory>
#include <chrono>

using namespace std;
using namespace chrono;

namespace {
  template<typename T> bool validate(const T&, const T&, const T&, size_t);
}


//
// Entry point
//
int main(int argc, char* argv[]) {
  //
  // Parse argv
  //
  size_t width = 4096;
  bool validation = false;
  for (int opt; (opt = getopt(argc, argv, "n:vh")) != -1;) {
    switch (opt) {
    case 'n':
      width = stoul(string(optarg));
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
      exit(opt != 'h');
    }
  }


  // Initialize buffer
  auto lhs    = unique_ptr<float[]>(new float[width*width]);
  auto rhs    = unique_ptr<float[]>(new float[width*width]);
  auto result = unique_ptr<float[]>(new float[width*width]);

  cout << "Initializing " << width << '*' << width << " sized matrices... ";
  for (size_t i = 0; i < width*width; ++i) { lhs[i] = rhs[i] = float(i) + 1.0f; }
  cout << "Done" << endl;


  // Start timer
  auto begin = system_clock::now();

  // Calc
  #pragma omp parallel for
  for (size_t i = 0; i < width; ++i) {
    for (size_t k = 0; k < width; ++k) {
      for (size_t j = 0; j < width; ++j) {
        result[i*width + j] += lhs[i*width + k] * rhs[k*width + j];
      }
    }
  }

  // Stop tiemr
  auto end = system_clock::now();
  cout << "\nTime elapsed: " << duration<double>(end - begin).count() << "s\n" << endl;


  //
  // Validation
  //
  bool valid = true;
  if (validation) {
    cout << "Validating";
    valid = validate(lhs, rhs, result, width);
    cout << (valid ? "OK" : "Failed") << endl;
  }
  return !valid;
}


//
// Error handling
//
namespace {
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
