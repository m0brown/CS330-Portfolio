/*
* Marysha Brown
* CS330 Computer Graphic and Visualization
* Final Project
*/
#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GLEW/glew.h>      // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <numbers> 

#include <camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Marysha Brown - CS330 Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 900;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handle for the vertex buffer object
        GLuint nVertices;   // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle mesh data
    GLMesh gMesh;
    GLMesh gLightMesh;
    GLMesh gAmbMesh;
    GLMesh gTableMesh;
    GLMesh gBookMesh;
    GLMesh gNotebookMesh;
    GLMesh gFluoriteMesh;
    GLMesh gBallMesh;
    GLMesh gScentBaseMesh;
    GLMesh gScentTopMesh;
    GLMesh gCoasterMesh;
    GLMesh gLightBaseMesh;
    GLMesh gLightTopMesh;

    // Texture
    GLuint gTextureId;
    GLuint gTableTextureId;
    GLuint gBookTextureId;
    GLuint gNotebookTextureId;
    GLuint gFluoriteTextureId;
    GLuint gBallTextureId;
    GLuint gScentBaseTextureId;
    GLuint gScentTopTextureId;
    GLuint gCoasterTextureId;
    GLuint gLightBaseTextureId;
    GLuint gLightTopTextureId;

    glm::vec2 gUVScale(1.0f, 1.0f); //Texture Scale
    GLint gTexWrapMode = GL_REPEAT; //Texture wrapping

    // Shader programs
    GLuint gProgramId;
    GLuint gLampProgramId;

    //Camera
    Camera gCamera(glm::vec3(0.0f, 10.0f, 30.0f)); //Camera location
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    //Timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    //Objects color
    glm::vec3 gObjectColor(1.f, 1.0f, 1.0f);

    //Spotlight color, position, scale
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f); //White
    glm::vec3 gLightPosition(10.f, 20.f, 20.0f);
    glm::vec3 gLightScale(0.5f);

    //Ambient light color, position, scale
    glm::vec3 gAmbLightColor(0.780f, 0.082f, 0.522f); //Light Salmon
    glm::vec3 gAmbLightPosition(-10.f, 20.f, 20.0f);
    glm::vec3 gAmbLightScale(0.5f);

    // Lamp animation
    bool gIsLampOrbiting = false;

    //View
    bool gOrthoView = false;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateCubeMesh(GLMesh& mesh);
void UCreatePyramidMesh(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreateFluoriteMesh(GLMesh& mesh);
void UCreateCylinderMesh(GLMesh& mesh);
void UCreateSphereMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//----------------------
//VERTEX SHADER SOURCE CODE
//----------------------
const GLchar* vertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal; // VAP position 1 for normals
    layout(location = 2) in vec2 textureCoordinate;

    //back
    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate; //For outgoing texture

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);

