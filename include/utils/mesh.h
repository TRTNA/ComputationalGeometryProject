// Based on https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/mesh.h

#pragma once

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>

// data structure for vertices
struct Vertex
{
    // vertex coordinates
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // Texture coordinates
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint VAO;

    Mesh(const Mesh &copy) = delete; // disallow copy
    Mesh &operator=(const Mesh &) = delete;

    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices) noexcept;
    Mesh(Mesh &&move) noexcept;

    Mesh &operator=(Mesh &&move) noexcept;
    ~Mesh() noexcept;

    void Draw();

private:
    GLuint VBO, EBO;
    void setupMesh();
    void freeGPUresources();
};
