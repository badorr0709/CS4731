// WPI CS4731 - Computer Graphics - Homework 2
// William Hartman

#include "Angel.h"
#include "Mesh.h"

#define TRANSLATE_AMOUNT 0.025f
#define SHEAR_AMOUNT 0.05f
#define ROTATE_AMOUNT 0.2f
#define PULSE_LENGTH 0.1f     //Total units (fraction of mesh size) traveled in one full pulse cycle
#define PULSE_SPEED 0.001f    //Units (fraction of mesh size) traveled per update
#define NORMAL_LINE_LEN 0.05f //As a fraction of model size

//remember to prototype
void setUpOpenGLBuffers();
void display();
void resize(int newWidth, int newHeight);
void update();
void resetModel();
void keyboard(unsigned char key, int x, int y);

typedef Angel::vec4  point4;

using namespace std;

//Globals
GLuint program;
int width  = 0;
int height = 0;

Mesh* models[43];
int currentModel;
point3 currentLocation	 = point3(0, 0, 0);
float currentXShear		 = 0;
float currentXRotation	 = 0;
float currentPulseAmount = 0;
float currentPulseTime   = 0;
bool isRotating			 = false;
bool isPulsing			 = false;
bool drawNormals		 = false;

//Set up openGL buffers
void setUpOpenGLBuffers() {	
    //Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//Load shaders and use the resulting shader program
    program = InitShader("vshader1.glsl", "fshader1.glsl");
    glUseProgram(program);

    //set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//sets the default color to clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0); // black background
}

//Draw the current model with the appropriate model and view matrices
void display() {
	//Clear the window and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Build the view matrix
	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)45.0, (GLfloat)width/(GLfloat)height, (GLfloat)0.1, (GLfloat) 100.0);
	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0];viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0];viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0];viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0];viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8]  = perspectiveMat[0][2];viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9]  = perspectiveMat[1][2];viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2];viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2];viewMatrixf[15] = perspectiveMat[3][3];
	
	//Build the model matrix
	point3 meshCenter = models[currentModel]->getCenter();
	float meshWidth = models[currentModel]->getWidth();
	float meshHeight = models[currentModel]->getHeight();
	float meshDepth = models[currentModel]->getDepth();

	//Figure out how to scale the mesh
	float biggestDimension = fmaxf(meshWidth, meshHeight);
	float scaleFactor = 1.0f / biggestDimension;

	//Build the model matrix
	mat4 CTM = Angel::identity();
	mat4 moveToOrigin = Angel::Translate(-meshCenter.x, -meshCenter.y, -meshCenter.z);
	mat4 scale = Angel::Scale(scaleFactor, scaleFactor, scaleFactor);
	mat4 shear = Angel::identity(); shear[0][1] = currentXShear;
	mat4 rotate = Angel::RotateX(currentXRotation);
	mat4 moveToCurrentLocation = Angel::Translate(currentLocation.x, currentLocation.y, currentLocation.z - 1.0f);
	CTM = CTM * moveToCurrentLocation * rotate * shear * scale * moveToOrigin;

	float CTMf[16];
	CTMf[0] = CTM[0][0];CTMf[4] = CTM[0][1];
	CTMf[1] = CTM[1][0];CTMf[5] = CTM[1][1];
	CTMf[2] = CTM[2][0];CTMf[6] = CTM[2][1];
	CTMf[3] = CTM[3][0];CTMf[7] = CTM[3][1];

	CTMf[8]  = CTM[0][2];CTMf[12] = CTM[0][3];
	CTMf[9]  = CTM[1][2];CTMf[13] = CTM[1][3];
	CTMf[10] = CTM[2][2];CTMf[14] = CTM[2][3];
	CTMf[11] = CTM[3][2];CTMf[15] = CTM[3][3];
	
	//Set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, CTMf);
	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, viewMatrixf);

	//Set up color
	GLuint vColor = glGetUniformLocationARB(program, "vColor");

	//Draw the PLY model
	glUniform4f(vColor, 0, 1, 1, 1); //Set to cyan
	models[currentModel]->setUpBuffers(currentPulseAmount / scaleFactor);
	models[currentModel]->drawMesh();

	//Draw the normals (if enabled)
	if (drawNormals) {
		glUniform4f(vColor, 1, 0, 0, 1); //Set to red
		models[currentModel]->setUpNormalsBuffers(NORMAL_LINE_LEN / scaleFactor, currentPulseAmount / scaleFactor);
		models[currentModel]->drawNormalsLines();
	}

    glFlush(); //force output to graphics hardware

	//use this call to double buffer
	glutSwapBuffers();
}

//Update the size of the viewport, report the new width and height, redraw the scene
void resize(int newWidth, int newHeight) {
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
	display();
}


//Update rotation and pulsing (or neither if none are active)
void update() {
	if (isRotating) {
		currentXRotation += ROTATE_AMOUNT;
		if (currentXRotation > 360) {
			currentXRotation = 0;
		}
	}

	if (isPulsing) {
		currentPulseTime += PULSE_SPEED;

		if (currentPulseTime >= PULSE_LENGTH) {
			currentPulseTime = 0;
			currentPulseAmount = 0;
		} else if (currentPulseTime < PULSE_LENGTH / 2.0f) {
			currentPulseAmount += PULSE_SPEED;
		} else {
			currentPulseAmount -= PULSE_SPEED;
		}
	}

	display();
}

