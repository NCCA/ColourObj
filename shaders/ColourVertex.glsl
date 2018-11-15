#version 410 core
layout (location = 0) in vec3 inVert;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColour;

/// @brief the current fragment normal for the vert being processed
out vec3 vertColour;
uniform mat4 MVP;


void main()
{

// calculate the vertex position
gl_Position = MVP*vec4(inVert,1.0);
vertColour=inColour;

}