//----------------------
//FRAGMENT SHADER SOURCE CODE
//----------------------
const GLchar* fragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;//For incoming texture coordinates

    out vec4 fragmentColor; // For outgoing color to the GPU

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 ambientLightColor;
    uniform vec3 lightPos;
    uniform vec3 ambientLightPos;
    uniform vec3 viewPosition;
    uniform vec3 ambViewPosition;

    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Spotlight lighting
        float spotStrength = 0.1f; // Set ambient or global lighting strength
        vec3 spot = spotStrength * lightColor; // Generate ambient light color

        //Calculate Ambient lighting
        float ambientStrength = 0.2f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * ambientLightColor; // Generate key light color

        //Calculate Diffuse lighting
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on pyramid
        vec3 ambientLightDirection = normalize(ambientLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on pyramid

        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        float ambientImpact = max(dot(norm, ambientLightDirection), 0.0);// Calculate key impact by generating dot product of normal and light

        vec3 diffuse = impact * lightColor; // Generate diffuse light color
        vec3 ambientDiffuse = ambientImpact * ambientLightColor; // Generate key light color

        //Calculate Specular lighting
        float specularIntensity = 0.1f; // Set specular light strength
        float highlightSize = 10.0f; // Set specular highlight size

        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 ambViewDir = normalize(ambViewPosition - vertexFragmentPos); // Calculate ambient view direction

        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        vec3 ambReflectDir = reflect(-ambientLightDirection, norm);// Calculate ambient reflection vector

        //Calculate specular components
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        float ambSpecularComponent = pow(max(dot(ambViewDir, ambReflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;
        vec3 ambientSpecular = specularIntensity * ambSpecularComponent * ambientLightColor;

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate phong result
        vec3 phong = (spot + ambient + diffuse + specular) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }
);

//----------------------
//LAMP SHADER SOURCE CODE
//----------------------
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


//----------------------
//LAMP FRAGMENT SHADER SOURCE CODE
//----------------------
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

//----------------------
//MAIN FUNCTION
//----------------------
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow)) {
        return EXIT_FAILURE;
    }

    //----------------------
    //CREATE THE MESHES
    //----------------------
    //UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
    //UCreatePyramidMesh(gPyramidMesh); //Pyramid
    UCreateCubeMesh(gLightMesh);                //Spotlight
    UCreateCubeMesh(gAmbMesh);                  //Ambient light
    UCreatePlaneMesh(gTableMesh);               //Tabletop
    UCreateCubeMesh(gBookMesh);                 //Book
    UCreateCubeMesh(gNotebookMesh);             //Notebook
    UCreateFluoriteMesh(gFluoriteMesh);         //Fluorite
    UCreateSphereMesh(gBallMesh);               //Lacrosse Ball
    UCreateCylinderMesh(gScentBaseMesh);        //Scent Base
    UCreateCylinderMesh(gScentTopMesh);         //Scent Top
    UCreateCubeMesh(gCoasterMesh);              //Coaster
    UCreateCylinderMesh(gLightBaseMesh);        //Light Base
    UCreateSphereMesh(gLightTopMesh);           //Light Top

    //----------------------
    //CREATE THE SHADER PROGRAMS
    //----------------------
    //if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId)) {
        cout << "ERROR CREATING SHADER." << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId)) {
        cout << "ERROR CREATING LAMP SHADER." << endl;
        return EXIT_FAILURE;
    }

    //----------------------
    //LOAD TEXTURES
    //----------------------

    //TABLETOP TEXTURE
    //----------------
    const char* texFilename = "include/table.jpg";
    if (!UCreateTexture(texFilename, gTableTextureId)) {
        cout << "ERROR LOADING TABLETOP TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "tableTexture"), 0);

    //BOOK TEXTURE
    //----------------
    texFilename = "include/bluebook.png";
    if (!UCreateTexture(texFilename, gBookTextureId)) {
        cout << "ERROR LOADING BOOK TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "bookTexture"), 0);

    //NOTEBOOK TEXTURE
    //----------------
    texFilename = "include/notebook.png";
    if (!UCreateTexture(texFilename, gNotebookTextureId)) {
        cout << "ERROR LOADING NOTEBOOK TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "notebookTexture"), 0);


    //FLUORITE TEXTURE
    //----------------
    texFilename = "include/fluorite.png";
    if (!UCreateTexture(texFilename, gFluoriteTextureId)) {
        cout << "ERROR LOADING FLUORITE TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "fluoriteTexture"), 0);


    //LACROSSE BALL TEXTURE
    //----------------
    texFilename = "include/ball.png";
    if (!UCreateTexture(texFilename, gBallTextureId)) {
        cout << "ERROR LOADING LAX BALL TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "ballTexture"), 0);


    //SCENT BASE TEXTURE
    //----------------
    texFilename = "include/green.png";
    if (!UCreateTexture(texFilename, gScentBaseTextureId)) {
        cout << "ERROR LOADING SCENT BASE TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "scentBaseTexture"), 0);


    //SCENT TOP TEXTURE
    //----------------
    texFilename = "include/cap.png";
    if (!UCreateTexture(texFilename, gScentTopTextureId)) {
        cout << "ERROR LOADING SCENT TOP TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "scentTopTexture"), 0);


    //COASTER TEXTURE
    //----------------
    texFilename = "include/coaster.png";
    if (!UCreateTexture(texFilename, gCoasterTextureId)) {
        cout << "ERROR LOADING COASTER TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "coasterTexture"), 0);


    //LIGHT BASE TEXTURE
    //----------------
    texFilename = "include/lightbottom.png";
    if (!UCreateTexture(texFilename, gLightBaseTextureId)) {
        cout << "ERROR LOADING LIGHT BASE TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "lightBaseTexture"), 0);


    //LIGHT TOP TEXTURE
    //----------------
    texFilename = "include/lighttop.png";
    if (!UCreateTexture(texFilename, gLightTopTextureId)) {
        cout << "ERROR LOADING LIGHT TOP TEXTURE." << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Tell opengl for each sampler to which texture unit it belongs to
    glUseProgram(gProgramId);

    //Set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "lightTopTexture"), 0);




    //Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Render loop
    //-----------
    while (!glfwWindowShouldClose(gWindow))
    {
        //Per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gLightMesh);           //Spotlight
    UDestroyMesh(gAmbMesh);             //Ambient light
    UDestroyMesh(gTableMesh);           //Tabletop
    UDestroyMesh(gBookMesh);            //Book
    UDestroyMesh(gNotebookMesh);        //Notebook
    UDestroyMesh(gFluoriteMesh);        //Fluorite
    UDestroyMesh(gBallMesh);            //Lacrosse Ball
    UDestroyMesh(gScentBaseMesh);       //Scent Base
    UDestroyMesh(gScentTopMesh);        //Scent Top
    UDestroyMesh(gCoasterMesh);         //Coaster
    UDestroyMesh(gLightBaseMesh);       //Light Base
    UDestroyMesh(gLightTopMesh);        //Light Top

    // Release texture
    UDestroyTexture(gTableTextureId);       //Tabletop
    UDestroyTexture(gBookTextureId);        //Book
    UDestroyTexture(gNotebookTextureId);    //Notebook
    UDestroyTexture(gFluoriteTextureId);    //Fluorite
    UDestroyTexture(gBallTextureId);        //Lacrosse Ball
    UDestroyTexture(gScentBaseTextureId);   //Scent Base
    UDestroyTexture(gScentTopTextureId);    //Scent Top
    UDestroyTexture(gCoasterTextureId);     //Coaster
    UDestroyTexture(gLightBaseTextureId);   //Light Base
    UDestroyTexture(gLightTopTextureId);    //Light Top

    // Release shader programs
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "ERROR CREATING GLFW WINDOW" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "OpenGL Version Info: " << glGetString(GL_VERSION) << endl;

    return true;
}


//----------------------
//KEYBOARD INPUT PROCESSING
//Query GLFW whether relevant keys are pressed/released this frame and react accordingly
//----------------------
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    }

    //Repeat texture wraping
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    //Mirrored repeat texture wraping
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    //Clamp to edge texture wraping
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    //Clamp to border texture wraping
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    //Ortho vs projection
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && !gOrthoView) {
        gOrthoView = true;
        cout << "Switched to orthographic view" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && gOrthoView) {
        gOrthoView = false;
        cout << "Switched to projection view" << endl;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting) {
        gIsLampOrbiting = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting) {
        gIsLampOrbiting = false;
    }

}


// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//----------------------
//MOUSE FUNCTIONS
//----------------------

// GLFW: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// GLFW: handle mouse button events
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


//----------------------
//RENDER A FRAME
//----------------------
void URender()
{
    //----------------------
    //LAMP ORBIT:
    //Orbit or Un-orbit by pressing L or K keys
    //Lamp orbits around the origin
    //----------------------
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition0 = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition0.x;
        gLightPosition.y = newPosition0.y;
        gLightPosition.z = newPosition0.z;

        glm::vec4 newPosition1 = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition1.x;
        gLightPosition.y = newPosition1.y;
        gLightPosition.z = newPosition1.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //RENDERING VARIABLES
    //----------------
    glm::vec2 uvScale;
    const glm::vec3 cameraPosition = gCamera.Position; //Get camera position
    glm::mat4 scale, translation, rotation, model, view, projection;

    //CAMERA MODES
    //----------------
    if (gOrthoView)
    {
        //Camera view transformation
        view = gCamera.GetViewMatrix();
        //Orthographic (2D) projection
        projection = glm::ortho(-(GLfloat)WINDOW_WIDTH * 0.01f, (GLfloat)WINDOW_WIDTH * 0.01f, -(GLfloat)WINDOW_HEIGHT * 0.01f, (GLfloat)WINDOW_HEIGHT * 0.01f, 0.1f, 100.0f);
    }
    else
    {
        //Camera view transformation
        view = gCamera.GetViewMatrix();
        //Perspective (3D) projection
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }


    //----------------------
    // LAMP (SPOTLIGHT):
    // DRAW LAMP - CUBE SHAPE
    //----------------------
    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gLightMesh.vao);

    // Set the shader to be used
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    GLint modelLoc = glGetUniformLocation(gLampProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gLampProgramId, "view");
    GLint projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gLightMesh.nVertices);


    //----------------------
    // AMBIENT LIGHT:
    // DRAW AMBIENT LIGHT - CUBE SHAPE
    //----------------------
    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gAmbMesh.vao);

    // Set the shader to be used
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gAmbLightPosition) * glm::scale(gAmbLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gAmbMesh.nVertices);


    //----------------------
    // TABLE TOP: 
    // DRAW TABLE - PLANE SHAPE
    //----------------------
    // Activate the table VAO
    glBindVertexArray(gTableMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));            //Move
    scale = glm::scale(glm::vec3(9.0f, 1.0f, 10.0f));                     //Scale
    model = translation * scale;                                          //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    GLint ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    GLint ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    GLint ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");

    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTableTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gTableMesh.nVertices, GL_UNSIGNED_SHORT, NULL);
    //glDrawArrays(GL_TRIANGLES, 0, gTableMesh.nVertices);


    //----------------------
    // BOOK: 
    // DRAW BOOK - CUBE SHAPE
    //----------------------
    // Activate the book VAO
    glBindVertexArray(gBookMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(-7.0f, 2.1f, -2.5f));                //Move
    rotation = glm::rotate(glm::radians(15.0f), glm::vec3(0.0f, 0.5f, 0.0f));   //Rotate
    scale = glm::scale(glm::vec3(15.f, 2.f, 10.f));                             //Scale
    model = translation * rotation * scale;                                     //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");



    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBookTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gBookMesh.nVertices);


    //----------------------
    // NOTEBOOK: 
    // DRAW NOTEBOOK - CUBE SHAPE
    //----------------------
    // Activate the Notebook VAO
    glBindVertexArray(gNotebookMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(-7.0f, 3.6f, -2.3f));                //Move
    rotation = glm::rotate(glm::radians(15.0f), glm::vec3(0.0f, 0.5f, 0.0f));   //Rotate
    scale = glm::scale(glm::vec3(12.f, 1.f, 7.f));                              //Scale
    model = translation * rotation * scale;                                     //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");


    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gNotebookTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gNotebookMesh.nVertices);


    //----------------------
    // COASTER: 
    // DRAW COASTER - CUBE SHAPE
    //----------------------
    // Activate the Coaster VAO
    glBindVertexArray(gCoasterMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(8.0f, 1.18f, 2.0f));                 //Move
    rotation = glm::rotate(glm::radians(15.0f), glm::vec3(0.0f, -0.5f, 0.0f));  //Rotate
    scale = glm::scale(glm::vec3(5.f, 0.3f, 5.f));                              //Scale
    model = translation * rotation * scale;                                     //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");



    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCoasterTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gCoasterMesh.nVertices);



    //----------------------
    // FLUORITE: 
    // DRAW FLUORITE OBJECT - PYRAMID SHAPE
    //----------------------
    // Activate the fluorite VAO
    glBindVertexArray(gFluoriteMesh.vao);
    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    //----------------
    translation = glm::translate(glm::vec3(-5.0f, 4.8f, -1.0f));                 //Move
    rotation = glm::rotate(glm::radians(60.0f), glm::vec3(1.0f, 0.8f, -15.0f));  //Rotate
    scale = glm::scale(glm::vec3(1.f, 1.f, 1.f));                                //Scale
    model = translation * rotation * scale;                                      //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera 
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");


    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFluoriteTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gFluoriteMesh.nVertices);

    
    //----------------------
    // LIGHT BASE: 
    // DRAW LIGHT BASE - CYLINDER OBJECT
    //----------------------
    // Activate the light base VAO
    glBindVertexArray(gLightBaseMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(8.0f, 2.2f, 2.0f));                  //Move
    rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));   //Rotate
    scale = glm::scale(glm::vec3(2.0f, 2.0f, 0.8f));                            //Scale
    model = translation * rotation * scale;                                     //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");



    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gLightBaseTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gLightBaseMesh.nVertices, GL_UNSIGNED_SHORT, NULL);
    

    //----------------------
    // LIGHT TOP
    // DRAW LIGHT TOP - SPHERE OBJECT
    //----------------------
    // Activate the light top VAO
    glBindVertexArray(gLightTopMesh.vao);
    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    //----------------
    translation = glm::translate(glm::vec3(8.0f, 3.0f, 2.0f));                //Move
    scale = glm::scale(glm::vec3(1.9f, 1.9f, 1.9f));                          //Scale
    model = translation * scale;                                              //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera 
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");

    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(15.0f, 15.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gLightTopTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gLightTopMesh.nVertices, GL_UNSIGNED_SHORT, NULL);



    //----------------------
    // LAX BALL 
    // DRAW LAX BALL - SPHERE OBJECT
    //----------------------
    // Activate the ball VAO
    glBindVertexArray(gBallMesh.vao);
    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    //----------------
    translation = glm::translate(glm::vec3(-10.0f, 3.0f, 5.6f));                //Move
    scale = glm::scale(glm::vec3(2.f, 2.f, 2.f));                               //Scale
    model = translation * scale;                                                //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera 
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");

    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(15.0f, 15.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBallTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gBallMesh.nVertices, GL_UNSIGNED_SHORT, NULL);


    //----------------------
    // SCENT BASE: 
    // DRAW SCENT BASE - CYLINDER OBJECT
    //----------------------
    // Activate the scent base object VAO
    glBindVertexArray(gScentBaseMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(-2.9f, 1.5f, 2.3f));                  //Move
    rotation = glm::rotate(glm::radians(105.0f), glm::vec3(0.0f, 1.0f, 0.0f));   //Rotate
    scale = glm::scale(glm::vec3(0.5f, 0.5f, 2.5f));                             //Scale
    model = translation * rotation * scale;                                      //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");

    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gScentBaseTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gScentBaseMesh.nVertices, GL_UNSIGNED_SHORT, NULL);
    

    //----------------------
    // SCENT TOP: 
    // DRAW SCENT TOP - CYLINDER OBJECT
    //----------------------
    // Activate the scent top object VAO
    glBindVertexArray(gScentBaseMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    //Set scale, rotation, and translation
    translation = glm::translate(glm::vec3(0.0f, 1.5f, 1.5f));                  //Move
    rotation = glm::rotate(glm::radians(105.0f), glm::vec3(0.0f, 1.0f, 0.0f));  //Rotate
    scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));                            //Scale
    model = translation * rotation * scale;                                     //Creates transform matrix

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //REFERENCE MATRIX UNIFORMS
    //----------------
    //Object
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //Spotlight
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //Ambient Light
    ambientLightColorLoc = glGetUniformLocation(gProgramId, "ambientLightColor");
    ambientLightPositionLoc = glGetUniformLocation(gProgramId, "ambientLightPos");
    //Camera
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambViewPositionLoc = glGetUniformLocation(gProgramId, "ambViewPosition");

    //Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //----------------
    //Object
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //Spotlight
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //Ambient Light
    glUniform3f(ambientLightColorLoc, gAmbLightColor.r, gAmbLightColor.g, gAmbLightColor.b);
    glUniform3f(ambientLightPositionLoc, gAmbLightPosition.x, gAmbLightPosition.y, gAmbLightPosition.z);
    //Camera
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(ambViewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //TEXTURES
    //----------------
    //Texture scale
    uvScale = glm::vec2(1.0f, 1.0f);
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(uvScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gScentTopTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gScentTopMesh.nVertices, GL_UNSIGNED_SHORT, NULL);




    //DEACTIVATE the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


//----------------------
//UCREATEMESH SHAPE FUNCTIONS
//----------------------

//CUBE MESH
void UCreateCubeMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Positions           // Normals            //Textures
        // ----------------------------------------------------
        //Back Face          //Negative Z Normal
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//PLANE MESH
void UCreatePlaneMesh(GLMesh& mesh) {
    GLfloat verts[] = {
        // Positions        // Normals            //Textures
        // -------------------------------------------------
       -2.0f,  1.0,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // Index 0 - top left
        2.0f,  1.0,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // Index 1 - top right
        2.0f,  1.0, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // Index 2 - bottom right
       -2.0f,  1.0, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f, // Index 3 - bottom left
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3   // Triangle 2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // We can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//PYRAMID MESH
void UCreatePyramidMesh(GLMesh& mesh)
{
    GLfloat verts[] = {
        // Positions           // Normals            //Textures
        // ----------------------------------------------------
        //Back Face     
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f,  0.0f, -1.0f,   0.5f, 1.0f,

         //Front Face
       -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,   0.0f,  0.0f,  1.0f,   0.5f, 1.0f,

        //Left Face
       -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,   -1.0f,  0.0f,  0.0f,   0.5f, 1.0f,

        //Right Face
        0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,   1.0f,  0.0f,  0.0f,   0.5f, 1.0f,

        //Bottom Face
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f, -1.0f,  0.0f,   0.5f, 1.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         0.0f,  0.5f,  0.0f,   0.0f,  1.0f,  0.0f,   0.5f, 1.0f,
    };

    // Index data to share position data
    GLushort indices[] = {
        1, 0, 3, // Triangle 1, pyramid side 1
        3, 0, 4, // Triangle 2, pyramid side 2
        4, 0, 2, // Triangle 3, pyramid side 3
        2, 0, 1, // Triangle 4, pyramid side 4
        2, 1, 3, // Triangle 5, pyramid bottom 1
        2, 4, 3, // Triangle 6, pyramid bottom 2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // We can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//FLORUITE MESH
void UCreateFluoriteMesh(GLMesh& mesh)
{
    GLfloat verts[] = {
        // Positions           // Normals            //Textures
        // ----------------------------------------------------
        //Back Face of Top Pyramid    
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,//0
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,//1
         0.0f,  0.5f,  0.0f,   0.0f,  0.0f, -1.0f,   0.5f, 1.0f,//2

         //Front Face
       -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,//3
        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,//4
        0.0f,  0.5f,  0.0f,   0.0f,  0.0f,  1.0f,   0.5f, 1.0f,//5

        //Left Face
       -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,//6
       -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,//7
        0.0f,  0.5f,  0.0f,   -1.0f,  0.0f,  0.0f,   0.5f, 1.0f,//8

        //Right Face
        0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,//9
        0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,//10
        0.0f,  0.5f,  0.0f,   1.0f,  0.0f,  0.0f,   0.5f, 1.0f,//11

        //Bottom Face
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,//12
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,//13
         0.0f,  0.5f,  0.0f,   0.0f, -1.0f,  0.0f,   0.5f, 1.0f,//14

        -0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,//15
        -0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,//16
         0.0f,  0.5f,  0.0f,   0.0f,  1.0f,  0.0f,   0.5f, 1.0f,//17

         //Front Face of Bottom Pyramid      
         -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,//18
          0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,//19
          0.0f,  -1.5f,  0.0f,   0.0f,  0.0f, -1.0f,   0.5f, 1.0f,//20

          //Left Face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,//21
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,//22
         0.0f,  -1.5f,  0.0f,   0.0f,  0.0f,  1.0f,   0.5f, 1.0f,//23

         //Right Face
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,//24
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,//25
         0.0f,  -1.5f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.5f, 1.0f,//26

         //Back Face
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,//27
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,//28
         0.0f,  -1.5f,  0.0f,  1.0f,  0.0f,  0.0f,   0.5f, 1.0f,//29

         //Bottom Face
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,//30
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,//31
          0.0f,  -1.5f,  0.0f,   0.0f, -1.0f,  0.0f,   0.5f, 1.0f,//32

         -0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,//33
         -0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,//34
          0.0f,  -1.5f,  0.0f,   0.0f,  1.0f,  0.0f,   0.5f, 1.0f,//35
    };

    // Index data to share position data
    GLushort indices[] = {
        //Top Pyramid
        1, 0, 3, // Triangle 1, pyramid side 1
        3, 0, 4, // Triangle 2, pyramid side 2
        4, 0, 2, // Triangle 3, pyramid side 3
        2, 0, 1, // Triangle 4, pyramid side 4
        2, 1, 3, // Triangle 5, pyramid bottom 1
        2, 4, 3, // Triangle 6, pyramid bottom 2
        //Bottom Pyramid
        1, 20, 3, // Triangle 1, pyramid side 1
        3, 20, 4, // Triangle 2, pyramid side 2
        4, 20, 2, // Triangle 3, pyramid side 3
        2, 20, 1, // Triangle 4, pyramid side 4
        2, 1, 3, // Triangle 5, pyramid bottom 1
        2, 4, 3, // Triangle 6, pyramid bottom 2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // We can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//CYLINDER MESH
void UCreateCylinderMesh(GLMesh& mesh) {
    // Variables
    const int triangles = 16;// Set number of triangles making up the cylinder
    const float angle = 2 * numbers::pi / triangles;
    const int circleOffset = triangles + 1; //Offset number
    const int triangleSize = 3; //Size of the triangle

    glm::vec3 vertex[(triangles + 1) * 2];
    const int numVerts = sizeof(vertex) / sizeof(vertex[0]) * 8; //Vert number
    const int numIndices = triangles * triangleSize * 4; //Indice number

    GLfloat verts[numVerts];
    GLushort indices[numIndices];

    //Angel math
    for (int i = 0; i <= triangles; i++)
    {
        if (i == triangles)
        {
            vertex[i] = glm::vec3(0, 0, 1);
            vertex[i + circleOffset] = glm::vec3(0, 0, -1);
        }
        else
        {
            vertex[i] = glm::vec3(glm::cos(angle * i), glm::sin(angle * i), 1);
            vertex[i + circleOffset] = glm::vec3(glm::cos(angle * i), glm::sin(angle * i), -1);
        }
    }

    // Position, normal, and texture data for the cylinder
    for (int i = 0; i < numVerts / 8; i++)
    {
        int point = i * 8;

        // Vertex positions
        verts[point] = vertex[i].x;
        verts[point + 1] = vertex[i].y;
        verts[point + 2] = vertex[i].z;

        // Normals
        verts[point + 3] = vertex[i].x;
        verts[point + 4] = vertex[i].y;
        verts[point + 5] = vertex[i].z;

        // Textures
        verts[point + 6] = vertex[i].x;
        verts[point + 7] = vertex[i].y;
    }

    // Index data to share position data
    for (int i = 0; i < triangles; i++)
    {
        int point2 = i * triangleSize;

        indices[point2] = i;
        indices[point2 + 1] = triangles;
        indices[point2 + 2] = i + 1 < triangles ? i + 1 : 0;

        point2 += triangles * triangleSize;

        indices[point2] = i + circleOffset;
        indices[point2 + 1] = triangles + circleOffset;
        indices[point2 + 2] = i + 1 < triangles ? i + 1 + circleOffset : circleOffset;

        point2 += triangles * triangleSize;

        indices[point2] = i;
        indices[point2 + 1] = i + circleOffset;
        indices[point2 + 2] = i + 1 < triangles ? i + 1 : 0;

        point2 += triangles * triangleSize;

        indices[point2] = i + circleOffset;
        indices[point2 + 1] = i + 1 < triangles ? i + 1 : 0;
        indices[point2 + 2] = i + 1 < triangles ? i + 1 + circleOffset : circleOffset;
    }

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // We can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//SPHERE MESH
void UCreateSphereMesh(GLMesh& mesh) {
    // Variables
    const int triangles = 16;
    const int numVerts = (triangles + 1) * (triangles + 1) * 8;
    const int numIndices = numVerts / 8 * 2 * 3;
    GLfloat verts[numVerts];
    GLushort indices[numIndices];
    glm::vec3 vertex[triangles + 1][triangles + 1];

    // Position, normal, and texture data for the sphere
    for (int i = 0; i < triangles + 1; i++)
    {
        float latitude = (((i - 0) * (numbers::pi + numbers::pi)) / triangles) - numbers::pi;

        for (int j = 0; j < triangles + 1; j++)
        {
            float longitude = (((j - 0) * (numbers::pi / 2 + numbers::pi / 2)) / triangles) - numbers::pi / 2;

            //Coords
            vertex[i][j] = glm::vec3 (
                glm::sin(latitude) * glm::cos(longitude),
                glm::sin(latitude) * glm::sin(longitude),
                glm::cos(latitude)
            );

            int point = ((i * (triangles + 1)) + j) * 8;

            // Vertex positions
            verts[point] = vertex[i][j].x;
            verts[point + 1] = vertex[i][j].y;
            verts[point + 2] = vertex[i][j].z;

            // Normal
            verts[point + 3] = vertex[i][j].x;
            verts[point + 4] = vertex[i][j].y;
            verts[point + 5] = vertex[i][j].z;

            // Texture
            verts[point + 6] = vertex[i][j].x / 2 + 0.5;
            verts[point + 7] = vertex[i][j].y / 2 + 0.5;

            point = ((i * (triangles + 1)) + j) * 6;

            indices[point] = i * (triangles + 1) + j;
            indices[point + 1] = (i + 1) * (triangles + 1) + j;
            indices[point + 2] = i * (triangles + 1) + (j + 1);

            indices[point + 3] = (i + 1) * (triangles + 1) + (j + 1);
            indices[point + 4] = (i + 1) * (triangles + 1) + j;
            indices[point + 5] = i * (triangles + 1) + (j + 1);
        }
    }

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Buffer for the vertex data and for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//----------------------
//OTHER FUNCTIONS
//----------------------

//DESTROYS MESHES
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

//GENERATE AND LOAD THE TEXTURE
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        }
        else if (channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        else {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

//DESTROYS TEXTURES
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

//IMPLEMENTS UCREATESHADERS FUNCTION
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

//DESTROYS SHADER PROGRAMS
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}