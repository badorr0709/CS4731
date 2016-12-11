#include "Line.h"

Line::Line(Mesh* m1, Mesh* m2) {
	//Build buffer
	vec3 m1Center = m1->getCenterPosition();
	vec3 m2Center = m2->getCenterPosition(m2->getModelMatrix());

	//Build a three part line strip.
	//	- First part, vertical down from parent
	//	- Second part, horizontal over to child, at the average height between the parent and child
	//	- Third part, vertical down to child
	int i = 0;
	buf[i] = m1Center;                                                          i++; buf[i] = vec4(1, 1, 1, 1); i++; buf[i] = vec4(0, 0, 0, 0); i++;
	buf[i] = vec4(m1Center.x, (m1Center.y + m2Center.y) / 2.0f, m1Center.z, 1); i++; buf[i] = vec4(1, 1, 1, 1); i++; buf[i] = vec4(0, 0, 0, 0); i++;
	buf[i] = vec4(m2Center.x, (m1Center.y + m2Center.y) / 2.0f, m2Center.z, 1); i++; buf[i] = vec4(1, 1, 1, 1); i++; buf[i] = vec4(0, 0, 0, 0); i++;
	buf[i] = m2Center;                                                          i++; buf[i] = vec4(1, 1, 1, 1); i++; buf[i] = vec4(0, 0, 0, 0); i++;
}


//Mesh 1 should be the child, mesh 2 should be the parent
void Line::drawLine(int program) {
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 12, buf, GL_STATIC_DRAW);

	//Set color to white
	GLuint vColor = glGetUniformLocationARB(program, "overrideColor");
	glUniform4f(vColor, 1, 1, 1, 1);

	//Draw the PLY model
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINE_STRIP, 0, 4);
	glDisable(GL_DEPTH_TEST);
}

void Line::drawLineShadow(int program, Spotlight* light, float dist, vec3 planeRotation, mat4 modelView) {
	//Shadow matrix
	vec4 rotatedLightPos = (Angel::RotateX(-planeRotation.x) * Angel::RotateY(-planeRotation.y) * Angel::RotateZ(-planeRotation.z)) * light->getPosition();
	vec4 offsetPos = vec4(rotatedLightPos.x,
		rotatedLightPos.y + dist - 0.00001, //Move the shadow slightly closer to avoid z-fighting
		rotatedLightPos.z,
		1);

	mat4 shadowMat = Angel::identity();
	shadowMat[3][1] = -1.0f / offsetPos.y;
	shadowMat[3][3] = 0;

	mat4 rotate = Angel::RotateX(-planeRotation.x) * Angel::RotateY(-planeRotation.y) * Angel::RotateZ(-planeRotation.z);
	mat4 rotateBack = Angel::RotateX(planeRotation.x) * Angel::RotateY(planeRotation.y) * Angel::RotateZ(planeRotation.z);
	mat4 rotateToPlane = Angel::RotateY(planeRotation.y) * Angel::RotateZ(-planeRotation.y);
	mat4 shadowProjMat = rotateToPlane * rotateBack * Angel::Translate(rotatedLightPos) * shadowMat * Angel::Translate(-rotatedLightPos) * rotate * modelView;

	float mm[16];
	mm[0] = shadowProjMat[0][0]; mm[4] = shadowProjMat[0][1];
	mm[1] = shadowProjMat[1][0]; mm[5] = shadowProjMat[1][1];
	mm[2] = shadowProjMat[2][0]; mm[6] = shadowProjMat[2][1];
	mm[3] = shadowProjMat[3][0]; mm[7] = shadowProjMat[3][1];
	mm[8] = shadowProjMat[0][2]; mm[12] = shadowProjMat[0][3];
	mm[9] = shadowProjMat[1][2]; mm[13] = shadowProjMat[1][3];
	mm[10] = shadowProjMat[2][2]; mm[14] = shadowProjMat[2][3];
	mm[11] = shadowProjMat[3][2]; mm[15] = shadowProjMat[3][3];
	GLuint ctmLocation = glGetUniformLocationARB(program, "modelMatrix");
	glUniformMatrix4fv(ctmLocation, 1, GL_FALSE, mm);

	//Send the line data to GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 12, buf, GL_STATIC_DRAW);

	//Clear global color
	GLuint vColor = glGetUniformLocationARB(program, "overrideColor");
	glUniform4f(vColor, 0, 0, 0, 0.5f);

	//Draw the shadow polygons
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glStencilFunc(GL_EQUAL, 111, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_LINE_STRIP, 0, 4);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	//Clean 
	mm[0] = modelView[0][0]; mm[4] = modelView[0][1];
	mm[1] = modelView[1][0]; mm[5] = modelView[1][1];
	mm[2] = modelView[2][0]; mm[6] = modelView[2][1];
	mm[3] = modelView[3][0]; mm[7] = modelView[3][1];
	mm[8] = modelView[0][2]; mm[12] = modelView[0][3];
	mm[9] = modelView[1][2]; mm[13] = modelView[1][3];
	mm[10] = modelView[2][2]; mm[14] = modelView[2][3];
	mm[11] = modelView[3][2]; mm[15] = modelView[3][3];
	glUniformMatrix4fv(ctmLocation, 1, GL_FALSE, mm);
}