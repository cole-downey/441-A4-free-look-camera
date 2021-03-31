#include "Material.h"

//using namespace std;

Material::Material() {
    ka = { 0.0f, 0.0f, 0.0f };
    kd = { 0.0f, 0.0f, 0.0f };
    ks = { 0.0f, 0.0f, 0.0f };
    s = 0.0f;
}

Material::Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float s) :
    ka(ka),
    kd(kd),
    ks(ks),
    s(s) {
};

Material::~Material() {}