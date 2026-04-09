#ifndef PROJECT_GRAPH_LOAD_DIMACS_H
#define PROJECT_GRAPH_LOAD_DIMACS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "Graph.h"

Graph load_dimacs(const std::string& filePath);
Graph load_txt(const std::string& filePath);

#endif //PROJECT_GRAPH_LOAD_DIMACS_H
