#ifndef PROJECT_GRAPH_SHADER_H
#define PROJECT_GRAPH_SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;

    void setBool    (const std::string& name, bool value) const;
    void setInt     (const std::string& name, int value) const;
    void setFloat   (const std::string& name, float value) const;
    void setVec3    (const std::string& name, const glm::vec3& value) const;
    void setMat4    (const std::string& name, const glm::mat4& value) const;

private:
    mutable std::unordered_map<std::string, int> uniformLocationCache;

    int getUniformLocation(const std::string& name) const;
    static std::string readFile(const std::string& filePath);

    static unsigned int compileShader(unsigned int type, const char* source, const std::string& typeName);

    static void checkCompileErrors(unsigned int shader, std::string type);
};

#endif //PROJECT_GRAPH_SHADER_H
