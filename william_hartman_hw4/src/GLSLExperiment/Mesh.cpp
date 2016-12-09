// WPI CS4731 - Computer Graphics - Homework 4
// William Hartman

#include <limits.h>
#include "Mesh.h"
#include "textfile.h"
#include "bmpread.h"

//Constructor, destructor
Mesh::Mesh(int polyCount, int vertCount) {
	position = vec3(0, 0, 0);
	rotationAboutPosition = vec3(0, 0, 0);
	scale = vec3(1, 1, 1);
	color = vec4(1, 1, 1, 1);

	polyListSize = polyCount;
	numPolys = 0;
	polys = new Face*[polyCount];

	vertsListSize = vertCount;
	numVerts = 0;
	verts = new point4*[vertCount];

	hasTexture = false;
}
Mesh::~Mesh() {
	for (int i = 0; i < numPolys; i++) { delete polys[i]; }
	for (int i = 0; i < numVerts; i++) { 
		delete verts[i]->point;
		delete verts[i]->normal;
		delete verts[i]; 
	}
	delete polys;
	delete verts;
}

//Methods for building meshes
void Mesh::addVertex(float x, float y, float z) {
	addVertex(x, y, z, 0, 0, 0);
}


void Mesh::addVertex(float x, float y, float z, float u, float v, float w) {
	if (numVerts >= vertsListSize) {
		printf("Failed to add point (%f, %f, %f) - verts list is full!\n", x, y, z);
		return;
	}
	point4* toAdd = new point4();
	toAdd->point = new vec4(x, y, z, 1);
	toAdd->normal = new vec4(0, 0, 0, 1);
	toAdd->texCoord = new vec4(u, v, w, 1);

	verts[numVerts] = toAdd;
	numVerts++;
}

