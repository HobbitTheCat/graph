#include "partitioning.h"

#include <cmath>
#include <vector>
#include <algorithm>

static int bisection(const Graph& G, std::vector<int> partition_map, int first, int last) {
    return first + (last - first) / 2;
}

static void iterate(const Graph& G, VertexSet& V, int first, int last, int k_prime, int num, Partition& Pk) {
    // Let G denote a graph, and V denote the set to be partitioned,
    // first: where to start, last: where to end, k_prime: how many parts there should be, num: current part number, Pk: this is where we'll store the data

    if (k_prime > 1) {
        int k1 = std::floor(k_prime / 2.0);       // Let's figure out where the halfway point is

        int mid = bisection(G, V, first, last); // We divide the current segment of the graph into two segments

        iterate(G, V, first, mid, k1, num, Pk); // Let's call it again for the first half
        iterate(G, V, mid, last, k_prime - k1, num + k1, Pk);    // Let's try again for the second half
    } else {
        for (int i = first; i < last; ++i) {     // We transfer all vertices from the partition to the partition array
            Pk[num - 1].push_back(V[i]);
        }
    }
}

void bisection_rec(const Graph& G, int k, Partition& Pk) {  // Let G be the initial graph, k the desired number of parts, and Pk the array into which the parts are stored.
    Pk.assign(k, VertexSet());                          // We create a subarray in each element of the array to store the vertices of the current segment
    VertexSet all_vertices(G.num_vertices);                 // We create a vertex array whose length matches the number of vertices in the graph

    for (int i = 0; i < G.num_vertices; ++i) all_vertices[i] = i; // We assign each array element its number

    iterate(G, all_vertices, 0, G.num_vertices, k, 1, Pk); // call the partition function
}

/* Observations:
 *  This approach, which involves partially copying the graph into memory, may not be suitable for large graphs
 *
 * Regarding the algorithm's behavior:
 *  The bisection algorithm should reorder the indices of the vertices in V and return the dividing point between the first and second parts
 */