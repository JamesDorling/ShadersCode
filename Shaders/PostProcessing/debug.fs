#version 410 core

uniform sampler2D scene;

const float near_plane = 1;
const float far_plane = 1000;



in vec2 vUV;

out vec4 FragColor;


void main()
{
	//ColorBuffer
	vec4 result = texture2D(scene, vUV);

	FragColor = result;
}
