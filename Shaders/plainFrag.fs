#version 410 core

out vec4 FragColor;

uniform vec3 viewPos; //View position

in VS_OUT
{
	vec3 vNormal;
	vec2 vUV;
	vec3 FragPos;
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

void main()
{    
	vec3	ambient = light.ambient * vec3(texture(texture_diffuse1, fs_in.vUV)); //Define the ambient light as the light.ambient value value multiplied by the texture's diffuse value

	vec3    lightDir = normalize(light.position - fs_in.FragPos); //Get the light position by normalising the Light position tangent - the fragment position tangent)
	vec3	normal = normalize(fs_in.vNormal); //Get the normal by normalising the fragment normal
	
	float	diffFactor = max(dot(lightDir, normal), 0.0); //Define the diffuse factor by taking the dot product of the light direction and the normal, and clamping it
	vec3	diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse1, fs_in.vUV)); //Define the diffuse by taking the diffuse factor multiplied by the light.diffuse value and the texture's diffuse values

	vec3    viewDir = normalize(viewPos - fs_in.FragPos); //Normalise the tangent of the view position minus the fragment position for the view position
	float	specFactor = 0.0; //define the specfactor as a float
	vec3	halfDir = normalize(lightDir + viewDir); //Normalise the lightdirection and the viewdirection for the specular
	specFactor = pow(max(dot(halfDir, normal), 0.0), 64.0); //Set the specular factor (Dot pruduct of the halfDir and the normal, clamped and to the power of 64
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular1, fs_in.vUV)); //Set the specular light via the specular factor, the light.specular value and the texture

	vec3	result = (ambient + diffuse + specular ); //Resulting blinn-phong shader is the ambient plus the diffuse plus the specular
	FragColor	= vec4(result , 1.0f); //Set the colour to the result
	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}

