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

// --- CÁMARAS Y CONTROLES ---
Camera camera;
bool isThirdPerson = true;
bool vKeyPressed = false;

bool isAerialView = false;
bool bKeyPressed = false;
glm::vec3 aerialPosition = glm::vec3(0.0f, 200.0f, 0.0f);
float aerialSpeed = 0.5f;

bool isTourCamera = false;
bool nKeyPressed = false;

// --- VARIABLES DEL AVATAR ---
Model Sephi_Torso;
Model Sephi_Bra1, Sephi_Bra2;
Model Sephi_Pie1, Sephi_Pie2;

glm::vec3 sephiPosition = glm::vec3(0.0f, -1.6f, 0.0f);
float sephiRotationY = 0.0f;
float moveSpeed = 0.5f;

float angBra1 = 0.0f; float angBra2 = 0.0f;
float angPie1 = 0.0f; float angPie2 = 0.0f;

glm::vec3 offHombroIzq = glm::vec3(0.00f, 0.15f, 0.0f);
glm::vec3 offHombroDer = glm::vec3(-0.00f, 0.15f, 0.0f);
glm::vec3 offCaderaIzq = glm::vec3(0.12f, -0.6f, 0.0f);
glm::vec3 offCaderaDer = glm::vec3(-0.12f, -0.6f, 0.0f);

// --- VARIABLES DE ESCENARIO Y NUEVOS MODELOS ---
Model Iglesia_M;
Texture iglesiaTexture;

Model Tifa_M;
Model Lamp_M;

float tifaScale = 0.05f;
float tifaYOffset = 0.0f;

// --- VARIABLES DE LA LOCOMOTORA Y RIELES ---
Model Loc_M;
Model Loc_r1, Loc_r2, Loc_r3;
Model Loc_r4, Loc_r5, Loc_r6;
Model Loc_p1, Loc_p2;
Model Riel_M;

float floorLevel = -2.0f;

glm::vec3 locPosition = glm::vec3(50.0f, floorLevel, -15.0f);
float locScale = 0.5f;
float rielScale = 0.15f;

float locRotX = -90.0f;
float locRotY = 0.0f;
float locRotZ = 0.0f;

glm::vec3 offR1 = glm::vec3(-1.2f, 0.6f, -1.5f);
glm::vec3 offR2 = glm::vec3(-1.2f, 0.6f, 0.0f);
glm::vec3 offR3 = glm::vec3(-1.2f, 0.6f, 1.5f);
glm::vec3 offR4 = glm::vec3(1.2f, 0.6f, -1.5f);
glm::vec3 offR5 = glm::vec3(1.2f, 0.6f, 0.0f);
glm::vec3 offR6 = glm::vec3(1.2f, 0.6f, 1.5f);
glm::vec3 offP1 = glm::vec3(-1.3f, 0.6f, 0.0f);
glm::vec3 offP2 = glm::vec3(1.3f, 0.6f, 0.0f);

// --- POSICIONES DE LAS LÁMPARAS ---
glm::vec3 posLamparas[4] = {
	glm::vec3(-5.0f, floorLevel,  -2.0f),
	glm::vec3(5.0f, floorLevel,  -2.0f),
	glm::vec3(-5.0f, floorLevel,   6.0f),
	glm::vec3(5.0f, floorLevel,   6.0f)
};

// --- FÍSICAS Y AJUSTES VISUALES ---
float sephiScale = 3.0f;
float sephiYOffset = 2.6f;

float gravity = -9.8f;
float jumpPower = 5.0f;
float sephiVelocityY = 0.0f;
bool isJumping = false;

