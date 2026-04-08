#include "partitioning.h"
#include "cmath"
#include <algorithm>

static int bisection(const Graph& G, VertexSet& V, int first, int last) {
    return first + (last - first) / 2;
}

static void iterate(const Graph& G, VertexSet& V, int first, int last, int k_prime, int num, Partition& Pk) {
    if (k_prime > 1) {
        int k1 = std::floor(k_prime / 2);

        int mid = bisection(G, V, first, last);

        iterate(G, V, first, mid, k1, num, Pk);
        iterate(G, V, mid, last, k_prime - k1, num + k1, Pk);
    } else {
        for (int i = first; i < last; ++i) {
            Pk[num - 1].push_back(V[i]);
        }
    }
}

void bisection_rec(const Graph& G, int k, Partition& Pk) {
    Pk.assign(k, VertexSet());
    VertexSet all_vertices(G.num_vertices);

    for (int i = 0; i < G.num_vertices; ++i) all_vertices[i] = i;

    iterate(G, all_vertices, 0, G.num_vertices, k, 1, Pk);
}