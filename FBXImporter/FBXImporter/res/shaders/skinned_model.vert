#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneID;
layout (location = 6) in vec4 aBoneWeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 Color;

void main() {
	TexCoord = aTexCoord;
	Color = aBoneWeight.rgb;
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}