#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

class Camera {
public:
	Camera();
	virtual ~Camera();
	void setAspect(float a) { aspect = a; };
	void setRotationFactor(float f) { rfactor = f; };
	void mouseMoved(float x, float y);
	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV);
	glm::mat4 Camera::getViewMatrix();

	void w();
	void a();
	void s();
	void d();
	void setMousePrev(float x, float y);
	void addFovy(float f);

private:
	glm::vec3 pos;
	float yaw;
	float pitch;
	glm::vec3 forward;
	float speed;
	glm::vec3 findPerp();
	void printPos();
	void addYaw(float angle);
	void addPitch(float angle);

	float aspect;
	float fovy;
	float znear;
	float zfar;
	glm::vec2 mousePrev;
	float rfactor;
};

#endif
