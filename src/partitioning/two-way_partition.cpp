#include "Graph.h"
#include "partitioning/multilevel_partition.h"

#include <vector>
#include <queue>
#include <climits>
#include <random>

void two_way_partition(const Graph& G, std::vector<int>& part) {
    int n = G.num_vertices;
    part.assign(n, -1);

    if (n == 0) return; // TODO: Check that this will always work

    // select a random vertex
    // std::mt19937 rng(std::random_device{}());
    // std::uniform_int_distribution<int> dist(0, n-1);
    // int start_node = dist(rng);
    int start_node = 0;
    int min_deg = INT_MAX;
    for (int i = 0; i < n; i++) {
        int deg = G.offsets[i + 1] - G.offsets[i];
        if (deg < min_deg) {min_deg = deg; start_node = i;}
        if (min_deg == 1) break;
    }

    std::priority_queue<std::pair<int, int>> frontier;  // Create an array of adjacent elements of type `pair<priority, index>`
    std::vector<int> gains(n, 0);

    part[start_node] = 0;                                   // add the starting vertex to the first part
    int current_weight_E = G.vertex_weights[start_node];    // initialize the weight of the first part to be equal to the weight of its only vertex
    int total_weight_V = 0;                                 // initialize the graph's weight
    for (double w : G.vertex_weights) total_weight_V += w;  // obtain the total weight of the graph

    for (int i = G.offsets[start_node]; i < G.offsets[start_node+1]; i++) {     // for all neighbors of start_node
        int neighbor_id = G.edges[i];                   // take a neighbor
        if (neighbor_id != start_node) {                // check the loop
            gains[neighbor_id] += G.edge_weights[i];    // gain is equal to the link weight to that neighbor
            frontier.push({gains[neighbor_id], neighbor_id});
        }
    }

    while (!frontier.empty() && current_weight_E < total_weight_V / 2.0) {
        int v = frontier.top().second;           // take the first vertex in the queue
        frontier.pop();                          // remove it from the queue

        if (part[v] == 0) continue;              // checked to ensure that vertex is not yet in this section

        part[v] = 0;                             // move the vertex to the zero part
        current_weight_E += G.vertex_weights[v]; // add its weight

        for (int i = G.offsets[v]; i < G.offsets[v+1]; i++) {
            int neighbor_id = G.edges[i];        // take the neighbor of the added vertex
            if (part[neighbor_id] == -1) {       // if it hasn't been allocated yet
                gains[neighbor_id] += G.edge_weights[i];    // add the winnings
                frontier.push({gains[neighbor_id], neighbor_id});   // add it to the boundary
            }
        }
    }

    for (int i = 0; i < n; i++) {               // anything that isn't 0 is 1
        if (part[i] == -1) part[i] = 1;
    }
}