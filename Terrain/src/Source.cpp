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
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
//unsigned int loadTexture2(char const * path);
void setVAO(vector <float> vertices);
void setDebugDrawMode();
void setShaderLightData(Shader& shader);
void renderQuad();

void setFBOColour();
void setFBODepth();

// camera
Camera camera(glm::vec3(260,150,300));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

unsigned int quadVAO, quadVBO, FBO, depthFBO;
unsigned int textureColorBuffer;
unsigned int textureDepthBuffer;

//arrays
unsigned int terrainVAO;

int scale = 100;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Light
glm::vec3 lightPos(295.f, -250.f, 1000.f);

glm::mat4 lightProjection;
glm::mat4 lightView;
glm::mat4 lightSpaceMatrix;

//Sky
glm::vec4 sky(0.3f, 0.3f, 0.3f, 1.0f);
//glm::vec4 sky(0.0f, 1.0f, 0.0f, 1.0f);

//Fog activated
bool fog = true;
int fogMode = 0;
bool colorMode = true;
bool showShadow = true;

//Terrain Generation Mode
int terrMode = 0;

//Draw Mode
int debugDrawMode = 0;
bool depthDraw = false;

int postProcessMode = 0;

//float dens = 1.0;
//float g = 0.45;

int main()
{
	glfwInit(); //Initialise GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Set the glfw context version major to 3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Set the glfw context version minor to 3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Set the opengl profile to the core profile
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL); // Create and set the glfw window using the screen width and height, with the name IMAT3907 on the primary monitor.
	if (window == NULL) //If the window failed to be created
	{
		std::cout << "Failed to create GLFW window" << std::endl; //Print error to the console
		glfwTerminate(); //Terminate glfw
		return -1; //Return, ending main
	}
	glfwMakeContextCurrent(window); //Make the current window's context the currently used context
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Set the framebuffer's size
	glfwSetCursorPosCallback(window, mouse_callback); //Set the mouse position callback
	glfwSetScrollCallback(window, scroll_callback); //Set the mouse scroll callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //disable the cursor and lock it to the middle of the screen

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Initialise glad
	{
		std::cout << "Failed to initialize GLAD" << std::endl; //If glad fails to initialise print that to the screen
		return -1;
	}
	
	//Create shaders, each shader's use is stated below.
	Shader CBshader("..\\shaders\\tessVert.vs", "..\\shaders\\tessFrag.fs", "..\\shaders\\tessGeo.gs", "..\\shaders\\tessControl.tcs", "..\\shaders\\tessEval.tes");
	Shader Dshader("..\\shaders\\Depth\\depthBVert.vs", "..\\shaders\\Depth\\depthBFrag.fs", nullptr, "..\\shaders\\Depth\\tessControlDepth.tcs", "..\\shaders\\Depth\\tessEvalDepth.tes");
	Shader postprocess("..\\shaders\\PostProcessing\\postProcess.vs", "..\\shaders\\PostProcessing\\postProcessing.fs");
	Shader depthShader("..\\shaders\\PostProcessing\\depthBufferVert.vs", "..\\shaders\\PostProcessing\\depthBufferFrag2.fs");
	//Shader normShader("..\\shaders\\plainVert.vs", "..\\shaders\\normFrag.fs", "..\\shaders\\normGeo.gs"); //Normal shader for drawing normals. Won't work with the terrain.

	/*
	Shader is the main shader used for rendering to the screen.
	Dshader is the shader used to render to the depth buffer.
	depthShader is used to render the depth buffer to screen. (It does this similarly to how a colour buffer would work.)
	postprocess is used to render the colour buffer to screen.
	*/

	unsigned int heightMap; //Unsigned int for the heightmap

	heightMap = loadTexture("..\\Resources\\maps\\HeightMap.jpg"); //Load the heightmap

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain(50, 50,10); //Terrain constructor, create a grid of vertices
	terrainVAO = terrain.getVAO(); //Set the terrainVAO to the VAO of the vertices grid

	setFBOColour(); //set up the colour FBO
	setFBODepth(); //set up the depth FBO

	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime(); //Get the timestep
		deltaTime = currentFrame - lastFrame; //Get the deltatime
		lastFrame = currentFrame; //set the last frame's timestep
		processInput(window); //Process the input

		lightPos.y +=(sin(glfwGetTime())*0.25); //Move the lightpos up and down with sin
		if (lightPos.y > 300) //Limit the height of the light to 300
			lightPos.y = 300;
		if (lightPos.y < -200) //Limit the height of the light to -200
			lightPos.y = -200; //Couple if statements as the "sin" function coupled with the get time makes it very random, and can sometimes not actually end up showing shadows due to just going too high / too low.
		//lightPos.x +=(cos(glfwGetTime())*0.25);
		//lightPos.z +=(sin(glfwGetTime())*0.25);
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1200.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		

		///////////////////////////////
		// Pass values to the shaders
		///////////////////////////////
		CBshader.use();
		CBshader.setMat4("projection", projection);
		CBshader.setMat4("view", view);
		CBshader.setMat4("model", model);
		CBshader.setInt("heightMap", 0); //Send the heightmap to the shader
		CBshader.setInt("shadowMap", 1);
		
		setShaderLightData(CBshader);
		
		CBshader.setVec3("camPos", camera.Position);
		
		CBshader.setInt("scale", scale);
		
		CBshader.setVec4("sky", sky);
		CBshader.setBool("fog", fog);
		CBshader.setBool("showShadow", showShadow);
		CBshader.setBool("colorMode", colorMode);
		CBshader.setInt("octaves", 10);
		
		CBshader.setInt("terrMode", terrMode);
		
		CBshader.setInt("fogMode", fogMode);
		////////////////////////////
		Dshader.use();
		Dshader.setMat4("model", model);
		Dshader.setMat4("view", lightView); //Pass the light's view and projection to draw the scene from the lights POV
		Dshader.setMat4("projection", lightProjection);

		Dshader.setVec3("camPos", camera.Position);
		Dshader.setInt("octaves", 10);
		Dshader.setInt("terrMode", terrMode);
		Dshader.setInt("heightMap", 0);
		Dshader.setInt("scale", scale); 
		///////////////////////////////

		setDebugDrawMode(); //function to set the polygon mode
		glDisable(GL_CULL_FACE); //Disable face culling so that the shadows will be drawn without it
		

		// fill depth buffer
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); //Set the viewport to the shadow map's size
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO); //Bind the depthFBO
		glEnable(GL_DEPTH_TEST); //Enable depth testing
		Dshader.use(); //Use Dshader for this pass
		glActiveTexture(GL_TEXTURE0); //Set the active texture
		glBindTexture(GL_TEXTURE_2D, heightMap); //Set GL_TEXTURE0 to the heightmap
		glBindVertexArray(terrainVAO); //Bind the vertex array
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the depth and colour buffer
		glDrawArrays(GL_PATCHES, 0, terrain.getSize()); //Draw the scene

		glEnable(GL_CULL_FACE); //Enable cull face
		glCullFace(GL_BACK); //Cull the back face

		// draw scene
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); //set the viewport to the screen's size
		//glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); //bind the colour buffer framebuffer
		CBshader.use(); //Use the colourbuffer shader
		glClearColor(sky.x, sky.y, sky.z, 1.0f); //set the background colour
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the depth and colour buffer
		glActiveTexture(GL_TEXTURE0); //Set the active texture to 0
		glBindTexture(GL_TEXTURE_2D, heightMap); //set GL_TEXTURE0 to the heightmap
		glActiveTexture(GL_TEXTURE1); //Set the active texture to 1
		glBindTexture(GL_TEXTURE_2D, textureDepthBuffer); //Set the shadow map to GL_TEXTURE1
		//shader.setInt("shadowMap", 1);
		glBindVertexArray(terrainVAO); //Bind the terrain vertex array
		glDrawArrays(GL_PATCHES, 0, terrain.getSize()); //Draw the scene

		//Colour Buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind framebuffers, to draw to screen
		glDisable(GL_DEPTH_TEST); //Disable depth testing
		postprocess.use(); //Use the postprocessing shader
		postprocess.setInt("postProcess", postProcessMode); //Pass through the post-processing mode
		postprocess.setFloat("time", glfwGetTime()); //Pass through the timestep
		//postprocess.setInt("scene", textureColorBuffer);
		glActiveTexture(GL_TEXTURE0); //Set the active texture to 0
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer); //Set GL_TEXTURE0 to be the colourbuffer texture
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Set the polygonmode to gl_fill
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the depth and colour buffers (Uneccesary as it doesnt do much, but just in case)
		renderQuad(); //Draw a quad to put the texture on



		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { //If q is pressed
			//glViewport(700, 800, 200, 200);
			//glfwSetWindowSize(window, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0); //Drawing to screen
			glDisable(GL_DEPTH_TEST); //disable depth testing
			depthShader.use(); //use the depthShader
			glActiveTexture(GL_TEXTURE0); //Set the active texture to 0
			glBindTexture(GL_TEXTURE_2D, textureDepthBuffer); //Set GL_TEXTURE0 to the shadow map
			//glClearColor(sky.x, sky.y, sky.z, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the depth and buffer bit
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			renderQuad();
		}
		
		glfwSwapBuffers(window); //Swap window buffers
		glfwPollEvents(); //Poll window events
	}


	glfwTerminate(); //close the window
	return 0; //Return 0, ending main
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	float speed = 5.f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true); //Close the window

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = 10.f; //Speed up the camera for convenience
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		speed = 1.f; //Slow down the camera for convenience

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime * speed); //Move the camera forward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime * speed); //Move the camera back
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime * speed); //Move the camera left
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime * speed); //Move the camera right

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) //0 key resets everything to default
	{
		debugDrawMode = 0;
		fog = true;
		showShadow = true;
		postProcessMode = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		debugDrawMode = 1; //Debug draw mode (changes how polygons are drawn)
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		debugDrawMode = 2; //Debug draw mode (changes how polygons are drawn)
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		fog = false; //Disable fog
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		showShadow = false; //Disable shadows

	//if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) //These inputs move the lights (WERE USED FOR DEBUGGING SHADOWMAPPING)
	//	lightPos.y += speed / 5;
	//if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	//	lightPos.y -= speed / 5;
	//if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	//	lightPos.x -= speed / 5;
	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	//	lightPos.x += speed / 5;
	//if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	//	lightPos.z -= speed / 5;
	//if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	//	lightPos.z += speed / 5;
	//std::cout << lightPos.y  << std::endl;

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
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
	{
		fogMode = 0; //Use colouring that uses the normals to remove "snow" from steep edges
	}
	if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
	{
		fogMode = 1; //Use colouring that uses the normals to remove "snow" from steep edges
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		postProcessMode = 1; //Turn on the first post processing method
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		postProcessMode = 2; //Turn on the second post processing method
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		postProcessMode = 3; //Turn on the third post processing method
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		postProcessMode = 4; //Turn on the fourth post processing method
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		postProcessMode = 5; //Turn on the fifth post processing method
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height); //Change the window size
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
	glGenTextures(1, &textureID); //Generate a texture at textureID

	int width, height, nrComponents; //Texture's properties
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0); //Load the file at filepath and get the amount referred to by the properties
	if (data) //If the texture loaded
	{
		GLenum format; //Channels for the colour
		if (nrComponents == 1)
			format = GL_RED; //If one colour, set it to red
		else if (nrComponents == 3)
			format = GL_RGB; //If three colours set it to RGB
		else if (nrComponents == 4)
			format = GL_RGBA; //If four colours set it to use RGBA

		glBindTexture(GL_TEXTURE_2D, textureID); //Bind the texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); //Generate the texture
		glGenerateMipmap(GL_TEXTURE_2D); //Generate a mipmap

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //Set the texture to repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //Set the texture to repeat
		float borderColour[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour); //Set the border colour
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //Set the texture's min filter params to use a linear mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //Set the textures mag filter params to use linear
		stbi_image_free(data); //Unload the image and free the space
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID <<  std::endl; //Print the outcome to console

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl; //Print the error to console
		stbi_image_free(data); //Free the texture's space
		
	}

	return textureID; //Return the texture's ID
}

