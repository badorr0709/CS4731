#pragma once

#include "mesh.h"

class Line {
public:
	Line(Mesh* m1, Mesh* m2);
	void drawLine(int program);
	void drawLineShadow(int program, Spotlight* light, float dist, vec3 planeRotation, mat4 modelView);
private:
	vec4 buf[12];
};
	




