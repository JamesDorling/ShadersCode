#version 330 core

in vec2 vUV;

uniform sampler2D depthTex;
//uniform sampler2D scene;

out vec4 FragColor;

void main()
{
   // draw depth texture orthographically
   float depthValue = texture(depthTex, vUV).r; //use the shadow map to get the depth value
   FragColor = vec4(vec3(depthValue), 1.0); //colour the fragments based on their depth
   //FragColor = texture2D(depthTex, vUV);

}