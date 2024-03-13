//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include "Window.h"

int glWindowWidth = 1500;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
glm::mat4 modelBall;


glm::vec3 position; 
GLuint positionLoc; 
glm::vec3 lightColorPoint; 
GLuint lightColorLocPoint; 

bool cameraAnimationOn = true;
float cameraSpeedcam = 0.005f;
glm::vec3 lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 positions[] = {glm::vec3(-0.12451f, 0.52615f, -0.10977f),glm::vec3(0.18836f,0.52476f,-0.5568f), glm::vec3(0.14252f, 0.53166f, -0.79194f), glm::vec3(-0.1826f, 0.51892f, -1.0325)};

gps::Camera myCamera(
	glm::vec3(1.188f, 0.8122f, 0.8122f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angleY = 0.0f;
float ballAngle = 0.0f;
GLfloat lightAngle;
// window

gps::Model3D object;
gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scena;
gps::Model3D ball_buna;
gps::Model3D wood;
bool jumpKeyPressed = false;

gps::Model3D coffeeCup;
glm::mat4 cupModel = glm::mat4(1.0f);

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;

gps::Shader depthShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
GLint isDay = 1;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
bool rotateCameraX = false;

GLuint fogDensityLoc;
GLfloat fogDensity;
bool ballAnimationOn = true;
float ballJumpHeight = 0.0f;
bool ballJumpUp = true;
float ballRotAngle = 0.0f;
glm::vec3 ballPosition = glm::vec3(-0.022f, -0.1f, -0.08f);

bool woodAnimationOn = true;
float woodJumpHeight = 0.0f;
bool woodJumpUp = true;
float woodRotAngle = 0.0f;
glm::vec3 woodPosition = glm::vec3(1.8f, 2.8f, 2.0f);

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized to width: %d, and height: %d\n", width, height);
	
}


// Define a variable to store the start time
double startTimeCameraAnimation = glfwGetTime();

void cameraAnimation() {
	if (cameraAnimationOn) {
		// Get the elapsed time
		double currentTime = glfwGetTime();
		double elapsedTime = currentTime - startTimeCameraAnimation;

		// Change direction every 10 seconds
		if (elapsedTime >= 20.0) {
			// Reset the start time to keep the animation continuous
			startTimeCameraAnimation = currentTime;

			// Reverse the direction
			cameraSpeedcam = -cameraSpeedcam;
		}

		// Move the camera based on the current direction
		myCamera.move(gps::MOVE_FORWARD, cameraSpeedcam);
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCamera.move(gps::MOVE_LEFT, cameraSpeedcam);
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCamera.move(gps::MOVE_BACKWARD, cameraSpeedcam);
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
}



void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	// Add this block to stop camera movement when 'C' key is pressed
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		cameraAnimationOn = !cameraAnimationOn;
	}
	if (key == GLFW_KEY_X) {
		if (action == GLFW_PRESS) {
			rotateCameraX = !rotateCameraX;
		}
	}
}

GLboolean firstMouse = true;
float lastX = 400, lastY = 300;
double yaw = -90.0f, pitch;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	myCamera.rotate(pitch, yaw);
}


void processMovement()
{


	if (pressedKeys[GLFW_KEY_UP]) {
		woodPosition.x += 0.01f;  // Adjust the left movement speed
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		woodPosition.x -= 0.01f;  // Adjust the right movement speed
	}

	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//pentru wireframe
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	//pentru point
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;		
		ballRotAngle = 0.0f;
		woodRotAngle = 0.0f;

	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;	
		ballRotAngle = 0.0f;
		woodRotAngle = 0.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_F]) {
		fogDensity += 0.002f;
		if (fogDensity >= 0.3f)
			fogDensity = 0.3f;
		myCustomShader.useShaderProgram();
		glUniform1fv(fogDensityLoc, 1, &fogDensity);
	}

	if (pressedKeys[GLFW_KEY_G]) {
		fogDensity -= 0.002f;
		if (fogDensity <= 0.0f)
			fogDensity = 0.0f;
		myCustomShader.useShaderProgram();
		glUniform1fv(fogDensityLoc, 1, &fogDensity);
	}

	if (pressedKeys[GLFW_KEY_C]) {

	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glWindow = glfwCreateWindow(mode->width, mode->height, "OpenGL Shader Example", monitor, NULL);

	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);   // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	
	scena.LoadModel("objects/scena/Proiect_PG.obj");
	ball_buna.LoadModel("objects/ball/ball_buna.obj");
	wood.LoadModel("objects/wood/wood.obj");
	coffeeCup.LoadModel("objects/glass/glass.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
	depthShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	cupModel = glm::mat4(1.0f);

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	////my modifications
	//// Apply rotation to the point
	//glm::vec4 rotatedPoint = lightRotation * glm::vec4(lightDir, 1.0f); // Extend point to a vec4

	//// Extract the new coordinates after rotation
	//lightDir = glm::vec3(rotatedPoint.x, rotatedPoint.y, rotatedPoint.z);
	////

	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	

	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//pentru point light
	position = glm::vec3(0.0f, 1.0f, 0.0f); 
	positionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "position");
	glUniform3fv(positionLoc, 1, glm::value_ptr(position));

	lightColorPoint = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLocPoint = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorPoint");
	glUniform3fv(lightColorLocPoint, 1, glm::value_ptr(lightColorPoint));
	
	//fog density
	fogDensity = 0.0f;
	fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1fv(fogDensityLoc, 1, &fogDensity);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = -7.94f, far_plane = 7.94f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}



