#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 Color;

uniform mat4 view;
uniform mat4 projection;

const vec3 luzPos = vec3(2.5, 2.5, 2.5);
const vec3 La = vec3(0.3, 0.3, 0.3);
const vec3 Ld = vec3(15.0, 15.0, 15.0);
const vec3 Le = vec3(15.0, 15.0, 15.0);

const vec3 Ka = vec3(0.5, 0.1, 0.1);
const vec3 Kd = vec3(0.8, 0.1, 0.1);
const vec3 Ke = vec3(1.0, 1.0, 1.0);

const float alpha = 32.0;

const vec3 camara = vec3(0.0, 0.0, 4.0);

void main()
{
  vec3 p = aPos;

  // n en una esfera centrada en el origen, la normal es el vértice normalizado
  vec3 n = normalize(p);

  // l = luz - P
  vec3 l = normalize(luzPos - p);

  // v = desde el P hacia la cámara
  vec3 v = normalize(camara - p);

  // reflector = 2 * (l . n) * n - l
  vec3 r = normalize(2.0 * dot(l, n) * n - l);

  // iluminacion ambiental
  vec3 Ia = La * Ka;

  float dist = pow(luzPos.x - p.x,2.0) + pow(luzPos.y - p.y,2.0) + pow(luzPos.z - p.z,2.0);
  float distInverso = 1.0 / dist;

  // iluminacion difusa
  float prodPuntoNL = dot(n, l);
  float diff = max(prodPuntoNL, 0.0);
  vec3 Id = (Kd * Ld * diff) * distInverso;

  // iluminacion especular
  float spec = pow(max(dot(r, v), 0.0), alpha);
  vec3 Ie = Ke * Le * spec * distInverso;

  vec3 colorFinal = Ia + Id +Ie;
  // clamp() es la función nativa de GLSL para limitar valores entre 0 y 1
  colorFinal = clamp(colorFinal, 0.0, 1.0);

  gl_Position = projection * view * vec4(aPos, 1.0);
  Color = colorFinal;
}