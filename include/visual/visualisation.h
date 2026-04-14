//
// Created by basilisk on 08/04/2026.
//

#ifndef PROJECT_GRAPH_VISUALISATION_H
#define PROJECT_GRAPH_VISUALISATION_H

#include "Graph.h"
#include "force_layout.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void runVisualisation(const Graph& hostGraph, std::vector<int>& colors);

#endif //PROJECT_GRAPH_VISUALISATION_H
