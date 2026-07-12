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
// 1. ESTRUCTURAS Y CLASES
// ==========================================

// AABB = "Axis Aligned Bounding Box".
// Es una caja imaginaria alineada con los ejes X, Y, Z que usamos
// para saber si dos objetos se están tocando (colisión simple).
struct AABB {
  glm::vec3 min;
  glm::vec3 max;
};

// Transform agrupa la posición, rotación y escala de un objeto.
// Separarlo en su propia estructura evita repetir estas 3 variables
// en cada clase que necesite ubicarse en el mundo 3D.
struct Transform {
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f); 
  glm::vec3 scale    = glm::vec3(1.0f);
};

// GameObject es la clase base de todo lo que se dibuja en el juego
// (paredes, bloques, paleta, pelota). Contiene lo que TODOS comparten:
// posición/rotación/escala, color, si está activo, y cómo calcular
// su matriz de modelo y su caja de colisión.
class GameObject {
  protected:
    glm::vec3 color;
    Transform transform;
    bool active = true;

  public:
    // Update() se sobreescribe en las clases hijas que necesitan
    // moverse o comportarse de forma especial (Paleta, Pelota).
    // Las paredes y bloques no la necesitan, por eso queda vacía aquí.
    virtual void Update() {}
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

    // Caja de colisión por defecto: un cubo centrado en la posición,
    // del tamaño de la escala. La Pelota la redefine porque es una esfera.
    virtual AABB GetCollider() const {
      glm::vec3 halfExtents = transform.scale * 0.5f;
      return { transform.position - halfExtents, transform.position + halfExtents };
    }

    // Calcula la matriz de modelo (traslada, rota y escala el cubo base
    // para convertirlo en este objeto concreto dentro del mundo).
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

// Paleta: la controla el jugador. Solo se mueve en X (izquierda/derecha)
// y en Z (adelante/atrás), y no puede salir del área de juego.
class Paleta : public GameObject {
  public:
    void Update() override {
      glm::vec3 pos = GetPosition();
      glm::vec3 mitadEscala = GetScale() * 0.5f;

      // --- Límite eje X (paredes izquierda/derecha en -10 / 10) ---
      const float xMin = -10.0f;
      const float xMax =  10.0f;

      if (pos.x - mitadEscala.x < xMin)
          pos.x = xMin + mitadEscala.x;
      if (pos.x + mitadEscala.x > xMax)
          pos.x = xMax - mitadEscala.x;

      // --- Límite eje Z (mismo rango que usa la pelota: 0 a 2) ---
      const float zMin = 0.0f;
      const float zMax = 2.0f;

      if (pos.z - mitadEscala.z < zMin)
          pos.z = zMin + mitadEscala.z;
      if (pos.z + mitadEscala.z > zMax)
          pos.z = zMax - mitadEscala.z;

      SetPosition(pos);
    }

    // Devuelve la paleta a su posición inicial. Se usa al reiniciar el juego.
    void Reiniciar(const glm::vec3& posicionInicial) {
      SetPosition(posicionInicial);
    }
};

// Pelota: se mueve sola a velocidad constante y rebota en las paredes.
// A diferencia de los demás objetos, su colisión es una esfera, no un cubo.
class Pelota : public GameObject {
  private:
    glm::vec3 velocidad;
    float radio;

    // Velocidad con la que arranca la pelota siempre que se (re)inicia el juego.
    // Se guarda en un solo lugar para no repetir estos valores en varias partes.
    static glm::vec3 VelocidadInicial() { return glm::vec3(7.0f, 9.0f, 1.0f); }

  public:
    Pelota() {
      velocidad = VelocidadInicial();
      radio = 0.4f;
    }
    void InvertirY() { velocidad.y = -velocidad.y; }

    AABB GetCollider() const override {
      glm::vec3 r(radio, radio, radio);
      return { GetPosition() - r, GetPosition() + r };
    }

