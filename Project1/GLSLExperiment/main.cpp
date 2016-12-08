// William Hartman
// CS4731 - Homework 1

#include "Angel.h"  // Angel.h is homegrown include file, which also includes glew and freeglut
#include "PolyDrawing.h"

// Number of points in polyline
const int NumPoints = 3;

// remember to prototype
void initGPUBuffers( void );
void shaderSetup( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void keyboardUp( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);

//Fractal forward defs
void drawGingerbreadMan(int program, float viewportX, float viewportY, float viewportWidth, float viewportHeight);
void drawFern(int program, float viewportX, float viewportY, float viewportWidth, float viewportHeight);

typedef vec2 point2;
using namespace std;

//Shader handle
GLuint program;
int color;

//State
#define STATE_POLYLINE_VIEWER 0
#define STATE_POLYLINE_DRAW 1
#define STATE_POLYLINE_TILES 2
#define STATE_FRACTAL_GINGERBREAD 3
#define STATE_FRACTAL_FERN 4
int currentState;

//Globals for STATE_POLYLINE_VIEWER
PolylineDrawing* allDrawings[10];
PolylineDrawing* currentDrawing;

//Globals for STATE_POLYLINE_DRAW
PolylineDrawing* drawing;
bool bPressed;

//Globals for STATE_POLYLINE_TILES
int seed;

void initGPUBuffers( void )
{
	// Create a vertex array object
	GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
}


void shaderSetup( void )
{
	// Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );

    // Initialize the vertex position attribute from the vertex shader
    GLuint loc = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    glClearColor( 1.0, 1.0, 1.0, 1.0 );     // sets white as color used to clear screen
}

void drawThumbnails(int windowWidth, int windowHeight) {
	float thumbnailSize = windowWidth / 10;
	for (int i = 0; i < 10; i++) {
		drawPolylineDrawing(allDrawings[i], GL_LINE_STRIP, program, i * thumbnailSize, windowHeight - thumbnailSize, thumbnailSize, thumbnailSize);
	}
}
void display( void )
{
	//Clear screen
	glClear(GL_COLOR_BUFFER_BIT);

	float windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float windowHeight = glutGet(GLUT_WINDOW_HEIGHT) - 6;
	float thumbnailSize = windowWidth / 10;
	float tiledImgWidth;
	float tiledImgHeight;

	switch (currentState)
	{
	case STATE_POLYLINE_VIEWER:
		drawThumbnails(windowWidth, windowHeight);
		drawPolylineDrawing(currentDrawing, GL_LINE_STRIP, program, 0, 0, windowWidth, windowHeight - thumbnailSize);
		break;

	case STATE_POLYLINE_DRAW:
		drawThumbnails(windowWidth, windowHeight);
		drawPolylineDrawing(drawing, GL_LINE_STRIP, program, 0, 0, windowWidth, windowHeight - thumbnailSize);
		break;

	case STATE_POLYLINE_TILES:
		drawThumbnails(windowWidth, windowHeight);

		tiledImgWidth = windowWidth / 5.0;
		tiledImgHeight = (windowHeight - thumbnailSize) / 5.0;
		srand(seed);
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				drawPolylineDrawing(allDrawings[(int) ((rand() / (float) RAND_MAX) * 10)], 
									GL_LINE_STRIP, 
									program,
									i * tiledImgWidth, 
									j * tiledImgHeight, 
									tiledImgWidth, 
									tiledImgHeight);
			}
		}
		break;

	case STATE_FRACTAL_GINGERBREAD:
		drawGingerbreadMan(program, 0, 0, windowWidth, windowHeight - thumbnailSize);
		break;

	case STATE_FRACTAL_FERN:
		drawFern(program, 0, 0, windowWidth, windowHeight - thumbnailSize);
		break;

	default:
		break;
	}

	//Flush to screen
	glFlush();
}

