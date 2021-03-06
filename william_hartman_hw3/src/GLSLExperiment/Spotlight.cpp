// WPI CS4731 - Computer Graphics - Homework 3
// William Hartman

#include "Spotlight.h"

Spotlight::Spotlight(vec4 pos, vec3 dir, float cut) {
	position = pos;
	direction = dir;
	cutoff = cut;
}

vec4 Spotlight::getPosition() {
	return position;
}

vec3 Spotlight::getDirection() {
	return direction;
}

float Spotlight::getCutoff() {
	return cutoff;
}

void Spotlight::setCutoff(float newCutoff) {
	cutoff = newCutoff;
}
