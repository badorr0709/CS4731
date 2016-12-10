William Hartman

CS 4731 - Computer Graphics
Homework 4

COMPILING/RUNNING
- Run "GLSLExperiment.exe" as normal from the "exe" folder
  - Make sure the Resources folder, the fragment shader, the vertex shader,
    the glut .dll and the glew .dll are all present. Without these files, the
    program will not run

- Compile as usual in Visual Studio. There may be some compile warnings about
  string function safety. These were ignored as I felt security was an unimportant
  issue for a project of this scale
	- The line endings on the shaders may pose a problem. If the program exits
	  unexpectedly and prints messages about shaders failing to compile, go to
	  "Advanced Save Options" -> "Line endings" -> "Unix (LF)" in Visual Studio
	  for both the fragment and vertex shader.

PROGRAM STRUCTURE

main.cpp:
  This file handles the application, keeping track of the state of the program and
  updating it in response to keypresses. The transformations for the model and camera
  are done here, in the display() function.

  The heirarchy and CTM stack are maintained here, but implemented in CTMStack.cpp

mesh.cpp:
  This file contains an implementation of class representing a mesh. Vertices and
  faces are stored with a vertex list structure. Vertex normals are also calculated
  and stored. Functions for sending point data to the GPU and drawing the mesh
  are included. Finally, this is a function for loading meshes from a PLY file.
  This class also handles drawing shadows, applying textures, and supplying the
  environemnt map.

Spotlight.cpp:
  This file holds a simple class that encapsulates important parameters about a
  spotlight.

CTMStack.cpp
  This file implements a stack using a linked list. Each node holds a matrix. when
  pop or peek is called, that matrix is applied as the model matrix in the shader.

vshader1.glsl
  This shader projects points and passes vertex information along to the
  fragment shader.

fshader1.glsl
  This shader implements smooth (phong) shading, texture mapping, and enviroment 
  mapping.
