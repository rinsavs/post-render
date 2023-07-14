#version 430 core
        
in vec2 TextureCoordinate;
layout (location = 0) out vec4 outcolor;

uniform float time;
uniform int shaderType;
uniform sampler2D Texture;

float intensity(vec4 color)
{
	return sqrt((color.x*color.x)+(color.y*color.y)+(color.z*color.z));
}

vec4 sobel(float step, vec2 center)
{
	//change to uniform later
	float limit = 0.1;
	
	// get samples around pixel
    float tleft = intensity(texture2D(Texture,center + vec2(-step,step)));
    float left = intensity(texture2D(Texture,center + vec2(-step,0)));
    float bleft = intensity(texture2D(Texture,center + vec2(-step,-step)));
    float top = intensity(texture2D(Texture,center + vec2(0,step)));
    float bottom = intensity(texture2D(Texture,center + vec2(0,-step)));
    float tright = intensity(texture2D(Texture,center + vec2(step,step)));
    float right = intensity(texture2D(Texture,center + vec2(step,0)));
    float bright = intensity(texture2D(Texture,center + vec2(step,-step)));
	
	// Sobel masks (to estimate gradient)
	//        1 0 -1     -1 -2 -1
	//    X = 2 0 -2  Y = 0  0  0
	//        1 0 -1      1  2  1

	float x = tleft + 2.0*left + bleft - tright - 2.0*right - bright;
	float y = -tleft - 2.0*top - tright + bleft + 2.0 * bottom + bright;
    float color = sqrt((x*x) + (y*y));
	
	vec4 texColor = texture2D(Texture, TextureCoordinate);
	vec4 texColorLeft = texture2D(Texture, TextureCoordinate + vec2(-step, 0));
	vec4 texColorRight = texture2D(Texture, TextureCoordinate + vec2(step, 0));
	vec4 meanColor = vec4(texColorLeft.xyz * 0.5 + texColorRight.xyz * 0.5, 1.0f);
	if (color > limit){return vec4(texture2D(Texture, TextureCoordinate).xyz - vec3(30.0/255.0, 40.0/255.0, 50.0/255.0), 0.2f);}
    //return vec4(1.0,1.0,1.0,1.0);
	return texColor;
}

