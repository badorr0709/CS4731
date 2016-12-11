#version 150

// William Hartman
// CS4731 - Homework 1

uniform mat4 proj;
in vec4 vPosition;

void
main()
{
    gl_Position = proj * vPosition;
}
