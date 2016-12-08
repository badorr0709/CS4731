// William Hartman
// CS4731 - Homework 1

#include "Angel.h"
#include "PolyDrawing.h"
#include "textfile.h"

/*
Build an empty (but properly initialized) drawing
*/
PolylineDrawing* genEmptyDrawing(int maxPoints) {
	struct PolylineDrawing* drawing = new PolylineDrawing;

	//Zero out attrs
	drawing->extent_left = 0;
	drawing->extent_top = 1;
	drawing->extent_right = 1;
	drawing->extent_bottom = 0;
	drawing->numPoints = 0;
	drawing->numLines = 0;

	//Zero out points and lines buffers
	for (int i = 0; i < maxPoints; i++) {
		drawing->points[i] = point2(0, 0);
		drawing->lines[i] = 0;
	}

	return drawing;
}

/*
Draw an entire polyline drawing in the given viewport. This function also handles aspect ratio
*/
void drawPolylineDrawing(PolylineDrawing* drawing, int mode, GLuint program, float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
	//Set up the buffers
	glBufferData(GL_ARRAY_BUFFER, sizeof(drawing->points), drawing->points, GL_STATIC_DRAW);

	//Set up the ortho
	float l = drawing->extent_left;
	float t = drawing->extent_top;
	float r = drawing->extent_right;
	float b = drawing->extent_bottom;
	mat4 ortho = Ortho2D(l, r, b, t);

	//Set up the viewport
	float aspectRatio = (r - l) / (t - b);
	if (aspectRatio > viewportWidth / viewportHeight) {
		glViewport(viewportX, viewportY, viewportWidth, viewportWidth / aspectRatio);
	} else if (aspectRatio < viewportWidth / viewportHeight) {
		glViewport(viewportX, viewportY, viewportHeight * aspectRatio, viewportHeight);
	}
	else {
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
	}

	//Send projection to the vertex shader
	int projLoc = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_TRUE, ortho);

	//Draw all the lines
	int lineStartIndex = 0;
	for (int i = 0; i < drawing->numLines; i++) {
		glDrawArrays(mode, lineStartIndex, drawing->lines[i]);

		int lineEndIndex = lineStartIndex + drawing->lines[i];
		lineStartIndex = lineEndIndex;
	}
}

/*
Add a point to the last line in the drawing (no verification or viewport correction)
*/
void addPointToCurrentLine(PolylineDrawing* drawing, float x, float y) {
	if (drawing->numPoints >= MAX_POINTS) {
		return;
	}

	int currentLineIndex = drawing->numLines - 1;
	int currentPointIndex = drawing->numPoints;

	drawing->lines[currentLineIndex]++;
	drawing->numPoints++;
	drawing->points[currentPointIndex] = point2(x, y);
}

/*
Add a point to the last line in the drawing (with verification and viewport correction)
*/
void addPointToCurrentLine(PolylineDrawing* drawing, float x, float y, float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
	float correctedX;
	float correctedY;

	float l = drawing->extent_left;
	float t = drawing->extent_top;
	float r = drawing->extent_right;
	float b = drawing->extent_bottom;
	float aspectRatio = (r - l) / (t - b);
	if (aspectRatio > viewportWidth / viewportHeight) {
		correctedX = (x / viewportWidth) * r;
		correctedY = (y / viewportWidth) * t;
	}
	else if (aspectRatio < viewportWidth / viewportHeight) {
		correctedX = (x / viewportHeight) * r;
		correctedY = (y / viewportHeight) * t;
	}
	else {
		correctedX = (x / viewportWidth) * r;
		correctedY = (y / viewportHeight) * t;
	}

	addPointToCurrentLine(drawing, correctedX, correctedY);
}

/*
Start a new line
*/
void addLine(PolylineDrawing* drawing) {
	if (drawing->numLines >= MAX_POINTS) {
		return;
	}

	drawing->numLines++;
}

/*
Load a polyline drawing from a GRS formatted file
*/
struct PolylineDrawing* loadPolylineDrawing(char* filename) {
	char* fileContents = textFileRead(filename);
	PolylineDrawing* drawing = genEmptyDrawing(4096);

	//Tokenize on newlines
	char* token = strtok(fileContents, "\n");

	//Get through the comments
	bool pastComments = (bool)strchr(fileContents, '*'); //If there aren't any '*'s in the string, we don't need to track comments
	while (!pastComments) {
		pastComments = strlen(token) > 0 && token[0] == '*';
		token = strtok(nullptr, "\n");
	}

	//Add all extents, using defaults if they fail to read
	float l, t, r, b;
	int success = sscanf(token, "%f  %f  %f  %f", &l, &t, &r, &b);
	if (success == 4) {
		token = strtok(nullptr, "\n");
	}
	drawing->extent_left = l;
	drawing->extent_top = t;
	drawing->extent_right = r;
	drawing->extent_bottom = b;

	//Get number of lines
	int numLines;
	sscanf(token, "%d\n", &numLines);
	drawing->numLines = numLines;
	token = strtok(nullptr, "\n");

	//Add all lines
	for(int i = 0; i < numLines && i < MAX_POINTS; i++) {
		//Get the number of points
		int numPoints;
		sscanf(token, "%d\n", &numPoints);
		drawing->lines[i] = numPoints;
		token = strtok(nullptr, "\n");

		//Read all points
		for (int j = 0; j < numPoints && drawing->numPoints + j < MAX_POINTS; j++) {
			float x, y;
			sscanf(token, "  %f  %f\n", &x, &y);
			drawing->points[drawing->numPoints + j] = point2(x, y);
			token = strtok(nullptr, "\n");
		}
		drawing->numPoints += numPoints;
	}
	
	return drawing;
}
