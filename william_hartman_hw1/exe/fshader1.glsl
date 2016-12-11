#version 150

// William Hartman
// CS4731 - Homework 1

uniform float r;
uniform float g;
uniform float b;
out vec4  fColor;

void
main()
{
    fColor = vec4( r, g, b, 1.0 );
}
