#pragma once
#include <vector>
#include <string>

class Polynomial
{
protected:
    std::vector<float> coeffs;
public:
    Polynomial(const std::vector<float>& _coeffs);
    float get(float x, float y) const;
    float get1stDerivativeDx(float x, float y) const;
    float get1stDerivativeDy(float x, float y) const;;
    std::string toString() const;
};

float polynomial1stDegreeSolver(const std::vector<float>& coeffs, float x, float y);

float polynomial2ndDegreeSolver(const std::vector<float>& coeffs, float x, float y);

float get1stDerivativeDy1stD(const std::vector<float>& coeffs, float x, float y);
float get1stDerivativeDx1stD(const std::vector<float>& coeffs, float x, float y);
float get1stDerivativeDx2ndD(const std::vector<float>& coeffs, float x, float y);

float get1stDerivativeDy2ndD(const std::vector<float>& coeffs, float x, float y);

