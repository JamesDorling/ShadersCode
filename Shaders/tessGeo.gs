#version 330 core
layout(triangles) in ;
layout(triangle_strip, max_vertices = 3) out ;
vec3 getNormal() ;

uniform int terrMode; //Get the terrain generation mode

in TE_OUT
{
	vec3 teFragPos;
	vec2 teUV;
	vec3 teNormal;
	float height;
	float vis;
	float dist;
	vec4 fragPosLightSpace;
}gs_in[]; //Array as 3 are recieved at a time, allows for calculation of normals

out GS_OUT
{
	vec3 gFragPos;
	vec2 gUV;
	vec3 gNormal;
	float gHeight;
	float vis;
	float dist;
	vec4 fragPosLightSpace;
}gs_out; //Output, sent to fragment shader

void main()
{
    for(int i = 0 ; i < 3; i++)
    {
       gl_Position = gl_in[i].gl_Position; //Pass through gl_Position
       gs_out.gFragPos = gs_in[i].teFragPos; //Pass through the fragment position
	   gs_out.gUV = gs_in[i].teUV; //Pass through the UV co-ordinates
	   if(terrMode != 1) //If the terrain is not generated with perlin noise
	   {
			gs_out.gNormal = gs_in[i].teNormal; //Pass through the normals
	   }
	   else if(terrMode == 1) //If the terrain is generated with perlin noise
	   {
			gs_out.gNormal = getNormal(); //Calculate the normals
	   }
	   
	   gs_out.gHeight = gs_in[i].height; //Pass through the height
	   gs_out.vis = gs_in[i].vis; //Pass through the visibility
	   gs_out.dist = gs_in[i].dist; //Pass through the distance
	   gs_out.fragPosLightSpace = gs_in[i].fragPosLightSpace; //Pass through the light space matrix
       EmitVertex() ; //Add the vertex
    }
    EndPrimitive() ; //End the primitive once finished

}


vec3 getNormal()
{
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position); //Take away the second vertex position from the first and assign that value to a
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position); //Take away the third vertex position from the first and assign that value to b
    return normalize(cross(a, b)); //Normalise the cross product of a and b to get the normal
}