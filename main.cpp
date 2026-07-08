#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_s.h"

// ==========================================
// 1. DECLARACIÓN DE FUNCIONES GLOBALES
// ==========================================
GLFWwindow* initGLFW(int width, int height);
bool initGLAD();
void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// STRUCTS para lógica geométrica/esferas
struct Vec3 { float x, y, z; };
struct Vertex { Vec3 posicion;};
struct Triangle { Vertex v1, v2, v3; };

void normalizar(Vec3& v);
Vertex punto_medio(const Vertex& v1, const Vertex& v2);
void dividir_triangulo(const Triangle& t, int n, std::vector<Triangle>& triangulosFinales);

// ==========================================
// 2. ESTRUCTURAS Y CLASES
// ==========================================
struct Transform {
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f); 
  glm::vec3 scale    = glm::vec3(1.0f);
};

class GameObject {
  protected:
    glm::vec3 color;
    Transform transform;
    bool active = true;

  public:
    virtual void Update(float dt) {}
    virtual ~GameObject() = default;

    // Getters y Setters 
    void SetPosition(const glm::vec3& position) { transform.position = position; }
    glm::vec3 GetPosition() const { return transform.position; }

    void SetRotation(const glm::vec3& rotation) { transform.rotation = rotation; }
    glm::vec3 GetRotation() const { return transform.rotation; }

    void SetScale(const glm::vec3& scale) { transform.scale = scale; }
    glm::vec3 GetScale() const { return transform.scale; }

    bool IsActive() const { return active; }
    void SetActive(bool value) { active = value; }

    void SetColor(const glm::vec3& color_update) {color = color_update;}
    glm::vec3 GetColor() const { return color; }

    // Calcula y devuelve la matriz de modelo
    glm::mat4 GetModelMatrix() const {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, transform.position);
      
      // Rotación consecutiva en X, Y, Z
      model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
      
      model = glm::scale(model, transform.scale);
      return model;
    }
};

class Pelota : public GameObject {
  private:
    glm::vec3 velocidad;
    float radio;
  public:
    Pelota() {
      velocidad = glm::vec3(7.0f, 9.0f, 0.0f); //movimiento diagonal
      radio = 0.4f;    //mitad de la escala (0.8)
    }

  void Update (float dt) override {
    glm::vec3 posActual = GetPosition();
    posActual += velocidad * dt; // Movimiento basado en tiempo

    // --- FÍSICAS DE REBOTE EN PAREDES ---
    // Pared Izquierda (borde interno aprox en X = -10.0)
    if (posActual.x - radio < -10.0f) {
        posActual.x = -10.0f + radio;
        velocidad.x = -velocidad.x;
    }
    // Pared Derecha (borde interno aprox en X = 10.0)
    if (posActual.x + radio > 10.0f) {
        posActual.x = 10.0f - radio;
        velocidad.x = -velocidad.x;
    }
    // Pared Superior (borde interno aprox en Y = 24.0)
    if (posActual.y + radio > 24.0f) {
        posActual.y = 24.0f - radio;
        velocidad.y = -velocidad.y;
    }
    // Suelo temporal (Para no perder la pelota mientras programamos la paleta)
    if (posActual.y - radio < -14.0f) {
        posActual.y = -14.0f + radio;
        velocidad.y = -velocidad.y;
    }

    SetPosition(posActual);
  }
};

// ==========================================
// 3. CONFIGURACIONES GLOBALES
// ==========================================
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

