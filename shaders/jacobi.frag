#version 430

in vec2 vCoords; // Coordenadas de textura
in vec2 vLeft; // Coordenadas de la vecina de la izquierda
in vec2 vRight; // Coordenadas de la vecina de la derecha
in vec2 vTop; // Coordenadas de la vecina de arriba
in vec2 vBottom; // Coordenadas de la vecina de abajo

out vec4 fFragColor; // valor de la velocidad en la iteración k+1 (después de la difusión)

layout(location = 1) uniform sampler2D uX; // Ax = b 
layout(location = 6) uniform sampler2D uB; // Ax = b 
layout(location = 9) uniform float uAlpha; // factor de difusión
layout(location = 10) uniform float uBeta; // factor de corrección

void main()
{
	vec3 xL = texture(uX,vLeft).xyz; // Se corresponde con x_{i-1,j}^{k} (la vecina de la izquierda)
	vec3 xR = texture(uX,vRight).xyz; // Se corresponde con x_{i+1,j}^{k} (la vecina de la derecha)
	vec3 xB = texture(uX,vBottom).xyz; // Se corresponde con x_{i,j-1}^{k} (la vecina de abajo)
	vec3 xT = texture(uX,vTop).xyz; // Se corresponde con x_{i,j+1}^{k} (la vecina de arriba) 
	vec3 bC = texture(uB, vCoords).xyz; // Se corresponde con b_{i,j} (el centro de la celda actual)	
	
	// x_{i,j}^{k+1} = (x_{i-1,j}^{k} + x_{i+1,j}^{k} + x_{i,j-1}^{k} + x_{i,j+1}^{k} + αb_{i,j}) / β
	vec3 jacobi = (xL + xR + xB + xT + (uAlpha * bC)) / uBeta; 
	fFragColor = vec4(jacobi, 1.0);
}