void setDebugDrawMode()
{
	switch (debugDrawMode)
	{
	case 0:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Set the polygons to be filled in
		break;
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Set the outlines of each polygon to be drawn
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //Set the individual vertices to be drawn
		break;
	default:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Default to fill
		break;

	}
}

void setShaderLightData(Shader& shader) //Function passes in light data to a shader, while creating the light space matrix.
{
	float near_plane = 1.0f, far_plane = 2000.5f, orthoSize = 250.0f;
	lightProjection  = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane); //Create the lights projection
	lightView        = glm::lookAt(lightPos, glm::vec3(100.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0)); //Create the light's view
	lightSpaceMatrix = lightProjection * lightView; //Create the light space matrix

	//Pass through the light data to the shader given
	shader.setVec3("light.ambient", 0.3f, 0.3f, 0.3f);
	shader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);
	shader.setVec3("light.specular", 0.25f, 0.25f, 0.25f);
	shader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
	shader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);

	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
}


void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = //Vertex coordinates for a square
		{
			-1.1f,  1.1f, 0.0f, 0.0f, 1.0f,
			-1.1f, -1.1f, 0.0f, 0.0f, 0.0f,
			 1.1f,  1.1f, 0.0f, 1.0f, 1.0f,
			 1.1f, -1.1f, 0.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &quadVAO);//Generate a vertex array
		glGenBuffers(1, &quadVBO);//Generate a vertex buffer
		glBindVertexArray(quadVAO); //Bind the vertex array
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO); //Bind the VBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW); //Set the buffer data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); //Pass through the vertex positions
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //Pass through the UVs
	}
	glBindVertexArray(quadVAO); //Bind the vertex array
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the square
	glBindVertexArray(0); //Unbind the vertex array
}