// ==========================================
// 4. FUNCIÓN PRINCIPAL (MAIN)
// ==========================================
int main() {

  GLFWwindow* window = initGLFW(SCR_WIDTH, SCR_HEIGHT);
  if (!window) return -1;

  if (!initGLAD()) return -1;
  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  Shader ourShader("shader-vertices.vs", "shader-fragmentos.fs");

  // Cubo Maestro
  Vertex verticesCubo[36] = {
    // Cara Trasera
    {{-0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f,  0.5f, -0.5f}},
    {{ 0.5f,  0.5f, -0.5f}}, {{-0.5f,  0.5f, -0.5f}}, {{-0.5f, -0.5f, -0.5f}},
    // Cara Frontal
    {{-0.5f, -0.5f,  0.5f}}, {{ 0.5f, -0.5f,  0.5f}}, {{ 0.5f,  0.5f,  0.5f}},
    {{ 0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}}, {{-0.5f, -0.5f,  0.5f}},
    // Cara Izquierda
    {{-0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f, -0.5f}}, {{-0.5f, -0.5f, -0.5f}},
    {{-0.5f, -0.5f, -0.5f}}, {{-0.5f, -0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}},
    // Cara Derecha
    {{ 0.5f,  0.5f,  0.5f}}, {{ 0.5f,  0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}},
    {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f,  0.5f}}, {{ 0.5f,  0.5f,  0.5f}},
    // Cara Inferior
    {{-0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f,  0.5f}},
    {{ 0.5f, -0.5f,  0.5f}}, {{-0.5f, -0.5f,  0.5f}}, {{-0.5f, -0.5f, -0.5f}},
    // Cara Superior
    {{-0.5f,  0.5f, -0.5f}}, {{ 0.5f,  0.5f, -0.5f}}, {{ 0.5f,  0.5f,  0.5f}},
    {{ 0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f, -0.5f}}
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCubo), verticesCubo, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // configuracion tetraedro
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

  unsigned int VBO_esfera, VAO_esfera;
  glGenVertexArrays(1, &VAO_esfera);
  glGenBuffers(1, &VBO_esfera);

  //vertices
  glBindVertexArray(VAO_esfera);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_esfera);
  glBufferData(GL_ARRAY_BUFFER, verticesEsfera.size() * sizeof(Vertex), verticesEsfera.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);


  // --- INSTANCIACIÓN DE LAS PAREDES ---
  GameObject paredIzquierda;
  paredIzquierda.SetPosition(glm::vec3(-10.5f, 0.0f, 0.0f));
  paredIzquierda.SetScale(glm::vec3(1.0f, 50.0f, 1.0f));
  paredIzquierda.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));

  GameObject paredDerecha;
  paredDerecha.SetPosition(glm::vec3(10.5f, 0.0f, 0.0f));
  paredDerecha.SetScale(glm::vec3(1.0f, 50.0f, 1.0f));
  paredDerecha.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));

  GameObject paredSuperior;
  paredSuperior.SetPosition(glm::vec3(0.0f, 24.5f, 0.0f));
  paredSuperior.SetScale(glm::vec3(20.0f, 1.0f, 1.0f));
  paredSuperior.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));

  // --- INSTANCIACIÓN DE LA PELOTA ---
  Pelota pelota;
  pelota.SetPosition(glm::vec3(0.0f, -5.0f, 0.0f));
  pelota.SetScale(glm::vec3(0.8f, 0.8f, 0.8f));
  pelota.SetColor(glm::vec3(1.0f, 0.5f, 0.0f)); // Naranja

  // --- GENERA LA MATRIZ DE BLOQUES (GRID) ---
  std::vector<GameObject> bloques;
  int rows = 5;
  int columns = 5;
  float stepHorizontal = 4.0f; 
  float stepVertical = 3.0f;

  glm::vec3 coloresPorColumna[5] = {
    glm::vec3(1.0f, 0.0f, 0.0f), // Columna 0: Rojo
    glm::vec3(1.0f, 0.6f, 0.0f), // Columna 1: Naranja
    glm::vec3(1.0f, 1.0f, 0.0f), // Columna 2: Amarillo
    glm::vec3(0.0f, 1.0f, 0.0f), // Columna 3: Verde
    glm::vec3(0.0f, 0.6f, 1.0f)  // Columna 4: Azul 
  };

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      GameObject bloque;
      
      float posX = -8.0f + (c * stepHorizontal);
      float posY = 20.0f - (r * stepVertical);
      
      bloque.SetPosition(glm::vec3(posX, posY, 0.0f));
      bloque.SetScale(glm::vec3(3.5f, 1.0f, 1.0f)); 

      bloque.SetColor(coloresPorColumna[c]);
      
      bloques.push_back(bloque);
    }
  }

  // --- PALETA ---
  GameObject paleta;
  paleta.SetPosition(glm::vec3(0.0f, -12.0f, 0.0f));
  paleta.SetScale(glm::vec3(8.0f, 1.0f, 1.0f));
  paleta.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));

  unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
  unsigned int objectColorLoc = glGetUniformLocation(ourShader.ID, "objectColor");

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  // Render Loop
  while (!glfwWindowShouldClose(window)) {
    // 1. Calcular el tiempo transcurrido (Delta Time)
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // 2. Actualizar la lógica y posición de la pelota
    pelota.Update(deltaTime);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();

    int currentWidth, currentHeight;
    glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
    if (currentHeight == 0) currentHeight = 1; 
    float aspectRatio = (float)currentWidth / (float)currentHeight;

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, -25.0f, 15.0f), 
      glm::vec3(0.0f, 0.0f, 0.0f),    
      glm::vec3(0.0f, 1.0f, 0.0f)     
    );

    unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);

    // --- DIBUJADO DE PAREDES --
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredIzquierda.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredIzquierda.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredDerecha.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredDerecha.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredSuperior.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredSuperior.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // --- RENDERIZADO DE GRID ---
    for (const auto& bloque : bloques) {
      if (bloque.IsActive()) {
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(bloque.GetColor()));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bloque.GetModelMatrix()));
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
    }

    // --- PALETA ---
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paleta.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paleta.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //  PELOTA
    glBindVertexArray(VAO_esfera); // ¡Cambiamos al molde de la esfera!
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(pelota.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pelota.GetModelMatrix()));
    // Dibujamos todos los vértices generados dinámicamente por la subdivisión
    glDrawArrays(GL_TRIANGLES, 0, verticesEsfera.size());


    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glfwTerminate();
  return 0;
}

