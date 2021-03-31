#ifndef BODY_H
#define BODY_H

#include <glm/glm.hpp>
#include <memory>
#include <random>
#include "MatrixStack.h"
#include "Material.h"
#include "Shape.h"
#include "Program.h"

class Body {
private:
    std::shared_ptr<Shape> shape;
    std::shared_ptr<MatrixStack> MV;
    Material mat;
    glm::vec3 pos;
    float yOffset;
    float scale;
    float rot;
    float pickColor();
    std::shared_ptr<double> t;
    float tOffset;
public:
    Body(glm::vec3 _pos, float _yOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV, bool sun = false);
    Body(std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    ~Body();
    void draw(const std::shared_ptr<Program> prog) const;
    void addAnimation(std::shared_ptr<double> t);
};



#endif