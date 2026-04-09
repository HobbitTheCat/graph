#include <iostream>
#include <cstdio>

#include <vector>
#include "Graph.h"
#include "visual/visualisation.h"
#include "visual/force_layout.h"
#include "import/load_dimacs.h"

int main() {
    setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);



    // int V = 8; int E = 12;
    // Graph hGraph(V, E);
    // hGraph.offsets = {0, 3, 6, 9, 12, 15, 18, 21, 24};
    // hGraph.edges = {1,3,5, 0,2,6, 1,3,7, 0,2,4, 3,5,7, 0,4,6, 1,5,7, 2,4,6};
    Graph hGraph(1,1);
    try {
        hGraph = load_txt("graph/graph.txt");
        std::cout << "Vertices: " << hGraph.num_vertices << ", Edges: " << hGraph.edges.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }



    runVisualisation(hGraph);


    return 0;
}