// ==========================================
// 5. IMPLEMENTACIÓN DE FUNCIONES AUXILIARES GLOBALES
// ==========================================
GLFWwindow* initGLFW(int width, int height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); 
  
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
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
  }
  return true;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void normalizar(Vec3 &v) {
  float longitud = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (longitud != 0) {
    v.x /= longitud; v.y /= longitud; v.z /= longitud;
  }
}

Vertex punto_medio(const Vertex& v1, const Vertex& v2) {
  Vertex p;
  p.posicion.x = (v1.posicion.x + v2.posicion.x) / 2.0f;
  p.posicion.y = (v1.posicion.y + v2.posicion.y) / 2.0f;
  p.posicion.z = (v1.posicion.z + v2.posicion.z) / 2.0f;
  //p.color = v1.color; 
  return p;
}

void dividir_triangulo(const Triangle& t, int n, std::vector<Triangle>& triangulosFinales) {
  if (n <= 0) {
    triangulosFinales.push_back(t);
    return;
  }
  Vertex pmAB = punto_medio(t.v1, t.v2);
  Vertex pmBC = punto_medio(t.v2, t.v3);
  Vertex pmCA = punto_medio(t.v3, t.v1);

  normalizar(pmAB.posicion);
  normalizar(pmBC.posicion);
  normalizar(pmCA.posicion);

  Triangle t1 = {t.v1, pmAB, pmCA};
  Triangle t2 = {t.v2, pmBC, pmAB};
  Triangle t3 = {t.v3, pmCA, pmBC};
  Triangle t4 = {pmAB, pmBC, pmCA};

  dividir_triangulo(t1, n-1, triangulosFinales);
  dividir_triangulo(t2, n-1, triangulosFinales);
  dividir_triangulo(t3, n-1, triangulosFinales);
  dividir_triangulo(t4, n-1, triangulosFinales);
}