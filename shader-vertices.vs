#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 objectColor;
uniform bool esSol;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Guardamos la posición del objeto en el mundo 3D
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // transformar las normales al marco del mundo (mat3 porque en las normales nos immporta la direccion unicamente (no importa la traslación))
    Normal = mat3(transpose(inverse(model))) * aNormal;

    if (esSol) {
        float factor = abs(aPos.y); 
        Color = mix(objectColor * 0.5, objectColor, factor);
    } else {
        Color = objectColor;
    }
}