#define _POSIX_C_SOURCE 199309L
#include <iostream>
#include <memory>
#include <string>
#include <array>
#include <chrono>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <CL/cl.h>

using namespace std;
using namespace chrono;

namespace {
  void check(cl_int);
  template<typename T> bool validate(const T&, const T&, const T&, size_t);
}


//
// Entry point
//
int main(int argc, char* argv[]) {
  //
  // Parse argv
  //
  size_t width = 2048;
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


  //
  // Initialize host buffer
  //
  auto lhs    = unique_ptr<float[]>(new float[width*width]);
  auto rhs    = unique_ptr<float[]>(new float[width*width]);
  auto result = unique_ptr<float[]>(new float[width*width]);
  const size_t size = width*width*sizeof(float);

  cout << "Initializing " << width << '*' << width << " sized matrices... ";
  for (size_t i = 0; i < width*width; ++i) { lhs[i] = rhs[i] = float(i) + 1.0f; }
  cout << "Done" << endl;


  //
  // Start timer
  //
  auto begin = system_clock::now();


  //
  // Initialize OpenCL resources
  //
  cl_platform_id platform;
  check(clGetPlatformIDs(1, &platform, NULL));

  cl_device_id device;
  check(clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL));

  cl_int e;
  auto ctxt = clCreateContext(NULL, 1, &device, NULL, NULL, &e); check(e);
  auto cmdq = clCreateCommandQueue(ctxt, device, 0, &e); check(e);
  auto buffer_lhs = clCreateBuffer(ctxt, CL_MEM_READ_ONLY, size, NULL, &e); check(e);
  auto buffer_rhs = clCreateBuffer(ctxt, CL_MEM_READ_ONLY, size, NULL, &e); check(e);
  auto buffer_result = clCreateBuffer(ctxt, CL_MEM_WRITE_ONLY, size, NULL, &e); check(e);


  //
  // Compile OpenCL kernel codes
  //
  auto code = R"(
    __kernel void multiply(
        __global float *lhs,
        __global float *rhs,
        __global float *result,
        int width)
    {
      int i = get_global_id(0);
      int j = get_global_id(1);
      float sum = 0.0f;
      for (int k = 0; k < width; ++k) {
        sum += lhs[j*width + k]*rhs[k*width + i];
      }
      result[j*width + i] = sum;
    }
  )";
  const auto codelen = strlen(code);
  auto program = clCreateProgramWithSource(ctxt, 1, &code, &codelen, &e); check(e);
  e = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (e == CL_BUILD_PROGRAM_FAILURE) {
    // Print detailed message
    cerr << endl;
    cerr << "OpenCL compile error" << endl;

    size_t len, size = 2048;
    auto buffer = unique_ptr<char[]>(new char[2048]);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, buffer.get(), &len);
    cerr << buffer.get() << endl;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, size, buffer.get(), &len);
    cerr << buffer.get() << endl;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_OPTIONS, size, buffer.get(), &len);
    cerr << buffer.get() << endl;
    exit(1);
  }
  check(e);


  //
  // Create kernel and calculate
  //
  auto kernel = clCreateKernel(program, "multiply", &e); check(e);

  check(clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_lhs));
  check(clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_rhs));
  check(clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_result));
  check(clSetKernelArg(kernel, 3, sizeof(cl_int), &width));

  check(clEnqueueWriteBuffer(cmdq, buffer_lhs, CL_FALSE, 0, size, lhs.get(), 0, NULL, NULL));
  check(clEnqueueWriteBuffer(cmdq, buffer_rhs, CL_FALSE, 0, size, rhs.get(), 0, NULL, NULL));

  array<size_t, 2> global = {{ width, width }};
  check(clEnqueueNDRangeKernel(cmdq, kernel, 2, NULL, global.data(), NULL, 0, NULL, NULL));
  check(clEnqueueReadBuffer(cmdq, buffer_result, CL_TRUE, 0, size, result.get(), 0, NULL, NULL));


  //
  // Stop tiemr
  //
  auto end = system_clock::now();
  cout << "\nTime elapsed: " << duration<double>(end - begin).count() << "s\n" << endl;


  //
  // Validation
  //
  if (validation) {
    cout << "Validating";
    cout << (validate(lhs, rhs, result, width) ? "OK" : "Failed") << endl;
  }


  //
  // Release OpenCL resources
  //
  check(clReleaseMemObject(buffer_lhs));
  check(clReleaseMemObject(buffer_rhs));
  check(clReleaseMemObject(buffer_result));
  check(clReleaseContext(ctxt));
  check(clReleaseKernel(kernel));
  check(clReleaseProgram(program));
  check(clReleaseCommandQueue(cmdq));
}


//
// Error handling
//
namespace {
  const char *clGetErrorMessage(cl_int error_code) {
    switch (error_code) {
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
  }

  void check(cl_int err) {
    if (err == CL_SUCCESS) { return; }
    cerr << clGetErrorMessage(err) << endl;
    exit(1);
  }

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
