// #version 150
//
// in  vec2 texCoord;
// out vec4 fColor;
//
// uniform sampler2D texture;
//
// void main()
// {
//     fColor = texture2D( texture, texCoord );
// }

#version 150

flat in vec4 flatColor;

in vec3 pos;
in vec3 fL;
in vec3 fE;
in vec3 fN;
in  vec2 texCoord;

out vec4 fColor;

uniform bool useTexture;
uniform vec4 overrideColor;
uniform sampler2D texture;

uniform vec4 ambient, diffuse, specular;
uniform vec4 lightPos;
uniform vec4 lightDir;
uniform float lightFalloff;
uniform float lightCutoff;
uniform float shininess;
uniform mat4 modelMatrix;

void main()
{
	vec4 diffuseToUse;
	if(useTexture) {
		diffuseToUse = texture2D(texture, texCoord);
	} else {
		diffuseToUse = diffuse;
	}

	vec3 N = normalize(fN);
	vec3 E = normalize(-pos);
	vec3 L = normalize(fL);
	vec3 H = normalize(L + E);

	//Find intensity
	float lightPosAngle = acos(dot(L, normalize(lightDir.xyz)));
	float intensity = 0;
	if(lightPosAngle < lightCutoff) {
		intensity = pow(cos(lightPosAngle), lightFalloff) * 0.5;
	}

	//Calculate phong shaded color terms
	vec4 sAmbient = ambient * intensity;

	float cosTheta = max(dot(L, N), 0.0);
	vec4 sDiffuse = cosTheta * diffuseToUse * intensity * 2;

	float cosPhi = pow(max(dot(N, H), 0.0), shininess);
	vec4 sSpecular = cosPhi * specular * intensity;
	if(dot(L, N) < 0.0) {
		sSpecular = vec4(0.0, 0.0, 0.0, 0.0);
	}

	//Build phong shaded color
	fColor = sAmbient + sDiffuse + sSpecular;
	fColor.a = 1.0;

	if(length(overrideColor) > 0.0) {
		fColor = overrideColor;
	}
}
