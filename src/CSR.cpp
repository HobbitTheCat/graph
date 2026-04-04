#include "CSR.h"


CSR::CSR(const std::vector<std::vector<int>>& adj) {
    size_t n = adj.size();
    this->offsets.resize(n + 1, 0);
    for (int i = 0; i < n; i++) {
        this->offsets[i + 1] = this->offsets[i] + adj[i].size();
    }

    this->edges.reserve(offsets.back());
    for (const auto& neighbors : adj) {
        this->edges.insert(edges.end(), neighbors.begin(), neighbors.end());
    }
}

std::span<const int> CSR::get_neighbors(int v) const {
    return { this->edges.data() + this->offsets[v], static_cast<size_t>(this->offsets[v + 1] - this->offsets[v]) };
}

int CSR::degree(int v) const {
    return this->offsets[v + 1] - this->offsets[v];
}

size_t CSR::size() const {return this->offsets.size() - 1;}
