#include "arrow_line.h"

using std::vector;

ArrowLine::~ArrowLine() noexcept
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
}

ArrowLine::ArrowLine(ArrowLine &&move) noexcept : points(std::move(move.points)), linePointsNum(std::move(move.linePointsNum))
{
}

ArrowLine &ArrowLine::operator=(ArrowLine &&move) noexcept
{
    if (move.VAO)
    {
        points = std::move(move.points);
    }
    else
    {
        VAO = 0;
    }
    return *this;
}

ArrowLine::ArrowLine(vector<Point> &_points, int _linePointsNum) : points(_points), linePointsNum(_linePointsNum)
{
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), &this->points[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (GLvoid *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (GLvoid *)offsetof(Point, Color));

    glBindVertexArray(0);
}

void ArrowLine::Draw() const
{
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_LINES, 0, linePointsNum);
    glDrawArrays(GL_TRIANGLES, linePointsNum, points.size());
    glBindVertexArray(0);
}