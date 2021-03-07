#version 410 core

out vec4 FragColor;

float calcShadow(vec4 fragPosLightSpace);

uniform vec3 viewPos; //View position

in GS_OUT
{
	vec2 gUV;
	vec3 gFragPos;
	vec3 gNormal;
	float gHeight;
	float vis;
	float dist;
}fs_in;

struct LightAttr
{
	vec3 position; //Light position
	vec3 ambient; //Ambient light values
	vec3 diffuse; //Diffuse light values
	vec3 specular; //Specular light values
};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

uniform LightAttr light; //Uniform light attribute

uniform vec4 sky;
uniform int scale;

uniform int terrMode;

uniform bool fog;
uniform bool colorMode;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform bool showShadow;

vec4 gray = vec4(0.2, 0.2, 0.2, 0.0);
vec4 white = vec4(1.0, 1.0, 1.0, 0.0);
vec4 green = vec4(0.1, 0.3, 0.1, 0.0);

vec3 ambient;
vec3 diffuse;
vec3 specular;

void main()
{    
	//vec3	ambient = light.ambient * vec3(texture(texture_diffuse1, fs_in.gUV)); //Define the ambient light as the light.ambient value value multiplied by the texture's diffuse value
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fs_in.gFragPos, 1.0);

	vec3    lightDir = normalize(light.position - fs_in.gFragPos); //Get the light position by normalising the Light position tangent - the fragment position tangent)
	vec3	normal = normalize(fs_in.gNormal); //Get the normal by normalising the fragment normal
	
	float	diffFactor = max(dot(lightDir, normal), 0.0); //Define the diffuse factor by taking the dot product of the light direction and the normal, and clamping it
	//vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse1, fs_in.gUV)); //Define the diffuse by taking the diffuse factor multiplied by the light.diffuse value and the texture's diffuse values
	
	

	//ambient = light.ambient * vec3(mix(vec4(height, 0.0, 0.0, 0.0), gray, smoothstep(0.3, 1.0, height)).rgb);
	//diffuse = diffFactor * light.diffuse * vec3(mix(vec4(height, 0.0, 0.0, 0.0), gray, smoothstep(0.3, 1.0, height)).rgb);
	
	vec3    viewDir = normalize(viewPos - fs_in.gFragPos); //Normalise the tangent of the view position minus the fragment position for the view position
	float	specFactor = 0.0; //define the specfactor as a float
	vec3	halfDir = normalize(lightDir + viewDir); //Normalise the lightdirection and the viewdirection for the specular
	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0); //Set the specular factor (Dot pruduct of the halfDir and the normal, clamped and to the power of 64
	if(terrMode != 1)
	{
		if (colorMode == false)
		{
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
		{
			if(fs_in.gHeight > 0.2 && fs_in.gNormal.y > 0.7)
			{
				
				ambient = light.ambient * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
				diffuse = diffFactor * light.diffuse * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
				specular = specFactor * light.specular * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
				
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
	else if(terrMode == 1)
	{
		//if(fs_in.gHeight > 6)
		//{
		//	ambient = light.ambient * vec3(white);
		//	diffuse = diffFactor * light.diffuse * vec3(white);
		//	specular = specFactor * light.specular * vec3(white);
		//}
		//else if (fs_in.gHeight > 5)
		//{
		//	ambient = light.ambient * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
		//	diffuse = diffFactor * light.diffuse * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
		//	specular = specFactor * light.specular * vec3(mix(gray, white, smoothstep(0.2, 1.0, fs_in.gHeight)).rgb);
		//}
		//if (fs_in.gNormal.y <= 0.2 || fs_in.gHeight <= 10)
		//{
		//	ambient = light.ambient * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
		//	diffuse = diffFactor * light.diffuse * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
		//	specular = specFactor * light.specular * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
		//}
		ambient = light.ambient * vec3(green);
		diffuse = diffFactor * light.diffuse * vec3(green);
		specular = specFactor * light.specular * vec3(green);
	}

	float shadow;

	vec3 result;
	//if(showShadow == false)
	//{
		shadow = 0;
		result = (ambient + diffuse + specular); //Resulting blinn-phong shader is the ambient plus the diffuse plus the specular
	//}
	//else
	//{
	//	shadow = calcShadow(fragPosLightSpace);
	//	result = (ambient + (1.0 - shadow) * ( diffuse + specular));
	//}

	//vec3	result = (ambient + (1.0 - shadow) * ( diffuse + specular));
	//vec3	result = (ambient + diffuse + specular); //Resulting blinn-phong shader is the ambient plus the diffuse plus the specular
	//result = (ambient + diffuse + specular);
	FragColor = vec4(result, 1.0);

	

	//ambient = light.ambient * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
	//diffuse = diffFactor * light.diffuse * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
	//specular = specFactor * light.specular * vec3(mix(gray, green, smoothstep(0.2, 0.001, fs_in.gHeight)).rgb);
	//vec3 result = vec3(green);
	//
	//

	//

	//FragColor = mix(FragColor, sky, fs_in.vis);
	if(fog == true)
	{
		FragColor = mix(sky, FragColor, fs_in.vis);
	}
	//FragColor = vec4(mix(sky, FragColor, -fs_in.vis).rgb, 1.0);
	//FragColor = vec4(fs_in.vis * 255, fs_in.vis * 255, fs_in.vis * 255, 1.0);

	//FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}

float calcShadow(vec4 fragPosLightSpace)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z;

	float shadow = 0;

	float bias = 0.09;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int i = -3; i < 2; i++)
	{
		for(int j = -3; j < 2; j++)
		{
			float pcf = texture(shadowMap, projCoords.xy + vec2(i, j) * texelSize).r;
			if(currentDepth - bias > pcf)
				shadow += 1;
		}
	}
	shadow = shadow / 9;

	if((currentDepth - bias) > closestDepth)
		shadow = 1;

	if(projCoords.z > 1.0)
		shadow - 0;

	return shadow;
}
