#include <string>
#include <vector>

#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
#error windows.h was included!
#endif

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/shader.h>
#include "surfaces/polynomial.h"
#include "surfaces/surface.h"
#include "surfaces/arrow_line.h"

using std::vector;

ImVec4 toImVec4(const glm::vec4 &vec);

glm::vec4 toGlmVec4(const ImVec4 &vec);

GLfloat *toFloatArray(const glm::vec4 &vec);
bool display2nDegreeGUI(vector<int> *xCoeffs2ndD, vector<int> *yCoeffs2ndD, vector<int> *zCoeffs2ndD);
bool display1stDegreeGUI(vector<int> *xCoeffs1stD, vector<int> *yCoeffs1stD, vector<int> *zCoeffs1stD);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
glm::vec3 calculateFaceNormal(Vertex a, Vertex b, Vertex c);
Mesh createSurfaceMesh(Polynomial &xPoly, Polynomial &yPoly, Polynomial &zPoly, glm::vec3 viewDir, int uSampleRate, int vSampleRate);
glm::vec3 calculateMeanPoint(const Mesh &mesh);

GLuint screenWidth = 1200, screenHeight = 900;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool wireframe = false;
bool coeffsChanged = false;

int isRotating = 0;
float orbitRadius = 40.0f;

const int coeffMin = -100;
const int coeffMax = 100;
const int defaultUSampleRate = 16;
const int defaultVSampleRate = 16;

int selectedColor = 0;

