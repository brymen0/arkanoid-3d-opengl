#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "shader_s.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLFWwindow* initGLFW(int width, int height);
bool initGLAD();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

struct Vec3 {
  float x, y, z;
};

struct Vertex {
  Vec3 posicion;
  Vec3 color;
};

struct Triangle {
  Vertex v1, v2, v3;
};


void normalizar(Vec3& v);
void dividir_triangulo(const Triangle& t, int n, std::vector<Triangle>& triangulosFinales);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

int main () {
  GLFWwindow* window = initGLFW(SCR_WIDTH, SCR_HEIGHT);
  if (!window) return -1;

  if (!initGLAD()) return -1;
  glEnable(GL_DEPTH_TEST);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  //ver modo alambre
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // Compilar y usar shaders
  Shader ourShader("shader-vertices.vs", "shader-fragmentos.fs");

  // Color base 
  Vec3 c = {1.0f, 1.0f, 1.0f};

  // El Cubo Maestro estructurado
  Vertex verticesCubo[36] = {
    // Cara Trasera
    {{-0.5f, -0.5f, -0.5f}, c}, {{ 0.5f, -0.5f, -0.5f}, c}, {{ 0.5f,  0.5f, -0.5f}, c},
    {{ 0.5f,  0.5f, -0.5f}, c}, {{-0.5f,  0.5f, -0.5f}, c}, {{-0.5f, -0.5f, -0.5f}, c},
    // Cara Frontal
    {{-0.5f, -0.5f,  0.5f}, c}, {{ 0.5f, -0.5f,  0.5f}, c}, {{ 0.5f,  0.5f,  0.5f}, c},
    {{ 0.5f,  0.5f,  0.5f}, c}, {{-0.5f,  0.5f,  0.5f}, c}, {{-0.5f, -0.5f,  0.5f}, c},
    // Cara Izquierda
    {{-0.5f,  0.5f,  0.5f}, c}, {{-0.5f,  0.5f, -0.5f}, c}, {{-0.5f, -0.5f, -0.5f}, c},
    {{-0.5f, -0.5f, -0.5f}, c}, {{-0.5f, -0.5f,  0.5f}, c}, {{-0.5f,  0.5f,  0.5f}, c},
    // Cara Derecha
    {{ 0.5f,  0.5f,  0.5f}, c}, {{ 0.5f,  0.5f, -0.5f}, c}, {{ 0.5f, -0.5f, -0.5f}, c},
    {{ 0.5f, -0.5f, -0.5f}, c}, {{ 0.5f, -0.5f,  0.5f}, c}, {{ 0.5f,  0.5f,  0.5f}, c},
    // Cara Inferior
    {{-0.5f, -0.5f, -0.5f}, c}, {{ 0.5f, -0.5f, -0.5f}, c}, {{ 0.5f, -0.5f,  0.5f}, c},
    {{ 0.5f, -0.5f,  0.5f}, c}, {{-0.5f, -0.5f,  0.5f}, c}, {{-0.5f, -0.5f, -0.5f}, c},
    // Cara Superior
    {{-0.5f,  0.5f, -0.5f}, c}, {{ 0.5f,  0.5f, -0.5f}, c}, {{ 0.5f,  0.5f,  0.5f}, c},
    {{ 0.5f,  0.5f,  0.5f}, c}, {{-0.5f,  0.5f,  0.5f}, c}, {{-0.5f,  0.5f, -0.5f}, c}
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCubo), verticesCubo, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
  glEnableVertexAttribArray(1);

  // Configurar el viewport inicial para los graficos coincida con el tamaño de la ventana
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // render loop
  while (!glfwWindowShouldClose(window)) {
    processInput(window);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Limpia el buffer de color y el buffer de profundidad antes de dibujar el nuevo frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();

    // --- 1. OBTENER EL TAMAÑO ACTUAL DE LA VENTANA ---
    int currentWidth, currentHeight;
    glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
    if (currentHeight == 0) currentHeight = 1; // Prevenir división por cero al minimizar
    float aspectRatio = (float)currentWidth / (float)currentHeight;

    // --- 2. MATRIZ DE PROYECCIÓN (Projection) ---
    // glm::perspective(Campo de visión, Aspect Ratio, Cerca, Lejos)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    //  Cámara 
    // Movemos la cámara hacia abajo (Y = -25), la subimos un poco (Z = 15) 
    // y la hacemos mirar hacia el centro del tablero (0, 0, 0)
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, -25.0f, 15.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f),    
        glm::vec3(0.0f, 1.0f, 0.0f)     
    );

    //envia las matrices al shader de vertices
    unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);

    // Obtener la ubicación de la matriz 'model'
    unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");

    // --- PARED IZQUIERDA ---
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-10.5f, 0.0f, 0.0f)); // Izquierda
    model = glm::scale(model, glm::vec3(1.0f, 30.0f, 2.0f));      // Alto y profundo
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // --- PARED DERECHA ---
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(10.5f, 0.0f, 0.0f));  // Derecha
    model = glm::scale(model, glm::vec3(1.0f, 30.0f, 2.0f));      // Alto y profundo
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // --- PARED SUPERIOR (TECHO O FONDO) ---
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 15.5f, 0.0f));  // Arriba
    model = glm::scale(model, glm::vec3(22.0f, 1.0f, 2.0f));      // Ancho para tapar huecos
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Liberar memoria
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
  return 0;
}


