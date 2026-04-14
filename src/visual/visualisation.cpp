#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "visual/shader.h"
#include "visual/visualisation.h"
#include "visual/force_layout.h"

struct CameraControls {
    float windowWidth = 1280.0f;
    float windowHeight = 720.0f;
    float lastX = 640.0f;
    float lastY = 360.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float cameraDist = 500.0f;
    bool firstMouse = true;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    auto* state = reinterpret_cast<CameraControls*>(glfwGetWindowUserPointer(window));
    if (state) {
        state->windowWidth = (float)width;
        state->windowHeight = (float)height;
        glViewport(0, 0, width, height);
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    auto* state = reinterpret_cast<CameraControls*>(glfwGetWindowUserPointer(window));
    if (!state) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (state->firstMouse) {
        state->lastX = xpos;
        state->lastY = ypos;
        state->firstMouse = false;
    }

    float xoffset = xpos - state->lastX;
    float yoffset = state->lastY - ypos;
    state->lastX = xpos;
    state->lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        float sensitivity = 0.1f;
        state->yaw += xoffset * sensitivity;
        state->pitch += yoffset * sensitivity;

        if (state->pitch > 89.0f) state->pitch = 89.0f;
        if (state->pitch < -89.0f) state->pitch = -89.0f;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* state = reinterpret_cast<CameraControls*>(glfwGetWindowUserPointer(window));
    if (state) {
        state->cameraDist -= (float)yoffset * 20.0f;
        if (state->cameraDist < 10.0f) state->cameraDist = 10.0f;
    }
}

void runVisualisation(const Graph& hostGraph, std::vector<int>& colors) {
    if (!glfwInit()) return;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Visualisation", NULL, NULL);

    CameraControls cam;
    glfwSetWindowUserPointer(window, &cam);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!window) { glfwTerminate(); return; }
    glfwMakeContextCurrent(window);
    glewInit();

    DeviceGraph deviceGraph = setupDeviceGraph(hostGraph);


    Shader shader("vertex.glsl", "fragment.glsl");

    GLuint vao, vbo, ebo, colorVbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &colorVbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, hostGraph.num_vertices * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(int), colors.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(1, 1, GL_INT, 0, (void*)0); // Используем I для int!
    glEnableVertexAttribArray(1);

    std::vector<unsigned int> indices;
    for (int i = 0; i < hostGraph.num_vertices; i++) {
        for (int j = hostGraph.offsets[i]; j <  hostGraph.offsets[i + 1]; j++) {
            indices.push_back(i);
            indices.push_back(hostGraph.edges[j]);
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    cudaGraphicsResource_t cudaVboRes;
    cudaError_t err = cudaGraphicsGLRegisterBuffer(&cudaVboRes, vbo, cudaGraphicsRegisterFlagsWriteDiscard);
    if (err != cudaSuccess) {
        std::cerr << "CUDA GL Register failed: " << cudaGetErrorString(err) << std::endl;
        return;
    }

    cudaDeviceSynchronize();
    while (!glfwWindowShouldClose(window)) {
        step(deviceGraph, 40.0f, 0.0002f, 0.9f); //physics проверить что 40 достаточно

        float* d_vboPtr;
        size_t size;
        cudaGraphicsMapResources(1, &cudaVboRes, 0);
        cudaGraphicsResourceGetMappedPointer((void**)&d_vboPtr, &size, cudaVboRes);

        launchRepackKernel(deviceGraph, d_vboPtr);

        cudaGraphicsUnmapResources(1, &cudaVboRes, 0);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(cam.fov),
                                               cam.windowWidth / cam.windowHeight,
                                               0.1f, 50000.0f);
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -cam.cameraDist));
        view = glm::rotate(view, glm::radians(cam.pitch), glm::vec3(-1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(cam.yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        shader.setMat4("mvp", projection * view);

        glBindVertexArray(vao);

        shader.setBool("useVertexColor", false);
        shader.setVec3("color", glm::vec3(0.3f, 0.3f, 0.3f));
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

        shader.setBool("useVertexColor", true);
        glPointSize(5.0f);
        // shader.setVec3("color", glm::vec3(0.0f, 0.7f, 1.0f));
        glDrawArrays(GL_POINTS, 0, deviceGraph.num_vertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cudaGraphicsUnregisterResource(cudaVboRes);

    freeDeviceGraph(deviceGraph);
    glfwDestroyWindow(window);
    glfwTerminate();
}