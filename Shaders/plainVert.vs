#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 tangent;

uniform mat4 model; //Model Matrix
uniform mat4 view; //View Matrix
uniform mat4 projection; //Projection Matrix
uniform vec3 lightPos; //Light position
uniform vec3 viewPos; //View position


out VS_OUT
{
	vec3 vNormal;
	vec2 vUV;
	vec3 FragPos;
}vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	vs_out.vNormal = aNormal;
	vs_out.vUV = aUV;
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
}