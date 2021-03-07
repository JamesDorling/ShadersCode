#ifndef TERRAIN_H
#define TERRAIN_H


#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "PerlinNoise.h"

class Terrain
{
public:
	Terrain(int widthIn, int heightIn, int stepSizeIn);
	Terrain();
	unsigned int getVAO();
	int getSize();
	void setFBOColour();
	void setFBODepth();
	void renderQuad();

	PerlinNoise perlin;
	
	unsigned int VAO, VBO, quadVAO, quadVBO, FBO, depthFBO;
	unsigned int textureColorBuffer;
	unsigned int textureDepthBuffer;

	// settings
	const unsigned int SCR_WIDTH = 1200;
	const unsigned int SCR_HEIGHT = 900;
	
private:
	std::vector<float> vertices;

	int width;
	int height;
	int stepSize;
	void makeVertices(std::vector<float> *vertices);
	void makeVertex(int x, int y, std::vector<float> *vertices);
	std::vector<float> getVertices();
	double cycleOctaves(glm::vec3 pos, int numOctaves);
};
#endif












