#version 430 core
 
in vec3 vertex;
in vec3 normal;
in vec4 ka;
in vec4 kd;
  
layout (location = 0) out vec4 outcolor;

uniform int shadingType;

void main(void)
{
	float lampKa, lampKd, lampKs;
	float matKa, matKd, matKs;
	lampKa = 0.7;
	lampKd = 0.9;
	lampKs = 0.4;
	matKa = 0.9;
	matKd = 0.9;
	matKs = 0.5;
	vec3 lampPos = vec3(0.0f, 10.0f, 30.0f);
	vec3 eyepos = vec3(0.0f, 0.0f, 50.0f);
	vec3 L = lampPos - vertex;
	L = normalize(L);
	
	vec4 phongColor;

	//if(shadingType == 0){	
		//phong shading
		vec4 ambient = lampKa * matKa * kd;
		float diffuseCoef = max(0, dot(L, normal));
		vec4 diffuse = lampKd * matKd * diffuseCoef * kd;
		
		vec3 R = normalize(L + normal * 2 * max(0, dot(L, normal)));
		vec3 StoE = normalize(eyepos - vertex);
		vec4 specular = vec4(1, 1, 1, 1) * lampKs * matKs * pow(max(dot(StoE, R), 0), 3.0);
		
		phongColor = ambient + diffuse + specular;
	//}
	if(shadingType == 1){
		//toon shading
		float intensity = dot(L, normal);
		if(intensity > 0.9)
			outcolor = 1.0 * phongColor;
		else if(intensity > 0.7)
			outcolor = 0.7 * phongColor;
		else if(intensity > 0.5)
			outcolor = 0.5 * phongColor;
		else if(intensity > 0.3)
			outcolor = 0.3 * phongColor;
		else
			outcolor = vec4(0, 0, 0, 0);
	}
	else{
		outcolor = phongColor;
	}
}