// --- VARIABLES CICLO DÍA/NOCHE ---
Skybox skyDay;
Skybox skyNight;
float dayNightTimer = 0.0f;
const float cycleDuration = 72000.0f;

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
	unsigned int floorIndices[] = { 0, 2, 1, 1, 2, 3 };
	GLfloat floorVertices[] = {
		-100.0f, 0.0f, -100.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		 100.0f, 0.0f, -100.0f,	100.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-100.0f, 0.0f,  100.0f,	0.0f, 100.0f,	0.0f, -1.0f, 0.0f,
		 100.0f, 0.0f,  100.0f,	100.0f, 100.0f,	0.0f, -1.0f, 0.0f
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

	// --- TEXTURA BLANCA BASE ---
	GLuint texBlanca;
	glGenTextures(1, &texBlanca);
	glBindTexture(GL_TEXTURE_2D, texBlanca);
	unsigned char pixelBlanco[] = { 255, 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBlanco);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// --- CARGA DE TEXTURAS Y MODELOS ---
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	Sephi_Torso = Model(); Sephi_Torso.LoadModel("Models/Sephi.fbx");
	Sephi_Bra1 = Model(); Sephi_Bra1.LoadModel("Models/Sephi_bra1.fbx");
	Sephi_Bra2 = Model(); Sephi_Bra2.LoadModel("Models/Sephi_bra2.fbx");
	Sephi_Pie1 = Model(); Sephi_Pie1.LoadModel("Models/Sephi_pie1.fbx");
	Sephi_Pie2 = Model(); Sephi_Pie2.LoadModel("Models/Sephi_pie2.fbx");

	Iglesia_M = Model(); Iglesia_M.LoadModel("Models/Igle.fbx");
	iglesiaTexture = Texture("Textures/aeris-25.png");
	iglesiaTexture.LoadTextureA();

	Tifa_M = Model(); Tifa_M.LoadModel("Models/Tifa.fbx");
	Lamp_M = Model(); Lamp_M.LoadModel("Models/Lamp.fbx");
	Riel_M = Model(); Riel_M.LoadModel("Models/riel.fbx");

	Loc_M = Model(); Loc_M.LoadModel("Models/Loc.fbx");
	Loc_r1 = Model(); Loc_r1.LoadModel("Models/Loc_r1.fbx");
	Loc_r2 = Model(); Loc_r2.LoadModel("Models/Loc_r2.fbx");
	Loc_r3 = Model(); Loc_r3.LoadModel("Models/Loc_r3.fbx");
	Loc_r4 = Model(); Loc_r4.LoadModel("Models/Loc_r4.fbx");
	Loc_r5 = Model(); Loc_r5.LoadModel("Models/Loc_r5.fbx");
	Loc_r6 = Model(); Loc_r6.LoadModel("Models/Loc_r6.fbx");
	Loc_p1 = Model(); Loc_p1.LoadModel("Models/Loc_p1.fbx");
	Loc_p2 = Model(); Loc_p2.LoadModel("Models/Loc_p2.fbx");

	std::vector<std::string> dayFaces = { "Textures/Skybox/day_rt.tga", "Textures/Skybox/day_lf.tga", "Textures/Skybox/day_dn.tga", "Textures/Skybox/day_up.tga", "Textures/Skybox/day_bk.tga", "Textures/Skybox/day_ft.tga" };
	skyDay = Skybox(dayFaces);

	std::vector<std::string> nightFaces = { "Textures/Skybox/ngt_rt.tga", "Textures/Skybox/ngt_lf.tga", "Textures/Skybox/ngt_dn.tga", "Textures/Skybox/ngt_up.tga", "Textures/Skybox/ngt_bk.tga", "Textures/Skybox/ngt_ft.tga" };
	skyNight = Skybox(nightFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	unsigned int pointLightCount = 1;
	unsigned int spotLightCount = 5;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(60.0f * toRadians, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::vec3 colorBlanco = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();

		// --- CONTROLES EN TIEMPO REAL ---
		if (mainWindow.getsKeys()[GLFW_KEY_U]) { locScale += 1.0f * deltaTime; }
		if (mainWindow.getsKeys()[GLFW_KEY_J]) { locScale -= 1.0f * deltaTime; if (locScale < 0.01f) locScale = 0.01f; }
		if (mainWindow.getsKeys()[GLFW_KEY_T]) { tifaYOffset += 5.0f * deltaTime; }
		if (mainWindow.getsKeys()[GLFW_KEY_G]) { tifaYOffset -= 5.0f * deltaTime; }
		if (mainWindow.getsKeys()[GLFW_KEY_Y]) { tifaScale += 0.5f * deltaTime; }
		if (mainWindow.getsKeys()[GLFW_KEY_H]) { tifaScale -= 0.5f * deltaTime; if (tifaScale < 0.001f) tifaScale = 0.001f; }

		// --- SISTEMA DE CÁMARAS (V, B, N) ---
		if (mainWindow.getsKeys()[GLFW_KEY_V]) {
			if (!vKeyPressed) {
				isThirdPerson = !isThirdPerson;
				isAerialView = false;
				isTourCamera = false;
				vKeyPressed = true;
			}
		}
		else { vKeyPressed = false; }

		if (mainWindow.getsKeys()[GLFW_KEY_B]) {
			if (!bKeyPressed) {
				isAerialView = !isAerialView;
				if (isAerialView) {
					aerialPosition.x = sephiPosition.x;
					aerialPosition.z = sephiPosition.z;
					isTourCamera = false;
				}
				bKeyPressed = true;
			}
		}
		else { bKeyPressed = false; }

		if (mainWindow.getsKeys()[GLFW_KEY_N]) {
			if (!nKeyPressed) {
				isTourCamera = !isTourCamera;
				if (isTourCamera) {
					isAerialView = false;
				}
				nKeyPressed = true;
			}
		}
		else { nKeyPressed = false; }

		// --- CICLO DÍA/NOCHE ---
		dayNightTimer += deltaTime;
		if (dayNightTimer > cycleDuration) dayNightTimer -= cycleDuration;
		float cycleRadians = (dayNightTimer / cycleDuration) * glm::pi<float>() * 2.0f;
		float blend = (cos(cycleRadians) * -0.5f) + 0.5f;

		mainLight = DirectionalLight(
			1.0f * (1.0f - blend) + 0.2f * blend, 1.0f * (1.0f - blend) + 0.2f * blend, 0.9f * (1.0f - blend) + 0.5f * blend,
			0.8f * (1.0f - blend) + 0.1f * blend, 0.5f * (1.0f - blend) + 0.1f * blend,
			0.0f, -1.0f, 1.0f * (1.0f - blend) + -1.0f * blend
		);

		// --- LUNA Y FAROLES ---
		spotLights[0] = SpotLight(0.4f, 0.6f, 1.0f, 0.0f, 0.2f * blend, 0.0f, 30.0f, -30.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 30.0f);

		for (int i = 0; i < 4; i++) {
			spotLights[i + 1] = SpotLight(
				1.0f, 1.0f, 0.0f,
				0.6f, 2.0f,
				posLamparas[i].x, posLamparas[i].y + 4.5f, posLamparas[i].z,
				0.0f, -1.0f, 0.0f,
				1.0f, 0.02f, 0.001f,
				30.0f
			);
		}

		// --- LÓGICA DE MOVIMIENTO ---
		bool estaCaminando = false;

		if (isAerialView) {
			if (mainWindow.getsKeys()[GLFW_KEY_W]) aerialPosition.z -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_S]) aerialPosition.z += aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_A]) aerialPosition.x -= aerialSpeed * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_D]) aerialPosition.x += aerialSpeed * deltaTime;
		}
		else if (!isTourCamera) {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			if (mainWindow.getsKeys()[GLFW_KEY_W]) { sephiPosition += sephiFront * moveSpeed * deltaTime; estaCaminando = true; }
			if (mainWindow.getsKeys()[GLFW_KEY_S]) { sephiPosition -= sephiFront * moveSpeed * deltaTime; estaCaminando = true; }
			if (mainWindow.getsKeys()[GLFW_KEY_A]) sephiRotationY += 5.0f * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_D]) sephiRotationY -= 5.0f * deltaTime;
			if (mainWindow.getsKeys()[GLFW_KEY_SPACE] && !isJumping) { sephiVelocityY = jumpPower; isJumping = true; }
		}

		if (isJumping) {
			sephiVelocityY += gravity * deltaTime; sephiPosition.y += sephiVelocityY * deltaTime;
			if (sephiPosition.y <= floorLevel) { sephiPosition.y = floorLevel; isJumping = false; sephiVelocityY = 0.0f; }
		}

		if (estaCaminando) {
			float tiempo = glfwGetTime() * 5.0f;
			angBra1 = (1.0f - cosf(tiempo)) * 15.0f;
			angPie1 = sinf(tiempo) * 25.0f;
			angPie2 = -sinf(tiempo) * 25.0f;
		}
		else {
			angBra1 = 0.0f; angPie1 = 0.0f; angPie2 = 0.0f;
		}

		// --- LÓGICA DE POSICIÓN DE CÁMARA ---
		glm::vec3 cameraPos, cameraTarget;
		glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		if (isTourCamera) {
			float tourSpeed = 0.2f;
			float tourRadius = 35.0f;
			float timeVal = glfwGetTime() * tourSpeed;

			cameraPos = glm::vec3(sin(timeVal) * tourRadius, 25.0f, cos(timeVal) * tourRadius - 15.0f);
			cameraTarget = glm::vec3(0.0f, 5.0f, -15.0f);
		}
		else if (isAerialView) {
			cameraPos = aerialPosition; cameraTarget = cameraPos + glm::vec3(0.0f, -1.0f, -0.01f);
		}
		else if (isThirdPerson) {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			cameraPos = sephiPosition - (sephiFront * 5.0f) + glm::vec3(0.0f, 5.0f, 0.0f);
			cameraTarget = sephiPosition + glm::vec3(0.0f, sephiYOffset + 0.6f, 0.0f);
		}
		else {
			glm::vec3 sephiFront(sin(sephiRotationY * toRadians), 0.0f, cos(sephiRotationY * toRadians));
			cameraPos = sephiPosition + glm::vec3(0.0f, sephiYOffset + 2.0f, 0.0f); cameraTarget = cameraPos + sephiFront;
		}

		glm::mat4 customViewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

		// --- RENDERIZADO PRINCIPAL ---
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (blend < 0.5f) { skyDay.DrawSkybox(customViewMatrix, projection); }
		else { skyNight.DrawSkybox(customViewMatrix, projection); }

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
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		glUniform3fv(uniformColor, 1, glm::value_ptr(colorBlanco));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		// --- DIBUJAR PISO ---
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, floorLevel, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f, 1.0f, 100.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();

		// --- DIBUJAR IGLESIA ---
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, floorLevel - 2.6f, -15.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		iglesiaTexture.UseTexture();
		Iglesia_M.RenderModel();

		// --- DIBUJAR AVATAR ---
		if (isThirdPerson || isAerialView || isTourCamera) {
			glm::mat4 joint_torso = glm::translate(glm::mat4(1.0f), sephiPosition + glm::vec3(0.0f, sephiYOffset, 0.0f));
			joint_torso = glm::rotate(joint_torso, sephiRotationY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(glm::scale(joint_torso, glm::vec3(sephiScale))));
			Sephi_Torso.RenderModel();

			glm::mat4 joint_bra1 = glm::rotate(glm::translate(joint_torso, offHombroIzq), angBra1 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(glm::scale(joint_bra1, glm::vec3(sephiScale))));
			Sephi_Bra1.RenderModel();

			glm::mat4 joint_bra2 = glm::rotate(glm::translate(joint_torso, offHombroDer), angBra2 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(glm::scale(joint_bra2, glm::vec3(sephiScale))));
			Sephi_Bra2.RenderModel();

			glm::mat4 joint_pie1 = glm::rotate(glm::translate(joint_torso, offCaderaIzq), angPie1 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(glm::scale(joint_pie1, glm::vec3(sephiScale))));
			Sephi_Pie1.RenderModel();

			glm::mat4 joint_pie2 = glm::rotate(glm::translate(joint_torso, offCaderaDer), angPie2 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(glm::scale(joint_pie2, glm::vec3(sephiScale))));
			Sephi_Pie2.RenderModel();
		}

		// --- RENDERIZADO CON PARCHE BLANCO ---
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texBlanca);
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		// --- DIBUJAR TIFA ---
		glm::mat4 modelTifa = glm::mat4(1.0f);
		modelTifa = glm::translate(modelTifa, glm::vec3(0.0f, floorLevel + tifaYOffset, 5.0f));
		modelTifa = glm::scale(modelTifa, glm::vec3(tifaScale));
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorBlanco));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelTifa));
		Tifa_M.RenderModel();

		// --- DIBUJAR LAMPARAS ---
		for (int i = 0; i < 4; i++) {
			glm::mat4 modelLamp = glm::mat4(1.0f);
			modelLamp = glm::translate(modelLamp, posLamparas[i]);
			modelLamp = glm::scale(modelLamp, glm::vec3(1.8f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelLamp));
			Lamp_M.RenderModel();
		}

		// --- DIBUJAR RIELES ---
		glm::mat4 joint_riel = glm::mat4(1.0f);
		joint_riel = glm::translate(joint_riel, locPosition);
		joint_riel = glm::rotate(joint_riel, locRotX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		joint_riel = glm::rotate(joint_riel, locRotY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		joint_riel = glm::rotate(joint_riel, locRotZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		joint_riel = glm::scale(joint_riel, glm::vec3(rielScale));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(joint_riel));
		Riel_M.RenderModel();

		// --- MATRIZ BASE TREN ---
		glm::mat4 joint_loc = glm::mat4(1.0f);
		joint_loc = glm::translate(joint_loc, locPosition);
		joint_loc = glm::rotate(joint_loc, locRotX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		joint_loc = glm::rotate(joint_loc, locRotY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		joint_loc = glm::rotate(joint_loc, locRotZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		joint_loc = glm::scale(joint_loc, glm::vec3(locScale));

		// --- DIBUJAR LOCOMOTORA ---
		glm::vec3 colorCuerpo = glm::vec3(0.1f, 0.1f, 0.1f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorCuerpo));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(joint_loc));
		Loc_M.RenderModel();

		auto DrawStaticPiece = [&](Model& m, glm::vec3 offset) {
			glm::mat4 joint_piece = glm::translate(joint_loc, offset);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(joint_piece));
			m.RenderModel();
			};

		// Ruedas 
		glm::vec3 colorAmarilloVerdoso = glm::vec3(0.880f, 0.926f, 0.427f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorAmarilloVerdoso));
		DrawStaticPiece(Loc_r1, offR1); DrawStaticPiece(Loc_r2, offR2); DrawStaticPiece(Loc_r3, offR3);
		DrawStaticPiece(Loc_r4, offR4); DrawStaticPiece(Loc_r5, offR5); DrawStaticPiece(Loc_r6, offR6);

		// Palos 
		glm::vec3 colorGrisClaro = glm::vec3(0.7f, 0.7f, 0.7f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(colorGrisClaro));
		DrawStaticPiece(Loc_p1, offP1); DrawStaticPiece(Loc_p2, offP2);

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}