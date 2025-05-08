#version 430

in vec2 vCoords; // Coordenadas de textura

out vec4 fFragColor; // Color del fragmento

layout(location = 3) uniform sampler2D color; // Textura a visualizar

void main()
{
	fFragColor =  texture(color, vCoords);
}