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
#include "Timerf.h"
#include <random>
#include "Spawnpoint.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define CAMERA_SPAWN glm::vec3(-5.0f, 60.0f, -20.0f);

using tigl::Vertex;

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

GLFWwindow* window;
ObjModel* model;
char modelPath[] = "models/world/world.obj";
double lastFrameTime = 0;
bool drawGui = true;

Spawnpoint Spawnpoints[] = { Spawnpoint(glm::vec3(-170, 110, 150), 270), Spawnpoint(glm::vec3(188, 20, -20), 180)};

void init();
void update();
void draw();
void enableLight(bool state);
void renderGUI();
void setColorGui();

std::shared_ptr<GameObject> camera;

int main(void)
{
    if (!glfwInit())
        throw "Could not initialize glwf";
    window = glfwCreateWindow(1400, 800, "Hello World", NULL, NULL);
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
        
        if (drawGui) {
            renderGUI();
        }
        
        glfwSwapBuffers(window);
    }

    glfwTerminate();


    return 0;
}

bool turning = false;

void init()
{   
    srand(time(nullptr));

    // ImGui initialisatie
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Stijl aanpassen (optioneel)
    ImGui::StyleColorsDark();
    setColorGui();

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE)
                glfwSetWindowShouldClose(window, true);
        });

    camera = std::make_shared<GameObject>();
    camera->position = CAMERA_SPAWN;
    
    camera->addComponent(std::make_shared<CameraComponent>(window));
    
    camera->addComponent(std::make_shared<RotateComponent>());

    enableLight(true);

    model = new ObjModel(modelPath);

    //Timerf *t = new Timerf(2000, &turning);
    //t->startTimer();
}

void update()
{

    double frameTime = glfwGetTime();
    float deltaTime = frameTime - lastFrameTime;
    lastFrameTime = frameTime;
    
    /*
    if (turning) 
    {
        camera->removeComponent<RotateComponent>();
        auto iets = glm::vec3(-170, 110, 150);
        camera->addComponent(std::make_shared<MoveToComponent>(iets, 270));
        turning = false;
    }
    */
    
    camera->update(deltaTime);
}

void draw()
{
    glClearColor(186.f / 255, 174.f / 255, 145.f / 255, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glm::mat4 projection = glm::perspective(glm::radians(75.0f), viewport[2] / (float)viewport[3], 0.01f, 500.0f);

    auto cameraComponent = camera->getComponent<CameraComponent>();

    tigl::shader->setProjectionMatrix(projection);
    tigl::shader->setViewMatrix(cameraComponent->getMatrix());
    tigl::shader->setModelMatrix(glm::mat4(1.0f));

    tigl::shader->enableColor(true);

    glEnable(GL_DEPTH_TEST);


    glPointSize(10.0f);
    model->draw();
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

void renderGUI()
{
    // ImGui-start van het frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize = io.DisplaySize;
    ImVec2 guiSize = ImVec2(200, 100);  

    // Bereken de positie van het midden van het scherm
    ImVec2 guiPosition = ImVec2((windowSize.x - guiSize.x) * 0.5f, (windowSize.y - guiSize.y) * 0.5f);

    ImGui::SetNextWindowPos(guiPosition);
    ImGui::SetNextWindowSize(guiSize);

    // GUI-opbouw
    ImGui::Begin("Mijn GUI", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    ImVec2 buttonSize = ImVec2(185, 80);
    if (ImGui::Button("PLAY!", buttonSize))
    {
        // Actie wanneer er op de knop wordt geklikt
        std::cout << "De knop is geklikt!" << std::endl;
        camera->removeComponent<RotateComponent>();
        auto i = Spawnpoints[rand()%2];
        camera->addComponent(std::make_shared<MoveToComponent>(i.pos, i.rot));
        drawGui = false;
    }
    
    ImGui::End();

    // ImGui-renderen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (drawGui) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void setColorGui() 
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.01f, 0.01f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.69f, 0.47f, 0.47f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.64f, 0.41f, 0.41f, 0.69f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.54f, 0.27f, 0.27f, 0.83f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.63f, 0.32f, 0.32f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.80f, 0.40f, 0.40f, 0.20f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.55f, 0.40f, 0.40f, 0.80f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.40f, 0.40f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.80f, 0.40f, 0.40f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.80f, 0.39f, 0.39f, 0.60f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.39f, 0.39f, 0.60f);
    colors[ImGuiCol_Button] = ImVec4(0.74f, 0.17f, 0.17f, 0.62f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.39f, 0.39f, 0.79f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.86f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.81f, 0.23f, 0.23f, 0.45f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.80f, 0.14f, 0.14f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.08f, 0.08f, 0.80f);
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 0.78f, 0.78f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab] = ImVec4(0.41f, 0.01f, 0.01f, 0.79f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.68f, 0.43f, 0.43f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.10f, 0.10f, 0.84f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.57f, 0.28f, 0.28f, 0.82f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.65f, 0.35f, 0.35f, 0.84f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.90f, 0.45f, 0.45f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}