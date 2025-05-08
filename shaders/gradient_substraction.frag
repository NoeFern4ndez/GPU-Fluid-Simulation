#version 430

in vec2 vCoords; // Coordenadas de textura
in vec2 vLeft;
in vec2 vRight;
in vec2 vTop;
in vec2 vBottom;

out vec4 fFragColor; // nuevo campo de velocidades después de restar el gradiente de la presión

layout(location = 1) uniform sampler2D uW; // campo de velocidades
layout(location = 2) uniform sampler2D uP; // campo de presiones 
layout(location = 0) uniform float uTexelSize; // Tamaño de un texel

void main()
{
	float pL = texture(uP, vLeft).x; // Se corresponde con x_{i-1,j}^{k} (la vecina de la izquierda)
	float pR = texture(uP, vRight).x; // Se corresponde con x_{i+1,j}^{k} (la vecina de la derecha)
	float pB = texture(uP, vBottom).x; // Se corresponde con x_{i,j-1}^{k} (la vecina de abajo)
	float pT = texture(uP, vTop).x; // Se corresponde con x_{i,j+1}^{k} (la vecina de arriba)
	
	vec2 gradient = vec2(pR - pL, pT - pB) / (2 * uTexelSize) ; // ∇p = (p_{i+1,j} - p_{i-1,j}, p_{i,j+1} - p_{i,j-1}) / 2
	fFragColor = texture(uW, vCoords);
	fFragColor.xy -= gradient; 
}