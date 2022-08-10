#include "surface.h"
#include <iostream>

using std::array;
using std::vector;

PolynomialSurface::PolynomialSurface(array<Polynomial*,3> _polys) : polys(_polys) {}

glm::vec3 PolynomialSurface::get(float u, float v) const {
    return glm::vec3(polys[0]->get(u, v), polys[1]->get(u, v), polys[2]->get(u, v));
}

glm::vec3 PolynomialSurface::getNormal(float u, float v) const {
    glm::vec3 tangU(polys[0]->get1stDerivativeDx(u, v), polys[1]->get1stDerivativeDx(u,v), polys[2]->get1stDerivativeDx(u,v));
    glm::vec3 tangV(polys[0]->get1stDerivativeDy(u, v), polys[1]->get1stDerivativeDy(u,v), polys[2]->get1stDerivativeDy(u, v));

    glm::vec3 norm = glm::normalize(glm::cross(tangU, tangV));
    return norm;
}

glm::vec3 PolynomialSurface::getMeanPointOnSurface() const {
    return this->get(0,0); 
}

vector<Vertex> PolynomialSurface::computeGrid(int uSampleRate, int vSampleRate) const {    
    const float uStep = (2*uvRange) / (float)(uSampleRate-1);
    const float vStep = (2*uvRange) / (float)(vSampleRate-1);

    vector<Vertex> vertices;
    
    for (size_t i = 0; i < uSampleRate; i++)
    {
        float currU = -uvRange + i*uStep; 
        for (size_t j = 0; j < vSampleRate; j++)
        {
            float currV = -uvRange + j*vStep;
            Vertex temp;
            temp.Position = this->get(currU, currV);
            temp.Normal = this->getNormal(currU, currV);
            vertices.push_back(temp);
        }
        
    }
    return vertices;  
}