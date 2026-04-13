#ifndef PROJECT_GRAPH_COARSENING_H
#define PROJECT_GRAPH_COARSENING_H

#include <vector>

#include "Graph.h"

struct CoarseLevel {
    Graph graph;
    std::vector<int> cmap;
};

void find_heavy_edge_matching(const Graph& G, std::vector<int>& cmap, int& nextVertexIdx);
void build_coarse_graph(const Graph& G, const std::vector<int>& cmap, int num_coarse_vertices, Graph& new_G);

void coarse_step(const Graph& G, CoarseLevel& next_level);

#endif //PROJECT_GRAPH_COARSENING_H
