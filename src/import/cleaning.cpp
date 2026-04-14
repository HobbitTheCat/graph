#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <numeric>

#include "Graph.h"

void clean_disconnected_parts(const Graph& G, Graph& clean_G) {
    if (G.num_vertices == 0) return;

    std::vector<int> component_id(G.num_vertices, -1);
    std::vector<int> component_sizes;
    int num_components = 0;

    for (int i = 0; i < G.num_vertices; i++) {
        if (component_id[i] == -1) {
            int size = 0;
            std::queue<int> q;
            q.push(i);
            component_id[i] = num_components;

            while (!q.empty()) {
                int v = q.front();
                q.pop();
                size++;

                for (int e = G.offsets[v]; e < G.offsets[v + 1]; e++) {
                    int neighbor = G.edges[e];
                    if (component_id[neighbor] == -1) {
                        component_id[neighbor] = num_components;
                        q.push(neighbor);
                    }
                }
            }
            component_sizes.push_back(size);
            num_components++;
        }
    }

    int largest_comp_id = std::distance(
        component_sizes.begin(),
         std::max_element(component_sizes.begin(), component_sizes.end())
    );

    std::vector<int> old_to_new(G.num_vertices, -1);
    int new_vertex_count = 0;
    for (int i = 0; i < G.num_vertices; i++) {
        if (component_id[i] == largest_comp_id) {
            old_to_new[i] = new_vertex_count++;
        }
    }

    clean_G.num_vertices = new_vertex_count;
    clean_G.offsets.push_back(0);

    for (int i = 0; i < G.num_vertices; i++) {
        if (old_to_new[i] != -1) {
            for (int e = G.offsets[i]; e < G.offsets[i + 1]; e++) {
                int old_neighbor = G.edges[e];
                if (old_to_new[old_neighbor] != -1) {
                    clean_G.edges.push_back(old_to_new[old_neighbor]);
                    clean_G.edge_weights.push_back(G.edge_weights[e]);
                }
            }
            clean_G.offsets.push_back(clean_G.edges.size());
            clean_G.vertex_weights.push_back(G.vertex_weights[i]);
        }
    }
    clean_G.num_edges = clean_G.edges.size();
}