void Mesh::addPoly(int vertIndex1, int vertIndex2, int vertIndex3) {
	if (numPolys >= polyListSize) {
		printf("Failed to add poly with verts %d, %d, %d - poly list is full!\n", vertIndex1, vertIndex2, vertIndex3);
		return;
	}

	Face* toAdd = new Face();
	toAdd->p1 = verts[vertIndex1];
	toAdd->p2 = verts[vertIndex2];
	toAdd->p3 = verts[vertIndex3];
	polys[numPolys] = toAdd;
	numPolys++;
}
void Mesh::normalize() {
	//Center and scale the mesh
	float meshWidth = getWidth();
	float meshHeight = getHeight();
	float meshDepth = getDepth();
	float biggestDimension = fmaxf(meshWidth, fmaxf(meshHeight, meshDepth));
	float scaleFactor = 1.0f / biggestDimension;

	mat4 correctionTransform = Angel::identity();
	mat4 moveToOrigin = Angel::Translate(getCenterPosition() * -1);
	mat4 normalizeScale = Angel::Scale(scaleFactor, scaleFactor, scaleFactor);
	correctionTransform = correctionTransform * normalizeScale;
	for (int i = 0; i < numVerts; i++) {
		vec4 temp = correctionTransform * *(verts[i]->point);
		verts[i]->point->x = temp.x;
		verts[i]->point->y = temp.y;
		verts[i]->point->z = temp.z;
	}
}
void Mesh::buildNormals() {
	for (int i = 0; i < numVerts; i++) {
		vec4 pointNormal = vec4(0, 0, 0, 1);
		for (int j = 0; j < numPolys; j++) {
			if (polys[j]->p1 == verts[i] || polys[j]->p2 == verts[i] || polys[j]->p3 == verts[i]) {
				pointNormal += calcNormal(polys[j]);
			}
		}

		vec4 normalized = Angel::normalize(pointNormal);
		normalized.w = 0;
		verts[i]->normal = new vec4(normalized);
	}
}
void Mesh::setColor(vec4 newColor) {
	color = newColor;
}
void Mesh::setTexture(const char* path) {
	bmpread_t bitmap;
	if (!bmpread(path, 0, &bitmap)) {
		fprintf(stderr, "%s:error loading bitmap file\n", path);
		exit(1);
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	bmpread_free(&bitmap);
	hasTexture = true;
}
void Mesh::drawWithTexture(bool shouldUseTexture) {
	hasTexture = shouldUseTexture;
}



//Methods for getting information about meshes
float Mesh::getWidth(mat4 CTM) {
	auto getXLambda = [mat = CTM](point4* point) -> float { return (mat * *(point->point)).x; };
	return getMax(getXLambda) - getMin(getXLambda);
}
float Mesh::getHeight(mat4 CTM) {
	auto getYLambda = [mat = CTM](point4* point) -> float { return (mat * *(point->point)).y; };
	return getMax(getYLambda) - getMin(getYLambda);
}
float Mesh::getDepth(mat4 CTM) {
	auto getZLambda = [mat = CTM](point4* point) -> float { return (mat * *(point->point)).z; };
	return getMax(getZLambda) - getMin(getZLambda);
}
vec3 Mesh::getCenterPosition(mat4 CTM) {
	auto getXLambda = [](point4* point) -> float { return point->point->x; };
	auto getYLambda = [](point4* point) -> float { return point->point->y; };
	auto getZLambda = [](point4* point) -> float { return point->point->z; };

	float xCenter = (getMin(getXLambda) + getMax(getXLambda)) / 2.0f;
	float yCenter = (getMin(getYLambda) + getMax(getYLambda)) / 2.0f;
	float zCenter = (getMin(getZLambda) + getMax(getZLambda)) / 2.0f;

	vec4 transformedPos = CTM * vec3(xCenter, yCenter, zCenter);
	return vec3(transformedPos.x, transformedPos.y, transformedPos.z);
}

//Methods for drawing
void Mesh::drawMesh(int program, Spotlight* light) {
	//Build a buffer of all points in the model
	int bufferSize = 3 * 3 * numPolys;
	vec4* points = new vec4[bufferSize];
	int pointsIndex = 0;
	for (int i = 0; i < numPolys; i++) {
		points[pointsIndex] = *(polys[i]->p1->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p1->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p1->texCoord); pointsIndex++;

		points[pointsIndex] = *(polys[i]->p2->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p2->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p2->texCoord); pointsIndex++;

		points[pointsIndex] = *(polys[i]->p3->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p3->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p3->texCoord); pointsIndex++;
	}

	//Send the data to GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * bufferSize, points, GL_STATIC_DRAW);
	delete points;

	//Clear global color
	GLuint vColor = glGetUniformLocationARB(program, "overrideColor");
	glUniform4f(vColor, 0, 0, 0, 0);

	//Set up color
	GLuint diffuse = glGetUniformLocationARB(program, "diffuse");
	glUniform4f(diffuse, color.x, color.y, color.z, color.w);

	//Set up lighting
	GLuint lightFalloff = glGetUniformLocationARB(program, "lightFalloff");
	glUniform1f(lightFalloff, 20.0f / (light->getCutoff() / 45.0f));

	GLuint lightCutoff = glGetUniformLocationARB(program, "lightCutoff");
	glUniform1f(lightCutoff, light->getCutoff() * Angel::DegreesToRadians);

	// Set up textures
	GLuint useTexture = glGetUniformLocationARB(program, "useTexture");
	glUniform1i(useTexture, hasTexture ? GL_TRUE : GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, texture);
	GLuint texture = glGetUniformLocation(program, "texture");
	glUniform1i(texture, 0);

	//Draw the PLY model
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numPolys);
	glDisable(GL_DEPTH_TEST);
}
void Mesh::drawShadows(int program, Spotlight* light, vec4 planeNormal, mat4 modelView) {
	//Shadow matrix
	vec3 offsetPos = vec3(light->getPosition().x, light->getPosition().y + 1, light->getPosition().z);

	mat4 shadowMat = Angel::identity();
	shadowMat[3][1] = -1.0f / offsetPos.y;
	shadowMat[3][3] = 0;

	mat4 rot = Angel::RotateY(45) * Angel::RotateZ(90);
	mat4 negrot = Angel::RotateY(-45) * Angel::RotateZ(-90);

	mat4 shadowProjMat = Angel::RotateY(90) * rot * Angel::Translate(light->getPosition()) * shadowMat * Angel::RotateY(45) * Angel::Translate(-light->getPosition()) * negrot * modelView;
	//mat4 shadowProjMat = Angel::Translate(0, -0.5, 0) * Angel::Translate(light->getPosition()) * shadowMat * Angel::Translate(-light->getPosition()) * modelView *  Angel::Translate(0, 0.5, 0);
	//mat4 shadowProjMat = Angel::RotateY(-45) * Angel::RotateX(-90) * Angel::Translate(light->getPosition()) * shadowMat * Angel::Translate(-light->getPosition()) * Angel::RotateY(45) * Angel::RotateX(90) * modelView;

	////Shadow Matrix adapted from: https://www.opengl.org/archives/resources/features/StencilTalk/tsld021.htm
	//mat4 shadowMat = Angel::identity();
	//float planeDotLight = Angel::dot(planeNormal, light->getPosition());

	//shadowMat[0][0] = planeDotLight - light->getPosition().x * planeNormal.x;
	//shadowMat[0][1] =               - light->getPosition().x * planeNormal.y;
	//shadowMat[0][2] =               - light->getPosition().x * planeNormal.z;
	//shadowMat[0][3] =               - light->getPosition().x * planeNormal.w;

	//shadowMat[1][0] =               - light->getPosition().y * planeNormal.x;
	//shadowMat[1][1] = planeDotLight - light->getPosition().y * planeNormal.y;
	//shadowMat[1][2] =               - light->getPosition().y * planeNormal.z;
	//shadowMat[1][3] =               - light->getPosition().y * planeNormal.w;

	//shadowMat[2][0] =               - light->getPosition().z * planeNormal.x;
	//shadowMat[2][1] =               - light->getPosition().z * planeNormal.y;
	//shadowMat[2][2] = planeDotLight - light->getPosition().z * planeNormal.z;
	//shadowMat[2][3] =               - light->getPosition().z * planeNormal.w;

	//shadowMat[3][0] =               - light->getPosition().w * planeNormal.x;
	//shadowMat[3][1] =               - light->getPosition().w * planeNormal.y;
	//shadowMat[3][2] =               - light->getPosition().w * planeNormal.z;
	//shadowMat[3][3] = planeDotLight - light->getPosition().w * planeNormal.w;

	//mat4 shadowProjMat = shadowMat * modelView;

	float mm[16];
	mm[0] =  shadowProjMat[0][0]; mm[4] =  shadowProjMat[0][1];
	mm[1] =  shadowProjMat[1][0]; mm[5] =  shadowProjMat[1][1];
	mm[2] =  shadowProjMat[2][0]; mm[6] =  shadowProjMat[2][1];
	mm[3] =  shadowProjMat[3][0]; mm[7] =  shadowProjMat[3][1];
	mm[8] =  shadowProjMat[0][2]; mm[12] = shadowProjMat[0][3];
	mm[9] =  shadowProjMat[1][2]; mm[13] = shadowProjMat[1][3];
	mm[10] = shadowProjMat[2][2]; mm[14] = shadowProjMat[2][3];
	mm[11] = shadowProjMat[3][2]; mm[15] = shadowProjMat[3][3];
	GLuint ctmLocation = glGetUniformLocationARB(program, "modelMatrix");
	glUniformMatrix4fv(ctmLocation, 1, GL_FALSE, mm);

	//Build a buffer of all points in the model
	int bufferSize = 3 * 3 * numPolys;
	vec4* points = new vec4[bufferSize];
	int pointsIndex = 0;
	for (int i = 0; i < numPolys; i++) {
		points[pointsIndex] = *(polys[i]->p1->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p1->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p1->texCoord); pointsIndex++;

		points[pointsIndex] = *(polys[i]->p2->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p2->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p2->texCoord); pointsIndex++;

		points[pointsIndex] = *(polys[i]->p3->point);  pointsIndex++;
		points[pointsIndex] = *(polys[i]->p3->normal); pointsIndex++;
		points[pointsIndex] = *(polys[i]->p3->texCoord); pointsIndex++;
	}

	//Send the data to GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * bufferSize, points, GL_STATIC_DRAW);
	delete points;

	//Clear global color
	GLuint vColor = glGetUniformLocationARB(program, "overrideColor");
	glUniform4f(vColor, 1, 0, 0, 1);

	//Draw the shadow polygons
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numPolys);
	glDisable(GL_DEPTH_TEST);

	//Clean 
	mm[0] =  modelView[0][0]; mm[4] =  modelView[0][1];
	mm[1] =  modelView[1][0]; mm[5] =  modelView[1][1];
	mm[2] =  modelView[2][0]; mm[6] =  modelView[2][1];
	mm[3] =  modelView[3][0]; mm[7] =  modelView[3][1];
	mm[8] =  modelView[0][2]; mm[12] = modelView[0][3];
	mm[9] =  modelView[1][2]; mm[13] = modelView[1][3];
	mm[10] = modelView[2][2]; mm[14] = modelView[2][3];
	mm[11] = modelView[3][2]; mm[15] = modelView[3][3];
	glUniformMatrix4fv(ctmLocation, 1, GL_FALSE, mm);
}

