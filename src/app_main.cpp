#include <iostream>
#include <cstdio>

#include <vector>
#include "Graph.h"
#include "../include/partitioning/coarsening.h"
#include "visual/visualisation.h"
#include "import/load_dimacs.h"

#include "partitioning/coarsening.h"
#include "partitioning/multilevel_partition.h"

int main() {
    setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);

    Graph hGraph;
    try {
        hGraph = load_txt("graph/graph.txt");
        // hGraph = load_dimacs("jean.col");
        std::cout << "Vertices: " << hGraph.num_vertices << ", Edges: " << hGraph.edges.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    Graph graph;
    clean_disconnected_parts(hGraph, graph);

    // int V = 8;
    // Graph hGraph(V, 24);
    // hGraph.offsets = {0, 3, 6, 9, 12, 15, 18, 21, 24};
    // hGraph.edges = {1,3,5, 0,2,6, 1,3,7, 0,2,4, 3,5,7, 0,4,6, 1,5,7, 2,4,6};
    // hGraph.edge_weights.assign(hGraph.edges.size(), 1);
    // hGraph.vertex_weights.assign(V, 3);

    // CoarseLevel level_1;
    // coarse_step(hGraph, level_1);
    // CoarseLevel level_2;
    // coarse_step(level_1.graph, level_2);

    // std::cout << "Vertices: " << level_2.graph.num_vertices << ", Edges: " << level_2.graph.edges.size() << std::endl;

    // std::cout << "Offsets: ";
    // for (int v : cmap) std::cout << v << " ";
    // std::cout << std::endl;
    multilevel_partition(graph, 2);


    return 0;
}