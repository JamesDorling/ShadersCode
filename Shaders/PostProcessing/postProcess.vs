#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

//Get the uniform values from the CPU
uniform int postProcess;
uniform float time;

out vec2 vUV;

void main()
{
	vUV = aUV; //Pass through the uvs
	gl_Position = vec4(aPos, 1.0); //Pass through the position

	if(postProcess == 5) //Move the quad if the post processing is of value 5
	{
		float strength = 0.01; //Strength of the shake
        gl_Position.x += cos(time * 10) * strength; //Move the quad based on the timestep with a cos function
        gl_Position.y += sin(time * 15) * strength; //Move the quad based on the timestep with a sin function
	}
}