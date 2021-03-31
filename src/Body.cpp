#include "Body.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

using namespace std;


Body::Body(glm::vec3 _pos, float _yOffset, shared_ptr<Shape> _shape, shared_ptr<MatrixStack> _MV, bool sun) :
    pos(_pos), yOffset(_yOffset),
    shape(_shape), MV(_MV), t(nullptr) {
    if (sun) {
        mat = Material({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 200.0f);
        rot = 0.0f;
        scale = 2.0f;
    } else {
        mat = Material({ 0.2f, 0.2f, 0.2f }, { pickColor(), pickColor(), pickColor() }, { 1.0f, 1.0f, 1.0f }, 200.0f);
        // make the ambient color similar to the diffuse color
        mat.ka = mat.kd / 8.0f;
        rot = (float)rand();
        scale = 0.5f + (float)rand() / (float)(RAND_MAX / (2.0f - 0.5f));
    }
}

Body::Body(shared_ptr<Shape> _shape, shared_ptr<MatrixStack> _MV) :
    shape(_shape),
    MV(_MV) {
    // ground
    pos = glm::vec3(0.0f);
    yOffset = 0.0f;
    scale = 1.0f;
    rot = 0.0f;
    rot = max(0.0f, rot);
    t = nullptr;
    //mat = Material({ 0.03f, 0.23f, 0.16f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1.0f);
    mat = Material({ 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 200.0f);
}

Body::~Body() {
}

float Body::pickColor() {
    float color = (float)(rand() % 255);
    color /= 255.0f;
    return color;
}


void Body::draw(const shared_ptr<Program> prog) const {
    // Set shader values
    glUniform3f(prog->getUniform("ka"), mat.ka.x, mat.ka.y, mat.ka.z);
    glUniform3f(prog->getUniform("kd"), mat.kd.x, mat.kd.y, mat.kd.z);
    glUniform3f(prog->getUniform("ks"), mat.ks.x, mat.ks.y, mat.ks.z);
    glUniform1f(prog->getUniform("s"), mat.s);

    // Apply model transforms
    MV->pushMatrix();
    MV->translate(pos);
    if (t == nullptr)
        MV->scale(scale);
    else
        MV->scale((float)cos(*t + tOffset) * scale * 0.10f + scale);
    MV->translate(0.0f, yOffset, 0.0f); // make correct height
    MV->rotate(rot, 0, 1, 0);

    // Draw	
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glm::mat4 it = glm::transpose(glm::inverse(MV->topMatrix()));
    glUniformMatrix4fv(prog->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(it));
    shape->draw(prog);
    MV->popMatrix();
}

void Body::addAnimation(shared_ptr<double> t) {
    this->t = t;
    tOffset = (float)rand() / (float)(RAND_MAX / (3.14f));
}