int main()
{
    // OpenGL init using GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Fogli semplici di superficie", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // MATRICES
    glm::mat4 surfaceModelMatrix = glm::mat4(1.0f);
    glm::mat3 surfaceNormalMatrix = glm::mat3(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth / (float)screenHeight, 0.1f, 10000.0f);

    // SHADERS
    Shader shader("src/shaders/illumination.vert", "src/shaders/illumination.frag");
    Shader flatShader("src/shaders/flat.vert", "src/shaders/flat.frag");

    // ILLUMINATION MODEL
    glm::vec3 pointLightPos = glm::vec3(0.0f, 1000.0f, 0.0f);

    vector<glm::vec4> surfaceColors = {
        {1.0f, 0.1f, 0.1f, 1.0f},
        {0.1f, 1.0f, 0.1f, 1.0f},
        {0.1f, 0.1f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.1f, 1.0f},
        {0.1f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.1f, 1.0f, 1.0f}};

    GLfloat specularColor[] = {1.0f, 1.0f, 1.0f};
    GLfloat ambientColor[] = {0.1f, 0.1f, 0.1f};
    GLfloat Kd = 0.940f;
    GLfloat Ks = 0.420f;
    GLfloat Ka = 0.670f;
    GLfloat shininess = 13.4f;

    // CAMERA
    float cameraRotationAngle = 0.0f;
    const float rotationSpeed = 5.0f;
    bool fixSurfaceInOrigin = false;

    // ImGui SETUP
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    bool menuIsActive = true;
    bool randomGen = false;
    bool firstDegreeMenuActive = false;
    bool secondDegreeMenuActive = false;
    int selectedPolynomialDegree = 1;

    // SURFACE'S PARAMETERS INIT
    int uSampleRate = defaultUSampleRate;
    int vSampleRate = defaultVSampleRate;

    vector<int> xCoeffs1stD = {0, 0, 0};
    vector<int> yCoeffs1stD = {0, 0, 0};
    vector<int> zCoeffs1stD = {0, 0, 0};

    vector<int> xCoeffs2ndD = {0, 0, 0, 0, 0, 0};
    vector<int> yCoeffs2ndD = {0, 0, 0, 0, 0, 0};
    vector<int> zCoeffs2ndD = {0, 0, 0, 0, 0, 0};

    Mesh surfaceMesh = Mesh(vector<Vertex>{}, vector<GLuint>{});

    // AXIS CREATION
    const GLfloat xColor[] = {1.0f, 0.0f, 0.0f, 0.7f};
    const GLfloat yColor[] = {0.0f, 1.0f, 0.0f, 0.7f};
    const GLfloat zColor[] = {0.0f, 0.0f, 1.0f, 0.7f};

    const glm::vec4 xColorVec = glm::vec4(xColor[0], xColor[1], xColor[2], xColor[3]);
    vector<Point> xPoints;
    vector<glm::vec3> xPointsPositions{
        glm::vec3(-100.0f, 0.0f, 0.0f),
        glm::vec3(100.0f, 0.0f, 0.0f),
        glm::vec3(100.0f, 5.0f, 0.0f),
        glm::vec3(100.0f, -5.0f, 0.0f),
        glm::vec3(105.0f, 0.0f, 0.0f)};
    for (auto &pos : xPointsPositions)
    {
        Point temp;
        temp.Position = pos;
        temp.Color = xColorVec;
        xPoints.push_back(temp);
    }

    const glm::vec4 yColorVec = glm::vec4(yColor[0], yColor[1], yColor[2], yColor[3]);
    vector<Point> yPoints;
    vector<glm::vec3> yPointsPositions{
        glm::vec3(0.0f, -100.0f, 0.0f),
        glm::vec3(0.0f, 100.0f, 0.0f),
        glm::vec3(5.0f, 100.0f, 0.0f),
        glm::vec3(-5.0f, 100.0f, 0.0f),
        glm::vec3(0.0f, 105.0f, 0.0f)};
    for (auto &pos : yPointsPositions)
    {
        Point temp;
        temp.Position = pos;
        temp.Color = yColorVec;
        yPoints.push_back(temp);
    }

    const glm::vec4 zColorVec = glm::vec4(zColor[0], zColor[1], zColor[2], zColor[3]);
    vector<Point> zPoints;
    vector<glm::vec3> zPointsPositions{
        glm::vec3(0.0f, 0.0f, -100.0f),
        glm::vec3(0.0f, 0.0f, 100.0f),
        glm::vec3(0.0f, 5.0f, 100.0f),
        glm::vec3(0.0f, -5.0f, 100.0f),
        glm::vec3(0.0f, 0.0f, 105.0f)};
    for (auto &pos : zPointsPositions)
    {
        Point temp;
        temp.Position = pos;
        temp.Color = zColorVec;
        zPoints.push_back(temp);
    }

    ArrowLine xAxis = ArrowLine(xPoints, 2);
    ArrowLine yAxis = ArrowLine(yPoints, 2);
    ArrowLine zAxis = ArrowLine(zPoints, 2);

    // MAIN LOOP
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // CAMERA UPDATE
        glm::vec3 cameraPosition = glm::vec3(0.0f, 20.0f, 0.0f);
        if (isRotating != 0)
        {
            cameraRotationAngle += isRotating * rotationSpeed * deltaTime;
        }
        cameraPosition.x = orbitRadius * glm::sin(cameraRotationAngle);
        cameraPosition.z = orbitRadius * glm::cos(cameraRotationAngle);

        view = glm::mat4(1.0f);
        view = glm::lookAt(cameraPosition, -cameraPosition, glm::vec3(0.0f, 1.0f, 0.0f));

        // AXIS RENDERING
        flatShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(flatShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(flatShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(flatShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        xAxis.Draw();
        yAxis.Draw();
        zAxis.Draw();

        // SURFACE CREATION

        Polynomial xPoly = selectedPolynomialDegree == 1 ? Polynomial(vector<float>(xCoeffs1stD.begin(), xCoeffs1stD.end())) : Polynomial(vector<float>(xCoeffs2ndD.begin(), xCoeffs2ndD.end()));
        Polynomial yPoly = selectedPolynomialDegree == 1 ? Polynomial(vector<float>(yCoeffs1stD.begin(), yCoeffs1stD.end())) : Polynomial(vector<float>(yCoeffs2ndD.begin(), yCoeffs2ndD.end()));
        Polynomial zPoly = selectedPolynomialDegree == 1 ? Polynomial(vector<float>(zCoeffs1stD.begin(), zCoeffs1stD.end())) : Polynomial(vector<float>(zCoeffs2ndD.begin(), zCoeffs2ndD.end()));
        glm::vec3 viewDir = glm::normalize(-cameraPosition);
        surfaceMesh = createSurfaceMesh(xPoly, yPoly, zPoly, viewDir, uSampleRate, vSampleRate);

        glm::vec3 meanPoint = calculateMeanPoint(surfaceMesh);

        surfaceModelMatrix = glm::mat4(1.0f);
        surfaceModelMatrix = glm::translate(surfaceModelMatrix, fixSurfaceInOrigin ? -meanPoint : glm::vec3(0.0f));
        surfaceModelMatrix = glm::scale(surfaceModelMatrix, glm::vec3(1.0f));
        surfaceNormalMatrix = glm::mat3(1.0f);
        surfaceNormalMatrix = glm::inverseTranspose(glm::mat3(view * surfaceModelMatrix));

        // SURFACE RENDERING
        shader.Use();

        glm::vec4 selColVec = surfaceColors[selectedColor];
        GLfloat selectedColorArr[] = {selColVec.x, selColVec.y, selColVec.z, selColVec.a};
        glUniform3fv(glGetUniformLocation(shader.Program, "diffuseColor"), 1, selectedColorArr);
        glUniform3fv(glGetUniformLocation(shader.Program, "ambientColor"), 1, ambientColor);
        glUniform3fv(glGetUniformLocation(shader.Program, "specularColor"), 1, specularColor);
        glUniform1f(glGetUniformLocation(shader.Program, "Kd"), Kd);
        glUniform1f(glGetUniformLocation(shader.Program, "Ka"), Ka);
        glUniform1f(glGetUniformLocation(shader.Program, "Ks"), Ks);
        glUniform1f(glGetUniformLocation(shader.Program, "shininess"), shininess);

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));

        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glUniform3fv(glGetUniformLocation(shader.Program, "lightPos"), 1, glm::value_ptr(cameraPosition));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(surfaceModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(surfaceNormalMatrix));

        surfaceMesh.Draw();

        // GUI RENDERING
        ImGui::Begin("Surface tools", &menuIsActive, ImGuiWindowFlags_MenuBar);
        ImGui::Text("Polynomials degree:");
        int changedPolyDegree = 0;
        changedPolyDegree += ImGui::RadioButton("1st degree polynomials", &selectedPolynomialDegree, 1);
        ImGui::SameLine();
        changedPolyDegree += ImGui::RadioButton("2nd degree polynomials", &selectedPolynomialDegree, 2);

        if (changedPolyDegree != 0)
        {
            surfaceMesh = Mesh(vector<Vertex>(), vector<GLuint>());
            xCoeffs1stD = {0, 0, 0};
            yCoeffs1stD = {0, 0, 0};
            zCoeffs1stD = {0, 0, 0};

            xCoeffs2ndD = {0, 0, 0, 0, 0, 0};
            yCoeffs2ndD = {0, 0, 0, 0, 0, 0};
            zCoeffs2ndD = {0, 0, 0, 0, 0, 0};
        }

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Positioning:");
        ImGui::Checkbox("Fix surface in the origin", &fixSurfaceInOrigin);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Surface color: ");
        for (int i = 0; i < surfaceColors.size(); i++)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, toImVec4(surfaceColors[i]));
            const char label[] = {'#', '#', 'c', 'o', 'l', 'o', 'r', '-', i};
            selectedColor = ImGui::Button(label, ImVec2(25, 25)) ? i : selectedColor;
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Polynomial coefficients:");

        if (selectedPolynomialDegree == 1)
        {
            display1stDegreeGUI(&xCoeffs1stD, &yCoeffs1stD, &zCoeffs1stD);
        }
        else
        {
            display2nDegreeGUI(&xCoeffs2ndD, &yCoeffs2ndD, &zCoeffs2ndD);
        }
        ImGui::BeginChild(3, ImVec2(500, 75));
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Sampling rate");
        int sampleRateChanged = 0;
        sampleRateChanged += ImGui::SliderInt("U sample rate", &uSampleRate, 2, 64);
        sampleRateChanged += ImGui::SliderInt("V sample rate", &vSampleRate, 2, 64);
        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    shader.Delete();
    flatShader.Delete();
    surfaceMesh.~Mesh();
    xAxis.~ArrowLine();
    yAxis.~ArrowLine();
    zAxis.~ArrowLine();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}

