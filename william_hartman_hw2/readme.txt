William Hartman

CS 4731 - Computer Graphics
Homework 2

COMPILING/RUNNING
- Run "GLSLExperiment.exe" as normal from the "exe" folder
  - Make sure the ply_files folder, the fragment shader, the vertex shader,
    the glut .dll and the glew .dll are all present. Without these files, the
    program will not run

- Compile as usual in Visual Studio. There may be some compile warnings about
  string function safety. These were ignored as I felt security was an unimportant
  issue for a project of this scale

PROGRAM STRUCTURE

main.cpp:
  This file handles the application, keeping track of the state of the program and
  updating it in response to keypresses. The transformations for the model and camera
  are done here, in the display() function.

mesh.cpp:
  This file contains an implementation of class representing a mesh. Vertices and
  faces are stored with a vertex list structure. Face normals are also calculated
  and stored. Functions for sending point data to the GPU and drawing the mesh
  are included. Finally, this is a function for loading meshes from a PLY file.