    void Update () override {
      glm::vec3 posActual = GetPosition();
      posActual += velocidad * 0.02f;

      // --- Colisión con paredes laterales (izquierda/derecha, eje X) ---
      if (posActual.x - radio < -10.0f) {
          posActual.x = -10.0f + radio;
          velocidad.x = -velocidad.x;
      }
      if (posActual.x + radio > 10.0f) {
          posActual.x = 10.0f - radio;
          velocidad.x = -velocidad.x;
      }

      // --- Colisión con la pared del fondo (arriba, donde están los bloques) ---
      if (posActual.y + radio > 24.0f) {
          posActual.y = 24.0f - radio;
          velocidad.y = -velocidad.y;
      }
 

      // --- Colisión en el eje Z (profundidad) ---
      const float zMin = 0.0f;   // "piso" en profundidad, no puede pasar a negativo
      const float zMax = 2.0f;   // límite invisible equivalente al "techo"

      if (posActual.z - radio < zMin) {
          posActual.z = zMin + radio;
          velocidad.z = -velocidad.z;
      }
      if (posActual.z + radio > zMax) {
          posActual.z = zMax - radio;
          velocidad.z = -velocidad.z;
      }

      SetPosition(posActual);
    }

    // Devuelve la pelota a su posición y velocidad inicial.
    // Se usa al reiniciar el juego (cuando se pierde).
    void Reiniciar(const glm::vec3& posicionInicial) {
      SetPosition(posicionInicial);
      velocidad = VelocidadInicial();
    }
};


// ==========================================
// 2. DECLARACIÓN DE FUNCIONES GLOBALES
// ==========================================
GLFWwindow* initGLFW(int width, int height);
bool initGLAD();
void processInput(GLFWwindow *window, GameObject& paleta);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool CheckCollision(const GameObject& a, const GameObject& b);

// Construye (o reconstruye) la matriz de bloques del juego.
void GenerarBloques(std::vector<GameObject>& bloques);

// Vuelve a dejar el juego como al principio: pelota, paleta y bloques.
void ReiniciarJuego(Pelota& pelota, Paleta& paleta, std::vector<GameObject>& bloques);

// STRUCTS para lógica geométrica/esferas
struct Vec3 { float x, y, z; };
struct Vertex { 
  Vec3 posicion;
  Vec3 normal;
};
struct Triangle { 
  Vertex v1, v2, v3; };

void normalizar(Vec3& v);
Vertex punto_medio(const Vertex& v1, const Vertex& v2);
void dividir_triangulo(const Triangle& t, int n, std::vector<Triangle>& triangulosFinales);

// ==========================================
// 3. CONFIGURACIONES GLOBALES
// ==========================================
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

// Posiciones iniciales de la pelota y la paleta. Se usan tanto al empezar
// el juego como cada vez que se reinicia, para no repetir estos valores.
const glm::vec3 POSICION_INICIAL_PELOTA(0.0f, -5.0f, 0.0f);
const glm::vec3 POSICION_INICIAL_PALETA(0.0f, -12.0f, 0.0f);

// Si la pelota cruza esta línea en Y es porque pasó detrás de la paleta
// sin ser golpeada: el jugador pierde y el juego se reinicia.
const float LIMITE_DERROTA_Y = -14.0f;

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

