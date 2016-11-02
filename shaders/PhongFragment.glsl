#version 410 core
/// @brief our output fragment colour
layout (location =0) out vec4 fragColour;
/// @brief[in] the vertex normal
in  vec3 fragmentNormal;
in vec3 vertColour;


void main ()
{
  fragColour=vec4(vertColour,1.0f);
}

