#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Color;

uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;   
uniform vec3 lightColor;
uniform vec3 viewPos;  

uniform bool esSol;

const vec3 La = vec3(0.5, 0.5, 0.5);
const vec3 Ld = vec3(1.0, 1.0, 1.0);
const vec3 Le = vec3(1.0, 1.0, 1.0);

uniform vec3 objectColor;
const vec3 Ka = vec3(0.5, 0.5, 0.5);
const vec3 Kd = vec3(5.0, 5.0, 5.0);
const vec3 Ke = vec3(5.0, 5.0, 5.0);

const float alpha = 50.0;

void main() {
  vec4 posMundo4 = model * vec4(aPos, 1.0);
  vec3 FragPos = posMundo4.xyz;

  gl_Position = projection * view * posMundo4;

  if (esSol) {
    Color = objectColor;
    return;
  }

  mat3 normalMatrix = mat3(transpose(inverse(model)));
  vec3 N = normalize(normalMatrix * aNormal);

  vec3 vecLuz = lightPos - FragPos;
  // |p - po|^2
  float distancia2 = dot(vecLuz, vecLuz); 
  
  vec3 L = normalize(vecLuz);
  vec3 V = normalize(viewPos - FragPos);
  vec3 R = reflect(-L, N);

  // Ambiental: no se atenúa, representa luz indirecta/ambiente general
  vec3 ambiental = Ka * La;

  // Difusa, atenuada por 1 / |p - po|^2
  float difFactor = max(dot(N, L), 0.0);
  vec3 difusa = (Kd * (Ld * lightColor) * difFactor) / distancia2;

  // Especular, atenuada por 1 / |p - po|^2
  float espFactor = pow(max(dot(R, V), 0.0), alpha);
  vec3 especular = (Ke * (Le * lightColor) * espFactor) / distancia2;

  Color = (ambiental + difusa + especular) * objectColor;
}