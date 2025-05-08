#version 430

in vec2 vCoords; // Coordenadas de textura
in vec2 vLeft;
in vec2 vRight;
in vec2 vTop;
in vec2 vBottom;

out vec4 fFragColor; // Divergencia en la celda actual

layout(location = 1) uniform sampler2D uW; // Campo de velocidades 
layout(location = 0) uniform float uTexelSize; // Tamaño de un texel

void main()
{
    float wL = texture(uW, vLeft).x; // Vecino izquierdo
    float wR = texture(uW, vRight).x; // Vecino derecho
    float wB = texture(uW, vBottom).y; // Vecino inferior
    float wT = texture(uW, vTop).y; // Vecino superior

    float div = ((wR - wL) + (wT - wB)) / (2.0 * uTexelSize); // div = ∇·w = (w_{i+1,j} - w_{i-1,j} + w_{i,j+1} - w_{i,j-1}) / 2 
    fFragColor = vec4(div, 0.0, 0.0, 1.0);
}
