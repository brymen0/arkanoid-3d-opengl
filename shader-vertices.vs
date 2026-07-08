#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 Color;

uniform mat4 model; //matriz de transformacion
uniform mat4 view;
uniform mat4 projection;
uniform vec3 objectColor;


void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  Color = objectColor;
}