//Methods for transforming meshes
void Mesh::moveTo(float x, float y, float z) {
	position = vec3(x, y, z);
}
void Mesh::moveBy(float x, float y, float z) {
	position += vec3(x, y, z);
}
void Mesh::rotateTo(float xRollAmount, float yRollAmount, float zRollAmount) {
	rotationAboutPosition = vec3(xRollAmount, yRollAmount, zRollAmount);
}
void Mesh::rotateBy(float xRollAmount, float yRollAmount, float zRollAmount) {
	rotationAboutPosition += vec3(xRollAmount, yRollAmount, zRollAmount);
}
void Mesh::scaleTo(float x, float y, float z) {
	scale = vec3(x, y, z);
}
void Mesh::scaleBy(float x) {
	scale *= x;
}
mat4 Mesh::getModelMatrix() {
	mat4 ModelMatrix = Angel::identity();
	mat4 scaleMatrix = Angel::Scale(scale);
	mat4 rotateMatrix = Angel::RotateX(rotationAboutPosition.x) * Angel::RotateY(rotationAboutPosition.y) * Angel::RotateZ(rotationAboutPosition.z);
	mat4 translateMatrix = Angel::Translate(position);

	ModelMatrix = ModelMatrix * translateMatrix * rotateMatrix * scaleMatrix;
	return ModelMatrix;
}

