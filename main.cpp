#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "tigl.h"
#include "ObjModel.h"
#include "GameObject.h"
#include "CameraComponent.h"
#include "MoveToComponent.h"
#include "RotateComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "RayCastComponent.h"

#include <irrKlang.h>
#pragma comment(lib, "irrKlang.lib")

#include "Camera.h"
#include "ModelComponent.h"
#include "HUDComponent.h"
#include "Timerf.h"
#include <random>

#include "GUIManager.h"
#include "MoveEnemyComponent.h"
#include "Spawnpoint.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "PlayerComponent.h"
#include "Webcam.h"

#define CAMERA_SPAWN glm::vec3(-5.0f, 60.0f, -20.0f);
#define OFFSET 75

using tigl::Vertex;

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

GLFWwindow* window;
GUIManager* guiManager;
Webcam* webcam;
std::vector<ObjModel*> models;

double lastFrameTime = 0;
bool drawGui = true;
bool drawEndScreen = false;
int score = 0;
int volume = 100;

int spawnPointIndex = 0;
bool cameraMoving = false;
bool spawnEnemies = false;

void init();
void update();
void draw();
void enableLight(bool state);
void renderEndGUI();
void renderGUI();
void setColorGui();

std::list<std::shared_ptr<GameObject>> objects;
std::shared_ptr<GameObject> camera;
irrklang::ISoundEngine* soundEngine;

Camera* debugCamera;

glm::mat4 projection;

// Callback for screen resizer
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    // Voer eventuele andere logica uit op basis van de nieuwe venstergrootte
}

int main(void)
{
    if (!glfwInit())
        throw "Could not initialize glwf";
    window = glfwCreateWindow(1400, 800, "Super Mario", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        throw "Could not initialize glwf";
    }
    glfwMakeContextCurrent(window);

    tigl::init();
    init();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        update();
        draw();

        if (drawGui)
        {
           // guiManager->renderGUI(camera);
        }
        if (drawEndScreen)
        {
            guiManager->renderEndGUI(window, camera, score);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

bool turning = false;

void init()
{
    // Initialising of soundEngine
    soundEngine = irrklang::createIrrKlangDevice();
    soundEngine->setSoundVolume(static_cast<float>(volume) / 100);
    irrklang::ISoundSource* soundSource = soundEngine->addSoundSourceFromFile("sounds/mariotheme.mp3");

    // Seed for random
    srand(time(nullptr));

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE)
                glfwSetWindowShouldClose(window, true);
        });

    models.push_back(new ObjModel("models/world/world.obj"));
    models.push_back(new ObjModel("models/goomba/Goomba_Mario.obj"));
    models.push_back(new ObjModel("models/boo/Boo_Mario.obj"));

    guiManager = new GUIManager(drawGui, drawEndScreen, soundEngine, volume, &spawnEnemies);

    guiManager->init(window);
    //debugCamera = new Camera(window);

    camera = std::make_shared<GameObject>();
    camera->position = CAMERA_SPAWN
    
    camera->addComponent(std::make_shared<CameraComponent>(window));
    camera->addComponent(std::make_shared<RotateComponent>());
    //camera->addComponent(std::make_shared<PlayerComponent>());

    webcam = new Webcam(window);
    auto hudComponent = std::make_shared<HUDComponent>(window, webcam, "webcam");
    camera->addComponent(hudComponent);

    auto rayCastComponent = std::make_shared<RayCastComponent>(
        webcam->getResolution(),
        &projection,
        webcam->getPoints()
    );
    camera->addComponent(rayCastComponent);

    auto gameWorld = std::make_shared<GameObject>();
    gameWorld->position = glm::vec3(0, 0 ,0);
    gameWorld->addComponent(std::make_shared<ModelComponent>(models[0]));
    objects.push_back(gameWorld);
    
    auto goomba = std::make_shared<GameObject>();
    goomba->position = glm::vec3(50, 0, 50);
    goomba->addComponent(std::make_shared<ModelComponent>(models[1]));

    enableLight(true);
    irrklang::ISound* sound = soundEngine->play2D(soundSource, false, false, true);
}

void update()
{
    double frameTime = glfwGetTime();
    float deltaTime = frameTime - lastFrameTime;
    lastFrameTime = frameTime;
    camera->update(deltaTime);

    for (const auto& object : objects)
    {
        object->update(deltaTime);
    }
    //debugCamera->update(window);
    if (!spawnEnemies) return;

    auto goomba = std::make_shared<GameObject>();
    goomba->position = glm::vec3(-(camera->position.x + (-sin(camera->rotation.y) * OFFSET)), camera->position.y, -(camera->position.z + (cos(camera->rotation.y) * OFFSET)));
    if((camera->rotation.y <= 100 && camera->rotation.y > 80) || camera->rotation.y > 230)
    {
        goomba->rotation.y = camera->rotation.y + glm::radians(180.f);
    }
	else
	{
        goomba->rotation.y = -camera->rotation.y;
	}
    
    goomba->addComponent(std::make_shared<ModelComponent>(models[2]));
    goomba->addComponent(std::make_shared<MoveEnemyComponent>(camera));
    objects.push_back(goomba);
    spawnEnemies = false;
}

void draw()
{
    glClearColor(186.f / 255, 174.f / 255, 145.f / 255, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    projectionMatrix = glm::perspective(glm::radians(75.0f), viewport[2] / (float)viewport[3], 0.01f, 250.0f);

    auto cameraComponent = camera->getComponent<CameraComponent>();

    tigl::shader->setProjectionMatrix(projectionMatrix);
    tigl::shader->setViewMatrix(cameraComponent->getMatrix());
    //tigl::shader->setViewMatrix(debugCamera->getMatrix());
    tigl::shader->setModelMatrix(glm::mat4(1.0f));

    tigl::shader->enableColor(true);

    glEnable(GL_DEPTH_TEST);

    glPointSize(10.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (auto& o : objects)
    {
        o->draw();
    }

    camera->getComponent<RayCastComponent>()->draw();
    camera->getComponent<HUDComponent>()->draw();
}

void enableLight(bool state)
{
    if (state) {
        tigl::shader->enableLighting(true);
        tigl::shader->setLightCount(1);
        tigl::shader->setLightDirectional(0, false);
        tigl::shader->setLightPosition(0, glm::vec3(0, 25, 0));
        tigl::shader->setLightAmbient(0, glm::vec3(123.f / 255, 137.f / 255, 147.f / 255));
        tigl::shader->setLightDiffuse(0, glm::vec3(0.8f, 0.8f, 0.8f));
        tigl::shader->setLightSpecular(1, glm::vec3(225.f / 255, 159.f / 255, 0));
        tigl::shader->setShinyness(5.f);
    }
    else {
        tigl::shader->enableLighting(false);
    }
}