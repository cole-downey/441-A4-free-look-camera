#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 IT;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 vertPos; 
varying vec3 norm;

void main()
{
	gl_Position = P * MV * aPos;
	vertPos = vec3(MV * aPos);
	norm = vec3(IT * vec4(aNor, 1.0));
}