//Reset the position and orientation of the current mesh. Also stop any rotation, pulsing, or normals.
void resetModel() {
	currentLocation = point3(0, 0, 0);
	currentXShear = 0;
	currentXRotation = 0;
	currentPulseAmount = 0;
	currentPulseTime = 0;
	isRotating = false;
	isPulsing = false;
	drawNormals = false;
}

//keyboard handler
void keyboard(unsigned char key, int x, int y)
{
    switch ( key ) {
	//Exit
    case 033:
        exit(EXIT_SUCCESS);
        break;

	//Reset
	case 'w':
	case 'W':
		resetModel();
		break;

	//Next and Previous
	case 'n':
	case 'N':
		if (currentModel < 42) { currentModel++; }
		resetModel();
		break;

	case 'p':
	case 'P':
		if (currentModel > 0) { currentModel--; }
		resetModel();
		break;

	//Translating
	case 'X':
		currentLocation.x += TRANSLATE_AMOUNT;
		break;
	case 'x':
		currentLocation.x -= TRANSLATE_AMOUNT;
		break;
	case 'Y':
		currentLocation.y += TRANSLATE_AMOUNT;
		break;
	case 'y':
		currentLocation.y -= TRANSLATE_AMOUNT;
		break;
	case 'Z':
		currentLocation.z += TRANSLATE_AMOUNT;
		break;
	case 'z':
		currentLocation.z -= TRANSLATE_AMOUNT;
		break;

	//Rotating
	case 'r':
	case 'R':
		isRotating = !isRotating;
		break;

	//Shearing
	case 'H':
		currentXShear += SHEAR_AMOUNT;
		break;
	case 'h':
		currentXShear -= SHEAR_AMOUNT;
		break;

	//Pulsing
	case 'b':
	case 'B':
		isPulsing = !isPulsing;
		break;

	//Normals
	case 'm':
	case 'M':
		drawNormals = !drawNormals;
		break;
    }
}

//entry point
int main( int argc, char **argv ) {
	//Load all models
	models[0]  = loadMeshFromPLY("ply_files/airplane.ply");
	models[1]  = loadMeshFromPLY("ply_files/ant.ply");
	models[2]  = loadMeshFromPLY("ply_files/apple.ply");
	models[3]  = loadMeshFromPLY("ply_files/balance.ply");
	models[4]  = loadMeshFromPLY("ply_files/beethoven.ply");
	models[5]  = loadMeshFromPLY("ply_files/big_atc.ply");
	models[6]  = loadMeshFromPLY("ply_files/big_dodge.ply");
	models[7]  = loadMeshFromPLY("ply_files/big_porsche.ply");
	models[8]  = loadMeshFromPLY("ply_files/big_spider.ply");
	models[9]  = loadMeshFromPLY("ply_files/canstick.ply");
	models[10] = loadMeshFromPLY("ply_files/chopper.ply");
	models[11] = loadMeshFromPLY("ply_files/cow.ply");
	models[12] = loadMeshFromPLY("ply_files/dolphins.ply");
	models[13] = loadMeshFromPLY("ply_files/egret.ply");
	models[14] = loadMeshFromPLY("ply_files/f16.ply");
	models[15] = loadMeshFromPLY("ply_files/footbones.ply");
	models[16] = loadMeshFromPLY("ply_files/fracttree.ply");
	models[17] = loadMeshFromPLY("ply_files/galleon.ply");
	models[18] = loadMeshFromPLY("ply_files/hammerhead.ply");
	models[19] = loadMeshFromPLY("ply_files/helix.ply");
	models[20] = loadMeshFromPLY("ply_files/hind.ply");
	models[21] = loadMeshFromPLY("ply_files/kerolamp.ply");
	models[22] = loadMeshFromPLY("ply_files/ketchup.ply");
	models[23] = loadMeshFromPLY("ply_files/mug.ply");
	models[24] = loadMeshFromPLY("ply_files/part.ply");
	models[25] = loadMeshFromPLY("ply_files/pickup_big.ply");
	models[26] = loadMeshFromPLY("ply_files/pump.ply");
	models[27] = loadMeshFromPLY("ply_files/pumpa_tb.ply");
	models[28] = loadMeshFromPLY("ply_files/sandal.ply");
	models[29] = loadMeshFromPLY("ply_files/saratoga.ply");
	models[30] = loadMeshFromPLY("ply_files/scissors.ply");
	models[31] = loadMeshFromPLY("ply_files/shark.ply");
	models[32] = loadMeshFromPLY("ply_files/steeringweel.ply");
	models[33] = loadMeshFromPLY("ply_files/stratocaster.ply");
	models[34] = loadMeshFromPLY("ply_files/street_lamp.ply");
	models[35] = loadMeshFromPLY("ply_files/teapot.ply");
	models[36] = loadMeshFromPLY("ply_files/tennis_shoe.ply");
	models[37] = loadMeshFromPLY("ply_files/tommygun.ply");
	models[38] = loadMeshFromPLY("ply_files/trashcan.ply");
	models[39] = loadMeshFromPLY("ply_files/turbine.ply");
	models[40] = loadMeshFromPLY("ply_files/urn2.ply");
	models[41] = loadMeshFromPLY("ply_files/walkman.ply");
	models[42] = loadMeshFromPLY("ply_files/weathervane.ply");
	currentModel = 0;

	//init glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
	width = 512;
	height = 512;

	//create window
    glutCreateWindow("CS 4731 - Homework 2 - William Hartman");

	//init glew
    glewInit();

	//Set up buffers
    setUpOpenGLBuffers();

	//assign handlers
    glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutIdleFunc(update);
    glutKeyboardFunc(keyboard);

	//enter the drawing loop
    glutMainLoop();
    return 0;
}
