/*
Proyecto Final - Base Limpia con Avatar y Ciclo Día/Noche
*/

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

// Cámaras y controles
Camera camera;
bool isThirdPerson = true;
bool vKeyPressed = false;

// --- VARIABLES CÁMARA AÉREA ---
bool isAerialView = false;
bool bKeyPressed = false;
glm::vec3 aerialPosition = glm::vec3(0.0f, 80.0f, 0.0f);
float aerialSpeed = 0.5f;
// --------------------------------------

// Variables del Avatar
Model Sephi_M;
glm::vec3 sephiPosition = glm::vec3(0.0f, -2.0f, 0.0f);
float sephiRotationY = 180.0f;
float moveSpeed = 0.5f;

// --- VARIABLES DE LA IGLESIA ---
Model Iglesia_M;
Texture iglesiaTexture;
// -------------------------------

// --- AJUSTES VISUALES ---
float sephiScale = 2.0f;
float sephiYOffset = 0.6f;
// ------------------------

// Físicas del salto
float gravity = -9.8f;
float jumpPower = 5.0f;
float sephiVelocityY = 0.0f;
bool isJumping = false;
float floorLevel = -2.0f;

// --- VARIABLES CICLO DÍA/NOCHE ---
Skybox skyDay;
Skybox skyNight;
float dayNightTimer = 0.0f;
const float cycleDuration = 7200.0f; // 2 minutos
// ---------------------------------

// sunx

