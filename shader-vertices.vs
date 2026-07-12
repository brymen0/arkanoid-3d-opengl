#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

// Variable de salida de color
out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

//matriz de transformacion
uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;

// Posición de la pelota
uniform vec3 luzPos;   
// Posición de la cámara
uniform vec3 viewPos;  

// Variables necesarias para el modelo de Phong

// Vectores matriz de insidencia
const vec3 La = vec3(0.5, 0.5, 0.5);
const vec3 Ld = vec3(1.0, 1.0, 1.0);
const vec3 Le = vec3(1.0, 1.0, 1.0);

// Vectores matriz reflexion
uniform vec3 objectColor;
const vec3 Ka = vec3(0.5, 0.5, 0.5);
const vec3 Kd = vec3(0.5, 0.5, 0.5);
const vec3 Ke = vec3(0.5, 0.5, 0.5);

// Constante para calcular intensidad especular
const float alpha = 50.0;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);

}