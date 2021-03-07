#version 410 core

float linearizeDepth(float depth);

uniform sampler2D scene;

const float near_plane = 1;
const float far_plane = 1000;

in vec2 vUV;

out vec4 FragColor;

void main()
{
	//ColorBuffer
	vec4 result = texture2D(scene, vUV);
	float avg = (result.x + result.y + result.z) / 3;

	//Greyscale
	//FragColor = vec4(vec3(avg), 1.0);

	//DepthBuffer
	//float depth = texture(scene, vUV).r;
	////vec4 result = vec4(vec3(depth), 1.0);
	//vec4 result = vec4(vec3(linearizeDepth(depth) / far_plane), 1.0);


	FragColor = result;
}

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}