#ifndef PROJECT_GRAPH_CSR_H
#define PROJECT_GRAPH_CSR_H

#include <vector>
#include <span>

class CSR {
    private:
    std::vector<int> edges;
    std::vector<int> offsets;

public:
    CSR(const std::vector<std::vector<int>> &adj);
    std::span<const int> get_neighbors(int v) const;
    int degree(int v) const;
    size_t size() const;
};

#endif //PROJECT_GRAPH_CSR_H