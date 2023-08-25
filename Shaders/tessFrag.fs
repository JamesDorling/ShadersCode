#version 410 core

out vec4 FragColor;

float calcShadow(vec4 fragPosLightSpace);

uniform vec3 viewPos; //View position

in GS_OUT
{
	vec3 gFragPos;
	vec2 gUV;
	vec3 gNormal;
	float gHeight;
	float vis;
	float dist;
	vec4 fragPosLightSpace;
}fs_in; //Inputs recieved from geography shader

struct LightAttr
{
	vec3 position; //Light position
	vec3 ambient; //Ambient light values
	vec3 diffuse; //Diffuse light values
	vec3 specular; //Specular light values
}; //Light attributes, recieved as a uniform

//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;
//uniform sampler2D texture_normal1;

//Recieve uniforms from the cpu
uniform LightAttr light; //Uniform light attribute

uniform vec4 sky;
uniform int scale;

uniform int terrMode;

uniform bool fog;
uniform bool colorMode;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform bool showShadow;

//Define colours, for use in colouring the terrain
vec4 gray = vec4(0.4, 0.4, 0.4, 0.0);
vec4 white = vec4(1.0, 1.0, 1.0, 0.0);
vec4 green = vec4(0.2, 0.4, 0.2, 0.0);

//Three lighting colours
vec3 ambient;
vec3 diffuse;
vec3 specular;

vec3 lightDir;//Light direction
vec3 normal; //Normal, used in lighting

void main()
{    
	lightDir = normalize(light.position - fs_in.gFragPos); //Get the light position by normalising the Light position tangent - the fragment position tangent)
	normal = normalize(fs_in.gNormal); //Get the normal by normalising the fragment normal
	
	float	diffFactor = max(dot(lightDir, normal), 0.0); //Define the diffuse factor by taking the dot product of the light direction and the normal, and clamping it
	
	vec3    viewDir = normalize(viewPos - fs_in.gFragPos); //Normalise the tangent of the view position minus the fragment position for the view position
	float	specFactor = 0.0; //define the specfactor as a float
	vec3	halfDir = normalize(lightDir + viewDir); //Normalise the lightdirection and the viewdirection for the specular
	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0); //Set the specular factor (Dot pruduct of the halfDir and the normal, clamped and to the power of 64

	if(terrMode != 1) //If perlin noise is not being used
	{
		if (colorMode == false)
		{
			//This colourmode specifically uses height to colour the terrain. Colour blends into one another based on height.
			if(fs_in.gHeight > 10)
			{
				ambient = light.ambient * vec3(white);
				diffuse = diffFactor * light.diffuse * vec3(white);
				specular = specFactor * light.specular * vec3(white);
			}
			else if (fs_in.gHeight >= 0.2)
			{
				ambient = light.ambient * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
				diffuse = diffFactor * light.diffuse * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
				specular = specFactor * light.specular * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
			}
			if (fs_in.gNormal.y <= 0.2 || fs_in.gHeight <= 0.2)
			{
				ambient = light.ambient * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
				diffuse = diffFactor * light.diffuse * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
				specular = specFactor * light.specular * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
			}
		}
		else if (colorMode == true)
		{ //Here is where the colour is decided based on normal and height. This is done by basically running the same as above, however white (The snow) is added dependant on slope as well as height.
			//Slopes are calculated based on the normal, as a low normal on the Y axis means that the fragment has a steep slope.
			if(fs_in.gHeight > 0.2 && fs_in.gNormal.y > ((fs_in.gHeight - 1.15) * -1))
			{
				ambient = light.ambient * vec3(mix(gray, white, smoothstep(0.3, 1.0, fs_in.gHeight)).rgb);
				diffuse = diffFactor * light.diffuse * vec3(mix(gray, white, smoothstep(0.3, 1.0, fs_in.gHeight)).rgb);
				specular = specFactor * light.specular * vec3(mix(gray, white, smoothstep(0.3, 1.0, fs_in.gHeight)).rgb);
			}
			else if (fs_in.gHeight >= 0.2 && fs_in.gNormal.y > 0.2)
			{
				ambient = light.ambient * vec3(gray);
				diffuse = diffFactor * light.diffuse * vec3(gray);
				specular = specFactor * light.specular * vec3(gray);
			}
			if (fs_in.gNormal.y <= 0.2 || fs_in.gHeight <= 0.2)
			{
				ambient = light.ambient * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
				diffuse = diffFactor * light.diffuse * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
				specular = specFactor * light.specular * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
			}
		}
	}
	else if(terrMode == 1) //Else if the terrain is generated via perlin noise, colour it green.
	{
		ambient = light.ambient * vec3(green);
		diffuse = diffFactor * light.diffuse * vec3(green);
		specular = specFactor * light.specular * vec3(green);
	}

	float shadow = calcShadow(fs_in.fragPosLightSpace); //Calculate the shadow level of the fragment

	vec3 result; //Predefine result
	if(showShadow == false) //If shadows are off
	{
		shadow = 0; //Shadows = 0, and are not applied
		result = (ambient + diffuse + specular); //Resulting blinn-phong shader is the ambient plus the diffuse plus the specular
	}
	else
	{
		result = (ambient + (1.0 - shadow) * ( diffuse + specular)); //Else add the shadow to the ambient, as it wont take part in the diffuse and specular anyway.
	}

	FragColor = vec4(result, 1.0); //Set fragcolor to the result
	//if(shadow > 0.0)
	//	FragColor = vec4(0.0, 0.0, 1.0, 1.0);

	if(fog == true)
	{
		FragColor = mix(sky, FragColor, fs_in.vis); //If fog is on, mix the fragment's colour with the background based on distance to the camera.
	}
}

float calcShadow(vec4 fragPosLightSpace)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; //Get the coodrinates of the  fragment in light space
    
	projCoords = (projCoords * 0.5) + 0.5; //Make the projcoords into a value between 1 and 0
    
	float closestDepth = texture(shadowMap, projCoords.xy).r; //Get the closest depth
    
	float currentDepth = projCoords.z; //Get the current depth
    
	float shadow = 0.0; //Pre-define shadow
    
	float bias = 0.015; //Low bias
    
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int i = -1; i < 9; i++)
	{
		for(int j = -1; j < 9; j++)
		{
			float pcf = texture(shadowMap, projCoords.xy + vec2(i, j) * texelSize).r; //Create a percentage closer filtering value based on the shadow map
			if((currentDepth - bias) > pcf) //If the depth of the fragment - the bias is larger than the percentage closer filtering value then add 1 to shadow
				shadow += 1;
		}
	}
	shadow = shadow / 81; //Divide the shadow by 64, as the texel is 9 * 9.
	//I have chosen 9*9 because it provides really smooth shadows. Although this will be heavy on the GPU, I figured it was worth it as its a noticeable difference from 7*7 but not noticeable from 11*11.
    
	//if((currentDepth - bias) > closestDepth) //Shadows applied without PCF if this is uncommented
	//	shadow = 1;
    
	if(projCoords.z > 1.0) //Doesnt do much, but for safety, if the projCoords.z > 0 then the fragment is not in shadow
		shadow = 0;
    
	return shadow; //Return the shadow value
}
