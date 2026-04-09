#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <vector>
#include <ctime>

#include "Graph.h"
#include "visual/force_layout.h"



__global__ void initNodesKernel(float* x, float* y, float* z,
                                float* vx, float* vy, float* vz,
                                curandState* states, unsigned long seed, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        curand_init(seed, i, 0, &states[i]);

        x[i] = curand_uniform(&states[i]) * 200.0f - 100.0f;
        y[i] = curand_uniform(&states[i]) * 200.0f - 100.0f;
        z[i] = curand_uniform(&states[i]) * 200.0f - 100.0f;

        vx[i] = vy[i] = vz[i] = 0.0f;
    }
}

DeviceGraph setupDeviceGraph(const Graph& hostGraph) {
    DeviceGraph dg;
    dg.num_vertices = hostGraph.num_vertices;
    dg.num_edges = hostGraph.num_edges;

    size_t vSize = dg.num_vertices * sizeof(float);
    size_t eSize = dg.num_edges * sizeof(int);
    size_t oSize = (dg.num_vertices + 1) * sizeof(int);

    cudaMalloc((void**) &dg.offsets, oSize);
    cudaMalloc((void**) &dg.edges, eSize);

    cudaMalloc((void**) &dg.posX, vSize);
    cudaMalloc((void**) &dg.posY, vSize);
    cudaMalloc((void**) &dg.posZ, vSize);
    cudaMalloc((void**) &dg.velX, vSize);
    cudaMalloc((void**) &dg.velY, vSize);
    cudaMalloc((void**) &dg.velZ, vSize);

    cudaMalloc((void**) &dg.randStates, dg.num_vertices * sizeof(curandState));

    cudaMemcpy(dg.offsets, hostGraph.offsets.data(), oSize, cudaMemcpyHostToDevice);
    cudaMemcpy(dg.edges, hostGraph.edges.data(), eSize, cudaMemcpyHostToDevice);

    int blockSize = 256;
    int gridSize = (dg.num_vertices + blockSize - 1) / blockSize;

    initNodesKernel<<<gridSize, blockSize>>>(
        dg.posX, dg.posY, dg.posZ,
        dg.velX, dg.velY, dg.velZ,
        dg.randStates, time(NULL), dg.num_vertices
    );

    cudaDeviceSynchronize();
    return dg;
}

__global__ void repulsionKernel(float* posX, float* posY, float* posZ,
                                float* velX, float* velY, float* velZ,
                                int num_vertices, float k) {
    __shared__ float shX[BLOCK_SIZE];
    __shared__ float shY[BLOCK_SIZE];
    __shared__ float shZ[BLOCK_SIZE];

    int i = blockIdx.x * blockDim.x + threadIdx.x;

    float ix = 0, iy = 0, iz = 0;
    float fx = 0.0f, fy = 0.0f, fz = 0.0f;

    if (i < num_vertices) {
        ix = posX[i]; iy = posY[i]; iz = posZ[i];
    }

    for (int j_start = 0; j_start < num_vertices; j_start += BLOCK_SIZE) {
        int j_load = j_start + threadIdx.x;
        if (j_load < num_vertices) {
            shX[threadIdx.x] = posX[j_load];
            shY[threadIdx.x] = posY[j_load];
            shZ[threadIdx.x] = posZ[j_load];
        }
        __syncthreads();

        if (i < num_vertices) {
            for (int t = 0; t < BLOCK_SIZE; t++) {
                int j_global = j_start + t;
                if (j_global < num_vertices && j_global != i) {
                    float dx = ix - shX[t];
                    float dy = iy - shY[t];
                    float dz = iz - shZ[t];

                    float distSq = dx * dx + dy * dy + dz * dz + 1e-6f;
                    float dist = sqrtf(distSq);

                    float force = (k * k) / dist;
                    if (force > 500.0f) force = 500.0f;

                    fx += (dx / dist) * force;
                    fy += (dy / dist) * force;
                    fz += (dz / dist) * force;
                }
            }
        }

        __syncthreads();
    }

    if (i < num_vertices) {
        velX[i] = fx;
        velY[i] = fy;
        velZ[i] = fz;
    }
}

__global__ void attractionKernel(
    const float* posX, const float* posY, const float* posZ,
    float* velX, float* velY, float* velZ,
    const int* offsets, const int* edges,
    int num_vertices, float k) {

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_vertices) return;

    float ix = posX[i];
    float iy = posY[i];
    float iz = posZ[i];

    float fx = 0.0f, fy = 0.0f, fz = 0.0f;

    int start = offsets[i];
    int end = offsets[i + 1];

    for (int e = start; e < end; e++) {
        int j = edges[e];

        float dx = posX[j] - ix;
        float dy = posY[j] - iy;
        float dz = posZ[j] - iz;

        float distSq = dx * dx + dy * dy + dz * dz + 1e-6f;
        float dist = sqrtf(distSq);

        float force = dist / k;

        fx += dx * force;
        fy += dy * force;
        fz += dz * force;
    }

    velX[i] += fx;
    velY[i] += fy;
    velZ[i] += fz;
}

__global__ void applyPhysicsKernel(float* posX, float* posY, float* posZ,
                                   float* velX, float* velY, float* velZ,
                                   int num_vertices, float dt, float damping) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= num_vertices) return;

    posX[i] += velX[i] * dt;
    posY[i] += velY[i] * dt;
    posZ[i] += velZ[i] * dt;

    velX[i] *= damping;
    velY[i] *= damping;
    velZ[i] *= damping;
}

__global__ void repackPositionsKernel(const float* x, const float* y, const float* z,
                                      float* vboPtr, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        vboPtr[i * 3 + 0] = x[i];
        vboPtr[i * 3 + 1] = y[i];
        vboPtr[i * 3 + 2] = z[i];
    }
}

extern "C" void launchRepackKernel(const DeviceGraph& dg, float* d_vboPtr) {
    int gridSize = (dg.num_vertices + BLOCK_SIZE - 1) / BLOCK_SIZE;
    repackPositionsKernel<<<gridSize, BLOCK_SIZE>>>(
        dg.posX, dg.posY, dg.posZ, d_vboPtr, dg.num_vertices
    );
    cudaDeviceSynchronize();
}

void step(DeviceGraph& dg, float k, float dt, float damping) {
    int blockSize = BLOCK_SIZE;
    int gridSize = (dg.num_vertices + blockSize - 1) / blockSize;

    repulsionKernel<<<gridSize, blockSize>>>(dg.posX, dg.posY, dg.posZ, dg.velX, dg.velY, dg.velZ, dg.num_vertices, k);
    attractionKernel<<<gridSize, blockSize>>>(dg.posX, dg.posY, dg.posZ, dg.velX, dg.velY, dg.velZ, dg.offsets, dg.edges, dg.num_vertices, k);
    applyPhysicsKernel<<<gridSize, blockSize>>>(dg.posX, dg.posY, dg.posZ, dg.velX, dg.velY, dg.velZ, dg.num_vertices, dt, damping);

    cudaDeviceSynchronize();
}

void freeDeviceGraph(DeviceGraph& dg) {
    cudaFree(dg.offsets);
    cudaFree(dg.edges);
    cudaFree(dg.posX);
    cudaFree(dg.posY);
    cudaFree(dg.posZ);
    cudaFree(dg.velX);
    cudaFree(dg.velY);
    cudaFree(dg.velZ);
    cudaFree(dg.randStates);
}