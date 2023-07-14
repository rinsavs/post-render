#version 430 core
        
in vec2 TextureCoordinate;
layout (location = 0) out vec4 outcolor;

uniform sampler2D Texture;

void main(void)
{
  outcolor = texture2D(Texture,TextureCoordinate);
}