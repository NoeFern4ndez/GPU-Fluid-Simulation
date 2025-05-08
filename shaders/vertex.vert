#version 430

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;

layout(location = 0) uniform float uTexelSize; 

out vec2 vCoords;
out vec2 vLeft;
out vec2 vRight;
out vec2 vTop;
out vec2 vBottom;

void main()
{
	vCoords = aTexCoord;
	vLeft = aTexCoord - vec2(uTexelSize, 0.0);
	vRight = aTexCoord + vec2(uTexelSize, 0.0);
	vTop = aTexCoord + vec2(0.0, uTexelSize);
	vBottom = aTexCoord - vec2(0.0, uTexelSize);

	gl_Position = vec4(aPosition.x, aPosition.z, 0.0 , 1.0);
}