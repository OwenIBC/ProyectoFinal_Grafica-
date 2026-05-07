/*
Proyecto Final - Base Limpia con Avatar (Sephiroth)
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

// Variables de Sephiroth (Avatar)
Model Sephi_M;
glm::vec3 sephiPosition = glm::vec3(0.0f, -2.0f, 0.0f); // -2.0f porque ahí está tu piso
float sephiRotationY = 180.0f; // Mirando hacia enfrente (-Z)
float moveSpeed = 5.0f;

// Físicas del salto
float gravity = -9.8f;
float jumpPower = 5.0f;
float sephiVelocityY = 0.0f;
bool isJumping = false;
float floorLevel = -2.0f; // Nivel del suelo

Texture pisoTexture;
Skybox skybox;

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
	meshList.push_back(objPiso); // meshList[0] será el piso
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

	// Texturas
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	// Cargar Avatar
	Sephi_M = Model();
	Sephi_M.LoadModel("Models/Sephi.fbx"); // Asegúrate de tenerlo en esta ruta

	// Skybox
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, -1.0f, -1.0f);

	unsigned int pointLightCount = 0;
	// Puedes agregar tus pointlights aquí después

	unsigned int spotLightCount = 0;
	// Puedes agregar tus spotlights aquí después

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

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
		// 1. INPUT DEL USUARIO (AVATAR)
		// ---------------------------------------------------------

		// Cambio de cámara (Tecla V)
		if (mainWindow.getsKeys()[GLFW_KEY_V]) {
			if (!vKeyPressed) {
				isThirdPerson = !isThirdPerson;
				vKeyPressed = true;
			}
		}
		else {
			vKeyPressed = false;
		}

		// Vector frontal de Sephiroth basado en su rotación en Y
		glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
		glm::vec3 sephiRight = glm::normalize(glm::cross(sephiFront, glm::vec3(0.0f, 1.0f, 0.0f)));

		// Movimiento WASD (Avanza en la dirección a la que mira)
		if (mainWindow.getsKeys()[GLFW_KEY_W]) sephiPosition += sephiFront * moveSpeed * deltaTime;
		if (mainWindow.getsKeys()[GLFW_KEY_S]) sephiPosition -= sephiFront * moveSpeed * deltaTime;

		// Rotación lateral (A y D)
		if (mainWindow.getsKeys()[GLFW_KEY_A]) sephiRotationY += 100.0f * deltaTime;
		if (mainWindow.getsKeys()[GLFW_KEY_D]) sephiRotationY -= 100.0f * deltaTime;

		// Salto (Barra Espaciadora)
		if (mainWindow.getsKeys()[GLFW_KEY_SPACE] && !isJumping) {
			sephiVelocityY = jumpPower;
			isJumping = true;
		}

		// Lógica de Gravedad
		if (isJumping) {
			sephiVelocityY += gravity * deltaTime;
			sephiPosition.y += sephiVelocityY * deltaTime;

			// Colisión con el piso
			if (sephiPosition.y <= floorLevel) {
				sephiPosition.y = floorLevel;
				isJumping = false;
				sephiVelocityY = 0.0f;
			}
		}

		// ---------------------------------------------------------
		// 2. LÓGICA DE LA CÁMARA LIGADA AL AVATAR
		// ---------------------------------------------------------
		glm::vec3 cameraPos;
		glm::vec3 cameraTarget;

		if (isThirdPerson) {
			// Cámara de 3ra Persona: Detrás y un poco arriba de Sephiroth
			float camDist = 6.0f;
			float camHeight = 3.0f;
			cameraPos = sephiPosition - (sephiFront * camDist) + glm::vec3(0.0f, camHeight, 0.0f);
			cameraTarget = sephiPosition + glm::vec3(0.0f, 1.5f, 0.0f); // Mira a la espalda/cabeza
		}
		else {
			// Cámara de 1ra Persona: En los ojos de Sephiroth
			// Ajusta el '1.8f' a la altura real de los ojos de tu modelo FBX
			cameraPos = sephiPosition + glm::vec3(0.0f, 1.8f, 0.0f);
			cameraTarget = cameraPos + sephiFront; // Mira hacia adelante
		}

		// Calculamos la matriz de vista manualmente para anular la clase Camera libre
		glm::mat4 customViewMatrix = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

		// ---------------------------------------------------------
		// 3. RENDERIZADO
		// ---------------------------------------------------------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox.DrawSkybox(customViewMatrix, projection);

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

		// --- DIBUJAR AVATAR (SEPHIROTH) ---
		// Solo dibujarlo si estamos en 3ra persona (para que la cámara 1ra persona no se meta en su malla)
		if (isThirdPerson) {
			model = glm::mat4(1.0);
			model = glm::translate(model, sephiPosition);
			// Rotamos al personaje para que mire a donde debe
			model = glm::rotate(model, sephiRotationY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			// Si tu modelo importado es gigante o diminuto, ajusta este scale:
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Sephi_M.RenderModel();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}