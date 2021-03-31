#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"


Camera::Camera() :
	pos(0.0f, 2.0f, 0.0f),
	yaw(0.0f),
	pitch(0.0f),
	forward(0.0f, 0.0f, 1.0f),
	speed(0.2f),
	aspect(1.0f),
	fovy((float)(45.0 * M_PI / 180.0)),
	znear(0.1f),
	zfar(1000.0f),
	mousePrev(0.0f, 0.0f),
	rfactor(-0.005f) {
}

Camera::~Camera() {
}

void Camera::mouseMoved(float x, float y) {
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	addYaw(rfactor * dv.x);
	addPitch(rfactor * dv.y);
	mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const {
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) {
	glm::mat4 view = glm::lookAt(pos, pos + forward + glm::vec3(0.0f, (float)sin(pitch), 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	MV->multMatrix(view);
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 view = glm::lookAt(pos, pos + forward + glm::vec3(0.0f, (float)sin(pitch), 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return view;
}

void Camera::w() {
	pos += speed * forward;
	printPos();
}

void Camera::a() {
	pos -= speed * findPerp();
	printPos();
}

void Camera::s() {
	pos -= speed * forward;
	printPos();
}

void Camera::d() {
	pos += speed * findPerp();
	printPos();
}

glm::vec3 Camera::findPerp() {
	return glm::vec3(-forward.z, 0.0f, forward.x);
}

void Camera::printPos() {
	if (false)
		std::cout << "Camera postion: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
}

void Camera::addYaw(float angle) {
	yaw += angle;
	forward.x = (float)sin(yaw);
	forward.z = (float)cos(yaw);
}

void Camera::addPitch(float angle) {
	pitch += angle;
	pitch = std::max(pitch, -1.25f);
	pitch = std::min(pitch, 1.25f);
}

void Camera::setMousePrev(float x, float y) {
	mousePrev = glm::vec2(x , y);
}

void Camera::addFovy(float f) {
	fovy += f;
	fovy = std::max(fovy, (float)(4.0 * M_PI / 180.0));
	fovy = std::min(fovy, (float)(114.0 * M_PI / 180.0));
}