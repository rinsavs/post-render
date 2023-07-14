#version 430 core
      
layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 inTextureCoordinate;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec2 TextureCoordinate;

void main(void)
{
	gl_Position = vec4(Position, 1.0);
	TextureCoordinate = vec2(1.0 - inTextureCoordinate.x, 1.0 - inTextureCoordinate.y);
}