void setFBOColour()
{
	glGenFramebuffers(1, &FBO); //Generate a framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO); //Bind said framebuffer

	glGenTextures(1, &textureColorBuffer); //Generate a texture
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer); //Bind said texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //CHANGE IT TO GET THE SCREENWIDTH AND HEIGHT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //Set the texture's min filter to GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //Set the texture's mag filter to GL_NEAREST
	//glBindFramebuffer(GL_FRAMEBUFFER, textureColorBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0); //Set the texture's params

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO); //Generate a render buffer object
	glBindRenderbuffer(GL_RENDERBUFFER, RBO); //bind the render buffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); //set the render buffer's params
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); //Set the render buffer
}

void setFBODepth()
{
	//const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glGenFramebuffers(1, &depthFBO); //Generate a framebuffer

	//create the depth texture
	glGenTextures(1, &textureDepthBuffer); //Generate a texture
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer); //Bind the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_HEIGHT, SCR_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); //Set the texture's params

	//Texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //Set the texture's min filter params to GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //Set the texture's mag filter params to GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //Set the texture to repeat at the edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //Set the texture to repeat at the edges

	//Attach the texture as the depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO); //Bind the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0); //Bind the framebuffer and texture

	//not attaching a colour buffer
	glDrawBuffer(GL_NONE); //Let openGL know we arent using a colour buffer here
	glDrawBuffer(GL_NONE); //Let openGL know we arent using a colour buffer here
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind the framebuffer


}