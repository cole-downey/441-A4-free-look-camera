#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <random>
#include <time.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Body.h"

using namespace std;

GLFWwindow* window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;
shared_ptr<double> t;
//int width = 640 * 3, height = 480 * 3; // viewport width, height

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<MatrixStack> MV;
shared_ptr<MatrixStack> P;
shared_ptr<Shape> bunny, teapot, sun_mesh, ground_mesh, arrow;
shared_ptr<Body> sun_obj;
shared_ptr<Body> ground;

vector<shared_ptr<Body>> bunnies; // contains all objects to be drawn

// lights
Light sun;

bool keyToggles[256] = { false }; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char* description) {
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse) {
	camera->mouseMoved((float)xmouse, (float)ymouse);
}

static void char_callback(GLFWwindow* window, unsigned int key) {
	keyToggles[key] = !keyToggles[key];
	switch (key) {
	case 'w':
		camera->w();
		break;
	case 'a':
		camera->a();
		break;
	case 's':
		camera->s();
		break;
	case 'd':
		camera->d();
		break;
	case 'z':
		camera->addFovy(-0.1f);
		break;
	case 'Z':
		camera->addFovy(0.1f);
		break;
	case 'p':
		// disable pointer and use raw mouse motion
		if (keyToggles[(unsigned)'p']) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		break;
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char* filepath, GLFWwindow* w) {
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if (rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init() {
	// Initialize time.
	glfwSetTime(0.0);
	srand((unsigned)time(NULL));
	t = make_shared<double>();
	*t = 0.0;


	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Set up programs
	prog = make_shared<Program>(); // bp shader
	prog->setShaderNames(RESOURCE_DIR + "bp_vert.glsl", RESOURCE_DIR + "bp_frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addUniform("MV");
	prog->addUniform("IT");
	prog->addUniform("P");
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->addUniform("lightPos");
	prog->addUniform("lightColor");
	prog->setVerbose(false);

	camera = make_shared<Camera>();
	double cursorx, cursory;
	glfwGetCursorPos(window, &cursorx, &cursory);
	camera->setMousePrev((float)cursorx, (float)cursory);

	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();
	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	sun_mesh = make_shared<Shape>();
	sun_mesh->loadMesh(RESOURCE_DIR + "sphere.obj");
	sun_mesh->init();
	ground_mesh = make_shared<Shape>();
	ground_mesh->loadMesh(RESOURCE_DIR + "ground.obj");
	ground_mesh->init();
	arrow = make_shared<Shape>();
	arrow->loadMesh(RESOURCE_DIR + "arrow.obj");
	arrow->init();

	GLSL::checkError(GET_FILE_LINE);

	// create matrix stacks
	P = make_shared<MatrixStack>();
	MV = make_shared<MatrixStack>();

	// create objects to be drawn
	glm::vec3 groundMax = glm::vec3(20.0f, 0.0f, 20.0f);
	glm::vec3 groundMin = glm::vec3(-20.0f, 0.0f, -20.0f);
	glm::vec3 groundSize = groundMax - groundMin;
	int num_rows = 11;
	int num_cols = 11;
	groundSize.x /= num_rows;
	groundSize.z /= num_cols;
	for (int x = 1; x < num_rows; x++) {
		for (int z = 1; z < num_cols; z++) {
			if ((x + z) % 2 == 0) {
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				bunnies.push_back(make_shared<Body>(pos, -0.333099f, bunny, MV));
			} else {
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				bunnies.push_back(make_shared<Body>(pos, 0.0f, teapot, MV));
			}
			bunnies.back()->addAnimation(t);
		}
	}

	// create lights
	sun = Light({ -20.0f, 10.0f, 0.0f }, { 0.7f, 0.7f, 1.0f }); // sun is blue
	sun_obj = make_shared<Body>(sun.pos, 0.0f, sun_mesh, MV, true);

	// create ground
	ground = make_shared<Body>(ground_mesh, MV);
}

// This function is called every frame to draw the scene.
static void render() {
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}



	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width / (float)height;
	camera->setAspect(aspect);
	glViewport(0, 0, width, height);

	*t = glfwGetTime();
	if (keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		*t = 0.0f;
	}

	prog->bind();

	// render HUD
	P->pushMatrix();
	glm::vec2 hudBounds = { 2.0f * aspect / 2, 2.0f / 2 };
	P->multMatrix(glm::ortho(-hudBounds.x, hudBounds.x, -hudBounds.y, hudBounds.y, -10.0f, 10.0f));
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	P->popMatrix();
	glUniform3f(prog->getUniform("ka"), 0.1f, 0.1f, 0.1f);
	glUniform3f(prog->getUniform("kd"), 1.0f, 1.0f, 1.0f);
	glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
	glUniform1f(prog->getUniform("s"), 200.0f);
	glUniform3f(prog->getUniform("lightPos"), 0.0f, 5.0f, 3.0f);
	MV->pushMatrix();
	// Draw	bunny
	MV->pushMatrix();
	MV->translate(hudBounds.x - 0.5f, 0.0f, 0.0f);
	MV->scale(0.25f);
	MV->translate(0.0f, 2.0f, 0.0f);
	MV->rotate((float)*t * 0.5f, 0, 1, 0);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glm::mat4 it = glm::transpose(glm::inverse(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(it));
	bunny->draw(prog);
	MV->popMatrix();
	// Draw	teapot
	MV->pushMatrix();
	MV->translate(-hudBounds.x + 0.5f, 0.0f, 0.0f);
	MV->scale(0.25f);
	MV->translate(0.0f, 2.3f, 0.0f);
	MV->rotate((float)*t * 0.5f, 0, 1, 0);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	it = glm::transpose(glm::inverse(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(it));
	teapot->draw(prog);
	MV->popMatrix();
	MV->popMatrix();


	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	// set variables
	sun.setPosCam(MV->topMatrix()); // put light pos in camera space
	glUniform3f(prog->getUniform("lightPos"), sun.posCam.x, sun.posCam.y, sun.posCam.z);
	glUniform3f(prog->getUniform("lightColor"), sun.color.x, sun.color.y, sun.color.z);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

	// Draw every object
	for (int i = 0; i < bunnies.size(); i++) {
		bunnies.at(i)->draw(prog);
	}
	sun_obj->draw(prog);
	ground->draw(prog);

	MV->popMatrix();
	P->popMatrix();

	// mini map
	if (keyToggles[(unsigned)'t']) {
		double s = 0.5;
		glViewport(0, 0, (int)(s * height), (int)(s * height));
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, (int)(s * height), (int)(s * height));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		P->pushMatrix();
		MV->pushMatrix();
		//APPLY PROJECTION MATRIX FOR TOP - DOWN VIEWPORT
		P->multMatrix(glm::ortho(-21.0, 21.0, -21.0, 21.0, 1.0, 51.0));
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		//APPLY VIEW MATRIX FOR TOP - DOWN VIEWPORT
		MV->rotate((float)glm::radians(90.0), 1, 0, 0);
		MV->translate(0.0f, -50.0f, 0.0f);
		//DRAW SCENE
		// set variables
		sun.setPosCam(MV->topMatrix()); // put light pos in camera space
		glUniform3f(prog->getUniform("lightPos"), sun.posCam.x, sun.posCam.y, sun.posCam.z);

		// Draw every object
		for (int i = 0; i < bunnies.size(); i++) {
			bunnies.at(i)->draw(prog);
		}
		ground->draw(prog);
		sun_obj->draw(prog);
		// frustum
		MV->pushMatrix();
		glm::mat4 inv = glm::inverse(camera->getViewMatrix());
		MV->multMatrix(inv);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		it = glm::transpose(glm::inverse(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(it));
		arrow->draw(prog);
		MV->popMatrix();

		MV->popMatrix();
		P->popMatrix();
	}
	prog->unbind();

	GLSL::checkError(GET_FILE_LINE);

	if (OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Optional argument
	if (argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if (!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	// default is 640 x 480
	window = glfwCreateWindow(640 * 3, 480 * 3, "A4 - Cole Downey", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
