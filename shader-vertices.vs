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

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Guardamos la posición del objeto en el mundo 3D
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Truco temporal: Usamos la posición local como "Normal". 
    // La matriz inversa evita que la luz se deforme si estiras el objeto.
    //Normal = mat3(transpose(inverse(model))) * aPos;
    Normal =  aNormal;

    Color = objectColor;
}