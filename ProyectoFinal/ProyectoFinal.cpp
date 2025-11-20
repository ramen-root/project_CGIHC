#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"  


// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 1000;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;


// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Posición base de la farola de calle
glm::vec3 lamparaPos(-28.0f, 0.0f, 0.0f); 
glm::vec3 pointLightPositions[] = {
	lamparaPos + glm::vec3(0.8f, 2.8f, 0.0f), // foco en la cabeza de la farola
	glm::vec3(4.0f,0.0f, 5.0f),
	glm::vec3(4.0f,0.0f, -5.0f),
	glm::vec3(-4.0f,0.0f,  5.0f),
	glm::vec3(-4.0f,0.0f, -5.0f),
	glm::vec3(6.0f,0.0f, 5.0f),
	glm::vec3(6.0f,0.0f, -5.0f)
};

glm::vec3 spotLightPos(0.0f, 3.0f, 0.0f);
glm::vec3 spotLightDir(0.0f, -1.0f, 0.0f);

// Variables para controlar la animación
bool lightsOn = false; // Indica si las luces están encendidas
float animationTime = 0.0f; // Temporizador para controlar la animación
int currentStage = 0; // Estado de la animación
float tiempoColor = glfwGetTime();
float r = 0.8f;
float g = abs(sin(tiempoColor * 5.0f)) * 0.6f + 0.4f;
float b = 0.6f;
bool ventanaDesplazada = false;  // Indica si las ventanas están desplazadas
float ventanaPosZ = 0.0f;  // Posición inicial de las ventanas en el eje z

// KeyFrame de la Puerta de la casa 
// Variables para Animacion de puerta con keyframes
float puertaInicio = 0.0f;                   // Angulo cerrado
float puertaFin = glm::radians(90.0f);       // Angulo abierto
float puertaActual = 0.0f;                   // Angulo interpolado
float puertaT = 0.0f;                        // Progreso 0 → 1
bool puertaAnimando = false;
bool puertaEstaAbierta = false;

// Animacion de la Puerta de la entrada (Estacionamiento)  
bool animPuerta = false;        // Para activar/desactivar la animación
float rotPuerta = 0.0f;         // Ángulo de apertura de las puertas (en grados)
bool puertaMuseoAbierta = false; //estado actual (abierta/cerrada)

// Keyframes
// Posición inicial global de la escena (offset general)
glm::vec3 PosIni(0.0f, 0.0f, 0.0f);   

// Variables ligadas a ese offset
float posX = PosIni.x;
float posY = PosIni.y;
float posZ = PosIni.z;

// --- Coche ---
bool animCoche = false;      // para prender/apagar la animación
bool sentidoCoche = false;   // false = va recto, true = ya dio vuelta
bool faseFinalCoche = false; // nueva bandera para la 3ra fase
float movCocheZ = 0.0f;      // avance hacia adelante/atrás
float movCocheX = 0.0f;      // avance lateral después de la curva
float rotCoche = 0.0f;      // rotación en Y (giro del coche)

// Variables para el kaliz y fuego 
float giro = 0;
float tiempoFuego;
float velocidad = 5.0f;


// Colores para la luz de la farola (punto de luz 0)
glm::vec3 lamparaColores[] = {
	glm::vec3(1.0f, 0.8f, 0.6f),  // cálido
	glm::vec3(0.6f, 0.8f, 1.0f),  // frío
	glm::vec3(0.8f, 0.2f, 0.2f)   // rojizo
};
int indiceColorLampara = 0;  // índice actual


// ---------- MESA + SILLAS TERRAZA ----------
glm::vec3 terrazaPos = glm::vec3(20.0f, 0.0f, -9.0f); // Posición base del set
glm::vec3 terrazaScale = glm::vec3(1.0f);             // Escala global (1 = normal)
float terrazaRotY = 0.0f;                        // Rotación global en Y (grados)

// ---------- CÁMARA DE SEGURIDAD ----------
glm::vec3 camSegPos = glm::vec3(20.0f, 3.5f, 18.3f); // posición base en el mundo
glm::vec3 camSegScale = glm::vec3(1.0f);             // escala global (1 = tamaño normal)
float camSegBaseRotY = 0.0f;                        // rotación fija del soporte (en Y)

// Paneo automático de la cabeza de la cámara
float camSegPanAngle = 0.0f;   // ángulo actual de paneo
float camSegPanSpeed = 25.0f;  // velocidad en grados por segundo
float camSegPanLimit = 45.0f;  // límite +/- del paneo
bool  camSegPanForward = true;   // true = girando hacia +, false = hacia -

// ---------- ARBUSTO LOW POLY ----------
glm::vec3 arbustoPos = glm::vec3(23.0f, 0.0f, 3.0f);  // posición en el mundo
glm::vec3 arbustoScale = glm::vec3(1.0f);                // escala global
float     arbustoRotY = 0.0f;                           // rotación en Y (grados)


// ---------- VENTILADOR DE TECHO ----------
glm::vec3 fanPos = glm::vec3(-5.0f, 3.2f, -8.5f); 
glm::vec3 fanScale = glm::vec3(0.5f);             // escala global
float     fanRotY = 0.0f;                        // rotación fija en Y (orientación del ventilador)
float     fanBladesAngle = 0.0f;                   // ángulo actual de las aspas
float     fanBladesSpeed = 180.0f;                 // velocidad de giro (grados por segundo)


// ---------- RELOJ DE PARED ----------
glm::vec3 clockPos = glm::vec3(-11.5f, 1.7f, -3.5f); // posición en la pared
glm::vec3 clockScale = glm::vec3(0.8f);               // escala global
float     clockRotY = 90.0f;                         

