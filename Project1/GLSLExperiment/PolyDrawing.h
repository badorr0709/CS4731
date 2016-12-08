// William Hartman
// CS4731 - Homework 1

#pragma once

#include "Angel.h"  // Angel.h is homegrown include file, which also includes glew and freeglut

#define MAX_POINTS 16384

typedef vec2 point2;

struct PolylineDrawing {
	float extent_left;
	float extent_top;
	float extent_right;
	float extent_bottom;

	int numPoints;				//The total number of points in the drawing
	point2 points[MAX_POINTS];	//The array of all points in the drawing
	int numLines;				//The number of lines in the drawing
	int lines[MAX_POINTS];		//The number of points in each polyline in the drawing
};

PolylineDrawing* genEmptyDrawing(int maxPoints);

void drawPolylineDrawing(PolylineDrawing* drawing, int mode, GLuint program, float viewportX, float viewportY, float viewportWidth, float viewportHeight);

void addPointToCurrentLine(PolylineDrawing* drawing, float x, float y, float viewportX, float viewportY, float viewportWidth, float viewportHeight);

void addPointToCurrentLine(PolylineDrawing* drawing, float x, float y);

void addLine(PolylineDrawing* drawing);

struct PolylineDrawing* loadPolylineDrawing(char* filename);
