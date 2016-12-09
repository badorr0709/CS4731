// #version 150
//
// in  vec4 vPosition;
// in  vec4 vTexCoord;
// out vec2 texCoord;
//
// uniform mat4 Projection;
//
// void main()
// {
//     // We only need the s and t values of the texture coordinate
//     texCoord = vTexCoord.st;
//     gl_Position = Projection * vPosition;
// }

#version 150

in  vec4 vPosition;
in  vec4 vNormal;
in  vec4 vTexCoord;
out vec2 texCoord;
flat out vec4 flatColor;
out vec3 pos;
out vec3 fL;
out vec3 fN;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform vec4 ambient, diffuse, specular;
uniform vec4 lightPos;
uniform vec4 lightDir;
uniform float lightFalloff;
uniform float lightCutoff;
uniform float shininess;

void main()
{
	//Calculate needed vectors (Need for both flat and smooth shading)
	pos = (modelMatrix * vPosition).xyz;
	fL = normalize(lightPos.xyz - pos);
	vec3 fE = normalize(-pos);
	vec3 fH = normalize(fL + fE);
	fN = normalize(modelMatrix * vNormal).xyz;

	texCoord = vTexCoord.st;
	gl_Position = projectionMatrix * modelMatrix * vPosition;
}