// ángulos de manecillas
float clockMinuteAngle = 19.0f;  // manecilla larga (segundero/minutero rápido)
float clockHourAngle = 8.0f;  // manecilla corta

// velocidades (grados por segundo)
float clockMinuteSpeed = 60.0f; // más rápido
float clockHourSpeed = 10.0f; // más lento



float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 Light1 = glm::vec3(1.0f);
glm::vec3 Light2 = glm::vec3(2.0f);
glm::vec3 Light3 = glm::vec3(2.5f);
glm::vec3 Light4 = glm::vec3(3.0f);
glm::vec3 Light5 = glm::vec3(3.5f);
glm::vec3 Light6 = glm::vec3(4.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame


//KEYFRAME PUERTA
struct KeyFrame {
	glm::vec3 position;
	glm::vec3 rotation;

	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float incX;		//Variable para IncrementoX
	float incY;		//Variable para IncrementoY
	float incZ;		//Variable para IncrementoZ
};

std::vector<KeyFrame> sillaKeyframes = {
	{ glm::vec3(1.16f, -0.15f, -0.73f), glm::vec3(0.0f, 0.0f, 0.0f) },      // posicion inicial
	{ glm::vec3(1.16f, -0.15f, -0.73f), glm::vec3(0.0f, 90.0f, 0.0f) },     // giro 90
	{ glm::vec3(1.20f, -0.15f, -0.50f), glm::vec3(0.0f, 180.0f, 0.0f) }     // se mueve y se gira más
};

int currentKeyframe = 0;
float interp = 0.0f;
bool animarSilla = false;



int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ProyectoFinal", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader tvNoiseShader("Shaders/tv.vs", "Shaders/tv.frag");
	Shader SkyBoxshader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");
	Shader colorShader("shaders/colorShader.vs", "shaders/colorShader.frag"); 
	Shader AnimFuego("Shaders/AnimFuego.vs", "Shaders/AnimFuego.frag");

	// MODELOS 3D
	Model Mono((char*)"Models/oscar/mono.obj");
	Model Kaliz((char*)"Models/oscar/kaliz.obj");
	Model Fuego((char*)"Models/oscar/Fuego.obj");
	Model Agua((char*)"Models/oscar/Agua.obj");
	Model BastonSom((char*)"Models/oscar/bastonSom.obj");
	Model CarroPared((char*)"Models/oscar/carroPared.obj");
	Model LLantaDer((char*)"Models/oscar/LlantaDer.obj");
	Model LlantaIzq((char*)"Models/oscar/LlantaIzq.obj");
	Model Estructura((char*)"Models/Estructura/Estructura.obj");
	Model Cuadros1((char*)"Models/Casa/cuadros1.obj");
	Model Cuadros2((char*)"Models/Casa/cuadros2.obj");
	Model casa((char*)"Models/Casa/casa.obj");

	Model Cuadro1((char*)"Models/cuadros/cuadro1.obj");
	Model Cuadros3((char*)"Models/cuadros/cuadros3.obj");
	Model Barra((char*)"Models/barra/barra.obj");
	Model Statua((char*)"Models/Statua/statua.obj");
	Model Statua2((char*)"Models/Statua/statua2.obj");
	Model Statua3((char*)"Models/Statua/statua3.obj");
	Model Statua4((char*)"Models/Statua/statua4.obj");
	Model Statua5((char*)"Models/Statua/statua5.obj");
	Model Statua6((char*)"Models/Statua/statua6.obj");

	Model tv((char*)"Models/tv_2/tv.obj");
	Model barra((char*)"Models/barra/barra.obj");
	Model puerta((char*)"Models/puerta/puerta.obj");
	Model ventanas((char*)"Models/ventanas/ventanas.obj");
	Model entrada((char*)"Models/entrada/entrada.obj");

	Model PuertaDer((char*)"Models/puerta/PuertaDer.obj");
	Model PuertaIzq((char*)"Models/puerta/PuertaIzq.obj");
	Model Coches((char*)"Models/Coches/Coches.obj");
	Model Coche((char*)"Models/Coches/Coche.obj");
	Model Cuadros31((char*)"Models/cuadros/cuadros31.obj");
	Model Cuadros32((char*)"Models/cuadros/cuadros32.obj");
	Model Cuadros33((char*)"Models/cuadros/cuadros33.obj");

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint skyboxVAO, skyboxVBO;
	GLuint cubemapTexture;

	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// Crear VAO y VBO para el skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0); // Desenlazar VAO

	// Cargar texturas del cubemap
	vector<const GLchar*> faces;
	faces.push_back("SkyBox/siege/siege_rt.tga");   // Right
	faces.push_back("SkyBox/siege/siege_lf.tga");   // Left
	faces.push_back("SkyBox/siege/siege_up.tga");   // Top
	faces.push_back("SkyBox/siege/siege_dn.tga");   // Bottom
	faces.push_back("SkyBox/siege/siege_bk.tga");   // Back
	faces.push_back("SkyBox/siege/siege_ft.tga");   // Front

	cubemapTexture = TextureLoading::LoadCubemap(faces);


	// Set texture units
	lightingShader.Use();

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();

		// ----------- ANIMACIÓN DE PUERTA  -----------
		if (puertaAnimando)
		{
			puertaT += deltaTime * 0.8f;  // velocidad de animación

			if (puertaT >= 1.0f) {
				puertaT = 1.0f;
				puertaAnimando = false;
			}

			if (puertaEstaAbierta)
				puertaActual = glm::mix(puertaInicio, puertaFin, puertaT);
			else
				puertaActual = glm::mix(puertaFin, puertaInicio, puertaT);
		}

		// ----- ANIMACIÓN DE PÁNEO DE LA CÁMARA DE SEGURIDAD -----
		if (camSegPanForward) {
			camSegPanAngle += camSegPanSpeed * deltaTime;
			if (camSegPanAngle > camSegPanLimit) {
				camSegPanAngle = camSegPanLimit;
				camSegPanForward = false;   // cambia de sentido
			}
		}
		else {
			camSegPanAngle -= camSegPanSpeed * deltaTime;
			if (camSegPanAngle < -camSegPanLimit) {
				camSegPanAngle = -camSegPanLimit;
				camSegPanForward = true;    // cambia de sentido
			}
		}

		// ----- ANIMACIÓN DEL VENTILADOR DE TECHO -----
		fanBladesAngle += fanBladesSpeed * deltaTime;
		if (fanBladesAngle > 360.0f)
			fanBladesAngle -= 360.0f;


		// ----- ANIMACIÓN DEL RELOJ DE PARED -----
		clockMinuteAngle += clockMinuteSpeed * deltaTime;
		clockHourAngle += clockHourSpeed * deltaTime;

		if (clockMinuteAngle > 360.0f) clockMinuteAngle -= 360.0f;
		if (clockHourAngle > 360.0f) clockHourAngle -= 360.0f;

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL options
		glEnable(GL_DEPTH_TEST);


		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();


		glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1); // Textura especular

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


		// Luz direccional (sol de mediodía)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"),
			-0.2f, -1.0f, -0.3f);
		// Sol más arriba, apuntando hacia abajo

		// Luz ambiental más brillante y neutra (luz general del día)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"),
			0.35f, 0.35f, 0.38f);

		// Luz difusa fuerte (iluminación directa del sol)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"),
			0.8f, 0.8f, 0.85f);

		// Especular blanca e intensa (brillos fuertes)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"),
			1.0f, 1.0f, 1.0f);


		// Point light 1 (farola)
		glm::vec3 lightColor1 = lamparaColores[indiceColorLampara];

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"),
			pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"),
			lightColor1.x * 0.20f, lightColor1.y * 0.20f, lightColor1.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"),
			lightColor1.x * 0.90f, lightColor1.y * 0.90f, lightColor1.z * 0.90f);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);


		// Point light 2
		glm::vec3 lightColor2;
		lightColor2.x = abs(sin(glfwGetTime() * Light2.x));
		lightColor2.y = abs(sin(glfwGetTime() * Light2.y));
		lightColor2.z = sin(glfwGetTime() * Light2.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"),
			pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"),
			lightColor2.x * 0.20f, lightColor2.y * 0.20f, lightColor2.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"),
			lightColor2.x * 0.90f, lightColor2.y * 0.90f, lightColor2.z * 0.90f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.075f);


		// Point light 3
		glm::vec3 lightColor3;
		lightColor3.x = abs(sin(glfwGetTime() * Light3.x));
		lightColor3.y = abs(sin(glfwGetTime() * Light3.y));
		lightColor3.z = sin(glfwGetTime() * Light3.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"),
			pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"),
			lightColor3.x * 0.20f, lightColor3.y * 0.20f, lightColor3.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"),
			lightColor3.x * 0.90f, lightColor3.y * 0.90f, lightColor3.z * 0.90f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.075f);


		// Point light 4
		glm::vec3 lightColor4;
		lightColor4.x = abs(sin(glfwGetTime() * Light4.x));
		lightColor4.y = abs(sin(glfwGetTime() * Light4.y));
		lightColor4.z = sin(glfwGetTime() * Light4.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"),
			pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"),
			lightColor4.x * 0.20f, lightColor4.y * 0.20f, lightColor4.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"),
			lightColor4.x * 0.90f, lightColor4.y * 0.90f, lightColor4.z * 0.90f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.075f);


		// Point light 5
		glm::vec3 lightColor5;
		lightColor5.x = abs(sin(glfwGetTime() * Light5.x));
		lightColor5.y = abs(sin(glfwGetTime() * Light5.y));
		lightColor5.z = sin(glfwGetTime() * Light5.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"),
			pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"),
			lightColor5.x * 0.20f, lightColor5.y * 0.20f, lightColor5.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"),
			lightColor5.x * 0.90f, lightColor5.y * 0.90f, lightColor5.z * 0.90f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.075f);

		// Point light 6
		glm::vec3 lightColor6;
		lightColor6.x = abs(sin(glfwGetTime() * Light6.x));
		lightColor6.y = abs(sin(glfwGetTime() * Light6.y));
		lightColor6.z = sin(glfwGetTime() * Light6.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"),
			pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"),
			lightColor6.x * 0.20f, lightColor6.y * 0.20f, lightColor6.z * 0.20f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"),
			lightColor6.x * 0.90f, lightColor6.y * 0.90f, lightColor6.z * 0.90f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.075f);

		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"),
			camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"),
			camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"),
			1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"),
			1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"),
			1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.35f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.44f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"),
			glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"),
			glm::cos(glm::radians(15.0f)));

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);



		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);

		//Carga de modelos 
		//modelos normales
		view = camera.GetViewMatrix();

		// Dobby
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Mono.Draw(lightingShader);

		// Kaliz
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Kaliz.Draw(lightingShader);

		// Baston Sombrero
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		BastonSom.Draw(lightingShader);

		// Carro Pared
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		CarroPared.Draw(lightingShader);

		// ====== LLANTA DERECHA ======
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		LLantaDer.Draw(lightingShader);

		// ====== LLANTA IZQUIERDA ======
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		LlantaIzq.Draw(lightingShader);
		
		// Cuadros Sala 1
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros1.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros2.Draw(lightingShader);

		// Cuadros Sala 2
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadro1.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros3.Draw(lightingShader);

		// Barra
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Barra.Draw(lightingShader);

		// Statua
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua.Draw(lightingShader);

		// Statua 2
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua2.Draw(lightingShader);

		// Statua 3
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua3.Draw(lightingShader);

		// Statua 4
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua4.Draw(lightingShader);

		// Statua 5
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua5.Draw(lightingShader);

		// Statua 6
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Statua6.Draw(lightingShader);

		// Cuadros Sala 3
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros31.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros32.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Cuadros33.Draw(lightingShader);

		// ESTRUCTURA EXTERNA 
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Estructura.Draw(lightingShader);

		// EDIFICIO
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		casa.Draw(lightingShader);

		// COCHES
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		Coches.Draw(lightingShader);
		
		//PUERTA DE LA ENTRADA DEL MUSEO (izquierda)
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(25.6f, 2.4f, 18.25f));
		model = glm::scale(model, glm::vec3(1.0f, 1.2f, 0.8f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(rotPuerta), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform4f(glGetUniformLocation(lightingShader.Program, "colorAlpha"),
			1.0f, 1.0f, 1.0f, 1.0f);

		PuertaIzq.Draw(lightingShader);

		//PUERTA DE LA ENTRADA DEL MUSEO (derecha)
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(posX, posY, posZ));
		model = glm::translate(model, glm::vec3(25.6f, 2.4f, 13.55f));
		model = glm::scale(model, glm::vec3(1.0f, 1.2f, 0.8f)); 
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(-rotPuerta), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform4f(glGetUniformLocation(lightingShader.Program, "colorAlpha"),
			1.0f, 1.0f, 1.0f, 1.0f);
		PuertaDer.Draw(lightingShader);


		// PUERTA ENTRADA DEL MUSEO 
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(25.6f, 0.0f, 2.3f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(1.2f, 1.45f, 2.5f));
		model = glm::rotate(model, puertaActual, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		puerta.Draw(lightingShader);


		// COCHE
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(posX + movCocheX, posY, posZ + movCocheZ));
		model = glm::translate(model, glm::vec3(28.0f, 0.85f, 15.5f));
		model = glm::scale(model, glm::vec3(2.0f, 1.5f, 2.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(rotCoche), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Coche.Draw(lightingShader);

		// Ventanas
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, ventanaPosZ));  // Aplicar el desplazamiento en z
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		ventanas.Draw(lightingShader);

		//Entrada
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		entrada.Draw(lightingShader);

		//Puerta con KEYFRAME
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-7.1f, 0.35f, 3.6f));
		model = glm::rotate(model, puertaActual, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		puerta.Draw(lightingShader);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-7.1f, 0.35f, 3.6f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.87f));
		model = glm::rotate(model, puertaActual, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		puerta.Draw(lightingShader);

		//barra
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "activaTransparencia"), 0);
		barra.Draw(lightingShader);
		
		//television ----usa Shader---
		tvNoiseShader.Use();
		glUniform1f(glGetUniformLocation(tvNoiseShader.Program, "time"), glfwGetTime());
		// Configuración de matrices para la TV
		model = glm::mat4(1);
		glUniformMatrix4fv(glGetUniformLocation(tvNoiseShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(tvNoiseShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(tvNoiseShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// Dibuja la TV utilizando el shader de ruido
		tv.Draw(tvNoiseShader);

		lightingShader.Use();

		
		// ===== AGUA en Kaliz =====
		AnimFuego.Use();
		tiempoFuego = glfwGetTime() * velocidad;

		// uniforms de matrices
		modelLoc = glGetUniformLocation(AnimFuego.Program, "model");
		viewLoc = glGetUniformLocation(AnimFuego.Program, "view");
		projLoc = glGetUniformLocation(AnimFuego.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// modelo base (sin transformaciones)
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glUniform1f(glGetUniformLocation(AnimFuego.Program, "time"), tiempoFuego);

		// dibuja el modelo Agua con el shader AnimFuego
		Agua.Draw(AnimFuego);
		glBindVertexArray(0);

//------------------Objetos generados con primitivas--------------------------------------
		// ================== MESA + 4 SILLAS TERRAZA (CUBOS) ==================
		lightingShader.Use();

		// Aseguramos que los uniforms estén correctos 
		modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);

		// Matriz base del set (traslación + rotación + escala global)
		glm::mat4 baseTerraza = glm::mat4(1.0f);
		baseTerraza = glm::translate(baseTerraza, terrazaPos);
		baseTerraza = glm::rotate(
			baseTerraza,
			glm::radians(terrazaRotY),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		baseTerraza = glm::scale(baseTerraza, terrazaScale);

		// ---------- MESA ----------

		// Tablero de la mesa
		glm::mat4 m = baseTerraza;
		m = glm::translate(m, glm::vec3(0.0f, 0.8f, 0.0f));       // altura del tablero
		m = glm::scale(m, glm::vec3(2.0f, 0.08f, 1.2f));          // largo x ancho x grosor
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Patas de la mesa (4 esquinas)
		float offsetX = 0.9f;
		float offsetZ = 0.5f;
		float alturaPataMesa = 0.8f;

		glm::vec3 patasMesa[4] = {
			glm::vec3(offsetX, alturaPataMesa / 2.0f,  offsetZ),
			glm::vec3(-offsetX, alturaPataMesa / 2.0f,  offsetZ),
			glm::vec3(offsetX, alturaPataMesa / 2.0f, -offsetZ),
			glm::vec3(-offsetX, alturaPataMesa / 2.0f, -offsetZ)
		};

		for (int i = 0; i < 4; ++i) {
			m = baseTerraza;
			m = glm::translate(m, patasMesa[i]);
			m = glm::scale(m, glm::vec3(0.12f, alturaPataMesa, 0.12f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ---------- SILLAS (4) ----------
		// Cada silla estará en un lado de la mesa
		// Usamos una función "lambda" mental: baseTerraza -> base de cada silla

		auto dibujarSilla = [&](glm::vec3 posLocalSilla, float rotLocalY)
			{
				glm::mat4 baseSilla = baseTerraza;
				baseSilla = glm::translate(baseSilla, posLocalSilla);
				baseSilla = glm::rotate(
					baseSilla,
					glm::radians(rotLocalY),
					glm::vec3(0.0f, 1.0f, 0.0f)
				);

				// Asiento
				glm::mat4 s = baseSilla;
				s = glm::translate(s, glm::vec3(0.0f, 0.45f, 0.0f));
				s = glm::scale(s, glm::vec3(0.6f, 0.08f, 0.6f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(s));
				glDrawArrays(GL_TRIANGLES, 0, 36);

				// Respaldo
				s = baseSilla;
				s = glm::translate(s, glm::vec3(0.0f, 0.75f, -0.25f));
				s = glm::scale(s, glm::vec3(0.6f, 0.7f, 0.08f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(s));
				glDrawArrays(GL_TRIANGLES, 0, 36);

				// Patas de la silla
				float hPata = 0.45f;
				glm::vec3 patasSilla[4] = {
					glm::vec3(0.22f, hPata / 2.0f,  0.22f),
					glm::vec3(-0.22f, hPata / 2.0f,  0.22f),
					glm::vec3(0.22f, hPata / 2.0f, -0.22f),
					glm::vec3(-0.22f, hPata / 2.0f, -0.22f)
				};

				for (int i = 0; i < 4; ++i) {
					s = baseSilla;
					s = glm::translate(s, patasSilla[i]);
					s = glm::scale(s, glm::vec3(0.08f, hPata, 0.08f));
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(s));
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			};

		// Posiciones locales de las 4 sillas alrededor de la mesa
		dibujarSilla(glm::vec3(0.0f, 0.0f, 1.4f), 180.0f); // frente
		dibujarSilla(glm::vec3(0.0f, 0.0f, -1.4f), 0.0f); // atrás
		dibujarSilla(glm::vec3(1.4f, 0.0f, 0.0f), -90.0f); // derecha
		dibujarSilla(glm::vec3(-1.4f, 0.0f, 0.0f), 90.0f); // izquierda

		glBindVertexArray(0);
		
		// ================== FAROLA (CUBOS) ==================
		lightingShader.Use();

		// VOLVER A OBTENER LOS UNIFORMS PARA ESTE SHADER
		modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Aseguramos view y projection correctos
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);

		// Escala global en X,Z (grosor) y factor de altura en Y
		float S = 1.0f;   // misma base/grosor
		float H = 2.5f;   // farola más alta

		// BASE de concreto 
		glm::mat4 farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.0f, 0.15f, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.7f * S, 0.3f * S, 0.7f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Poste principal (vertical) – MÁS ALTO
		farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.0f, 1.3f * H, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.15f * S, 2.5f * H, 0.15f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Brazo horizontal – sube junto con la altura
		farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.4f * S, 2.5f * H, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.8f * S, 0.12f * S, 0.12f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Poste pequeño que baja del brazo
		farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.8f * S, 2.3f * H, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.10f * S, 0.6f * H, 0.10f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Cabeza de la lámpara
		farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.8f * S, 2.0f * H, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.5f * S, 0.35f * S, 0.5f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Techo de la lámpara
		farolaModel = glm::mat4(1.0f);
		farolaModel = glm::translate(
			farolaModel,
			lamparaPos + glm::vec3(0.8f * S, 2.25f * H, 0.0f)
		);
		farolaModel = glm::scale(
			farolaModel,
			glm::vec3(0.7f * S, 0.1f * S, 0.7f * S)
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(farolaModel));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

// ================== CÁMARA DE SEGURIDAD (CUBOS) ==================
		colorShader.Use();

		modelLoc = glGetUniformLocation(colorShader.Program, "model");
		viewLoc = glGetUniformLocation(colorShader.Program, "view");
		projLoc = glGetUniformLocation(colorShader.Program, "projection");
		GLint colorLoc = glGetUniformLocation(colorShader.Program, "colorAlpha");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		GLint useTexLoc = glGetUniformLocation(colorShader.Program, "useTexture");
		if (useTexLoc != -1)
			glUniform1i(useTexLoc, 0);

		glBindVertexArray(VAO);

		// ================== MATRIZ BASE ==================
		glm::mat4 baseCam = glm::mat4(1.0f);
		baseCam = glm::translate(baseCam, camSegPos);
		baseCam = glm::rotate(baseCam, glm::radians(camSegBaseRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		baseCam = glm::scale(baseCam, camSegScale);

		// ---------- COLOR PRINCIPAL (PLATEADO FRÍO) ----------
		glUniform4f(colorLoc, 0.65f, 0.65f, 0.7f, 1.0f); // para el brazo y placa

		// ---------- PLACA DE PARED ----------
		m = baseCam;
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, 0.0f));
		m = glm::scale(m, glm::vec3(0.8f, 1.0f, 0.1f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- BRAZO DEL SOPORTE ----------
		m = baseCam;
		m = glm::translate(m, glm::vec3(0.0f, -0.1f, -0.35f));
		m = glm::scale(m, glm::vec3(0.15f, 0.15f, 0.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- ARTICULACIÓN ----------
		m = baseCam;
		m = glm::translate(m, glm::vec3(0.0f, -0.1f, -0.7f));
		m = glm::scale(m, glm::vec3(0.25f, 0.25f, 0.25f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ================== CABEZA DE LA CÁMARA ==================
		glm::mat4 baseCamHead = baseCam;
		baseCamHead = glm::translate(baseCamHead, glm::vec3(0.0f, -0.1f, -0.9f));
		baseCamHead = glm::rotate(baseCamHead, glm::radians(camSegPanAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		// ---------- CUERPO (PLATEADO) ----------
		glUniform4f(colorLoc, 0.75f, 0.75f, 0.8f, 1.0f);
		m = baseCamHead;
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, -0.2f));
		m = glm::scale(m, glm::vec3(0.9f, 0.6f, 0.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- FRONTAL (PLATEADO) ----------
		m = baseCamHead;
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, -0.65f));
		m = glm::scale(m, glm::vec3(0.7f, 0.5f, 0.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- LENTE (NEGRO BRILLANTE) ----------
		glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 1.0f);
		m = baseCamHead;
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, -0.9f));
		m = glm::scale(m, glm::vec3(0.25f, 0.25f, 0.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ================== RESET ==================
		glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(0);

		// ================== ARBUSTO LOW POLY (CUBOS) ==================
		colorShader.Use();

		// Uniforms de matrices
		modelLoc = glGetUniformLocation(colorShader.Program, "model");
		viewLoc = glGetUniformLocation(colorShader.Program, "view");
		projLoc = glGetUniformLocation(colorShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(glGetUniformLocation(colorShader.Program, "useTexture"), 0);

		glBindVertexArray(VAO);

		// Matriz base del arbusto
		glm::mat4 arbBase = glm::mat4(1.0f);
		arbBase = glm::translate(arbBase, arbustoPos);
		arbBase = glm::rotate(arbBase, glm::radians(arbustoRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		arbBase = glm::scale(arbBase, arbustoScale * 0.8f); 

		glm::mat4 arbPart;

		// ---------- TRONCO ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.35f, 0.22f, 0.07f, 1.0f); 

		arbPart = arbBase;
		arbPart = glm::translate(arbPart, glm::vec3(0.0f, 0.20f, 0.0f));
		arbPart = glm::scale(arbPart, glm::vec3(0.20f, 0.4f, 0.20f)); 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(arbPart));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- HOJAS ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.10f, 0.38f, 0.15f, 1.0f); 

		// Bloque central
		arbPart = arbBase;
		arbPart = glm::translate(arbPart, glm::vec3(0.0f, 0.8f, 0.0f));
		arbPart = glm::scale(arbPart, glm::vec3(1.3f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(arbPart));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Lateral izquierdo
		arbPart = arbBase;
		arbPart = glm::translate(arbPart, glm::vec3(-0.5f, 0.7f, 0.0f));
		arbPart = glm::scale(arbPart, glm::vec3(0.9f, 0.9f, 0.9f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(arbPart));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Lateral derecho
		arbPart = arbBase;
		arbPart = glm::translate(arbPart, glm::vec3(0.5f, 0.7f, 0.0f));
		arbPart = glm::scale(arbPart, glm::vec3(0.9f, 0.9f, 0.9f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(arbPart));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Superior
		arbPart = arbBase;
		arbPart = glm::translate(arbPart, glm::vec3(0.0f, 1.25f, 0.0f));
		arbPart = glm::scale(arbPart, glm::vec3(1.0f, 0.8f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(arbPart));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Reset color
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(0);

		// ================== VENTILADOR DE TECHO (CUBOS) ==================
		colorShader.Use();

		// Uniforms de matrices
		modelLoc = glGetUniformLocation(colorShader.Program, "model");
		viewLoc = glGetUniformLocation(colorShader.Program, "view");
		projLoc = glGetUniformLocation(colorShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(glGetUniformLocation(colorShader.Program, "useTexture"), 0);

		glBindVertexArray(VAO);

		// Matriz base del ventilador (traslación + rotación + escala global)
		glm::mat4 fanBase = glm::mat4(1.0f);
		fanBase = glm::translate(fanBase, fanPos);
		fanBase = glm::rotate(fanBase, glm::radians(fanRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		fanBase = glm::scale(fanBase, fanScale);

		glm::mat4 fm;

		// ---------- PLAFÓN (base pegada al techo) ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.7f, 0.7f, 0.7f, 1.0f);  // gris claro

		fm = fanBase;
		fm = glm::translate(fm, glm::vec3(0.0f, 0.05f, 0.0f)); 
		fm = glm::scale(fm, glm::vec3(0.8f, 0.1f, 0.8f));      
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- TUBO VERTICAL ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.55f, 0.55f, 0.55f, 1.0f);  // gris medio

		fm = fanBase;
		fm = glm::translate(fm, glm::vec3(0.0f, -0.35f, 0.0f)); 
		fm = glm::scale(fm, glm::vec3(0.12f, 0.7f, 0.12f));    
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- CUERPO / MOTOR ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.4f, 0.4f, 0.4f, 1.0f); 

		fm = fanBase;
		fm = glm::translate(fm, glm::vec3(0.0f, -0.75f, 0.0f));
		fm = glm::scale(fm, glm::vec3(0.6f, 0.25f, 0.6f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ========= BASE DE LAS ASPAS =========
		glm::mat4 fanHead = fanBase;
		fanHead = glm::translate(fanHead, glm::vec3(0.0f, -0.75f, 0.0f));
		fanHead = glm::rotate(fanHead, glm::radians(fanBladesAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		// ---------- ASPAS ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.65f, 0.65f, 0.65f, 1.0f); 

		for (int i = 0; i < 4; ++i) {
			float offsetAngle = i * 90.0f;

			glm::mat4 blade = fanHead;
			blade = glm::rotate(blade, glm::radians(offsetAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			blade = glm::translate(blade, glm::vec3(0.9f, 0.0f, 0.0f));
			blade = glm::scale(blade, glm::vec3(1.6f, 0.07f, 0.25f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(blade));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Reset color
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(0);

// ================== RELOJ DE PARED (CUBOS) ==================
		colorShader.Use();

		// Uniforms de matrices
		modelLoc = glGetUniformLocation(colorShader.Program, "model");
		viewLoc = glGetUniformLocation(colorShader.Program, "view");
		projLoc = glGetUniformLocation(colorShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);

		// Matriz base del reloj (posición + rotación + escala)
		glm::mat4 clockBase = glm::mat4(1.0f);
		clockBase = glm::translate(clockBase, clockPos);
		clockBase = glm::rotate(clockBase, glm::radians(clockRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		clockBase = glm::scale(clockBase, clockScale);

		glm::mat4 cm;

		// ========== COLOR BASE GRIS ==========
		glUniform1i(glGetUniformLocation(colorShader.Program, "useTexture"), 0);

		// ---------- CUERPO DEL RELOJ (FONDO) ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.6f, 0.6f, 0.6f, 1.0f); // gris medio

		cm = clockBase;
		cm = glm::translate(cm, glm::vec3(0.0f, 0.0f, 0.0f));
		cm = glm::scale(cm, glm::vec3(1.8f, 1.8f, 0.15f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- ARO EXTERIOR ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.3f, 0.3f, 0.3f, 1.0f); // gris oscuro

		cm = clockBase;
		cm = glm::scale(cm, glm::vec3(1.95f, 1.95f, 0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- MARCAS HORARIAS ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.1f, 0.1f, 0.1f, 1.0f); // casi negro

		for (int i = 0; i < 12; ++i) {
			float ang = glm::radians(i * 30.0f); // 360 / 12
			glm::mat4 mark = clockBase;
			mark = glm::rotate(mark, ang, glm::vec3(0.0f, 0.0f, 1.0f));
			mark = glm::translate(mark, glm::vec3(0.0f, 0.9f, 0.06f));
			mark = glm::scale(mark, glm::vec3(0.15f, 0.25f, 0.12f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mark));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ---------- MANECILLA LARGA (MINUTERO) ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.15f, 0.15f, 0.15f, 1.0f); // gris oscuro

		glm::mat4 minH = clockBase;
		minH = glm::rotate(minH, glm::radians(clockMinuteAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		minH = glm::translate(minH, glm::vec3(0.0f, 0.9f, 0.08f));
		minH = glm::scale(minH, glm::vec3(0.12f, 1.8f, 0.14f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(minH));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- MANECILLA CORTA (HORA) ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.15f, 0.15f, 0.15f, 1.0f); // gris oscuro

		glm::mat4 hourH = clockBase;
		hourH = glm::rotate(hourH, glm::radians(clockHourAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		hourH = glm::translate(hourH, glm::vec3(0.0f, 0.6f, 0.09f));
		hourH = glm::scale(hourH, glm::vec3(0.16f, 1.2f, 0.16f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(hourH));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ---------- TAPITA CENTRAL ----------
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			0.05f, 0.05f, 0.05f, 1.0f); // gris casi negro

		cm = clockBase;
		cm = glm::translate(cm, glm::vec3(0.0f, 0.0f, 0.11f));
		cm = glm::scale(cm, glm::vec3(0.3f, 0.3f, 0.18f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cm));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Reset
		glUniform4f(glGetUniformLocation(colorShader.Program, "colorAlpha"),
			1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(0);

		// ===== FUEGO EN EL CÁLIZ =====
		AnimFuego.Use();
		tiempoFuego = glfwGetTime() * velocidad;

		// uniforms de matrices
		modelLoc = glGetUniformLocation(AnimFuego.Program, "model");
		viewLoc = glGetUniformLocation(AnimFuego.Program, "view");
		projLoc = glGetUniformLocation(AnimFuego.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// MATRIZ DEL FUEGO
		model = glm::mat4(1.0f);

		// SOLO movimiento vertical (sube / baja alrededor de su posición actual)
		model = glm::translate(model, glm::vec3(0.0f, 0.2f * sin(tiempoFuego), 0.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1f(glGetUniformLocation(AnimFuego.Program, "time"), tiempoFuego);

		Fuego.Draw(AnimFuego);
		glBindVertexArray(0);

		// Dibujar el skybox al final de la escena
		glDepthFunc(GL_LEQUAL);  // Cambiar función de profundidad para el skybox
		SkyBoxshader.Use();
		// Configurar vista y proyección para el skybox
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Quitar traslación
		glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// Vincular y dibujar el cubemap
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);  // Restablecer la función de profundidad

		glBindVertexArray(0);
		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	// Posición del spotlight
	if (keys[GLFW_KEY_H])
	{
		spotLightPos.x += 0.01f;
	}
	if (keys[GLFW_KEY_F])
	{
		spotLightPos.x -= 0.01f;
	}

	if (keys[GLFW_KEY_Y])
	{
		spotLightPos.y += 0.01f;
	}

	if (keys[GLFW_KEY_R])
	{
		spotLightPos.y -= 0.01f;
	}
	if (keys[GLFW_KEY_T])
	{
		spotLightPos.z -= 0.01f;
	}
	if (keys[GLFW_KEY_G])
	{
		spotLightPos.z += 0.01f;
	}

	// Dirección del Spotlight
	if (keys[GLFW_KEY_L])
	{
		spotLightDir.x += 0.01f;
	}
	if (keys[GLFW_KEY_J])
	{
		spotLightDir.x -= 0.01f;
	}
	if (keys[GLFW_KEY_O])
	{
		spotLightDir.y += 0.01f;
	}
	if (keys[GLFW_KEY_U])
	{
		spotLightDir.y -= 0.01f;
	}
	if (keys[GLFW_KEY_I])
	{
		spotLightDir.z -= 0.01f;
	}
	if (keys[GLFW_KEY_K])
	{
		spotLightDir.z += 0.01f;
	}

	// Giro para el Kaliz y Fuego 
	if (giro >= 0) {
		giro += 0.15f;
	}
 
	// PUERTA DE LA ENTRADA DEL MUSEO (abrir/cerrar con la misma tecla)
	if (animPuerta) {
		float velocidadPuerta = 40.0f; // grados por segundo

		if (!puertaMuseoAbierta) {
			// Abriendo (de 0° a 90°)
			rotPuerta += velocidadPuerta * deltaTime;
			if (rotPuerta >= 90.0f) {
				rotPuerta = 90.0f;
				animPuerta = false;          // detener animación
				puertaMuseoAbierta = true;   // ya quedó abierta
			}
		}
		else {
			// Cerrando (de 90° a 0°)
			rotPuerta -= velocidadPuerta * deltaTime;
			if (rotPuerta <= 0.0f) {
				rotPuerta = 0.0f;
				animPuerta = false;          // detener animación
				puertaMuseoAbierta = false;  // ya quedó cerrada
			}
		}
	}

	// ANIMACION DEL COCHE
	if (animCoche) {
		float D1 = 49.0f;   // distancia en X negativo antes de girar
		float D2 = 7.5f;    // distancia recta después de girar
		float velX = 0.04f;   // velocidad en X
		float velZ = 0.04f;   // velocidad en Z
		float velRot = 1.0f;    // velocidad de giro (grados por frame)
		float angGiro = -90.0f;  // giro a la derecha

		// FASE 1: ir recto en X negativo
		if (!sentidoCoche && !faseFinalCoche) {
			// movCocheX va de 0 a -D1
			if (movCocheX > -D1) {
				movCocheX -= velX;
			}
			else {
				// cuando llegue a la distancia deseada, pasamos a la fase del giro
				sentidoCoche = true;
			}
		}

		// FASE 2: girar a la derecha (sobre Y)
		if (sentidoCoche && !faseFinalCoche) {
			if (rotCoche > angGiro) {           
				rotCoche -= velRot;             
			}
			else {
				// ya completó el giro → pasamos a la fase final
				faseFinalCoche = true;
			}
		}

		// FASE 3: avanzar un pequeño tramo recto
		if (faseFinalCoche) {
			// Se moverá de 0 a -D2 en Z
			if (movCocheZ > -D2) {
				movCocheZ -= velZ;
			}
			else {
				// llegó al cajón → detenemos animación
				animCoche = false;
				sentidoCoche = false;
				faseFinalCoche = false;
			}
		}
	}


}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		active = !active;
		lightsOn = active;

		if (!active) { // apagar
			Light1 = Light2 = Light3 = Light4 = glm::vec3(0.0f);
		}
		else {       // encender
			Light1 = glm::vec3(1.0f);
			Light2 = glm::vec3(2.0f);
			Light3 = glm::vec3(2.5f);
			Light4 = glm::vec3(3.0f);
		}
	}
	//Apertura/Cerrado de ventanas
	if (key == GLFW_KEY_V && action == GLFW_PRESS) {
		if (ventanaDesplazada) {
			ventanaPosZ = 0.0f;  // Regresar a la posición inicial
		}
		else {
			ventanaPosZ = -0.5f;  // Desplazar 1 unidad en el eje z
		}
		ventanaDesplazada = !ventanaDesplazada;  // Cambiar el estado de desplazamiento
	}
	// PUERTA DEL EDIFICIO 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (!puertaEstaAbierta) {
			puertaT = 0.0f;
			puertaAnimando = true;
			puertaEstaAbierta = true;
		}
		else {
			puertaT = 0.0f;
			puertaAnimando = true;
			puertaEstaAbierta = false;
		}
	}
	// PUERTA DE LA ENTRADA DEL MUSEO
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		if (!animPuerta) {
			animPuerta = true;
		}
	}
	
	// COCHE
	if (keys[GLFW_KEY_2])
	{
		animCoche = !animCoche;
	}

	//Animación del faro
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		indiceColorLampara = (indiceColorLampara + 1) % 3;
	}
}


void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}