#include <iostream>
#include <cstdio>

#include <vector>
#include "Graph.h"
#include "visual/visualisation.h"
#include "visual/force_layout.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


int main() {
    setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Visualisation", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewInit();

    int V = 8; int E = 12;
    Graph hGraph(V, E);
    hGraph.offsets = {0, 3, 6, 9, 12, 15, 18, 21, 24};
    hGraph.edges = {1,3,5, 0,2,6, 1,3,7, 0,2,4, 3,5,7, 0,4,6, 1,5,7, 2,4,6};

    DeviceGraph dGraph = setupDeviceGraph(hGraph);

    runVisualisation(hGraph, dGraph, window);

    freeDeviceGraph(dGraph);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}