#version 420 core

in vec2 TexCoord;
out vec4 FragColor;

in vec3 Color;

void main() {
	FragColor = vec4(Color, 1);
}