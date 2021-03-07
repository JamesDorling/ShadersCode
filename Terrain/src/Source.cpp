#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"

#include <Model.h>
#include "Terrain.h"

#include<string>
#include <iostream>
#include <numeric>



// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
//unsigned int loadTexture2(char const * path);
void setVAO(vector <float> vertices);

// camera
Camera camera(glm::vec3(260,150,300));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//arrays
unsigned int terrainVAO;

int scale = 100;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Light
glm::vec3 lightPos(700.f, 300.f, 700.f);

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

//Sky
glm::vec4 sky(0.3f, 0.3f, 0.3f, 1.0f);
//glm::vec4 sky(0.0f, 1.0f, 0.0f, 1.0f);

//Fog activated
bool fog = true;
bool colorMode = false;
bool showShadow = true;

//Terrain Generation Mode (Used before the window loop to load a texture, nad therefore cannot change height maps at runtime without coding that wouldnt gain me many marks if any. Set to 0 as thats the one that looks the best, and is also the one I created everything around.)
int terrMode = 0;

//Draw Mode
int debugMode = 0;
int debugDrawMode = 0;
bool depthDraw = false;

//float dens = 1.0;
//float g = 0.45;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	

	// simple vertex and fragment shader - add your own tess and geo shader
	//Shader shader("..\\shaders\\plainVert.vs", "..\\shaders\\plainFrag.fs");
	Shader shader("..\\shaders\\tessVert.vs", "..\\shaders\\tessFrag.fs", "..\\shaders\\tessGeo.gs", "..\\shaders\\tessControl.tcs", "..\\shaders\\tessEval.tes");
	Shader Dshader("..\\shaders\\tessVert.vs", "..\\shaders\\PostProcessing\\depthBufferFrag.fs", "..\\shaders\\tessGeo.gs", "..\\shaders\\tessControl.tcs", "..\\shaders\\tessEval.tes");
	Shader postprocess("..\\shaders\\PostProcessing\\debug.vs", "..\\shaders\\PostProcessing\\debug.fs");
	Shader depthShader("..\\shaders\\PostProcessing\\depthBufferVert.vs", "..\\shaders\\PostProcessing\\depthBufferFrag2.fs");
	//Shader normShader("..\\shaders\\plainVert.vs", "..\\shaders\\normFrag.fs", "..\\shaders\\normGeo.gs");

	unsigned int heightMap;

	//textures
	if (terrMode == 0)
	{ 
		//DESIGNED AROUND THIS HEIGHTMAP (So this will look the best. Colours work on this heightmap, and may not on others. They definitely dont work on the perlin noise generated map.)
		heightMap = loadTexture("..\\Resources\\maps\\HeightMap.jpg");
	}
	if (terrMode == 2)
	{
		heightMap = loadTexture("..\\Resources\\maps\\CraterHeightMap.jpg");
	}
	if (terrMode == 3)
	{
		heightMap = loadTexture("..\\Resources\\maps\\crevHeightMap.jpg");
	}
	//unsigned int heightMap = loadTexture("..\\Resources\\textures\\tempGrass.jpg");

	//unsigned int white = loadTexture("..\\Resources\\textures\\white.jpg");
	//unsigned int tempGrass = loadTexture("..\\Resources\\textures\\tempGrass.jpg");

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain(50, 50,10);
	terrainVAO = terrain.getVAO();
	terrain.setFBOColour();
	terrain.setFBODepth();

	//Model
	//Model model1("..\\resources\\nano\\nanosuit\\nanosuit.obj");

	while (!glfwWindowShouldClose(window))
	{

		float near_plane = 1.0f, far_plane = 7.5f, orthoSize = 250.0f;
		glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1200.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		
		///////////////////////////

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		shader.setInt("heightMap", 1); //Send the heightmap to the shader (Made it as GL_TEXTURE1 early on because I was testing something, and now its just staying like that really)

		shader.setVec3("light.ambient", 0.3f, 0.3f, 0.3f);
		shader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);
		shader.setVec3("light.specular", 0.25f, 0.25f, 0.25f);
		shader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
		shader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
		shader.setVec3("camPos", camera.Position);

		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		shader.setInt("scale", scale);

		shader.setVec4("sky", sky);
		shader.setBool("fog", fog);
		shader.setBool("showShadow", showShadow);
		shader.setBool("colorMode", colorMode);
		shader.setInt("octaves", 10);

		shader.setInt("terrMode", terrMode);

		////////////////////////////
		Dshader.use();
		Dshader.setMat4("projection", projection);
		Dshader.setMat4("view", view);
		Dshader.setMat4("model", model);
		Dshader.setInt("heightMap", 1); //Send the heightmap to the shader (Made it as GL_TEXTURE1 early on because I was testing something, and now its just staying like that really)

		Dshader.setVec3("light.ambient", 0.3f, 0.3f, 0.3f);
		Dshader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);
		Dshader.setVec3("light.specular", 0.25f, 0.25f, 0.25f);
		Dshader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
		Dshader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
		Dshader.setVec3("camPos", camera.Position);

		Dshader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		Dshader.setInt("scale", scale);

		Dshader.setVec4("sky", sky);
		Dshader.setBool("fog", fog);
		Dshader.setBool("showShadow", showShadow);
		Dshader.setBool("colorMode", colorMode);
		Dshader.setInt("octaves", 10);

		Dshader.setInt("terrMode", terrMode);
		/////////

		//shader.setFloat("DENS", dens);
		//shader.setFloat("G", g);

		if (debugDrawMode == 0)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (debugDrawMode == 1)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (debugDrawMode == 2)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

		//normShader.use();
		//normShader.setMat4("projection", projection);
		//normShader.setMat4("view", view);
		//normShader.setMat4("model", model);
		//normShader.setInt("Debug", debugMode);
		//model1.Draw(normShader);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, heightMap);

		//Attempt at shadow
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		Dshader.use();
		Dshader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		Dshader.setMat4("view", lightView);
		Dshader.setMat4("projection", lightProjection);
		Dshader.setBool("showShadow", false);
		glBindFramebuffer(GL_FRAMEBUFFER, terrain.depthFBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(sky.x, sky.y, sky.z, 1.0f);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, terrain.textureDepthBuffer);
		glBindVertexArray(terrainVAO);
		//shader.use();
		glDrawArrays(GL_PATCHES, 0, terrain.getSize());
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//terrain.renderQuad();
		//shader.setInt("shadowMap", terrain.textureDepthBuffer);
		


		if (depthDraw == false)
		{
			//First Pass////////////////////////////////////////////////////////////////////////
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			shader.use();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			shader.setBool("showShadow", showShadow);
			glEnable(GL_DEPTH_TEST);
			glClearColor(sky.x, sky.y, sky.z, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrain.textureDepthBuffer);
			shader.setInt("shadowMap", terrain.textureDepthBuffer);

			glBindVertexArray(terrainVAO);
			glDrawArrays(GL_PATCHES, 0, terrain.getSize()); //Breaks if previous draw call uses patches (As depthShader does not have a tess shader.)
		}
		else
		{
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			depthShader.use();
			depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
			depthShader.setMat4("view", lightView);
			depthShader.setMat4("projection", lightProjection);
			depthShader.setBool("showShadow", false);
			glClearColor(sky.x, sky.y, sky.z, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrain.textureDepthBuffer);
			depthShader.setInt("depthTex", terrain.textureDepthBuffer);

			//glBindVertexArray(terrainVAO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			terrain.renderQuad();
		}
		
		//Second pass///////////////////////////////////////////////////////////////////////
		//Colour FrameBuffer
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		//postprocess.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, terrain.textureColorBuffer);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//terrain.renderQuad();

		//Depth FrameBuffer
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		//postprocess.use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, terrain.textureDepthBuffer);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//terrain.renderQuad();




		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		
		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	float speed = 5.f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = 10.f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		speed = 1.f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime * speed);

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
	{
		debugMode = 0;
		debugDrawMode = 0;
		fog = true;
		showShadow = true;
	}

	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
	{
		camera.Position = lightPos;
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		debugMode = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		debugMode = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		debugDrawMode = 1;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		debugDrawMode = 2;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		fog = false;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		showShadow = false;
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
		debugMode = 0;
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
		debugDrawMode = 0;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //These inputs move the lights
		lightPos.y += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		lightPos.y -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		lightPos.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		lightPos.x += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		lightPos.z -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		lightPos.z += 0.1f;

	if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
	{
		terrMode = 0; //Set the terrain to the height map terrain
	}
	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
	{
		terrMode = 1; //Set the terrain to be generated by perlin noise (Not coloured properly)
	}
	if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
	{
		colorMode = false; //Use old colouring of top is white, mid is gray, bottom is green
	}
	if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
	{
		colorMode = true; //Use colouring that uses the normals to remove "snow" from steep edges
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		depthDraw = true;
	}
	else
	{
		depthDraw = false;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}





// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID <<  std::endl;

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		
	}

	return textureID;
}



