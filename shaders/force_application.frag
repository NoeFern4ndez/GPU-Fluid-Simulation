#version 430

in vec2 vCoords; // Coordenadas de textura

out vec4 fFragColor; // Campo con la fuerza aplicada

layout(location = 1) uniform sampler2D uC; // campo al que aplicar la fuerza (o color en el caso de la tinta)
layout(location = 11) uniform float uXClick; // Coordenada x del click
layout(location = 12) uniform float uYClick; // Coordenada y del click
layout(location = 13) uniform vec3 uForce; // Fuerza aplicada (o color en el caso de la tinta)
layout(location = 14) uniform float uRadius; // Radio de la fuerza

void main()
{
    vec2 click = vec2(uXClick, uYClick);
	vec3 f = uForce * exp(-dot(click - vCoords, click - vCoords) / uRadius); // f = F * exp(-||x - x_{click}||^2 / r^2)
	vec3 color = texture(uC, vCoords).xyz;
	fFragColor = vec4(color + f, 1.0); 
} 