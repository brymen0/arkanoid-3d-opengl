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
};

struct Triangle {
  Vertex v1, v2, v3;
};

struct Light {
  Vec3 posicion;

  Vec3 Lambiental;
  Vec3 Ldifusa;
  Vec3 Lespecular;
};

struct Material {
  Vec3 Kambiental;
  Vec3 Kdifusa;
  Vec3 Kespecular;

  float alpha; //brillo
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

  Vertex vertices[4] = {
    {{0.0f, 0.0f, 1.0f}}, // Vértice A

    {{0.0f, 0.942809f, -0.333333f}}, // Vértice B

    {{-0.816497f, -0.471405f, -0.333333f}}, // Vértice C

    {{0.816497f, -0.471405f, -0.333333f}}, // Vértice D
  };

  Triangle caras[4];
  caras[0] = {vertices[0], vertices[1], vertices[2]};
  caras[1] = {vertices[0], vertices[2], vertices[3]};
  caras[2] = {vertices[0], vertices[3], vertices[1]};
  caras[3] = {vertices[1], vertices[2], vertices[3]};

  std::vector<Triangle> triangulosFinales;

  int iteraciones = 4;
  for (int i=0; i<4; i++){
    dividir_triangulo(caras[i], iteraciones, triangulosFinales);
  }

  std::vector<Vertex> verticesEsfera;
  for (const Triangle& t : triangulosFinales) {
    verticesEsfera.push_back(t.v1);
    verticesEsfera.push_back(t.v2);
    verticesEsfera.push_back(t.v3);
  }

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  //vertices
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, verticesEsfera.size() * sizeof(Vertex), verticesEsfera.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

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

    // --- 3. MATRIZ DE VISTA (View / Cámara) ---
    // glm::lookAt(Posición cámara, Punto al que mira, Vector Arriba)
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 4.0f), // Tu cámara real en Z=4
        glm::vec3(0.0f, 0.0f, 0.0f), // Mirando directo al centro de la esfera
        glm::vec3(0.0f, 1.0f, 0.0f)  // Indicamos que Y positivo es "hacia arriba"
    );

    // --- 4. ENVIAR MATRICES AL VERTEX SHADER ---
    unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, verticesEsfera.size());

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

  GLFWwindow* window = glfwCreateWindow(width, height, "Esfera Iluminada V2", NULL, NULL);
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