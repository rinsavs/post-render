#version 430 core
      
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Ka;
layout (location = 3) in vec3 Kd;


uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 vertex;
out vec3 normal;
out vec4 ka;
out vec4 kd;

void main(void)
{
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
	vertex = Position;
	normal = normalize(Normal);
	ka = vec4(Ka, 1.0f);
	kd = vec4(Kd, 1.0f);
}