// Inicializa GLFW y crea la ventana de OpenGL
GLFWwindow* initGLFW(int width, int height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // maximizar ventana
  GLFWwindow* window = glfwCreateWindow(width, height, "Arkanoid 3D", NULL, NULL);
  if (!window) {
      std::cout << "Error creando ventana\n";
      glfwTerminate();
      return nullptr;
  }

  glfwMakeContextCurrent(window);
  return window;
}

bool initGLAD() {
  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return false;
  }
  return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

//normaliza el vertice proyectandolo en la suoerficie de la esfera de radio 1
void normalizar (Vec3 &v) {
  float longitud = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (longitud != 0) {
    v.x /= longitud;
    v.y /= longitud;
    v.z /= longitud;
  }
}

Vertex punto_medio (const Vertex& v1, const Vertex& v2) { //const para indicar que no se modificara y & para pasar referencia y no necesitar una copia de la variable
  Vertex p;
  p.posicion.x = (v1.posicion.x + v2.posicion.x) / 2.0f;
  p.posicion.y = (v1.posicion.y + v2.posicion.y) / 2.0f;
  p.posicion.z = (v1.posicion.z + v2.posicion.z) / 2.0f;

  return p;
}

void dividir_triangulo (const Triangle& t, int n, std::vector<Triangle>& triangulosFinales) {
  if (n <= 0) {
    triangulosFinales.push_back(t);
    return;
  }
  //          B
  //         / \
  //        /   \
  //       /     \
  //      C-------A 

  //calc puntos medios
  Vertex pmAB, pmBC, pmCA;
  pmAB = punto_medio(t.v1, t.v2);
  pmBC = punto_medio(t.v2, t.v3);
  pmCA = punto_medio(t.v3, t.v1);
  //proyectar a la sup de la esfera
  normalizar(pmAB.posicion);
  normalizar(pmBC.posicion);
  normalizar(pmCA.posicion);

  //crear los nuevos 4 triangulos
  Triangle t1, t2, t3, t4;
  t1 = {t.v1, pmAB, pmCA};
  t2 = {t.v2, pmBC, pmAB};
  t3 = {t.v3, pmCA, pmBC};
  t4 = {pmAB, pmBC, pmCA};

  dividir_triangulo(t1, n-1, triangulosFinales);
  dividir_triangulo(t2, n-1, triangulosFinales);
  dividir_triangulo(t3, n-1, triangulosFinales);
  dividir_triangulo(t4, n-1, triangulosFinales);
}