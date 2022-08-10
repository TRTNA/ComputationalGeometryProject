#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glad/glad.h>

struct Point {
    glm::vec3 Position;
    glm::vec4 Color;
};

class ArrowLine {
    private:
        std::vector<Point> points;
        GLuint VAO;
        GLuint VBO;
        const int linePointsNum;
    public:
        ArrowLine(const ArrowLine& copy) = delete;
        ArrowLine& operator=(const ArrowLine &) = delete;
        ~ArrowLine() noexcept;
        ArrowLine(ArrowLine&& move) noexcept;
        ArrowLine& operator=(ArrowLine&& move) noexcept;
        ArrowLine(std::vector<Point>& points, int _linePointsNum);
        void Draw() const;

};