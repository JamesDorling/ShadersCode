#version 330 core
layout(triangles) in ;
layout(triangle_strip, max_vertices = 3) out ;
vec3 getNormal() ;

uniform int terrMode;

in TE_OUT
{
	vec3 teFragPos;
	vec2 teUV;
	vec3 teNormal;
	float height;
	float vis;
	float dist;
}gs_in[];

out GS_OUT
{
	vec3 gFragPos;
	vec2 gUV;
	vec3 gNormal;
	float gHeight;
	float vis;
	float dist;
}gs_out;

void main()
{
    for(int i = 0 ; i < 3; i++)
    {
       gl_Position = gl_in[i].gl_Position;
       gs_out.gFragPos = gs_in[i].teFragPos;
	   gs_out.gUV = gs_in[i].teUV;
	   if(terrMode != 1)
	   {
			gs_out.gNormal = gs_in[i].teNormal;
	   }
	   else if(terrMode == 1)
	   {
			gs_out.gNormal = getNormal();
	   }
	   
	   gs_out.gHeight = gs_in[i].height;
	   gs_out.vis = gs_in[i].vis;
	   gs_out.dist = gs_in[i].dist;
       EmitVertex() ;
    }
    EndPrimitive() ;

}


vec3 getNormal()
{
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(a, b));
}