  /*
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
  */
 Vertex verticesCubo[36] = {
    // Cara Trasera (Normal apunta hacia -Z)
    {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}}, 
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}}, 
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}}, 
    {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}}, 
    {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}},

    // Cara Frontal (Normal apunta hacia +Z)
    {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, 
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, 
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, 
    {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, 
    {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}},

    // Cara Izquierda (Normal apunta hacia -X)
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}}, 
    {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}}, 
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}}, 
    {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}}, 
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},

    // Cara Derecha (Normal apunta hacia +X)
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}}, 
    {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}}, 
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}}, 
    {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}}, 
    {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}},

    // Cara Inferior (Normal apunta hacia -Y)
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}}, 
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}}, 
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}}, 
    {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}}, 
    {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}},

    // Cara Superior (Normal apunta hacia +Y)
    {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}}, 
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}}, 
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}}, 
    {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}}, 
    {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}}
  };

  //VAO del cubo
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCubo), verticesCubo, GL_STATIC_DRAW);
  //posicion
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  //normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);


  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // configuracion tetraedro
  Vertex vertices[4] = {
    {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // Vértice A

    {{0.0f, 0.942809f, -0.333333f}, {0.0f, 0.942809f, -0.333333f}}, // Vértice B

    {{-0.816497f, -0.471405f, -0.333333f}, {-0.816497f, -0.471405f, -0.333333f}}, // Vértice C

    {{0.816497f, -0.471405f, -0.333333f}, {0.816497f, -0.471405f, -0.333333f}}, // Vértice D
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

  //VAO esfera
  unsigned int VBO_esfera, VAO_esfera;
  glGenVertexArrays(1, &VAO_esfera);
  glGenBuffers(1, &VBO_esfera);

  //vertices
  glBindVertexArray(VAO_esfera);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_esfera);
  glBufferData(GL_ARRAY_BUFFER, verticesEsfera.size() * sizeof(Vertex), verticesEsfera.data(), GL_STATIC_DRAW);
  //posicion
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  //normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);


  // --- INSTANCIACIÓN DE LAS PAREDES ---
  GameObject paredIzquierda;
  paredIzquierda.SetPosition(glm::vec3(-10.5f, 0.0f, 0.0f));
  paredIzquierda.SetScale(glm::vec3(0.5f, 50.0f, 3.0f));
  paredIzquierda.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));

  GameObject paredDerecha;
  paredDerecha.SetPosition(glm::vec3(10.5f, 0.0f, 0.0f));
  paredDerecha.SetScale(glm::vec3(0.5f, 50.0f, 3.0f));
  paredDerecha.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));

  GameObject paredFondo;
  paredFondo.SetPosition(glm::vec3(0.0f, 25.0f, 0.0f));
  paredFondo.SetScale(glm::vec3(21.5f, 0.5f, 3.0f));
  paredFondo.SetColor(glm::vec3(0.4f, 0.4f, 0.4f));

  GameObject profundidad;
  profundidad.SetPosition(glm::vec3(0.0f, 0.0f, -1.0f));
  profundidad.SetScale(glm::vec3(20.0f, 50.0f, 1.0f));
  profundidad.SetColor(glm::vec3(0.4f, 0.4f, 0.4f));

  // --- INSTANCIACIÓN DE LA PELOTA ---
  Pelota pelota;
  pelota.SetPosition(POSICION_INICIAL_PELOTA);
  pelota.SetScale(glm::vec3(0.8f, 0.8f, 0.8f));
  pelota.SetColor(glm::vec3(1.0f, 0.5f, 0.0f)); // Naranja

  // --- GENERA LA MATRIZ DE BLOQUES (GRID) ---
  // La construcción del grid vive en GenerarBloques() para no repetir
  // este mismo código en ReiniciarJuego() cuando el jugador pierde.
  std::vector<GameObject> bloques;
  GenerarBloques(bloques);

  // --- PALETA ---
  Paleta paleta;
  paleta.SetPosition(POSICION_INICIAL_PALETA);
  paleta.SetScale(glm::vec3(8.0f, 1.0f, 1.0f));
  paleta.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));

  unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
  unsigned int objectColorLoc = glGetUniformLocation(ourShader.ID, "objectColor");

  // NUEVAS UBICACIONES
  unsigned int lightPosLoc = glGetUniformLocation(ourShader.ID, "lightPos");
  unsigned int lightColorLoc = glGetUniformLocation(ourShader.ID, "lightColor");
  unsigned int esSolLoc = glGetUniformLocation(ourShader.ID, "esSol");
  
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;

  // Render Loop
  while (!glfwWindowShouldClose(window)) {
 

    processInput(window, paleta);

    pelota.Update();
    paleta.Update();

    // --- CONDICIÓN DE DERROTA ---
    // Si la pelota bajó más allá del límite trasero de la paleta, significa
    // que la paleta no llegó a golpearla: se perdió el juego y se reinicia
    // todo (pelota, paleta y bloques) a su estado original.
    if (pelota.GetPosition().y < LIMITE_DERROTA_Y) {
      ReiniciarJuego(pelota, paleta, bloques);
    }

    if (CheckCollision(pelota, paleta)) {
      pelota.InvertirY();
    }

    for (auto& bloque : bloques) {
      if (bloque.IsActive() && CheckCollision(pelota, bloque)) {
        bloque.SetActive(false);
        pelota.InvertirY();
        break;
      }
    }
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();
    // La fuente de luz es exactamente la posición actual de la pelota
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(pelota.GetPosition()));
    
    // Luz blanca para que los bloques mantengan sus propios colores naturales
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

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

    glUniform1i(esSolLoc, 0);
    // --- DIBUJADO DE PAREDES --
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredIzquierda.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredIzquierda.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredDerecha.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredDerecha.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(paredFondo.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredFondo.GetModelMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(profundidad.GetColor()));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(profundidad.GetModelMatrix()));
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
    glUniform1i(esSolLoc, 1);
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

