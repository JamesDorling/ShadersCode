#include "Terrain.h"


//Terrain constructors
Terrain::Terrain(int widthIn, int heightIn, int stepSizeIn)
{
	width = widthIn;
	height = heightIn;
	stepSize = stepSizeIn;
	makeVertices(&vertices);

}

Terrain::Terrain() {
	width = 50;
	height = 50;
	stepSize = 10;
	makeVertices(&vertices);

}

unsigned int Terrain::getVAO() {
	
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_STATIC_DRAW);

		//xyz
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//texture
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	
		return VAO;
}

int Terrain::getSize() {
	return vertices.size();
}

void Terrain::setFBOColour()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); //CHANGE IT TO GET THE SCREENWIDTH AND HEIGHT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, textureColorBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void Terrain::setFBODepth()
{
	//const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glGenFramebuffers(1, &depthFBO);

	//create the depth texture
	glGenTextures(1, &textureDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_HEIGHT, SCR_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//Texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Attach the texture as the depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	//not attaching a colour buffer
	glDrawBuffer(GL_NONE);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void Terrain::renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


std::vector<float> Terrain::getVertices() {
	return vertices;
}

void Terrain::makeVertices(std::vector<float> *vertices) {
	/* triangle a b c
		   b
		   | \
		   a _ c


		 triangle d f e
		   f _ e
			 \ |
			   d

		 c == d
		 b == f
		 Duplicate vertices but easier in long run! (tesselation and LOD)

		a = (x,y,z)
		b = (x, y+1)
		c = (x+1,y)

		d = (x+1,y)
		e = (x, y+1)
		f = (x+1,y+1)

		 each vertex a, b,c, etc. will have 5 data:
		 x y z u v
		  */

	for (int y = 0; y < height - 1; y++) {
		float  offSetY = y * stepSize;
		for (int x = 0; x < width - 1; x++) {
			float offSetX = x * stepSize;
			makeVertex(offSetX, offSetY, vertices);  // a
			makeVertex(offSetX, offSetY + stepSize, vertices);  // b
			makeVertex(offSetX + stepSize, offSetY, vertices);   // c
			makeVertex(offSetX + stepSize, offSetY, vertices);  //d
			makeVertex(offSetX, offSetY + stepSize, vertices);  //e
			makeVertex(offSetX + stepSize, offSetY + stepSize, vertices);  //f
		}
	}
}

void Terrain::makeVertex(int x, int y, std::vector<float> *vertices) 
{
	//glm::vec3 pos = glm::vec3(x, y, 0.001);
	//double pn = cycleOctaves(pos, 10);


	//x y z position
	vertices->push_back((float)x); //xPos
	//vertices->push_back(pn);
	vertices->push_back(0); //yPos - always 0 for now. Going to calculate this on GPU - can change to calclulate it here.
	vertices->push_back((float)y); //zPos

   // add texture coords
	vertices->push_back((float)x / (width*stepSize));
	vertices->push_back((float)y / (height*stepSize));

	//std::cout << "Made vertex";


}

double Terrain::cycleOctaves(glm::vec3 pos, int numOctaves)
{
	float total = 0.0f;
	float maxAmplitude = 0.0f;

	float amplitude = 100.0f;
	float freq = 0.005f;

	for (int i = 0; i < numOctaves; i++)
	{
		double x = pos.x * freq;
		double y = pos.y * freq;
		total += perlin.noise(x, y, 0.01) * amplitude;
		maxAmplitude += amplitude;
		freq *= 2;
		amplitude /= 2;
	}
	return(total / maxAmplitude);
}