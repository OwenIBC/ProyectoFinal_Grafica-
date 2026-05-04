/*
Práctica 8: Iluminación 2 
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
#include "Sphere.h"
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

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture cocheTexture;
Texture dadoTexture;

Model Kitt_M;
Model Llanta_M;
Model Llanta_M2;
Model Llanta_M3;
Model Llanta_M4;
Model Parb;
Model Blackhawk_M;
Model Lamp_M;

Skybox skybox;


Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

GLfloat heliX = 0.0f;

// Variables controles de luces
bool lampOn = true;
bool lKeyPressed = false;

int colorIndex = 0;
bool cKeyPressed = false;


DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";



void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);
	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


void CrearDado()
{
	unsigned int octa_indices[] = {
		0, 1, 2,    // Cara 1
		3, 4, 5,    // Cara 2
		6, 7, 8,    // Cara 3
		9, 10, 11,  // Cara 4
		12, 13, 14, // Cara 5
		15, 16, 17, // Cara 6
		18, 19, 20, // Cara 7
		21, 22, 23  // Cara 8
	};

	
	
	GLfloat octa_vertices[] = {
		// x     y      z         S      T          NX       NY       NZ 

	// MITAD SUPERIOR 
		// Cara 1 
		 0.0f,  0.5f,  0.0f,    0.5f, 0.25f,     -0.577f, 0.577f, 0.577f,
		-0.5f,  0.0f,  0.0f,    0.75f, 0.5f,     -0.577f, 0.577f, 0.577f,
		 0.0f,  0.0f,  0.5f,    0.25f, 0.5f,     -0.577f, 0.577f, 0.577f,

		 // Cara 2 
		 0.0f,  0.5f,  0.0f,    0.0f, 0.25f,     0.577f, 0.577f, 0.577f,
		 0.0f,  0.0f,  0.5f,    0.5f, 0.25f,     0.577f, 0.577f, 0.577f,
		 0.5f,  0.0f,  0.0f,    0.25f, 0.5f,     0.577f, 0.577f, 0.577f,

		 // Cara 3 
		 0.0f,  0.5f,  0.0f,    0.25f, 0.0f,     0.577f, 0.577f, -0.577f,
		 0.5f,  0.0f,  0.0f,    0.5f, 0.25f,     0.577f, 0.577f, -0.577f,
		 0.0f,  0.0f, -0.5f,    0.0f, 0.25f,     0.577f, 0.577f, -0.577f,

		 // Cara 4 
		 0.0f,  0.5f,  0.0f,    1.0f, 0.25f,     -0.577f, 0.577f, -0.577f,
		 0.0f,  0.0f, -0.5f,    0.75f, 0.5f,     -0.577f, 0.577f, -0.577f,
		-0.5f,  0.0f,  0.0f,    0.5f, 0.25f,     -0.577f, 0.577f, -0.577f,

		// MITAD INFERIOR 
			 // Cara 5 
			 0.0f, -0.5f,  0.0f,    0.5f, 0.75f,     -0.577f, -0.577f, 0.577f,
			 0.0f,  0.0f,  0.5f,    0.25f, 0.5f,     -0.577f, -0.577f, 0.577f,
			-0.5f,  0.0f,  0.0f,    0.75f, 0.5f,     -0.577f, -0.577f, 0.577f,

			// Cara 6 
			0.0f, -0.5f,  0.0f,    0.0f, 0.75f,     0.577f, -0.577f, 0.577f,
			0.5f,  0.0f,  0.0f,    0.25f, 0.5f,     0.577f, -0.577f, 0.577f,
			0.0f,  0.0f,  0.5f,    0.5f, 0.75f,     0.577f, -0.577f, 0.577f,

			// Cara 7 
			0.0f, -0.5f,  0.0f,    0.25f, 1.0f,     0.577f, -0.577f, -0.577f,
			0.0f,  0.0f, -0.5f,    0.0f, 0.75f,     0.577f, -0.577f, -0.577f,
			0.5f,  0.0f,  0.0f,    0.5f, 0.75f,     0.577f, -0.577f, -0.577f,

			// Cara 8 
			0.0f, -0.5f,  0.0f,    1.0f, 0.75f,     -0.577f, -0.577f, -0.577f,
		   -0.5f,  0.0f,  0.0f,    0.5f, 0.75f,     -0.577f, -0.577f, -0.577f,
			0.0f,  0.0f, -0.5f,    0.75f, 0.5f,     -0.577f, -0.577f, -0.577f
	};

	

	Mesh* dado = new Mesh();
	dado->CreateMesh(octa_vertices, octa_indices, 192, 24);
	meshList.push_back(dado);
}


void ProcesarTeclado()
{
	bool* keys = mainWindow.getsKeys();
	if (keys[GLFW_KEY_T]) {
		heliX -= 1.5f * deltaTime;
	}
	if (keys[GLFW_KEY_G]) {
		heliX += 1.5f * deltaTime;
	}

	// APAGAR/ENCENDER LÁMPARA 
	if (keys[GLFW_KEY_L]) {
		if (!lKeyPressed) {
			lampOn = !lampOn;
			lKeyPressed = true;
		}
	}
	else {
		lKeyPressed = false;
	}

	// COLOR DEL LUZ 
	if (keys[GLFW_KEY_C]) {
		if (!cKeyPressed) {
			colorIndex = (colorIndex + 1) % 6; 
			cKeyPressed = true;
		}
	}
	else {
		cKeyPressed = false;
	}
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();
	CreateObjects();
	CrearDado();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	cocheTexture = Texture("Textures/bob.png");
	cocheTexture.LoadTextureA();
	dadoTexture = Texture("Textures/dadomeme.png");
	dadoTexture.LoadTextureA();

	Kitt_M = Model();
	Kitt_M.LoadModel("Models/Coche.fbx");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/L1.fbx");
	Llanta_M2 = Model();
	Llanta_M2.LoadModel("Models/L2.fbx");
	Llanta_M3 = Model();
	Llanta_M3.LoadModel("Models/L3.fbx");
	Llanta_M4 = Model();
	Llanta_M4.LoadModel("Models/L4.fbx");
	Parb = Model();
	Parb.LoadModel("Models/CoPa.fbx");
	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");

	Lamp_M = Model();
	Lamp_M.LoadModel("Models/lamp.fbx");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	
	


	Material_brillante = Material(10.0f, 32);
	Material_opaco = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);

	unsigned int pointLightCount = 0;

	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		-6.0f, 1.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	// Luz de lámpara.
	pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,
		-5.0f, 2.0f, -5.0f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;

	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.5f, 3.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;


	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		1.0f, 5.0f, 
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		40.0f); 
	spotLightCount++;

	spotLights[2] = SpotLight(1.0f, 1.0f, 0.0f,
		1.0f, 2.0f,
		heliX, 5.0f, 6.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	GLfloat now = 0.0f;
	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);
	glm::vec3 color(1.0f, 1.0f, 1.0f);
	float escalaCoche = 1.2f;
	float escalaLlanta = 0.425f;
	glm::mat4 modelBaseCoche(1.0f);

	glm::vec4 localHeadlightPos(-0.88f, -1.0f, 0.91f, 1.0f);
	glm::vec4 localHeadlightDir(0.0f, -1.0f, -0.3f, 0.0f);

	glm::vec3 worldHeadlightPos;
	glm::vec3 worldHeadlightDir;
	glm::mat4 modelChasis(1.0f);

	while (!mainWindow.getShouldClose())
	{
		now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		ProcesarTeclado();

		
		

		if (lampOn) {

			pointLights[1] = PointLight(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -5.0f, 2.0f, -5.0f, 0.3f, 0.2f, 0.1f);
		}
		else {


			pointLights[1] = PointLight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -5.0f, 2.0f, -5.0f, 0.3f, 0.2f, 0.1f);
		}




		glm::vec3 headColor(1.0f);
		if (colorIndex == 0) headColor = glm::vec3(1.0f, 0.0f, 0.0f);      // Rojo
		else if (colorIndex == 1) headColor = glm::vec3(1.0f, 1.0f, 0.0f); // Amarillo
		else if (colorIndex == 2) headColor = glm::vec3(0.0f, 1.0f, 0.0f); // Verde
		else if (colorIndex == 3) headColor = glm::vec3(0.0f, 1.0f, 1.0f); // Cian
		else if (colorIndex == 4) headColor = glm::vec3(0.0f, 0.0f, 1.0f); // Azul
		else if (colorIndex == 5) headColor = glm::vec3(1.0f, 0.0f, 1.0f); // Magenta

		spotLights[1] = SpotLight(headColor.r, headColor.g, headColor.b,
			1.0f, 5.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			40.0f);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();

		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		
		glm::mat4 modelLinterna = glm::mat4(1.0f);
		modelLinterna = glm::translate(modelLinterna, camera.getCameraPosition());

		glm::vec4 localLinternaPos(0.0f, -0.3f, 0.0f, 1.0f);
		glm::vec3 worldLinternaPos = glm::vec3(modelLinterna * localLinternaPos);
		spotLights[0].SetFlash(worldLinternaPos, camera.getCameraDirection());

		
		glm::mat4 modelHelicopteroBase = glm::mat4(1.0f);
		modelHelicopteroBase = glm::translate(modelHelicopteroBase, glm::vec3(heliX, 5.0f, 6.0f));

		glm::vec4 localHeliLightPos(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 localHeliLightDir(0.0f, -1.0f, 0.0f, 0.0f);
		glm::vec3 worldHeliLightPos = glm::vec3(modelHelicopteroBase * localHeliLightPos);
		glm::vec3 worldHeliLightDir = glm::normalize(glm::vec3(modelHelicopteroBase * localHeliLightDir));

		spotLights[2].SetFlash(worldHeliLightPos, worldHeliLightDir);

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);


		model = glm::mat4(1.0f);
		modelaux = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		cocheTexture.UseTexture();

		modelBaseCoche = glm::mat4(1.0f);
		modelBaseCoche = glm::translate(modelBaseCoche, glm::vec3(0.0f + mainWindow.getmuevex(), -0.5f, -3.0f));
		modelBaseCoche = glm::rotate(modelBaseCoche, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

		worldHeadlightPos = glm::vec3(modelBaseCoche * localHeadlightPos);
		worldHeadlightDir = glm::normalize(glm::vec3(modelBaseCoche * localHeadlightDir));
		spotLights[1].SetFlash(worldHeadlightPos, worldHeadlightDir);

		modelChasis = glm::scale(modelBaseCoche, glm::vec3(escalaCoche, escalaCoche, escalaCoche));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelChasis));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Kitt_M.RenderModel();
		Parb.RenderModel();

		// --- LLANTAS ---
		model = modelBaseCoche;
		model = glm::translate(model, glm::vec3(-1.23f, 1.35f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(escalaLlanta));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		cocheTexture.UseTexture();
		Llanta_M.RenderModel();

		model = modelBaseCoche;
		model = glm::translate(model, glm::vec3(1.23f, 1.35f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(escalaLlanta));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		cocheTexture.UseTexture();
		Llanta_M2.RenderModel();

		model = modelBaseCoche;
		model = glm::translate(model, glm::vec3(-1.23f, -1.25f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaLlanta));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		cocheTexture.UseTexture();
		Llanta_M3.RenderModel();

		model = modelBaseCoche;
		model = glm::translate(model, glm::vec3(1.23f, -1.25f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, 180 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaLlanta));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		cocheTexture.UseTexture();
		Llanta_M4.RenderModel();

		// HELICÓPTERO 
		model = modelHelicopteroBase;
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Blackhawk_M.RenderModel();

		// DADO DE 8 CARAS
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.5f, 2.5f, -2.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();

		// Lampara 
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-3.0f, 2.18f, -5.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Lamp_M.RenderModel();

		//Agave
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}