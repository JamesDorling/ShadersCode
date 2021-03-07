#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 view; //View Matrix
uniform mat4 projection; //Projection Matrix
uniform vec3 viewPos; //View position

out vec2 gUV;

void main()
{    
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(viewPos - pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(toCamera, up);
	mat4 VP = view * projection;

	gl_Position = VP * vec4(pos, 1.0);
	gUV = vec2(0.0, 0.0);
	EmitVertex();

	pos.y += 1.0;
	gl_Position = VP * vec4(pos, 1.0);
	gUV = vec2(0.0, 1.0);
	EmitVertex();

	pos.y -= 1.0;
	pos += right;
	gl_Position = VP * vec4(pos, 1.0);
	gUV = vec2(1.0, 0.0);
	EmitVertex();

	pos.y += 1.0;
	gl_Position = VP * vec4(pos, 1.0);
	gUV = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}  