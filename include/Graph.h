#ifndef PROJECT_GRAPH_GRAPH_H
#define PROJECT_GRAPH_GRAPH_H

#include <vector>
#include <cstdint>
#include <iostream>

struct Graph {
    int num_vertices;
    int num_edges;


    std::vector<int> offsets;
    std::vector<int> edges;

    std::vector<int> edge_weights;
    std::vector<int> vertex_weights;

    Graph() = default;
    Graph(int v, int e) : num_vertices(v), num_edges(e) {
        offsets.resize(v + 1, 0);
        edges.resize(e);
    }

    void debugPrint() {
        std::cout << "Offsets: ";
        for (int v : this->offsets) std::cout << v << " ";
        std::cout << "\nEdges: ";
        for (int e : this->edges) std::cout << e << " ";
        std::cout << std::endl;
    }
};

void split_into_two_graphs(const Graph& initial, const std::vector<int>& parts, Graph& g0, Graph& g1);

#endif //PROJECT_GRAPH_GRAPH_H