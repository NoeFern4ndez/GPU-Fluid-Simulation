#version 430

in vec2 vCoords; // Coordenadas de textura
in vec2 vLeft;
in vec2 vRight;
in vec2 vTop;
in vec2 vBottom;

out vec4 fFragColor; // Vorticidad en la celda actual

layout(location = 1) uniform sampler2D uV; // Textura con la velocidad
layout (location = 0) uniform float uTexelSize; // Tamaño de un texel

void main()
{
	float uL = texture(uV,vLeft).y; // Se corresponde con x_{i-1,j}^{k} (la vecina de la izquierda)
	float uR = texture(uV,vRight).y; // Se corresponde con x_{i+1,j}^{k} (la vecina de la derecha)
	float uB = texture(uV,vBottom).x; // Se corresponde con x_{i,j-1}^{k} (la vecina de abajo)
	float uT = texture(uV,vTop).x; // Se corresponde con x_{i,j+1}^{k} (la vecina de arriba) 
	
	float vorticity = (uR - uL - uT + uB) / (2.0 * uTexelSize);  // vorticity = ∇.w = (v_{i+1,j} - v_{i-1,j} - v_{i,j+1} + v_{i,j-1}) / 2
	fFragColor = vec4(vorticity, 0.0, 0.0, 1.0);
}