Texture pisoTexture;

Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-20.0f, 0.0f, -20.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		 20.0f, 0.0f, -20.0f,	20.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-20.0f, 0.0f,  20.0f,	0.0f, 20.0f,	0.0f, -1.0f, 0.0f,
		 20.0f, 0.0f,  20.0f,	20.0f, 20.0f,	0.0f, -1.0f, 0.0f
	};

	Mesh* objPiso = new Mesh();
	objPiso->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(objPiso);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	Sephi_M = Model();
	Sephi_M.LoadModel("Models/Estatua.fbx"); //MODIFICADO PARA ESTATUA 

	// --- CARGAR IGLESIA Y TEXTURA ---
	Iglesia_M = Model();
	Iglesia_M.LoadModel("Models/Igle.fbx");

	iglesiaTexture = Texture("Textures/aeris-25.png");
	iglesiaTexture.LoadTextureA(); // Usamos LoadTextureA asumiendo que el PNG tiene canal alfa
	// --------------------------------

	// --- CARGAR SKYBOX DE DÍA ---
	std::vector<std::string> dayFaces;
	dayFaces.push_back("Textures/Skybox/day_rt.tga");
	dayFaces.push_back("Textures/Skybox/day_lf.tga");
	dayFaces.push_back("Textures/Skybox/day_dn.tga");
	dayFaces.push_back("Textures/Skybox/day_up.tga");
	dayFaces.push_back("Textures/Skybox/day_bk.tga");
	dayFaces.push_back("Textures/Skybox/day_ft.tga");
	skyDay = Skybox(dayFaces);

	// --- CARGAR SKYBOX DE NOCHE ---
	std::vector<std::string> nightFaces;
	nightFaces.push_back("Textures/Skybox/ngt_rt.tga");
	nightFaces.push_back("Textures/Skybox/ngt_lf.tga");
	nightFaces.push_back("Textures/Skybox/ngt_dn.tga");
	nightFaces.push_back("Textures/Skybox/ngt_up.tga");
	nightFaces.push_back("Textures/Skybox/ngt_bk.tga");
	nightFaces.push_back("Textures/Skybox/ngt_ft.tga");
	skyNight = Skybox(nightFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	// Activamos 1 PointLight para el sol y 1 SpotLight para la luna
	unsigned int pointLightCount = 1;
	unsigned int spotLightCount = 1;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(60.0f * toRadians, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();

		// ---------------------------------------------------------
		// LÓGICA DE TIEMPO: CICLO DÍA / NOCHE SUAVE
		// ---------------------------------------------------------
		dayNightTimer += deltaTime;
		if (dayNightTimer > cycleDuration) {
			dayNightTimer -= cycleDuration; // Reiniciar ciclo tras 2 minutos
		}

		// Cálculo matemático para transición suave (0.0 = Día, 1.0 = Noche)
		float cycleRadians = (dayNightTimer / cycleDuration) * glm::pi<float>() * 2.0f;
		float blend = (cos(cycleRadians) * -0.5f) + 0.5f;

		// Intensidades calculadas para ser "no muy fuertes"
		float dayIntensity = 0.4f * (1.0f - blend); // Baja a 0 suavemente
		float nightIntensity = 0.2f * blend;        // Sube a 0.2 suavemente

		// LUZ DIRECCIONAL GLOBAL (Acompaña la transición)
		mainLight = DirectionalLight(
			1.0f * (1.0f - blend) + 0.2f * blend, // Color R
			1.0f * (1.0f - blend) + 0.2f * blend, // Color G
			0.9f * (1.0f - blend) + 0.5f * blend, // Color B
			0.8f * (1.0f - blend) + 0.1f * blend, // Intensidad Ambiental
			0.5f * (1.0f - blend) + 0.1f * blend, // Intensidad Difusa
			0.0f, -1.0f, 1.0f * (1.0f - blend) + -1.0f * blend // Dirección de Front a Back
		);

		// POINTLIGHT 1: SOL (DÍA) - Luz puntual que viaja de horizonte a horizonte
		// Usamos un radio amplio (40.0f) para simular la distancia del arco solar
		float sunX = sin(cycleRadians) * 400.0f;
		float sunY = cos(cycleRadians) * 400.0f;

		pointLights[0] = PointLight(
			1.0f, 0.95f, 0.8f, // Color blanco cálido
			0.1f, dayIntensity * 0.7f, // Intensidad difusa dinámica (controlada para no saturar)
			sunX, sunY, 0.0f, // Posición que forma el arco
			0.1f, 0.01f, 0.001f // Constante, Lineal, Exponencial (cubre bien pero suave)
		);

		// SPOTLIGHT 1: LUNA (NOCHE) - Más tenue, tono azul claro, por el lado ngt_bk (-Z)
		spotLights[0] = SpotLight(
			0.4f, 0.6f, 1.0f, // Color azul claro
			0.0f, nightIntensity, // Intensidad difusa dinámica
			0.0f, 30.0f, -30.0f, // Posición (Arriba y Atrás)
			0.0f, -1.0f, 1.0f, // Dirección hacia el origen
			1.0f, 0.0f, 0.0f, 30.0f // Parámetros físicos de la luz y apertura
		);
		// ---------------------------------------------------------


		// --- TOGGLE CÁMARA 1RA / 3RA (Tecla V) ---
		if (mainWindow.getsKeys()[GLFW_KEY_V]) {
			if (!vKeyPressed) {
				isThirdPerson = !isThirdPerson;
				isAerialView = false;
				vKeyPressed = true;
			}
		}
		else {
			vKeyPressed = false;
		}

		// --- TOGGLE CÁMARA AÉREA (Tecla B) ---
		if (mainWindow.getsKeys()[GLFW_KEY_B]) {
			if (!bKeyPressed) {
				isAerialView = !isAerialView;
				if (isAerialView) {
					aerialPosition.x = sephiPosition.x;
					aerialPosition.z = sephiPosition.z;
				}
				bKeyPressed = true;
			}
		}
		else {
			bKeyPressed = false;
		}

		// ---------------------------------------------------------
		// LÓGICA DE MOVIMIENTO
		// ---------------------------------------------------------
		if (isAerialView) {
			if (mainWindow.getsKeys()[GLFW_KEY_W]) aerialPosition.z -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_S]) aerialPosition.z += aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_A]) aerialPosition.x -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_D]) aerialPosition.x += aerialSpeed * deltaTime;
		}
		else {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));

			if (mainWindow.getsKeys()[GLFW_KEY_W]) sephiPosition += sephiFront * moveSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_S]) sephiPosition -= sephiFront * moveSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_A]) sephiRotationY += 5.0f * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_D]) sephiRotationY -= 5.0f * deltaTime;

			if (mainWindow.getsKeys()[GLFW_KEY_SPACE] && !isJumping) {
				sephiVelocityY = jumpPower;
				isJumping = true;
			}
		}

		if (isJumping) {
			sephiVelocityY += gravity * deltaTime;
			sephiPosition.y += sephiVelocityY * deltaTime;

			if (sephiPosition.y <= floorLevel) {
				sephiPosition.y = floorLevel;
				isJumping = false;
				sephiVelocityY = 0.0f;
			}
		}

		// ---------------------------------------------------------
		// LÓGICA DE LA CÁMARA 
		// ---------------------------------------------------------
		glm::vec3 cameraPos;
		glm::vec3 cameraTarget;
		glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		if (isAerialView) {
			cameraPos = aerialPosition;
			cameraTarget = cameraPos + glm::vec3(0.0f, -1.0f, -0.01f);
		}
		else if (isThirdPerson) {
			float camDist = 5.0f;
			float camHeight = 2.5f;
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			cameraPos = sephiPosition - (sephiFront * camDist) + glm::vec3(0.0f, camHeight, 0.0f);
			cameraTarget = sephiPosition + glm::vec3(0.0f, sephiYOffset + 1.5f, 0.0f);
		}
		else {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			cameraPos = sephiPosition + glm::vec3(0.0f, sephiYOffset + 2.0f, 0.0f);
			cameraTarget = cameraPos + sephiFront;
		}

		glm::mat4 customViewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

		// ---------------------------------------------------------
		// RENDERIZADO
		// ---------------------------------------------------------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Dibujar el Skybox correspondiente según el progreso del blend
		if (blend < 0.5f) {
			skyDay.DrawSkybox(customViewMatrix, projection);
		}
		else {
			skyNight.DrawSkybox(customViewMatrix, projection);
		}

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(customViewMatrix));
		glUniform3f(uniformEyePosition, cameraPos.x, cameraPos.y, cameraPos.z);

		// Configuración de las luces dinámicas calculadas arriba
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		// --- DIBUJAR PISO ---
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, floorLevel, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		// --- DIBUJAR IGLESIA ---
		model = glm::mat4(1.0);
		// La desplazamos -15 en Z para que se dibuje detrás y no estorbe el movimiento
		model = glm::translate(model, glm::vec3(0.0f, floorLevel, -15.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		iglesiaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Iglesia_M.RenderModel();
		// -----------------------

		// --- DIBUJAR AVATAR (ESTATUA) ---
		if (isThirdPerson || isAerialView) {
			model = glm::mat4(1.0);

			model = glm::translate(model, sephiPosition + glm::vec3(0.0f, sephiYOffset, 0.0f));
			model = glm::rotate(model, sephiRotationY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(sephiScale, sephiScale, sephiScale));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Sephi_M.RenderModel();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}