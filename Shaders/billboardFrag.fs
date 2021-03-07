#version 410 core


in vec2 gUV;


uniform sampler2D billboard;
out vec4 FragColor;

uniform float threshold;

void main()
{
	FragColor = texture(billboard, gUV);

	if(FragColor.r > threshold && FragColor.g > threshold && FragColor.b > threshold)
	{
		discard;
	}
}