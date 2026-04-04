#ifndef PROJECT_GRAPH_GRAPH_H
#define PROJECT_GRAPH_GRAPH_H

#include <set>

#include "CSR.h"

class Graph {
private:
    CSR csr;

    std::vector<int> colors;
    std::vector<int> sat_deg;

    std::vector<std::set<int>> neighbor_colors;
    std::vector<std::vector<int>> buckets;

    std::vector<int> last_seen_at;
    int max_sat = 0;

    int find_min_available_color(int v);
public:
    Graph(CSR&& source_csr);

    void run_dsatur();


};

#endif //PROJECT_GRAPH_GRAPH_H