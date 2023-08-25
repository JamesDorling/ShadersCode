#version 410 core

uniform sampler2D scene;
float invert(float i);

const float near_plane = 1;
const float far_plane = 1000;

uniform int postProcess;
uniform float time;

in vec2 vUV;

out vec4 FragColor;

const float offset = 1.0 / 300.0;  //!< Offset is used in post processing that uses a kernal

void main()
{
	//ColorBuffer
	vec4 result = texture2D(scene, vUV);
	FragColor = result;
	
	// Many of the post processing methods use the following code to apply an effect. Here is how it does this.
	//(There are multiple uses for this method to demonstrate my understanding.)
	//
	//vec2 offsets[9] = vec2[]( //Firstly the offsets are calculated for where to get samples around the part being operated on
	//	vec2(-offset,  offset), // top-left
	//	vec2( 0.0f,    offset), // top-center
	//	vec2( offset,  offset), // top-right
	//	vec2(-offset,  0.0f),   // center-left
	//	vec2( 0.0f,    0.0f),   // center-center
	//	vec2( offset,  0.0f),   // center-right
	//	vec2(-offset, -offset), // bottom-left
	//	vec2( 0.0f,   -offset), // bottom-center
	//	vec2( offset, -offset)  // bottom-right    
	//	);
	//
	//Then a kernel is created. This will be used to get how much the colour values will be used around the part being operated on.
	//float kernel[9] = float[](
	//    -1, -1, -1,
	//    -1,  9, -1,
	//    -1, -1, -1
	//);
	//
	//
	//vec3 sampleTex[9];
	//for(int i = 0; i < 9; i++)
	//{
	//    sampleTex[i] = vec3(texture(scene, vUV + offsets[i])); //Get a sampleTexture based on the offsets
	//}
	//vec3 col = vec3(0.0);
	//for(int i = 0; i < 9; i++)
	//    col += sampleTex[i] * kernel[i]; //Multiply the sampletexture's value by the kernal's value and use that as the colour.
	//
	//FragColor = vec4(col, 1.0); //Set the colour

	if(postProcess == 1)
	{
		float avg = (result.x + result.y + result.z) / 3; //!< Calculate the average of the colour and divide it by 3, to get 1 colour
		FragColor = vec4(vec3(avg), 1.0); //!< give that colour as the r,g and b values
	}
	else if(postProcess == 2)
	{
		FragColor = vec4(invert(FragColor.r), invert(FragColor.g), invert(FragColor.b), FragColor.w); //!< Invert the colours
	}
	else if(postProcess == 3)
	{
		vec2 offsets[9] = vec2[](
			vec2(-offset,  offset), // top-left
			vec2( 0.0f,    offset), // top-center
			vec2( offset,  offset), // top-right
			vec2(-offset,  0.0f),   // center-left
			vec2( 0.0f,    0.0f),   // center-center
			vec2( offset,  0.0f),   // center-right
			vec2(-offset, -offset), // bottom-left
			vec2( 0.0f,   -offset), // bottom-center
			vec2( offset, -offset)  // bottom-right    
			);
	
		float kernel[9] = float[](
		    -1, -1, -1,
		    -1,  9, -1,
		    -1, -1, -1
		);
		

		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
		    sampleTex[i] = vec3(texture(scene, vUV + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
		    col += sampleTex[i] * kernel[i];
		
		FragColor = vec4(col, 1.0); 
	}
	else if(postProcess == 4)
	{
		vec2 offsets[9] = vec2[](
			vec2(-offset,  offset), // top-left
			vec2( 0.0f,    offset), // top-center
			vec2( offset,  offset), // top-right
			vec2(-offset,  0.0f),   // center-left
			vec2( 0.0f,    0.0f),   // center-center
			vec2( offset,  0.0f),   // center-right
			vec2(-offset, -offset), // bottom-left
			vec2( 0.0f,   -offset), // bottom-center
			vec2( offset, -offset)  // bottom-right    
			);
	
		float kernel[9] = float[](
			1.0 / 16, 2.0 / 16, 1.0 / 16,
			2.0 / 16, 4.0 / 16, 2.0 / 16,
			1.0 / 16, 2.0 / 16, 1.0 / 16  
			);
		
		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
		    sampleTex[i] = vec3(texture(scene, vUV + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
		    col += sampleTex[i] * kernel[i];
		
		FragColor = vec4(col, 1.0); 
	}
	else if(postProcess == 5)
	{
		float strength = 0.01;
		float diss = 1;
		diss += (strength * cos(time * 15)) * 250;
		vec2 offsets[9] = vec2[](
			vec2(-offset * diss,  offset * diss), // top-left
			vec2( 0.0f,        offset * diss), // top-center
			vec2( offset * diss,  offset * diss), // top-right
			vec2(-offset * diss,  0.0f),   // center-left
			vec2( 0.0f,        0.0f),   // center-center
			vec2( offset * diss,  0.0f),   // center-right
			vec2(-offset * diss, -offset * diss), // bottom-left
			vec2( 0.0f,       -offset * diss), // bottom-center
			vec2( offset * diss, -offset * diss)  // bottom-right    
			);

		float c = 1.0 / 16 + (cos(time * 15) * strength);
		float s = 2.0 / 16 + (cos(time * 10) * strength);
		float m = 4.0 / 16;
		float kernel[9] = float[](
			c, s, c,
			s, m, s,
			c, s, c  
			);
		
		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
		    sampleTex[i] = vec3(texture(scene, vUV + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
		    col += sampleTex[i] * kernel[i];
		
		FragColor = vec4(col, 1.0); 
	}
	else
	{
		FragColor = result; //If no post processing then just pass through the colour
	}
	//FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

float invert(float i)
{
	return (i - 1) * -1; //The colour value will be between 0 and 1, so this equation will invert that. (8 will become 2, 3 will become 7)
}