bool display1stDegreeGUI(vector<int> *xCoeffs1stD, vector<int> *yCoeffs1stD, vector<int> *zCoeffs1stD)
{
    int coeffChangeCount = 0;
    // 1st degree polynomial menu
    ImGui::BeginChild(2, ImVec2(500, 260));
    ImGui::Text("X coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u0", &xCoeffs1stD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v0", &xCoeffs1stD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant0", &xCoeffs1stD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");

    ImGui::Text("Y coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u1", &yCoeffs1stD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v1", &yCoeffs1stD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant1", &yCoeffs1stD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");

    ImGui::Text("Z coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u2", &zCoeffs1stD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v2", &zCoeffs1stD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant2", &zCoeffs1stD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");
    ImGui::EndChild();

    return coeffChangeCount != 0;
}

bool display2nDegreeGUI(vector<int> *xCoeffs2ndD, vector<int> *yCoeffs2ndD, vector<int> *zCoeffs2ndD)
{
    int coeffChangeCount = 0;
    ImGui::BeginChild(2, ImVec2(500, 465));
    ImGui::Text("X coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u0^2", &xCoeffs2ndD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u^2");
    coeffChangeCount += ImGui::SliderInt("##u0v0", &xCoeffs2ndD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("uv");
    coeffChangeCount += ImGui::SliderInt("##v0^2", &xCoeffs2ndD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v^2");
    coeffChangeCount += ImGui::SliderInt("##u0", &xCoeffs2ndD->at(3), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v0", &xCoeffs2ndD->at(4), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant0", &xCoeffs2ndD->at(5), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");

    ImGui::Text("Y coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u1^2", &yCoeffs2ndD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u^2");
    coeffChangeCount += ImGui::SliderInt("##u1v1", &yCoeffs2ndD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("uv");
    coeffChangeCount += ImGui::SliderInt("##v1^2", &yCoeffs2ndD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v^2");
    coeffChangeCount += ImGui::SliderInt("##u1", &yCoeffs2ndD->at(3), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v1", &yCoeffs2ndD->at(4), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant1", &yCoeffs2ndD->at(5), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");

    ImGui::Text("Z coefficients: ");
    coeffChangeCount += ImGui::SliderInt("##u2^2", &zCoeffs2ndD->at(0), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u^2");
    coeffChangeCount += ImGui::SliderInt("##u2v2", &zCoeffs2ndD->at(1), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("uv");
    coeffChangeCount += ImGui::SliderInt("##v2^2", &zCoeffs2ndD->at(2), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v^2");
    coeffChangeCount += ImGui::SliderInt("##u2", &zCoeffs2ndD->at(3), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("u");
    coeffChangeCount += ImGui::SliderInt("##v2", &zCoeffs2ndD->at(4), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("v");
    coeffChangeCount += ImGui::SliderInt("##constant2", &zCoeffs2ndD->at(5), coeffMin, coeffMax, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::Text("constant");
    ImGui::EndChild();
    return coeffChangeCount != 0;
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        orbitRadius -= 10.0f;
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        orbitRadius += 10.0f;
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        isRotating = -1;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        isRotating = 1;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        isRotating = 0;
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        isRotating = 0;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        wireframe = !wireframe;
}

glm::vec3 calculateFaceNormal(Vertex a, Vertex b, Vertex c)
{
    glm::vec3 v1 = a.Position - b.Position;
    glm::vec3 v2 = c.Position - b.Position;
    return glm::normalize(glm::cross(v2, v1));
}

Mesh createSurfaceMesh(Polynomial &xPoly, Polynomial &yPoly, Polynomial &zPoly, glm::vec3 viewDir, int uSampleRate, int vSampleRate)
{
    PolynomialSurface *surfacePntr = nullptr;

    PolynomialSurface surface = PolynomialSurface({&xPoly, &yPoly, &zPoly});

    vector<Vertex> samples = surface.computeGrid(uSampleRate, vSampleRate);
    vector<GLuint> indices;
    for (size_t i = 0; i < uSampleRate - 1; i++)
    {
        for (size_t j = 0; j < vSampleRate - 1; j++)
        {
            // 1st triangle
            int indexA, indexB, indexC;
            indexA = i * vSampleRate + j;
            indexB = i * vSampleRate + j + 1;
            indexC = (i + 1) * vSampleRate + j + 1;
            indices.push_back(indexA);
            indices.push_back(indexB);
            indices.push_back(indexC);


            // 2nd triangle
            indexA = i * vSampleRate + j;
            indexB = (i + 1) * vSampleRate + j + 1;
            indexC = (i + 1) * vSampleRate + j;
            indices.push_back(indexA);
            indices.push_back(indexB);
            indices.push_back(indexC);
        }
    }

    for (auto& sample : samples) {
        if (glm::dot(sample.Normal, viewDir) < 0) {
            sample.Normal = -sample.Normal;
        }
    }

    return Mesh(samples, indices);
}

glm::vec3 calculateMeanPoint(const Mesh &mesh)
{
    glm::vec3 sum = glm::vec3(0.0);
    for (auto &vertex : mesh.vertices)
    {
        sum += vertex.Position;
    }
    return (1.0f / mesh.vertices.size()) * sum;
}

ImVec4 toImVec4(const glm::vec4 &vec)
{
    return ImVec4(vec.x, vec.y, vec.z, vec.a);
}

glm::vec4 toGlmVec4(const ImVec4 &vec)
{
    return glm::vec4(vec.x, vec.y, vec.z, vec.w);
}

GLfloat *toFloatArray(const glm::vec4 &vec)
{
    GLfloat temp[] = {vec.x, vec.y, vec.z, vec.a};
    return temp;
}