//Convenience stuff
vec4 Mesh::calcNormal(Face* face) {
	vec4 vectors[4];
	vectors[0] = *face->p1->point;
	vectors[1] = *face->p2->point;
	vectors[2] = *face->p3->point;
	vectors[3] = *face->p1->point;

	float mx = 0;
	float my = 0;
	float mz = 0;

	for (int i = 0; i < 3; i++) {
		mx += (vectors[i].y - vectors[i + 1].y) * (vectors[i].z + vectors[i + 1].z);
		my += (vectors[i].z - vectors[i + 1].z) * (vectors[i].x + vectors[i + 1].x);
		mz += (vectors[i].x - vectors[i + 1].x) * (vectors[i].y + vectors[i + 1].y);
	}

	vec4 normalized = vec4(Angel::normalize(vec3(mx, my, mz)), 0);
	return normalized;
}
float Mesh::getMin(std::function<float(point4*)> func) {
	float min = INFINITY;
	for (int i = 0; i < numVerts; i++) {
		if (func(verts[i]) < min) { min = func(verts[i]); }
	}
	return min;
}
float Mesh::getMax(std::function<float(point4*)> func) {
	float max = -INFINITY;
	for (int i = 0; i < numVerts; i++) {
		if (func(verts[i]) > max) { max = func(verts[i]); }
	}
	return max;
}

//Loading meshes
Mesh* loadMeshFromPLY(char* filename) {
	int success;
	char* fileContents = textFileRead(filename);

	//Tokenize on newlines
	char* token = strtok(fileContents, "\n");

	//Bail if not PLY
	if (!strcmp(token, "ply\n")) {
		printf("File \"%s\" is not a PLY file!\n", filename);
		return new Mesh(0, 0);
	}
	token = strtok(nullptr, "\n");
	token = strtok(nullptr, "\n"); //Skip ASCII line

	//Read in verts
	int numVerts;
	success = sscanf(token, "element vertex %d", &numVerts);
	if (success != 1) {
		printf("could not read number of verts from file \"%s\"\n", filename);
		return new Mesh(0, 0);
	}
	token = strtok(nullptr, "\n");
	token = strtok(nullptr, "\n"); //Skip 1st property line
	token = strtok(nullptr, "\n"); //Skip 2nd property line
	token = strtok(nullptr, "\n"); //Skip 3rd property line

	//Read in polys
	int numPolys;
	success = sscanf(token, "element face %d", &numPolys);
	if (success != 1) {
		printf("could not read number of polys from file \"%s\"\n", filename);
		return new Mesh(0, 0);
	}
	token = strtok(nullptr, "\n");
	token = strtok(nullptr, "\n"); //Skip 4th property line
	token = strtok(nullptr, "\n"); //Skip "end_header" line

	//Build the vertex list
	Mesh* mesh = new Mesh(numPolys, numVerts);

	//Read in verts
	for (int i = 0; i < numVerts; i++) {
		float x, y, z;
		success = sscanf(token, "%f %f %f", &x, &y, &z);
		if (success != 3) {
			printf("could not read vert #%d from file \"%s\"\n", i, filename);
			return new Mesh(0, 0);
		}
		mesh->addVertex(x, y, z);
		token = strtok(nullptr, "\n");
	}

	//Read in polys
	for (int i = 0; i < numPolys; i++) {
		int p1, p2, p3;
		success = sscanf(token, "3 %d %d %d", &p1, &p2, &p3);
		if (success != 3) {
			printf("could not read poly #%d from file \"%s\"\n", i, filename);
			return new Mesh(0, 0);
		}
		mesh->addPoly(p1, p2, p3);
		token = strtok(nullptr, "\n");
	}

	mesh->normalize();
	mesh->buildNormals();

	return mesh;
}
