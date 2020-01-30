#version 330

in vec3 Color;

uniform vec3 SolidColor = vec3(-1,-1,-1);
uniform bool isComp = false;

out vec4 Fragment;

void main()
{
	vec3 finalColor = Color;
	
	if(SolidColor.r != -1.0 && SolidColor.g != -1.0 && SolidColor.b != -1.0)
	{
		finalColor = SolidColor;
	}
		
	if (isComp == true)
	{
		finalColor = vec3(1.0f, 1.0f, 1.0f) - finalColor;
	}

	Fragment = vec4(finalColor, 1);
	
	return;
}