void animateBallJump() {
	if (ballAnimationOn) {
		// Only update the jump height
		const float jumpIncrement = 0.01f;
		if (ballJumpUp) {
			ballJumpHeight += jumpIncrement;
		}
		else {
			ballJumpHeight -= jumpIncrement;
		}

		
		if (ballJumpHeight >= 0.13587675f) { 
			ballJumpUp = false;
		}

		if (ballJumpHeight < -0.0087f) {
			ballJumpUp = true;
		}

		
		ballPosition.y =  0.01 + ballJumpHeight; // Adjust for the initial quarter-height
	}
}

void animateWood() {
	if (woodAnimationOn) {
		// Only update the jump height
		const float jumpIncrement = 0.001f;
		if (woodJumpUp) {
			woodJumpHeight += jumpIncrement;
		}
		else {
			woodJumpHeight -= jumpIncrement;
		}

		if (woodJumpHeight >= 0.02f) {
			woodJumpUp = false;
		}

		if (woodJumpHeight < -0.05f) {
			woodJumpUp = true;
		}

		// Update the wood position
		woodPosition.y =  woodJumpHeight;
	}
}

bool press_t = false, boundCoffee = false;
void processCoffeeFlag() {
	if (pressedKeys[GLFW_KEY_T] && !press_t) {
		press_t = true;
		if (boundCoffee) {
			cupModel = glm::mat4(1.0f);
			boundCoffee = false;
		}
		else if (glm::distance(myCamera.cameraPosition, coffeeCup.getCenter()) < 0.3f) {
			boundCoffee = true;
		}
	}
	else if (!pressedKeys[GLFW_KEY_T]) {
		press_t = false;
	}
}

void coffeeInteraction() {
	if (!boundCoffee)
		return;
	//cupModel = glm::translate(glm::mat4(1.0f), myCamera.cameraPosition);
	cupModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.04f, 0.0f, -0.15f));
	cupModel = glm::inverse(myCamera.getViewMatrix()) * cupModel;
	cupModel = glm::rotate(cupModel, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	cupModel = glm::translate(cupModel, -coffeeCup.getCenter());
}

void drawObjects(gps::Shader shader, bool depthPass) {
	// ... (existing code)

	shader.useShaderProgram();

	// Rotate the entire scene
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	nanosuit.Draw(shader);
	scena.Draw(shader);


	// Rotate the ball with a different angle
	modelBall = glm::translate(glm::mat4(1.0f), ballPosition);
	modelBall = glm::rotate(modelBall, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f)); 
	modelBall = glm::rotate(modelBall, glm::radians(ballRotAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 
 // Additional rotation for the ball
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelBall));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * modelBall));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ball_buna.Draw(shader);

	glm::mat4 modelWood = glm::translate(glm::mat4(1.0f), woodPosition);
	modelWood = glm::rotate(modelWood, glm::radians(woodRotAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWood));

	// Do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * modelWood));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	// Draw the wood
	wood.Draw(shader);

	processCoffeeFlag();
	coffeeInteraction();
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cupModel));
	coffeeCup.Draw(shader);
}



void initSkybox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/coward-cove_rt.tga");
	faces.push_back("skybox/coward-cove_lf.tga");
	faces.push_back("skybox/coward-cove_up.tga");
	faces.push_back("skybox/coward-cove_dn.tga");
	faces.push_back("skybox/coward-cove_bk.tga");
	faces.push_back("skybox/coward-cove_ft.tga");
	mySkyBox.Load(faces);
}

float animationTime = 0.0f;
float animationDuration = 5.0f; // Adjust the duration of each animation


void renderScene() {


	// render depth map on screen - toggled with the M key
	
	//render the scene to the depth buffer
	depthShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);

	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//lightCube.Draw(lightShader);

	    
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}


int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}
	
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();
	

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();

		animateBallJump();
		animateWood();
		cameraAnimation();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
