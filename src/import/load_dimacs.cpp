#include "import/load_dimacs.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>

#include "../../include/Graph.h"

Graph load_dimacs(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {throw std::runtime_error("Could not open file");}

    std::string line;
    int v = 0, e = 0;

    std::vector<std::vector<int>> adj;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;

        std::istringstream stream(line);
        char type;
        if (!(stream >> type)) continue;

        if (type == 'p') {
            std::string format;
            stream >> format >> v >> e;
            adj.resize(v + 1);
        } else if (type == 'e') {
            int u, target;
            if (stream >> u >> target) {
                adj[u].push_back(target);
                adj[target].push_back(u);
            }
        }
    }

    int actual_e = 0;
    for (int i = 0; i <= v; ++i) actual_e += adj[i].size();

    Graph g(v,actual_e);
    g.edge_weights.assign(actual_e, 1);
    g.vertex_weights.assign(v, 1);

    int current_offset = 0;
    for (int i = 1; i <= v; i++) {
        g.offsets[i - 1] = current_offset;

        std::sort(adj[i].begin(), adj[i].end());
        for (int neighbor : adj[i]) {
            g.edges[current_offset] = neighbor - 1;
            current_offset++;
        }
    }
    g.offsets[v] = current_offset;
    return g;
}

Graph load_txt(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {throw std::runtime_error("Could not open file");}


    std::string line;
    int nodes_count = 0, edges_count = 0;
    std::vector<std::pair<int, int>> temp_edges;


    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == '#') {
            if (line.find("Nodes:") != std::string::npos) {
                sscanf(line.c_str(), "# Nodes: %d Edges: %d", &nodes_count, &edges_count);
            }
            continue;
        }

        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            temp_edges.push_back({u, v});
        }
    }

    std::map<int, int> id_map;
    int current_id = 0;
    for (auto& edge : temp_edges) {
        if (id_map.find(edge.first) == id_map.end()) id_map[edge.first] = current_id ++;
        if (id_map.find(edge.second) == id_map.end()) id_map[edge.second] = current_id ++;
    }

    int num_edges = 0;
    int V = id_map.size();

    std::vector<std::vector<int>> adj(V);
    for (const auto& edge : temp_edges) {
        int u = id_map[edge.first];
        int v = id_map[edge.second];
        if (u != v) {
            adj[u].push_back(v);
            adj[v].push_back(u);
            num_edges += 2;
        }
    }


    // int E = temp_edges.size();
    Graph g(V, num_edges);
    g.edge_weights.assign(num_edges, 1);
    g.vertex_weights.assign(V, 1);

    int current_offset = 0;
    for (int i = 0; i < V; i++) {
        g.offsets[i] = current_offset;
        for (int neighbor : adj[i]) {
            g.edges[current_offset++] = neighbor;
        }
    }

    g.offsets[V] = current_offset;
    return g;

}
