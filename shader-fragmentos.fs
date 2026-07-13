#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color; // color real del cubo 
in vec3 LocalPos;

uniform vec3 lightPos;   // Posición de la pelota
uniform vec3 lightColor; 
uniform bool esSol;     
uniform vec3 viewPos;    // posición de la cámara (necesaria para el brillo)

// Matriz L 
const vec3 La = vec3(0.5, 0.5, 0.5);
const vec3 Ld = vec3(15.0, 15.0, 15.0);
const vec3 Le = vec3(5.0, 5.0, 5.0);

// Matriz k - Constantes de Material
const vec3 Ke = vec3(0.3, 0.3, 0.3); // Reflejo blanco
const float alpha = 64.0;            
//ka y kd sera igual al color del cubo, ya que cada cubo tiene un color diferente, y se pasa como variable de entrada al shader

void main() {
  // Si este objeto es la pelota
  if(esSol) {
    // atan(z, x) nos da un ángulo radial alrededor de la pelota.
      float angulo = atan(LocalPos.z, LocalPos.x);
      
      // Multiplicamos por 4 para tener 4 rayas rojas y 4 blancas
      if (sin(angulo * 4.0) > 0.0) {
          FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Franja Roja
      } else {
          FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Franja Blanca
      }
      return;
  }

  // Si no es la pelota, calculamos cómo le pega la luz
  vec3 Ka = Color; 
  vec3 Kd = Color;

  //calculo de vectores
  vec3 n = normalize(Normal);
  vec3 l = normalize(lightPos - FragPos);
  vec3 v = normalize(viewPos - FragPos);
  vec3 r = normalize(2.0 * dot(n, l) * n - l);

  // ILuminación ambiental
  vec3 Ia = La * Ka;

  float dist = pow(lightPos.x - FragPos.x,2.0) + pow(lightPos.y - FragPos.y,2.0) + pow(lightPos.z - FragPos.z,2.0);
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
  
   FragColor = vec4(colorFinal, 1.0);
}