void keyboard( unsigned char key, int x, int y ) {
	// keyboard handler

    switch ( key ) {
    case 033:		// 033 is Escape key octal value
        exit(1);	// quit program
        break;

	case 'p':
		currentState = STATE_POLYLINE_VIEWER;
		display();
		break;

	case 'e':
		currentState = STATE_POLYLINE_DRAW;
		display();
		break;

	case 't':
		currentState = STATE_POLYLINE_TILES;
		seed = glutGet(GLUT_ELAPSED_TIME);
		display();
		break;

	case 'g':
		currentState = STATE_FRACTAL_GINGERBREAD;
		display();
		break;

	case 'f':
		currentState = STATE_FRACTAL_FERN;
		display();
		break;

	case 'c':
		color++;
		if (color % 3 == 0) {
			glUniform1f(glGetUniformLocation(program, "r"), 1.0);
			glUniform1f(glGetUniformLocation(program, "g"), 0.0);
			glUniform1f(glGetUniformLocation(program, "b"), 0.0);
		}
		if (color % 3 == 1) {
			glUniform1f(glGetUniformLocation(program, "r"), 0.0);
			glUniform1f(glGetUniformLocation(program, "g"), 1.0);
			glUniform1f(glGetUniformLocation(program, "b"), 0.0);
		}
		if (color % 3 == 2) {
			glUniform1f(glGetUniformLocation(program, "r"), 0.0);
			glUniform1f(glGetUniformLocation(program, "g"), 0.0);
			glUniform1f(glGetUniformLocation(program, "b"), 1.0);
		}
		display();
		break;

	case 'b':
		bPressed = true;
		break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
	// keyboard handler

	switch (key) {
	case 'b':
		bPressed = false;
		break;
	}
}

void goToPicture(float x, float y, float windowWidth) {
	if (y < (windowWidth / 10.0)) {
		//The "+ 1" ensures we don't go past the array without needing fancy bounds checking, but doesn't influence accuracy too much
		currentState = STATE_POLYLINE_VIEWER;
		currentDrawing = allDrawings[(int)(x / (windowWidth + 1) * 10)];
		display();
	}
}

void mouse(int button, int state, int x, int y) {
	float windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	float windowHeight = glutGet(GLUT_WINDOW_HEIGHT) - 6;

	//If mouse pressed
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		switch (currentState)
		{
		case STATE_POLYLINE_VIEWER:
			goToPicture(x, y, windowWidth);
			break;

		case STATE_POLYLINE_DRAW:
			goToPicture(x, y, windowWidth);
			if (bPressed) {
				addLine(drawing);
			}
			addPointToCurrentLine(drawing, x, (windowHeight - y), 0, 0, windowWidth, windowHeight - (windowWidth / 10.0));
			display();
			break;

		case STATE_POLYLINE_TILES:
			goToPicture(x, y, windowWidth);
			break;
		}
	}
}

int main( int argc, char **argv ) {
	color = 0;

	// Set to default state and default drawing
	currentState = STATE_POLYLINE_VIEWER;
	allDrawings[0] = loadPolylineDrawing("Resource/birdhead.dat");
	allDrawings[1] = loadPolylineDrawing("Resource/dino.dat");
	allDrawings[2] = loadPolylineDrawing("Resource/dragon.dat");
	allDrawings[3] = loadPolylineDrawing("Resource/dragon.dat"); //Second dragon, as requested in the HW handout
	allDrawings[4] = loadPolylineDrawing("Resource/house.dat");
	allDrawings[5] = loadPolylineDrawing("Resource/knight.dat");
	allDrawings[6] = loadPolylineDrawing("Resource/rex.dat");
	allDrawings[7] = loadPolylineDrawing("Resource/scene.dat");
	allDrawings[8] = loadPolylineDrawing("Resource/usa.dat");
	allDrawings[9] = loadPolylineDrawing("Resource/vinci.dat");
	currentDrawing = allDrawings[9];

	//Set up polyline drawing
	drawing = genEmptyDrawing(4096);
	addLine(drawing);

	//OpenGL setup
    glutInit( &argc, argv );                       // intialize GLUT  
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB ); // single framebuffer, colors in RGB
    glutInitWindowSize( 640, 480 );                // Window size: 640 wide X 480 high
	glutInitWindowPosition(100,150);               // Top left corner at (100, 150)
    glutCreateWindow( "CS 4731 HW 1" );            // Create Window

    glewInit();									   // init glew
    initGPUBuffers( );							   // Create GPU buffers
    shaderSetup( );                                // Connect this .cpp file to shader file

    glutDisplayFunc( display );                    // Register display callback function
    glutKeyboardFunc( keyboard );                  // Register keyboard callback function
	glutMouseFunc(mouse);						   // Register mouse callback function
	glutKeyboardUpFunc( keyboardUp );			   // Register keyboard up callback function

	//Set initial colors
	glUniform1f(glGetUniformLocation(program, "r"), 1.0);
	glUniform1f(glGetUniformLocation(program, "g"), 0.0);
	glUniform1f(glGetUniformLocation(program, "b"), 0.0);

	// enter the drawing loop
    glutMainLoop();
    return 0;
}
