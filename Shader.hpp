#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <GL/glew.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h> // Just for macOS
#else
    #include <GL/gl.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * @class Shader
 * @brief A class for managing GLSL shader programs.
 *
 * This class handles reading shader source from files, compiling and linking them,
 * and provides an interface for setting uniforms.
 * Heavily inspired by https://learnopengl.com/
 */
class Shader {
public:

    /*
    * @brief Constructs a shader program from vertex and fragment shader files.
    * @param vertexPath Path to vertex shader source.
    * @param fragmentPath Path to fragment shader source.
    */
    Shader(const char* vertexPath, const char* fragmentPath) {
        // Get code from file
        std::string vertexCode = readFile(vertexPath);
        std::string fragmentCode = readFile(fragmentPath);


        // Compile shaders
        unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
        unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);

        // Link
        m_ID = linkProgram(vertex, fragment);

        // Delete linked shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader() {
        if (m_ID != 0) {
            glDeleteProgram(m_ID);
        }
    }

    // move
    Shader(Shader&& other) noexcept : m_ID(other.m_ID), m_uniformLocationCache(std::move(other.m_uniformLocationCache)) {
        other.m_ID = 0; // Invalidate other's ID to prevent double-delete
    }
    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (m_ID != 0) glDeleteProgram(m_ID);
            m_ID = other.m_ID;
            m_uniformLocationCache = std::move(other.m_uniformLocationCache);
            other.m_ID = 0;
        }
        return *this;
    }

    // copy
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /**
    * @brief Activates shader program
    */
    void use() const {
        glUseProgram(m_ID);
    }

    /**
    * @brief Gets OpenGL ID of shader program
    * @return Program ID
    */
    unsigned int getID() const {
        return m_ID;
    }


    // Utility uniform functions
    void setBool(const std::string &name, bool value) { glUniform1i(getUniformLocation(name), (int)value); }
    void setInt(const std::string &name, int value) { glUniform1i(getUniformLocation(name), value); }
    void setFloat(const std::string &name, float value) { glUniform1f(getUniformLocation(name), value); }
    void setVec2(const std::string &name, const glm::vec2 &value) { glUniform2fv(getUniformLocation(name), 1, &value[0]); }
    void setVec2(const std::string &name, float x, float y) { glUniform2f(getUniformLocation(name), x, y); }
    void setVec3(const std::string &name, const glm::vec3 &value) { glUniform3fv(getUniformLocation(name), 1, &value[0]); }
    void setVec3(const std::string &name, float x, float y, float z) { glUniform3f(getUniformLocation(name), x, y, z); }
    void setVec4(const std::string &name, const glm::vec4 &value) { glUniform4fv(getUniformLocation(name), 1, &value[0]); }
    void setVec4(const std::string &name, float x, float y, float z, float w) { glUniform4f(getUniformLocation(name), x, y, z, w); }
    void setMat2(const std::string &name, const glm::mat2 &mat) { glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]); }
    void setMat3(const std::string &name, const glm::mat3 &mat) { glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]); }
    void setMat4(const std::string &name, const glm::mat4 &mat) { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]); }

private:
    unsigned int m_ID = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformLocationCache;

    /**
     * @brief Gets location of a uniform variable, using a cache.
     * @param name Name of the uniform.
     * @return Integer location of the uniform.
     */
    GLint getUniformLocation(const std::string& name) {
        if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
            return m_uniformLocationCache[name];
        }

        GLint location = glGetUniformLocation(m_ID, name.c_str());
        if (location == -1) {
            // May not be an error if optimized
            std::cerr << "Warning: uniform '" << name << "' not found in shader program " << m_ID << std::endl;
        }
        m_uniformLocationCache[name] = location;
        return location;
    }

    /**
     * @brief Reads the entire content of a file into a string.
     * @param filePath Path to the file.
     * @return Content of the file as a string.
     */
    std::string readFile(const std::string& filePath) {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(filePath);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            return shaderStream.str();
        } catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << " (" << e.what() << ")" << std::endl;
            return "";
        }
    }

    /**
     * @brief Compiles a single shader object.
     * @param type Type of shader (e.g., GL_VERTEX_SHADER).
     * @param source GLSL source code.
     * @param path File path of the shader (for error logging).
     * @return OpenGL ID of the compiled shader object.
     */
    unsigned int compileShader(GLenum type, const char* source, const std::string& path) {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        int success;
        char infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED of type: " << type << "\nFile: " << path << "\n" << infoLog << std::endl;
        }
        return shader;
    }

    /**
     * @brief Links compiled shaders into a final shader program.
     * @param vertexShader ID of the compiled vertex shader.
     * @param fragmentShader ID of the compiled fragment shader.
     * @return OpenGL ID of the linked shader program.
     */
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
        unsigned int programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);

        int success;
        char infoLog[1024];
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programID, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        return programID;
    }
};

#endif // !SHADER_HPP
