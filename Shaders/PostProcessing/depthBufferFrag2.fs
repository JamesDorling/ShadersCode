#version 330 core

in vec2 vUV;

uniform sampler2D depthTex;

out vec4 FragColor;

void main()
{
	// draw depth texture orthographically
	float depthValue = texture(depthTex, vUV).r;
    FragColor = vec4(vec3(depthValue), 1.0);

}