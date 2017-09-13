#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "CudaCosineSimilarity.hpp"

#define NUMBER_OF_BLOCKS 256
#define NUMBER_OF_THREADS 64

// ==========
// Macro taken from:
// https://stackoverflow.com/questions/14038589/what-is-the-canonical-way-to-check-for-errors-using-the-cuda-runtime-api
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      printf("GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}
// ==========

__device__ int getElementsPerUnit(int total, int number_of_units) {
    int elements_per_unit = total / number_of_units;
    double remains = total % number_of_units;

    if(remains != 0) {
        elements_per_unit += 1;
    }

    return elements_per_unit;
}

__device__ double dotProduct(double *a, double *b, int size) {
    double result = 0;

    for(int i = 0; i < size; i++) {
        result += a[i] * b[i];
    }

    return result;
}

__global__ void dotProductKernel(double *a, double *b, int a_size, int b_size, double *results, int vector_size) {
    int a_vectors_per_block = getElementsPerUnit(a_size, gridDim.x);
    int b_vectors_per_thread = getElementsPerUnit(b_size, blockDim.x);

    // Get range of 'a' vectors we will work with
    int a_start = blockIdx.x * a_vectors_per_block;
    int a_end = a_start + a_vectors_per_block;

    if(a_end > a_size) {
        a_end = a_size;
    }

    // Get range of 'b' vectors we will work with
    int b_start = threadIdx.x * b_vectors_per_thread;
    int b_end = b_start + b_vectors_per_thread;

    if(b_end > b_size) {
        b_end = b_size;
    }

    if(a_start < a_size && b_start < b_size) {
        for(int a_index = a_start; a_index < a_end; a_index++) {
            for(int b_index = b_start; b_index < b_end; b_index++) {
                results[a_index*b_size + b_index] = dotProduct(&a[a_index*vector_size], &b[b_index*vector_size], vector_size);
            }
        }
    }
}

__global__ void normKernel(double *vectors, int size, double *results, int vector_size) {
    int vectors_per_block = getElementsPerUnit(size, gridDim.x);

    // Get range of vectors we will work with
    int start = blockIdx.x * vectors_per_block;
    int end = start + vectors_per_block;

    if(end > size) {
        end = size;
    }

    for(int vec_index = start; vec_index < end; vec_index++) {
        for(int i = 0; i < vector_size; i++) {
            results[vec_index] += pow(vectors[vec_index*vector_size + i], 2);
        }

        results[vec_index] = sqrt(results[vec_index]);
    }
}

__global__ void cosineSimilarityKernel(double *dot_products, int a_size, int b_size, double *a_norms, double *b_norms, double *results) {
    int a_vectors_per_block = getElementsPerUnit(a_size, gridDim.x);
    int b_vectors_per_thread = getElementsPerUnit(b_size, blockDim.x);

    int a_start = blockIdx.x * a_vectors_per_block;
    int a_end = a_start + a_vectors_per_block;

    if(a_end > a_size) {
        a_end = a_size;
    }

    int b_start = threadIdx.x * b_vectors_per_thread;
    int b_end = b_start + b_vectors_per_thread;

    if(b_end > b_size) {
        b_end = b_size;
    }

    for(int a_index = a_start; a_index < a_end; a_index++) {
        for(int b_index = b_start; b_index < b_end; b_index++) {
            results[a_index*b_size + b_index] = (double) dot_products[a_index*b_size + b_index] / (a_norms[a_index] * b_norms[b_index]);
        }
    }
}

double *cudaCosine(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2) {
    // Get size of first vector of v1 (same for all vectors, v2 also)
    int vector_size = v1[0].size();

    std::cout << "V1 Size: " << v1.size() << " V2 Size: " << v2.size();
    std::cout << " V1 Vector Size: " << v1[0].size() << " V2 Vector Size: " << v2[0].size();
    std::cout << " Results Size: " << v1.size()*v2.size() << std::endl;

    int v1_total_length = v1.size()*vector_size;
    int v2_total_length = v2.size()*vector_size;

    double *v1_ptr = new double[v1_total_length];
    double *v2_ptr = new double[v2_total_length];
    double *results = new double[v1.size()*v2.size()];

    for(int i = 0; i < v1.size(); i++) {
        for(int j = 0; j < vector_size; j++) {
            v1_ptr[i*vector_size + j] = v1[i][j];
        }
    }

    for(int i = 0; i < v2.size(); i++) {
        for(int j = 0; j < vector_size; j++) {
            v2_ptr[i*vector_size + j] = v2[i][j];
        }
    }

    size_t free_mem, total_mem;
    gpuErrchk(cudaMemGetInfo(&free_mem, &total_mem));
    std::cout << "GPU Memory: " << free_mem/1000 << " KB free, " << total_mem/1000 << " KB total." << std::endl;

    size_t v1_size = v1.size() * vector_size * sizeof(double);
    size_t v2_size = v2.size() * vector_size * sizeof(double);
    size_t v1_norms_size = v1.size() * sizeof(double);
    size_t v2_norms_size = v2.size() * sizeof(double);
    size_t results_size = v1.size() * v2.size() * sizeof(double);

    double *cu_v1_ptr, *cu_v2_ptr;
    double *cu_v1_norms, *cu_v2_norms;
    double *cu_dot_products;

    std::cout << "Allocating memory in the GPU" << std::endl;
    std::cout << "Allocating " << v1_size/1000 << " KB for V1" << std::endl;
    gpuErrchk(cudaMalloc((void **) &cu_v1_ptr, v1_size));
    std::cout << "Allocating " << v2_size/1000 << " KB for V2" << std::endl;
    gpuErrchk(cudaMalloc((void **) &cu_v2_ptr, v2_size));

    std::cout << "Allocating " << results_size/1000 << " KB for Dot Products" << std::endl;
    gpuErrchk(cudaMalloc((void **) &cu_dot_products, results_size));
    std::cout << "Allocating " << v1_norms_size/1000 << " KB for V1 Norms" << std::endl;
    gpuErrchk(cudaMalloc((void **) &cu_v1_norms, v1_norms_size));
    std::cout << "Allocating " << v2_norms_size/1000 << " KB for V2 Norms" << std::endl;
    gpuErrchk(cudaMalloc((void **) &cu_v2_norms, v2_norms_size));
    std::cout << "Initializing Dot Products array with zero values" << std::endl;
    gpuErrchk(cudaMemset(cu_dot_products, 0, results_size));

    std::cout << "Copying V1 vectors to the GPU" << std::endl;
    gpuErrchk(cudaMemcpy(cu_v1_ptr, v1_ptr, v1_size, cudaMemcpyHostToDevice));
    std::cout << "Copying V2 vectors to the GPU" << std::endl;
    gpuErrchk(cudaMemcpy(cu_v2_ptr, v2_ptr, v2_size, cudaMemcpyHostToDevice));

    cudaDeviceSynchronize();

    std::cout << "Executing Dot-Product Kernel on " << NUMBER_OF_BLOCKS << " Blocks and " << NUMBER_OF_THREADS << " Threads." << std::endl;
    dotProductKernel<<<NUMBER_OF_BLOCKS, NUMBER_OF_THREADS>>>(cu_v1_ptr, cu_v2_ptr, v1.size(), v2.size(), cu_dot_products, vector_size);

    std::cout << "Executing V1 Norms Kernel" << std::endl;
    normKernel<<<NUMBER_OF_BLOCKS, 1>>>(cu_v1_ptr, v1.size(), cu_v1_norms, vector_size);
    std::cout << "Executing V2 Norms Kernel" << std::endl;
    normKernel<<<NUMBER_OF_BLOCKS, 1>>>(cu_v2_ptr, v2.size(), cu_v2_norms, vector_size);

    cudaDeviceSynchronize();

    std::cout << "Executing Cosine Similarity Kernel" << std::endl;
    cosineSimilarityKernel<<<NUMBER_OF_BLOCKS, NUMBER_OF_THREADS>>>(cu_dot_products, v1.size(), v2.size(), cu_v1_norms, cu_v2_norms, cu_dot_products);

    cudaDeviceSynchronize();

    std::cout << "Getting results from GPU memory" << std::endl;
    gpuErrchk(cudaMemcpy(results, cu_dot_products, results_size, cudaMemcpyDeviceToHost));

    cudaFree(cu_v1_ptr);
    cudaFree(cu_v2_ptr);
    cudaFree(cu_v1_norms);
    cudaFree(cu_v2_norms);
    cudaFree(cu_dot_products);

    delete v1_ptr;
    delete v2_ptr;

    return results;
}
