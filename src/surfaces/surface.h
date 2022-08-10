#pragma once

#include "polynomial.h"
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <utils/mesh.h>


class PolynomialSurface {
private:
    const std::array<Polynomial*, 3> polys;
    const int uvRange = 1;
public:
    PolynomialSurface(std::array<Polynomial*, 3> _polys);
    glm::vec3 get(float u, float v) const;
    glm::vec3 getNormal(float u, float v) const;
    glm::vec3 getMeanPointOnSurface() const;
    std::vector<Vertex> computeGrid(int uSampleRate, int vSampleRate) const;
};