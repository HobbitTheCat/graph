#include "visual/shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        std::cerr << "Error: Shader: FileNotReadSuccessfully" << std::endl;
    }

    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), "");
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), "");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {glDeleteProgram(ID);}

void Shader::use() const {glUseProgram(ID);}

void Shader::setBool(const std::string &name, bool value) const {glUniform1i(getUniformLocation(name), (int) value);}
void Shader::setInt(const std::string &name, int value) const {glUniform1i(getUniformLocation(name), value);}
void Shader::setFloat(const std::string& name, float value) const {glUniform1f(getUniformLocation(name), value);}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

int Shader::getUniformLocation(const std::string& name) const {
    if (uniformLocationCache.find(name) != uniformLocationCache.end())
        return uniformLocationCache[name];

    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }

    uniformLocationCache[name] = location;
    return location;
}

std::string Shader::readFile(const std::string& filePath) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(filePath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "Error: Shader: File not successfully read: " << filePath << std::endl;
        return "";
    }
}

unsigned int Shader::compileShader(unsigned int type, const char* source, const std::string& typeName) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);
    checkCompileErrors(id, typeName);
    return id;
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "Error: Shader_compilation_error of type: " << type << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "Error: Program_linking_error of type: " << type << "\n" << infoLog << std::endl;
        }
    }
}