#version 410 core

out vec4 FragColor;

uniform vec3 viewPos; //View position

in GS_OUT
{
	vec3 vNormal;
	vec2 vUV;
	vec3 FragPos;
}fs_in;

void main()
{
	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}