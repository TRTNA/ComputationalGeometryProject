//Variation of https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/shader.h

#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    GLuint Program;

    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    void Use();

    void Delete();

private:

    void checkCompileErrors(GLuint shader, std::string type);
};
