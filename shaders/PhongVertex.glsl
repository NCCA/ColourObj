#version 410 core
layout (location = 0) in vec3 inVert;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inColour;

/// @brief the current fragment normal for the vert being processed
out  vec3 fragmentNormal;
out vec3 vertColour;

uniform mat4 MVP;
uniform mat3 normalMatrix;


void main()
{
// calculate the fragments surface normal
fragmentNormal = (normalMatrix*inNormal);



// calculate the vertex position
gl_Position = MVP*vec4(inVert,1.0);
vertColour=inColour;

}
