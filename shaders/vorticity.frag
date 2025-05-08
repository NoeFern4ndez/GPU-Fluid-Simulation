#version 430

in vec2 vCoords; 
in vec2 vLeft;
in vec2 vRight;
in vec2 vTop;
in vec2 vBottom;

out vec4 fFragColor;  // Nuevo campo de velocidades después de añadir la vorticidad

layout(location = 1) uniform sampler2D uV; // Campo de velocidades
layout(location = 4) uniform sampler2D uVorticity; // Campo con el valor de la vorticidad
layout(location = 7) uniform float uTimestep; // Δt
layout(location = 15) uniform float uFactor;  // Factor de vorticidad
layout(location = 0) uniform float uTexelSize; // Tamaño de un texel

void main()
{
	float vL = texture(uVorticity,vLeft).x; // Se corresponde con x_{i-1,j}^{k} (la vecina de la izquierda)
	float vR = texture(uVorticity,vRight).x; // Se corresponde con x_{i+1,j}^{k} (la vecina de la derecha)
	float vB = texture(uVorticity,vBottom).x; // Se corresponde con x_{i,j-1}^{k} (la vecina de abajo)
	float vT = texture(uVorticity,vTop).x; // Se corresponde con x_{i,j+1}^{k} (la vecina de arriba) 
	float vC = texture(uVorticity, vCoords).x; // Se corresponde con b_{i,j} (el centro de la celda actual)	
	vec3 velocity = texture(uV, vCoords).xyz;

	vec2 vorticity = vec2(abs(vT) - abs(vB), abs(vR) - abs(vL)) / (2 * uTexelSize); // vorticity =  η = ∇.|ω| = (|ω_{i,j+1}| - |ω_{i,j-1}|, |ω_{i+1,j}| - |ω_{i-1,j}|) / 2
	vorticity /= length(vorticity) + 0.0001; //  Ψ = η / ||η||

	// f_{vc} = ε(v_{i,j} * Ψ) * b_{i,j}
	vorticity *= uFactor * vC; // ω = εvΨb 
	vorticity.y *= -1.0;  // ω = εvΨb
	velocity.xy += uTimestep * vorticity; // v^{n+1} = v^{n} + Δt * ω
	
	fFragColor = vec4(velocity, 1.0);
}