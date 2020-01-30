#version 330

in vec3 Color;

uniform vec3 SolidColor = vec3(-1,-1,-1);
uniform bool compOn = false;

out vec4 Fragment;

void main()
{
	vec3 currColor = Color;
	
	if(SolidColor.r != -1.0 && SolidColor.g != -1.0 && SolidColor.b != -1.0)
	{
		currColor = SolidColor;
	}
		
	if (compOn == true)
	{
		currColor = vec3(1.0f, 1.0f, 1.0f) - currColor;
	}

	Fragment = vec4(currColor, 1);
	
	return;
}