void processInput(GLFWwindow *window, GameObject& paleta) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  glm::vec3 pos = paleta.GetPosition();
  
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    pos.x -= 0.3f;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    pos.x += 0.3f;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    pos.z += 0.3f;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    pos.z -= 0.3f;

  paleta.SetPosition(pos);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Crea la cuadrícula de 5x5 bloques con sus posiciones y colores por columna.
// Se llama una vez al iniciar el programa, y otra vez cada vez que el
// jugador pierde y el juego se reinicia (ver ReiniciarJuego).
void GenerarBloques(std::vector<GameObject>& bloques) {
  bloques.clear(); // por si ya había bloques de una partida anterior

  const int filas = 5;
  const int columnas = 5;
  const float pasoHorizontal = 4.0f;
  const float pasoVertical = 3.0f;

  glm::vec3 coloresPorColumna[5] = {
    glm::vec3(1.0f, 0.0f, 0.0f), // Columna 0: Rojo
    glm::vec3(1.0f, 0.6f, 0.0f), // Columna 1: Naranja
    glm::vec3(1.0f, 1.0f, 0.0f), // Columna 2: Amarillo
    glm::vec3(0.0f, 1.0f, 0.0f), // Columna 3: Verde
    glm::vec3(0.0f, 0.6f, 1.0f)  // Columna 4: Azul
  };

  for (int fila = 0; fila < filas; ++fila) {
    for (int columna = 0; columna < columnas; ++columna) {
      GameObject bloque;

      float posX = -8.0f + (columna * pasoHorizontal);
      float posY = 20.0f - (fila * pasoVertical);

      bloque.SetPosition(glm::vec3(posX, posY, 0.0f));
      bloque.SetScale(glm::vec3(3.5f, 1.0f, 1.0f));
      bloque.SetColor(coloresPorColumna[columna]);

      bloques.push_back(bloque);
    }
  }
}

// Reinicia el juego por completo: se llama cuando la pelota pasa detrás
// de la paleta sin ser golpeada. Deja la pelota, la paleta y los bloques
// exactamente como estaban al iniciar el programa.
void ReiniciarJuego(Pelota& pelota, Paleta& paleta, std::vector<GameObject>& bloques) {
  pelota.Reiniciar(POSICION_INICIAL_PELOTA);
  paleta.Reiniciar(POSICION_INICIAL_PALETA);
  GenerarBloques(bloques);

  std::cout << "Perdiste. Juego reiniciado.\n";
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
  p.normal = p.posicion;
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

// Compara las cajas de colisión (AABB) de dos objetos y devuelve true
// si se están tocando en los 3 ejes (X, Y, Z) al mismo tiempo.
// Esta es la única función de colisión del juego: tanto pelota-paleta
// como pelota-bloque la usan, así no hay dos versiones distintas del
// mismo cálculo.
bool CheckCollision(const GameObject& a, const GameObject& b) {
  AABB cajaA = a.GetCollider();
  AABB cajaB = b.GetCollider();
  return (cajaA.min.x <= cajaB.max.x && cajaA.max.x >= cajaB.min.x) &&
         (cajaA.min.y <= cajaB.max.y && cajaA.max.y >= cajaB.min.y) &&
         (cajaA.min.z <= cajaB.max.z && cajaA.max.z >= cajaB.min.z);
}