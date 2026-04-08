//
// Created by basilisk on 08/04/2026.
//

#ifndef PROJECT_GRAPH_FORCE_LAYOUT_H
#define PROJECT_GRAPH_FORCE_LAYOUT_H

#include <cuda_runtime.h>
#include <curand_kernel.h>
#include "Graph.h"

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

struct DeviceGraph {
    int num_vertices;
    int num_edges;

    int* offsets;
    int* edges;

    float *posX, *posY, *posZ;
    float *velX, *velY, *velZ;

    curandState* randStates;
};

DeviceGraph setupDeviceGraph(const Graph& hostGraph);
void step(DeviceGraph& dg, float k, float dt, float damping);
void freeDeviceGraph(DeviceGraph& dg);

extern "C" void launchRepackKernel(const DeviceGraph& dg, float* d_vboPtr);

#endif //PROJECT_GRAPH_FORCE_LAYOUT_H