void main(void)
{
	float PI = 3.1415926535;
	//vec2 frag_uv = gl_FragCoord.xy / vec2(500,500);
	if(shaderType == 2){	//Blur
		float blurSizeH = 1.0 / 500.0;
		float blurSizeV = 1.0 / 500.0;
		vec4 sum = vec4(0.0);
		for (int x = -4; x <= 4; x++)
			for (int y = -4; y <= 4; y++)
				sum += texture2D(
					Texture,
					vec2(TextureCoordinate.x + x * blurSizeH, TextureCoordinate.y + y * blurSizeV)
				) / 81.0;
		outcolor = sum;
	}
	else if(shaderType == 3){	//Fish eye
		float aperture = 150.0;
		float apertureHalf = 0.5 * aperture * (PI / 180.0);
		float maxFactor = sin(apertureHalf);
		
		vec2 uv;
		vec2 xy = 2.0 * TextureCoordinate.xy - 1.0;
		float d = length(xy);
		if (d < (2.0-maxFactor))
		{
			d = length(xy * maxFactor);
			float z = sqrt(1.0 - d * d);
			float r = atan(d, z) / PI;
			float phi = atan(xy.y, xy.x);
			
			uv.x = r * cos(phi) + 0.5;
			uv.y = r * sin(phi) + 0.5;
		}
		else
		{
			uv = TextureCoordinate;
		}
		outcolor = texture2D(Texture, uv);
	}
	else if(shaderType == 4){	//pixelate
		float sides = 500.0f;
		float dx = 5.0 * (1.0 / sides);
        float dy = 5.0 * (1.0 / sides);
        vec2 Coord = vec2(dx * floor(TextureCoordinate.x / dx),
                          dy * floor(TextureCoordinate.y / dy));
        outcolor = texture2D(Texture, Coord);
	}
	else if(shaderType == 5){
		float wavyAmplitude = 1;
		float wavyWaveLength = 0.03;
		float wavyPhase = 0;
		float wavyAngle = 0;
		float rad = mod(time, 1.0) * 200.0 / 500.0;
		vec2 center = vec2(0.5, 0.5);
	
		float v = dot(TextureCoordinate, vec2(sin(wavyAngle), -cos(wavyAngle)));
		float f = wavyAmplitude * sin(wavyWaveLength * v + wavyPhase);
		vec2 newCoord = TextureCoordinate + f * vec2(cos(wavyAngle), sin(wavyAngle));
	
		int i = 0;
		for(i = 0; i < 3; i++){
			if(length(newCoord - center) < rad)
				newCoord = newCoord;
			else{
				float dist = length(newCoord - center);
				float coeff = sqrt(1 - (rad * rad) / (dist * dist));
				newCoord = center + (newCoord - center) * coeff;
			}
		}
		outcolor = texture2D(Texture, newCoord);
	}
	else if(shaderType == 6){
		float combOmega = 80;
		float combLambda = mod(time, 1.0) * 0.03;
		float combSpacing = 40.0 / 500.0;
		
		vec2 combDir;
		if(mod(time * 1000 / 1000.0f, 2.0f) < 1.0)
			combDir = vec2(1.0 / 500.0, 0.0);
		else
			combDir = vec2(-1.0 / 500.0, 0.0);
		vec2 combStart = vec2(0.0, 0.0);
		vec2 combNormal = normalize(vec2(-combDir[1], combDir[0]));
		
		vec2 pointDiff = TextureCoordinate - combStart;
		float d = abs(pointDiff[0] * combNormal[0] + pointDiff[1] * combNormal[1]);
		float d2 = combSpacing / 2.0 - abs(mod(d, combSpacing) - combSpacing / 2.0);
		vec2 newCoord = TextureCoordinate - ((combOmega * combLambda) / (d2 + combLambda)) * combDir;
		
		outcolor = texture2D(Texture, newCoord);
	}
	else if(shaderType == 7){
		vec2 circleCenter = vec2(0.5, 0.5);
		
		float circleRadius = 300.0 / 500.0;
		float circleOmega = 0.7;
		float circleLambda = time * 0.3;
		
		float d = abs(length(TextureCoordinate - circleCenter) - circleRadius);
		float l = (circleOmega * circleLambda) / (d + circleLambda);
		float theta = l / length(TextureCoordinate - circleCenter);
		mat2 rotMat = mat2(cos(theta), sin(theta), -sin(theta), cos(theta));
		vec2 newCoord = circleCenter + (TextureCoordinate - circleCenter) * rotMat;
		
		outcolor = texture2D(Texture, newCoord);
	}
	else if(shaderType == 8){
		vec2 vortexCenter = vec2(0.5, 0.5);
		
		float vortexOmega = 0.7;
		float vortexLambda = time * 0.3;
	
		float d = length(TextureCoordinate - vortexCenter);
		float l = (vortexOmega * vortexLambda) / (d + vortexLambda);
		float theta = l / length(TextureCoordinate - vortexCenter);
		mat2 rotMat = mat2(cos(theta), sin(theta), -sin(theta), cos(theta));
		vec2 newCoord = vortexCenter + (TextureCoordinate - vortexCenter) * rotMat;
		
		outcolor = texture2D(Texture, newCoord);
	}
	else if(shaderType == 9){
		float step = 1.0/500.0;	//later change into 1 / width <uniform>
		vec2 center = TextureCoordinate;
		outcolor = sobel(step,center);
	}
	else if(shaderType == 10){
		float wavyAmplitude = 1;
		float wavyWaveLength = 0.03;
		float wavyPhase = 0;
		float wavyAngle = 0;
		float rad = mod(time, 1.0) * 200.0 / 500.0;
		vec2 center = vec2(0.5, 0.5);
	
		float v = dot(TextureCoordinate, vec2(sin(wavyAngle), -cos(wavyAngle)));
		float f = wavyAmplitude * sin(wavyWaveLength * v + wavyPhase);
		vec2 newCoord = TextureCoordinate + f * vec2(cos(wavyAngle), sin(wavyAngle));
	
		int i = 0;
		for(i = 0; i < 5; i++){
			if(length(newCoord - center) < rad)
				newCoord = newCoord;
			else{
				float dist = length(newCoord - center);
				float coeff = sqrt(1 - (rad * rad));
				newCoord = center + (newCoord - center) * coeff;
			}
		}
		outcolor = texture2D(Texture, newCoord);
	}
	else if(shaderType == 11){
		vec2 newCoord = vec2(TextureCoordinate.x / 0.5f, TextureCoordinate.y / 0.5f);
		if(TextureCoordinate.x > 0.5)
			newCoord.x -= 1.0;
		if(TextureCoordinate.y > 0.5)
			newCoord.y -= 1.0;
		outcolor = texture2D(Texture, newCoord);
	}
	else{
		outcolor = texture2D(Texture, TextureCoordinate);
	}
}