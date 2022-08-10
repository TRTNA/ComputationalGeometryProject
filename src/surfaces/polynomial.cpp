#include "polynomial.h"

#include <iostream>

using std::iostream;
using std::vector;

Polynomial::Polynomial(const vector<float>& _coeffs) : coeffs(_coeffs) {}

float Polynomial::get(float x, float y) const {
    if (coeffs.size() == 3) return polynomial1stDegreeSolver(coeffs, x, y);
    if (coeffs.size() == 6) return polynomial2ndDegreeSolver(coeffs, x, y);
    return 0.0f;
}

float Polynomial::get1stDerivativeDx(float x, float y) const {
    if (coeffs.size() == 3) return get1stDerivativeDx1stD(coeffs, x, y);
    if (coeffs.size() == 6) return get1stDerivativeDx2ndD(coeffs, x, y);
    return 0.0f;
}

float Polynomial::get1stDerivativeDy(float x, float y) const {
    if (coeffs.size() == 3) return get1stDerivativeDy1stD(coeffs, x, y);
    if (coeffs.size() == 6) return get1stDerivativeDy2ndD(coeffs, x, y);
    return 0.0f;
}

std::string Polynomial::toString() const {
    std::string repr = "";
    for(auto& coeff : coeffs) {
        repr += std::to_string(coeff);
        repr += " ";
    }
    return repr;
}

float polynomial1stDegreeSolver(const vector<float>& coeffs, float x, float y) {
    return coeffs[0] * x + coeffs[1] * y + coeffs[2];
}
float polynomial2ndDegreeSolver(const vector<float>& coeffs, float x, float y) {
    return coeffs[0]*x*x + coeffs[1]*x*y + coeffs[2]*y*y + coeffs[3]*x + coeffs[4]*y + coeffs[5];
}

float get1stDerivativeDy1stD(const vector<float>& coeffs, float x, float y) {
    return coeffs[1];
}
float get1stDerivativeDx1stD(const vector<float>& coeffs, float x, float y) {
    return coeffs[0];
}
float get1stDerivativeDx2ndD(const vector<float>& coeffs, float x, float y) {
    return 2 * coeffs[0] * x + coeffs[1] * y + coeffs[3];
}

float get1stDerivativeDy2ndD(const vector<float>& coeffs, float x, float y) {
    return coeffs[1] * x + 2 * coeffs[2] * y + coeffs[4];
}
