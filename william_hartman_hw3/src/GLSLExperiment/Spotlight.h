// WPI CS4731 - Computer Graphics - Homework 3
// William Hartman

#pragma once
#include "Angel.h"

class Spotlight {
public:
	Spotlight(vec4 pos, vec3 dir, float cut);
	vec4 getPosition();
	vec3 getDirection();
	float getCutoff();
	void setCutoff(float newCutoff);

private:
	vec4 position;
	vec3 direction;
	float cutoff;
};