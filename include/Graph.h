#ifndef PROJECT_GRAPH_GRAPH_H
#define PROJECT_GRAPH_GRAPH_H

#include <vector>

struct Graph {
    int num_vertices;
    int num_edges;

    std::vector<int> offsets;
    std::vector<int> edges;
    std::vector<float> weights;

    Graph(int v, int e) : num_vertices(v), num_edges(e) {
        offsets.resize(v + 1, 0);
        edges.resize(e);
    }
};

#endif //PROJECT_GRAPH_GRAPH_H