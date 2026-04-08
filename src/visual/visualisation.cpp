#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "visual/shader.h"
#include "visual/visualisation.h"
#include "visual/force_layout.h"

void runVisualisation(const Graph& hostGraph, DeviceGraph& deviceGraph, GLFWwindow* window) {

    Shader shader("vertex.glsl", "fragment.glsl");

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, hostGraph.num_vertices * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    float rotation = 0.0f;

    cudaDeviceSynchronize();
    while (!glfwWindowShouldClose(window)) {
        step(deviceGraph, 40.0f, 0.01f, 0.9f); //physics проверить что 40 достаточно

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
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 5000.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -500));

        rotation += 0.001f;

        view = glm::rotate(view, rotation, glm::vec3(0,1,0));
        shader.setMat4("mvp", projection * view);

        glBindVertexArray(vao);

        shader.setVec3("color", glm::vec3(0.3f, 0.3f, 0.3f));
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

        glPointSize(5.0f);
        shader.setVec3("color", glm::vec3(0.0f, 0.7f, 1.0f));
        glDrawArrays(GL_POINTS, 0, deviceGraph.num_vertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cudaGraphicsUnregisterResource(cudaVboRes);
    glfwDestroyWindow(window);
}