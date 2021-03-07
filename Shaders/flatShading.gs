#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 vNormal;
	vec2 vUV;
	vec3 FragPos;
}gs_in[];

out GS_OUT
{
	vec3 vNormal;
	vec2 vUV;
	vec3 FragPos;
}gs_out;



vec3 GetNormal()
{
    // Return the surface normal of the triangle
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}


void main()
{    
	vec3 normal = GetNormal();

	for(int i = 0; i < 3; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		gs_out.vNormal = normal;
		gs_out.vUV = gs_in[i].vUV;
		gs_out.FragPos = gs_in[i].FragPos;
		EmitVertex();

		//gl_Position = gl_in[i].gl_Position + vec4(normal * 0.1, 0.0);
		//gs_out.vNormal = normal;
		//gs_out.vUV = gs_in[i].vUV;
		//gs_out.FragPos = gs_in[i].FragPos;
		//EmitVertex();

		EndPrimitive();
	}

}  