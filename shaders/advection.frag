#version 430

in vec2 vCoords; // Coordenadas de textura de entrada

out vec4 fFragColor; // Valor de la textura advectada en la iteración k+1

layout(location = 1) uniform sampler2D uV; // Campo de velocidades 
layout(location = 5) uniform sampler2D uX; // Campo a advectar 
layout(location = 7) uniform float uTimestep; // Δt
layout(location = 8) uniform float uDissipation; // Disipación de la textura


void main()
{
   vec3 velocity = texture(uV, vCoords).xyz;
   vec2 pos = vCoords - uTimestep * velocity.xy; // q(x, t + Δt) = q(x - Δt * v(x, t), t) 

   fFragColor = texture(uX, pos) * uDissipation; 
}
