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
glm::vec3 aerialPosition = glm::vec3(0.0f, 200.0f, 0.0f);
float aerialSpeed = 0.5f;
// --------------------------------------

// --- VARIABLES DEL AVATAR JERÁRQUICO SIMPLIFICADO ---
Model Sephi_Torso;
Model Sephi_Bra1, Sephi_Bra2; // 1 Izquierdo, 2 Derecho
Model Sephi_Pie1, Sephi_Pie2; // 1 Izquierdo, 2 Derecho

glm::vec3 sephiPosition = glm::vec3(0.0f, -1.6f, 0.0f);
float sephiRotationY = 0.0f; // Mantenemos tu rotación original
float moveSpeed = 0.5f;

// Ángulos de articulación (preparados para animación)
float angBra1 = 0.0f; // Brazo Izquierdo Completo
float angBra2 = 0.0f; // Brazo Derecho Completo
float angPie1 = 0.0f; // Pierna Izquierda Completa
float angPie2 = 0.0f; // Pierna Derecha Completa

// --- OFFSETS PARA ACOMODAR LAS EXTREMIDADES ---
// BRAZO IZQUIERDO (El de la espada)
glm::vec3 offHombroIzq = glm::vec3(0.00f, 0.15f, 0.0f);  // Bajamos el brazo (Y de 0.28f a 0.15f)
// BRAZO DERECHO
glm::vec3 offHombroDer = glm::vec3(-0.00f, 0.15f, 0.0f); // Bajamos el brazo (Y de 0.28f a 0.15f)

// PIERNAS
glm::vec3 offCaderaIzq = glm::vec3(0.12f, -0.6f, 0.0f);  // Distancia del centro del torso a la cadera izq
glm::vec3 offCaderaDer = glm::vec3(-0.12f, -0.6f, 0.0f); // Distancia del centro del torso a la cadera der
// ----------------------------------------------

// --- VARIABLES DE LA IGLESIA ---
Model Iglesia_M;
Texture iglesiaTexture;
// -------------------------------

