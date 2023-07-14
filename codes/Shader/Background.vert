#version 430 core
      
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTextureCoordinate;

out vec2 TextureCoordinate;

void main(void)
{
	gl_Position = vec4(inPosition.xyz, 1.0);
	TextureCoordinate = inTextureCoordinate;
}