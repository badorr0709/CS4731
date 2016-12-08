// William Hartman
// CS4731 - Homework 1

#include "Angel.h"
#include "PolyDrawing.h"
typedef vec2 point2;

#define FRACTAL_POINTS 16384 //The number of points in the fractal. This should not exceed MAX_POINTS

/*
Build the needed points for the gingerbread man fractal
*/
void makeGingerbreadMan(point2 points[], int iters, float M, float L, float x, float y) {
	float pX = x;
	float pY = y;
	for (int i = 0; i < iters; i++) {
		float qX = M * (1 + (2 * L)) - pY + fabs(pX - (L * M));
		float qY = pX;
		points[i] = point2(qX, qY);
		pX = qX;
		pY = qY;
	}
}

/*
Draw the gingerbread man
*/
void drawGingerbreadMan(int program, float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
	//Build gingerbread man
	point2* points = new point2[FRACTAL_POINTS];
	makeGingerbreadMan(points, FRACTAL_POINTS, 40.0, 3.0, 115.0, 121.0);

	//Translate this into a PolyDrawing
	PolylineDrawing* drawing = genEmptyDrawing(FRACTAL_POINTS);
	drawing->extent_top = 480;
	drawing->extent_right = 640;

	addLine(drawing);
	for (int i = 0; i < FRACTAL_POINTS; i++) {
		addPointToCurrentLine(drawing, points[i].x, points[i].y);
	}

	drawPolylineDrawing(drawing, GL_POINTS, program, viewportX, viewportY, viewportWidth, viewportHeight);
	delete points;
}

/*
Build the needed points for the fern fractal
*/
void makeFern(point2 points[], int iters, float x, float y) {
	static float a[] =  { 0.00,  0.20, -0.15,  0.85 };
	static float b[] =  { 0.00,  0.23,  0.26, -0.04 };
	static float c[] =  { 0.00, -0.26,  0.28,  0.04 };
	static float d[] =  { 0.16,  0.22,  0.24,  0.85 };
	static float tx[] = { 0.00,  0.20,  0.00,  0.00 };
	static float ty[] = { 0.00,  1.60,  0.44,  1.60 };

	float pX = x;
	float pY = y;
	for (int i = 0; i < iters; i++) {
		int randVal = (rand() % 100) + 1;
		int index;
		//7% chance of f2
		if (randVal <= 7) {
			index = 1;
		}
		//7% chance of f3
		else if (randVal <= 14) {
			index = 2;
		}
		//85% chance of f4
		else if (randVal <= 99) {
			index = 3;
		}
		//1% chance of f1
		else {
			index = 0;
		}

		float qX = a[index] * pX + c[index] * pY + tx[index];
		float qY = b[index] * pX + d[index] * pY + ty[index];
		points[i] = point2(qX, qY);
		pX = qX;
		pY = qY;
	}
}

/*
Draw the fern fractal
*/
void drawFern(int program, float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
	//Build gingerbread man
	point2* points = new point2[FRACTAL_POINTS];
	makeFern(points, FRACTAL_POINTS, 0.0, 0.0);

	//Translate this into a PolyDrawing
	PolylineDrawing* drawing = genEmptyDrawing(FRACTAL_POINTS);

	addLine(drawing);
	for (int i = 0; i < FRACTAL_POINTS; i++) {
		//Set up extents
		if (points[i].x > drawing->extent_right) { drawing->extent_right = points[i].x; }
		if (points[i].x < drawing->extent_left) { drawing->extent_left = points[i].x; }
		if (points[i].y > drawing->extent_top) { drawing->extent_top = points[i].y; }

		//Add to drawing
		addPointToCurrentLine(drawing, points[i].x, points[i].y);
	}

	drawPolylineDrawing(drawing, GL_POINTS, program, viewportX, viewportY, viewportWidth, viewportHeight);
	delete points;
}
