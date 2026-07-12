#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 lightPos;   // Posición de la pelota
uniform vec3 lightColor; 
uniform bool esSol;      // ¡Nuestro interruptor mágico!

void main() {
    // Si este objeto es la pelota, se dibuja con su color puro (como un sol)
    if(esSol) {
        FragColor = vec4(Color, 1.0);
        return; 
    }

    // --- Si NO es el sol, calculamos cómo le pega la luz ---

    // 1. Luz Ambiente (muy bajita para que el juego se vea oscuro lejos de la pelota)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // 2. Luz Difusa
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 3. Atenuación (La luz muere con la distancia)
    // Calculamos la distancia entre este píxel y la pelota
    float distance = length(lightPos - FragPos);
    // Fórmula para que la luz caiga suavemente (puedes ajustar el 0.05 para que alumbre más o menos lejos)
    float attenuation = 1.0 / (1.0 + 0.05 * (distance * distance));
            
    // Aplicamos la atenuación a la luz ambiente y difusa
    ambient *= attenuation;
    diffuse *= attenuation;

    // 4. Resultado Final
    vec3 result = (ambient + diffuse) * Color;
    FragColor = vec4(result, 1.0);
}