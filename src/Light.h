#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <memory>
#include "MatrixStack.h"

struct Light {
    glm::vec3 pos;
    glm::vec4 posCam; // position camera view
    glm::vec3 color;
    Light() {
        pos = glm::vec3(0.0f);
        color = glm::vec3(0.0f);
    };
    Light(glm::vec3 _pos, glm::vec3 _color) {
        pos = _pos;
        color = _color;
    };
    void setPosCam(glm::mat4 MV) {
        // set MV to calculated position in camera space
        posCam = glm::vec4(pos, 1.0f);
        posCam = MV * posCam;
    };
};



#endif