// --- AJUSTES VISUALES ---
float sephiScale = 3.0f;
float sephiYOffset = 2.6f;
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
const float cycleDuration = 72000.0f; // 2 minutos
// ---------------------------------

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

	// --- CARGAR AVATAR JERÁRQUICO ---
	Sephi_Torso = Model();
	Sephi_Torso.LoadModel("Models/Sephi.fbx");

	Sephi_Bra1 = Model(); Sephi_Bra1.LoadModel("Models/Sephi_bra1.fbx");
	Sephi_Bra2 = Model(); Sephi_Bra2.LoadModel("Models/Sephi_bra2.fbx");

	Sephi_Pie1 = Model(); Sephi_Pie1.LoadModel("Models/Sephi_pie1.fbx");
	Sephi_Pie2 = Model(); Sephi_Pie2.LoadModel("Models/Sephi_pie2.fbx");
	// ------------------------------------------------------------------------

	// --- CARGAR IGLESIA Y TEXTURA ---
	Iglesia_M = Model();
	Iglesia_M.LoadModel("Models/Igle.fbx");

	iglesiaTexture = Texture("Textures/aeris-25.png");
	iglesiaTexture.LoadTextureA();
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
			dayNightTimer -= cycleDuration;
		}

		float cycleRadians = (dayNightTimer / cycleDuration) * glm::pi<float>() * 2.0f;
		float blend = (cos(cycleRadians) * -0.5f) + 0.5f;

		float dayIntensity = 0.4f * (1.0f - blend);
		float nightIntensity = 0.2f * blend;

		mainLight = DirectionalLight(
			1.0f * (1.0f - blend) + 0.2f * blend,
			1.0f * (1.0f - blend) + 0.2f * blend,
			0.9f * (1.0f - blend) + 0.5f * blend,
			0.8f * (1.0f - blend) + 0.1f * blend,
			0.5f * (1.0f - blend) + 0.1f * blend,
			0.0f, -1.0f, 1.0f * (1.0f - blend) + -1.0f * blend
		);

		float sunX = sin(cycleRadians) * 400.0f;
		float sunY = cos(cycleRadians) * 400.0f;

		pointLights[0] = PointLight(
			1.0f, 0.95f, 0.8f,
			0.1f, dayIntensity * 0.7f,
			sunX, sunY, 0.0f,
			0.1f, 0.01f, 0.001f
		);

		spotLights[0] = SpotLight(
			0.4f, 0.6f, 1.0f,
			0.0f, nightIntensity,
			0.0f, 30.0f, -30.0f,
			0.0f, -1.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 30.0f
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
		bool estaCaminando = false; // Variable para saber si debemos animar el brazo

		if (isAerialView) {
			if (mainWindow.getsKeys()[GLFW_KEY_W]) aerialPosition.z -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_S]) aerialPosition.z += aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_A]) aerialPosition.x -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_D]) aerialPosition.x += aerialSpeed * deltaTime;
		}
		else {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));

			if (mainWindow.getsKeys()[GLFW_KEY_W]) {
				sephiPosition += sephiFront * moveSpeed * deltaTime;
				estaCaminando = true;
			}
			if (mainWindow.getsKeys()[GLFW_KEY_S]) {
				sephiPosition -= sephiFront * moveSpeed * deltaTime;
				estaCaminando = true;
			}
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

		// --- LÓGICA DE ANIMACIÓN DE CAMINATA ---
		if (estaCaminando) {
			float tiempo = glfwGetTime() * 5.0f;

			// Brazo izquierdo (Lo dejamos como lo teníamos)
			angBra1 = (1.0f - cosf(tiempo)) * 15.0f;

			// Piernas (Seno puro para hacer un péndulo completo de adelante hacia atrás)
			// Multiplicamos por 25.0f para que el paso se note bien.
			angPie1 = sinf(tiempo) * 25.0f;   // Pierna Izquierda
			angPie2 = -sinf(tiempo) * 25.0f;  // Pierna Derecha (El signo '-' hace lo opuesto)

		}
		else {
			// Volver a pose de descanso
			angBra1 = 0.0f;
			angPie1 = 0.0f;
			angPie2 = 0.0f;
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
			float camHeight = 5.0f;
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			cameraPos = sephiPosition - (sephiFront * camDist) + glm::vec3(0.0f, camHeight, 0.0f);
			cameraTarget = sephiPosition + glm::vec3(0.0f, sephiYOffset + 0.6f, 0.0f);
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
		// AQUÍ SE BAJÓ LA IGLESIA 0.5 UNIDADES EN Y (floorLevel - 0.5f)
		model = glm::translate(model, glm::vec3(0.0f, floorLevel - 2.6f, -15.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		iglesiaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Iglesia_M.RenderModel();
		// -----------------------

		// --- DIBUJAR AVATAR JERÁRQUICO SIMPLIFICADO ---
		if (isThirdPerson || isAerialView) {

			// 1. MATRIZ PADRE (TORSO - ARTICULACIÓN CENTRAL)
			glm::mat4 joint_torso = glm::mat4(1.0f);
			joint_torso = glm::translate(joint_torso, sephiPosition + glm::vec3(0.0f, sephiYOffset, 0.0f));
			joint_torso = glm::rotate(joint_torso, sephiRotationY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

			// Matriz exclusiva para DIBUJAR el torso
			glm::mat4 draw_torso = glm::scale(joint_torso, glm::vec3(sephiScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(draw_torso));
			Sephi_Torso.RenderModel();

			// --- BRAZO IZQUIERDO (PRUEBA FINAL DE EJE) ---
			glm::mat4 joint_bra1 = joint_torso;
			joint_bra1 = glm::translate(joint_bra1, offHombroIzq);

			// PROBEMOS EL EJE Z (0,0,1). 
			// Si con X aletea, Z TIENE que ser el que lo mueve hacia adelante/atrás.
			joint_bra1 = glm::rotate(joint_bra1, angBra1 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

			glm::mat4 draw_bra1 = glm::scale(joint_bra1, glm::vec3(sephiScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(draw_bra1));
			Sephi_Bra1.RenderModel();

			// --- BRAZO DERECHO ---
			glm::mat4 joint_bra2 = glm::translate(joint_torso, offHombroDer);
			joint_bra2 = glm::rotate(joint_bra2, angBra2 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 draw_bra2 = glm::scale(joint_bra2, glm::vec3(sephiScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(draw_bra2));
			Sephi_Bra2.RenderModel();

			// --- PIERNA IZQUIERDA ---
			glm::mat4 joint_pie1 = glm::translate(joint_torso, offCaderaIzq);
			// Usamos el Eje Z (0, 0, 1) que es el del balanceo frontal
			joint_pie1 = glm::rotate(joint_pie1, angPie1 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 draw_pie1 = glm::scale(joint_pie1, glm::vec3(sephiScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(draw_pie1));
			Sephi_Pie1.RenderModel();

			// --- PIERNA DERECHA ---
			glm::mat4 joint_pie2 = glm::translate(joint_torso, offCaderaDer);
			// Usamos el Eje Z (0, 0, 1) que es el del balanceo frontal
			joint_pie2 = glm::rotate(joint_pie2, angPie2 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::mat4 draw_pie2 = glm::scale(joint_pie2, glm::vec3(sephiScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(draw_pie2));
			Sephi_Pie2.RenderModel();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}