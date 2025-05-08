#version 430

in vec2 vCoords; // Coordenadas de textura de entrada

out vec4 fFragColor; // Valor de la textura una vez aplicadas las condiciones de contorno

layout(location = 3) uniform sampler2D uC; // Campo al que aplicar las condiciones de contorno
layout(location = 0) uniform float uTexelSize; // Tamaño de un texel

void main()
{
   fFragColor = texture(uC, vCoords);
   float dist = uTexelSize * 20.0;
   fFragColor.x = step(dist, vCoords.x) * step(vCoords.x, 1.0 - dist) * fFragColor.x; // Si x < dist o x > 1.0 - dist, fFragColor.x = 0
   fFragColor.y = step(dist, vCoords.y) * step(vCoords.y, 1.0 - dist) * fFragColor.y; // Si y < dist o y > 1.0 - dist, fFragColor.y = 0
}
