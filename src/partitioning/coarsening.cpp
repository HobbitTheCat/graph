#include "partitioning/coarsening.h"

#include <numeric>
#include <random>
#include <algorithm>

void find_heavy_edge_matching(const Graph& G, std::vector<int>& cmap, int& nextVertexIdx) {
    int n = G.num_vertices;

    std::vector<int> match(n, -1);                // information on which pairs have already formed
    std::vector<int> p(n);                              // Create an array of length equal to the number of vertices (permutation)
    std::iota(p.begin(), p.end(), 0);    // Fill it with vertex indices from 0 to n-1

    // std::ranges::shuffle(p, std::mt19937(std::random_device()()));  // Shuffle to ensure a random traversal order
    std::sort(p.begin(), p.end(), [&](int a, int b) {
        int degA = G.offsets[a+1] - G.offsets[a];
        int degB = G.offsets[b+1] - G.offsets[b];
        return degA < degB;
    });

    nextVertexIdx = 0;
    cmap.assign(n, -1);                  // Allocate space for the map and set it to -1 to start

    for (int vertexId : p) {                 // We traverse all vertices in random order
        if (match[vertexId] != -1) continue; // This vertex has already formed a pair

        int best_neighbor_id = -1;
        int max_edge_weight = -1; // TODO в случае с отрицательными весами заменить

        for (int i = G.offsets[vertexId]; i < G.offsets[vertexId + 1]; i++) {   // We visit all the neighbors of the vertex
            int neighbor_id = G.edges[i];                                       // get the vertex ID
            if (neighbor_id != vertexId && match[neighbor_id] == -1) {          // Check that it isn't a loop and that the top hasn't collapsed
                if (G.edge_weights[i] > max_edge_weight) {                      // Here, we choose the heaviest of the chains
                    max_edge_weight = G.edge_weights[i];
                    best_neighbor_id = neighbor_id;
                }
            }
        }

        if (best_neighbor_id != -1) {                                           // if we've found a suitable vertex
            match[vertexId] = best_neighbor_id;                                 // record it as a compound
            match[best_neighbor_id] = vertexId;                                 // and the second one too
            cmap[vertexId] = cmap[best_neighbor_id] = nextVertexIdx++;    // mapping both to the new vertex, keeping the index
        } else {
            cmap[vertexId] = nextVertexIdx++;                        // If can't find a suitable vertex, just map it to a new one
        }
    }
}

void build_coarse_graph(const Graph& G, const std::vector<int>& cmap, int num_coarse_vertices, Graph& new_G) {
    new_G.num_vertices = num_coarse_vertices;                       // Specify the number of vertices in the new graph
    new_G.vertex_weights.assign(num_coarse_vertices, 0);        // Allocate an array for the vertex weights
    new_G.offsets.assign(num_coarse_vertices + 1, 0);        // Allocate an array for shifts

    std::vector<std::vector<int>> groups(num_coarse_vertices);
    for (int i = 0; i < G.num_vertices; i++) {
        groups[cmap[i]].push_back(i);
        new_G.vertex_weights[cmap[i]] += G.vertex_weights[i];       // The weight of a vertex is calculated as the sum of the weights of its child vertices
    }

    std::vector<int> new_edges;
    std::vector<int> new_edge_weights;
    std::vector<int> marker(num_coarse_vertices, -1);
    std::vector<int> acc_weights(num_coarse_vertices, 0);     // Enter the weights of the new connections here

    for (int super_vertex = 0; super_vertex < num_coarse_vertices; super_vertex++) {
        new_G.offsets[super_vertex] = new_edges.size(); // We note the starting point of the description of the current supervertex
        // new_edges.size() This is the current size of the array

        for (int vertex : groups[super_vertex]) {   // We iterate through all vertices that are part of the supervertex
            for (int i = G.offsets[vertex]; i < G.offsets[vertex + 1]; i++) { // We iterate through all neighbors in the initial graph
                int super_vertex_of_neighbor = cmap[G.edges[i]];  // Let's see what amazing peak our neighbor has reached

                if (super_vertex_of_neighbor != super_vertex) {   // unless it's the same one (in which case it's not a loop)
                    if (marker[super_vertex_of_neighbor] < super_vertex) {  // check whether we have encountered this edge for the current supervertex
                        marker[super_vertex_of_neighbor] = super_vertex;    // wrote down that they had met
                        new_edges.push_back(super_vertex_of_neighbor);      // added a new link
                        acc_weights[super_vertex_of_neighbor] = G.edge_weights[i];
                    } else {                                      // already passed this vertex
                        acc_weights[super_vertex_of_neighbor] += G.edge_weights[i];
                    }
                }
            }
        }

        for (int i = new_G.offsets[super_vertex]; i < new_edges.size(); i++) {
            new_edge_weights.push_back(acc_weights[new_edges[i]]);
        }
    }

    new_G.offsets[num_coarse_vertices] = new_edges.size();
    new_G.edges = std::move(new_edges);
    new_G.edge_weights = std::move(new_edge_weights);
}

void coarse_step(const Graph& G, CoarseLevel& next_level) {
    int num_coarse_vertices = 0;
    find_heavy_edge_matching(G, next_level.cmap, num_coarse_vertices);
    build_coarse_graph(G, next_level.cmap, num_coarse_vertices, next_level.graph);
}