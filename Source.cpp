#include "Linker.h"


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Dennis Enwiya Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture id
    GLuint gTextureId01;
    GLuint gTextureId02;

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;
    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gPosition(0.0f, -5.0f, 0.0f);
    glm::vec3 gScale(2.0f);

    // Cube and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
    glm::vec3 gLightScale(0.3f);
    //bool gIsLampOrbiting = true;

}


bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);



const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoordinate;


out vec3 vertexNormal;
out vec3 vertexFragmentPos;

out vec2 vertexTextureCoordinate;





uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));
    vertexNormal = mat3(transpose(inverse(model))) * normal;
    vertexTextureCoordinate = textureCoordinate;
}
);


const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal;
    in vec3 vertexFragmentPos;
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    //------------------multiply textures----------------
   // uniform sampler2D uTextureBase;
    /*
   
    uniform bool multipleTextures;*/

    //-------------------------------------------

//uniform sampler2D uTexture;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture;
//=====================================
/*
uniform sampler2D uTextureExtra;
uniform bool multipleTextures;*/
//==============================
uniform vec2 uvScale;

void main()
{

    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

//Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;
    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0f); // Send lighting results to GPU
    //multiple textures
    //-----------------------multiple textures------------------------
  /* fragmentColor = texture(uTexture, vertexTextureCoordinate);
    if (multipleTextures) {
        vec4 extraTexture = texture(uTextureExtra, vertexTextureCoordinate);
        if (extraTexture.a != 0.0)
            fragmentColor = extraTexture;
    
    }*/
    //------------------------------------------------

}
);

/* Lamp Shader Source Code*/
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


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


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


int main(int argc, char* argv[])
{   

    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;


    UCreateMesh(gMesh);


    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;





    const char* texFilename = "resources/textures/keyBoard.jpg";
    const char* texFilename1 = "resources/textures/OuterView.jpg";
    if (!UCreateTexture(texFilename, gTextureId01))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    
    if (!UCreateTexture(texFilename1, gTextureId02)) {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }

    glUseProgram(gProgramId);

    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureExtra"), 1);




    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(gWindow))
    {

        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;


        UProcessInput(gWindow);


        URender();

        glfwPollEvents();
    }


    UDestroyMesh(gMesh);


    UDestroyTexture(gTextureId01);
    UDestroyTexture(gTextureId02);


    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS);
}



bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);


    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }


    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


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
    //-----Up and Down Logic Handeler-----
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId01);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId02);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId01);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
        
        
    
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId01);
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

    // Pause and resume lamp orbiting
  

}



void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}



void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}


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



void URender()
{

    glEnable(GL_DEPTH_TEST);


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(gMesh.vao);

    glUseProgram(gProgramId);

    //=====================MT=======================================================

    /*
    GLuint multipleTexturesLoc = glGetUniformLocation(gProgramId, "multipleTextures");
    glUniform1i(multipleTexturesLoc, gTextureId02);
    */
    //==============================================================================
    
    //create a sphear 
    
    // Activate the cube VAO (used by cube and lamp)

    /*
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));

    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
  */
    glm::mat4 model = glm::translate(gPosition) * glm::scale(gScale);

    glm::mat4 view = gCamera.GetViewMatrix();


    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);





    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    //---added
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId01);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureId02);


    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glBindVertexArray(0);

    glUseProgram(0);

    glfwSwapBuffers(gWindow);
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
  // x     y    z

        //pyramid  ======done======
        1.0f, -0.47f, 3.4f,       1.0f, 1.0f, -1.0f,      0.0f, 0.0f, 
        1.0f, -0.485f, 3.5f,      1.0f, 1.0f, -1.0f,      0.0f, 0.0f, 
        1.01f, -0.47f, 3.4f,      1.0f, 1.0f, -1.0f,      0.0f, 0.0f, 

        1.0f, -0.5f, 3.4f,        1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f,-0.485f,3.5f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.01f,-0.5f,3.4f,         1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        
        0.99f,-0.48f, 3.4f,       1.0f, 1.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.485f, 3.5f,     1.0f, 1.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.47f, 3.4f,      1.0f, 1.0f, -1.0f,      0.0f, 0.0f,

        1.02f,-0.48f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f,-0.485f, 3.5f,      1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.01f, -0.47f, 3.4f,      1.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        1.02f,-0.49f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.485, 3.5f,      1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.02f,-0.48f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        1.02f,-0.49f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.485f, 3.5f,     1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.01f,-0.50f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        0.99f,-0.49f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.485f, 3.5f,     1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f,-0.50f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        0.99f,-0.48f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        1.00f, -0.485, 3.5f,      1.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        0.99f,-0.49f, 3.4f,       1.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        //pencil cylinder======done======
        //bottom
        1.0f, -0.5f, 3.4f,        0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        1.0f, -0.5f, 2.1f,        0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        1.01f, -0.5f, 2.1f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        1.01f, -0.5f, 3.4f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        1.0f, -0.5f, 3.4f,        0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        1.01f, -0.50f, 2.1f,      0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 
        //top
        1.0f, -0.47f, 3.4f,       0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.47f, 2.1f,       0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 2.1f,      0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 3.4f,      0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.47f, 3.4f,       0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 2.1f,      0.0f, 0.7f, 0.0f,       0.0f, 0.0f, 
        //left bottom connect
        //x     y       z
        //left bottom pannel 
        1.0f, -0.5f, 3.4f,       0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.5f, 2.1f,       0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.49f, 2.1f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.49f, 3.4f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.5f, 3.4f,       0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.49f, 2.1f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 


        0.99f, -0.49f, 3.4f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.49f, 2.1f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.48f, 2.1f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.48f, 3.4f,     0.5f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.49f, 3.4f,     0.5f, 1.0, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.48f, 2.1f,     0.5f, 1.f, 0.0f,       0.0f, 0.0f, 

        0.99f, -0.48f, 3.4f,      0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.48f, 2.1f,      0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.47f, 2.1f,      0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.0f, -0.47f, 3.4f,      0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.99f, -0.48f, 3.4f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.00f, -0.47f, 2.1f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
      
        //right side pannels
        1.01f, -0.5f, 3.4f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f,
        1.01f, -0.5f, 2.1f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f,
        1.02f, -0.49f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f,
        1.02f, -0.49f, 3.4f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f,
        1.01f, -0.5f, 3.4f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.49f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
     
        1.02f, -0.49f, 3.4f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.49f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.48f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.48f, 3.4f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.49f, 3.4f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.48f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 

        1.02f, -0.48f, 3.4f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.48f, 2.1f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 2.1f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 3.4f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.02f, -0.48f, 3.4f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        1.01f, -0.47f, 2.1f,     0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        
        
        //
        //==========================cube1===========================================

        -0.1f, -0.5f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.1f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.1f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.4f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.50f, 2.1f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        //Top Face
        -0.1f, -0.2f, 2.4f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.1f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.4f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        //Left Face
        -0.1f, -0.5f, 2.4f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.1f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.1f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.4f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.1f,     0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Right Face
        0.2f, -0.5f, 2.4f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.1f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.4f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.4f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Front Face
        -0.1f, -0.2f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        -0.1f, -0.5f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.2f, -0.2f, 2.4f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.2f, -0.5f, 2.4f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.2f, -0.2f, 2.4f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        -0.1f, -0.5f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        //Back Face
        -0.1f, -0.2f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.1f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.1f,      -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.1f,     -0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 

        //=======================cube2===============================================

        -0.1f, -0.5f, 2.7f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 3.0f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 3.0f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.7f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.7f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.50f, 3.0f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        //Top Face
        -0.1f, -0.2f, 2.7f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 3.0f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 3.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      0.0f, -1.0f, 0.0f,       0.0f, 0.0f,
        -0.1f, -0.2f, 2.7f,     0.0f, -1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 3.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        //Left Face
        -0.1f, -0.5f, 3.0f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.7f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 3.0f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.7f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.7f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 3.0f,     0.50f, 0.50f, 0.0f,       0.0f, 0.0f, 
        //Right Face
        0.2f, -0.5f, 3.0f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.7f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 3.0f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 3.0f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Front Face
        -0.1f, -0.2f, 3.0f,     1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 3.0f,     1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 3.0f,     1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        //Back Face
        -0.1f, -0.2f, 2.7f,     0.0f, -1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.7f,     0.0f, 1.0f,  0.0f,      0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      0.0f, -1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.5f, 2.7f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      0.0f, -1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.5f, 2.7f,     0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 

        //==========================cube3============================================
        //bottom
        -0.1f, -0.2f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.7f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.4f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      -1.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        //Top Face
        -0.1f, 0.1f, 2.4f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f, 0.1f, 2.7f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.4f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        -0.1f,0.1f, 2.4f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        //Left Face
        -0.1f, -0.2f, 2.7f,     1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, 0.1f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, 0.1f, 2.7f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.7f,     1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, 0.1f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Right Face 
        0.2f, -0.2f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.4f,       1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Front Face
        -0.1f, 0.1f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.4f,       1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.4f,      1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.4f,       1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.4f,     1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        //Back Face
        -0.1f, 0.1f, 2.7f,      -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.7f,     -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, -0.2f, 2.7f,      -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.2f, 0.1f, 2.7f,       -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        -0.1f, -0.2f, 2.7f,     -1.0f, 0.5f, 0.0f,       0.0f, 0.0f, 

        //cone
        
        1.15f,0.5f,-0.40f,       0.0f, 0.5f, 1.0f,       0.0f, 0.0f, 
        1.1f, 0.80f, -0.45f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f, 
        1.05f, 0.80f, -0.425f,   0.0f, 0.5f,1.0f,       0.0f, 0.0f, 

        1.15f, 0.5f, -0.40f,    0.0f, 0.5f, 1.0f,       0.0f, 0.0f, 
        1.05f, 0.80f, -0.425f,   0.0f, 0.5f, 1.0f,       0.0f, 0.0f, 
        1.05f,0.5f,-0.35f,       0.0f,0.5f, 1.0f,        0.0f, 0.0f,


        1.15f, 0.5f, -0.40f,     1.0f, 0.5f,0.0f,        0.0f, 0.0f, 
        1.1f, 0.80f, -0.45f,     0.5f, 0.5f, 1.0f,       0.0f, 0.0f, 
        1.125f, 0.80f, -0.50f,   0.5f, 0.5f, 0.5f,       0.0f, 0.0f, 


        1.15f, 0.5f, -0.40f,     1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        1.125f, 0.80f, -0.50f,   1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        1.20f, 0.50f, -0.50f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.15f, 0.5f, -0.60f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.125f, 0.80f, -0.50f,   0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.20f, 0.50f, -0.50f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
 
        1.15f, 0.5f,-0.60f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.125f, 0.80f,-0.50f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.1f, 0.80f,-0.55f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.15f, 0.5f,-0.60f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, 0.50f,-0.65f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.1f, 0.80f,-0.55f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.05f, 0.50f,-0.65f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.1f, 0.80f,-0.55f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, 0.80f,-0.575f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        1.05f, 0.80f,-0.575f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        1.05f, 0.50f,-0.65f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.90f, 0.50f,-0.60f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        1.05f, 0.80f,-0.575f,    0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.90f, 0.50f,-0.60f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        1.0f, 0.80f,-0.55f,      0.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        0.90f, 0.50f,-0.60f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.0f, 0.80f,-0.55f,      1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.87f, 0.50f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,

        1.0f, 0.80f,-0.55f,      1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.87f, 0.50f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.98f, 0.80f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,

        0.87f, 0.50f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.98f, 0.80f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.90f, 0.50f,-0.40f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,

        0.98f, 0.80f,-0.50f,     1.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        0.90f, 0.50f,-0.40f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.00f, 0.80f,-0.45f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,

        0.90f, 0.50f,-0.40f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.00f, 0.80f,-0.45f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.05f, 0.50f,-0.35f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,

        1.00f, 0.80f,-0.45f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.05f, 0.50f,-0.35f,     0.0f, 0.5f, 1.0f,       0.0f, 0.0f,
        1.05f, 0.80f,-0.425f,    0.0f, 0.5f, 1.0f,       0.0f, 0.0f,

      

        // 
        // water bottle 
        //cap of bottle
        1.05f, 0.8f, -0.425f,       0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.05f, 0.9f, -0.425f,       0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.0f, 0.8f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 

		1.05f, 0.9f, -0.425f,       0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.0f, 0.8f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.0f, 0.9f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 

		1.0f, 0.8f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.0f, 0.9f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
	    0.98f, 0.8f, -0.50f,        1.0f, 0.5f, 1.0f,      0.0f, 0.0f, 

		0.98f, 0.8f, -0.50f,        1.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.0f, 0.9f, -0.45f,         0.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		0.98f, 0.9f, -0.5f,         1.0f, 0.5f, 1.0f,      0.0f, 0.0f, 

		0.98f, 0.8f, -0.5f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		0.98f, 0.9f, -0.5f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		1.0f, 0.8f, -0.55f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
        
		1.0f, 0.8f, -0.55f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		1.0f, 0.9f, -0.55f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		0.98f, 0.9f, -0.5f,         0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 

		1.0f, 0.80f, -0.55f,        0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		1.0f, 0.90f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.05f, 0.80f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
       
		1.05f, 0.80f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.05f, 0.90f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.0f, 0.90f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.05f, 0.80f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.05f, 0.90f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.80f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.1f, 0.80f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.90f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.05f, 0.90f, -0.575f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.1f, 0.80f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.90f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.125f, 0.80f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.125f, 0.80f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.125f, 0.90f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.90f, -0.55f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.125f, 0.80f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.125f, 0.90f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.80f, -0.45f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
   
		1.1f, 0.80f, -0.45f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.90f, -0.45f,        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 
		1.125f, 0.90f, -0.50f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 

		1.1f, 0.80f, -0.45f,        0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 
		1.1f, 0.90f, -0.45f,        1.0f, 0.5f, 0.0f,      0.0f, 0.0f, 
		1.05f, 0.80f, -0.425f,      1.0f, 0.5f, 1.0f,      0.0f, 0.0f, 

		1.05f, 0.80f, -0.425f,      1.0f, 0.5f, 1.0f,      0.0f, 0.0f, 
		1.05f, 0.90f, -0.425f,      0.0f, 0.0f, 1.00f,     0.0f, 0.0f, 
		1.1f, 0.90f, -0.45f,        1.0f, 0.0f, 1.00f,     0.0f, 0.0f, 

        //bottle cylinder
        1.05f, -0.5f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.05f, 0.50f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        0.90f, -0.5f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 

        1.05f, 0.50f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        0.90f, -0.5f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 

        0.90f, -0.5f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        0.87f, -0.5f, -0.50f,       0.5f, 1.0f, 1.0f,       0.0f, 0.0f, 

        0.87f, -0.5f, -0.50f,       0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.40f,       0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 
        0.87f, 0.50f, -0.50f,       0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 

        0.87f, -0.5f, -0.50f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.87f, 0.50f, -0.50f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.90f, -0.5f, -0.60f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 

        0.90f, -0.5f, -0.60f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.60f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 
        0.87f, 0.50f, -0.50f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 

        0.90f, -0.5f, -0.60f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.60f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, -0.5f, -0.65f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.05f, -0.5f, -0.65f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, 0.50f, -0.65f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        0.90f, 0.50f, -0.60f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.05f, -0.5f, -0.65f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, 0.50f, -0.65f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.15f, -0.5f, -0.60f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.15f, -0.5f, -0.60f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.60f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.05f, 0.50f, -0.65f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.15f, -0.5f, -0.60f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.60f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.20f, -0.5f, -0.50f,       0.5f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.20f, -0.5f, -0.50f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.20f, 0.50f, -0.50f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.60f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 

        1.20f, -0.5f, -0.50f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.20f, 0.50f, -0.50f,       0.50f, 0.0f, 0.0f,       0.0f, 0.0f, 
        1.15f, -0.5f, -0.40f,       0.50f, 0.0f, 1.0f,       0.0f, 0.0f, 

        1.15f, -0.5f, -0.40f,       1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.40f,       1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.20f, 0.50f, -0.50f,       1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 

        1.15f, -0.5f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.40f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.05f, -0.5f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        
        1.05f, -0.5f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.05f, 0.50f, -0.35f,       0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 
        1.15f, 0.50f, -0.40f,       0.0f, 1.0f, 1.0f,      0.0f, 0.0f, 

        //laptop shape
       -1.0f, -0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
        1.0f, -0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
        1.0f,  0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
        1.0f,  0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
       -1.0f,  0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
       -1.0f, -0.5f, 0.4f,          1.0f, 0.0f, 0.0f,      0.0f,0.0f,
       //green

      -1.0f, -0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.00f, 0.0f,
       1.0f, -0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.0f, 0.0f,
       1.0f,  0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.01f, 0.03f,
       1.0f,  0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.0f, 0.0f,
      -1.0f,  0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.0f, 0.0f,
      -1.0f, -0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.03f, 0.03f,

      //blue/ right top side pannel
     -1.0f,  0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f,  0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f, -0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f, -0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f, -0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f,  0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
     //yellow lef ttop side pannel
      1.0f,  0.5f,  0.5f,           0.0f,0.5f,-0.5f,        0.0f,0.0f,
      1.0f,  0.5f, 0.4f,            0.0f,0.5f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f, 0.4f,            0.0f,0.5f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f, 0.4f,            0.0f,0.5f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f,  0.5f,           0.0f,0.5f,-0.5f,        0.0f,0.0f,
      1.0f,  0.5f,  0.5f,           0.0f,0.5f,-0.5f,        0.0f,0.0f,
    //light blue bottm small cover
     -1.0f, -0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
      1.0f, -0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f, -0.5f,  0.5f,           0.0f,0.0f,-0.5f,        0.0f,0.0f,
     -1.0f, -0.5f, 0.4f,            0.0f,0.0f,-0.5f,        0.0f,0.0f,
   //purple top small cover

     -1.0f,  0.5f, 0.4f,            0.0f,0.0f,1.0f,         0.0f,0.0f,
      1.0f,  0.5f, 0.4f,            0.0f,0.0f,1.0f,         0.0f,0.0f,
      1.0f,  0.5f,  0.5f,           0.0f,0.0f,1.0f,         0.0f,0.0f,
      1.0f,  0.5f,  0.5f,           0.0f,0.0f,1.0f,         0.0f,0.0f,
     -1.0f,  0.5f,  0.5f,           0.0f,0.0f,1.0f,         0.0f,0.0f,
     -1.0f,  0.5f, 0.4f,            0.0f,0.0f,1.0f,         0.0f,0.0f,

   //second square
     -1.0f, -0.5f, 1.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,
     -1.0f, -0.4f, 1.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,
     -1.0f, -0.5f, 0.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,
     -1.0f, -0.4f, 0.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,
     -1.0f, -0.4f, 1.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,
     -1.0f, -0.5f, 0.5f,            0.0f, 0.0f,1.0f,        0.0f,0.0f,

      1.0f, -0.5f, 1.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,
      1.0f, -0.4f, 1.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,
      1.0f, -0.5f, 0.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,

      1.0f, -0.4f, 0.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,
      1.0f, -0.4f, 1.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,
      1.0f, -0.5f, 0.5f,            0.0f, -0.5f, 1.0f,      0.0f,0.0f,

   //top pannal
     -1.0f,-0.4f,1.5f,              0.0f,1.0f,1.0f,         0.0f,0.0f,
      1.0f,  -0.4f, 1.5f,           0.0f,1.0f,1.0f,         0.19f,0.0f,
      1.0f,  -0.4f, 0.5f,           0.0f,1.0f,1.0f,         0.19f,0.22f,

      1.0f,  -0.4f, 0.5f,           0.0f,1.0f,1.0f,         0.0f,0.20f,
     -1.0f,  -0.4f, 1.5f,           0.0f,1.0f,1.0f,         0.19f,0.0f,
     -1.0f,  -0.4f, 0.5f,           0.0f,1.0f,1.0f,         0.19f,0.22f,

   //bottom pannal
     -1.0f,  -0.5f, 1.5f,           0.0f,0.0f,0.0f,         0.0f,0.0f,
      1.0f,  -0.5f, 1.5f,           0.0f,0.0f,0.0f,         0.0f,0.0f,
      1.0f,  -0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.0f,0.0f,
      1.0f,  -0.5f, 0.5f,           0.0f,0.0f,0.0f,         0.0f,0.0f,
     -1.0f,  -0.5f, 1.5f,           0.0f,0.0f,0.0f,         0.0f,0.0f,
     -1.0f,  -0.5f,  0.5f,          0.0f,0.0f,0.0f,         0.0f,0.0f,
   //front small panel bottom
     -1.0f, -0.5f,  1.5f,           0.5f,0.5f,0.0f,        0.0f,0.0f,
      1.0f, -0.5f,  1.5f,           0.5f,0.5f,0.0f,        0.0f,0.0f,
      1.0f,  -0.4f,  1.5f,          0.5f,0.5f,0.0f,        0.0f,0.0f,
      1.0f,  -0.4f,  1.5f,          0.5f,0.5f,0.0f,        0.0f,0.0f,
     -1.0f,  -0.4f,  1.5f,          0.5f,0.5f,0.0f,        0.0f,0.0f,
     -1.0f, -0.5f,  1.5f,           0.5f,0.5f,0.0f,        0.0f,0.0f,
   //plain
     -5.0f,-0.5f,5.0f,              1.0f,1.0f,1.0f,        0.0f,0.0f,
      5.0f,-0.5f, 5.0f,             1.0f,1.0f,1.0f,        0.0f,0.0f,
     -5.0f, -0.5f, -5.0f,           1.0f,1.0f,1.0f,        0.0f,0.0f,
     -5.0f,-0.5f, -5.0f,            1.0f,1.0f,1.0f,        0.0f,0.0f,
      5.0f,-0.5f, -5.0f,            1.0f,1.0f,1.0f,        0.0f,0.0f,
      5.0f, -0.5f, 5.0f,            1.0f,1.0f,1.0f,       0.0f,0.0f,

//new shape
    };

    const GLuint floatsPerVertex = 3;
    //const GLuint floatsPerUV = 2; 
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex  + floatsPerNormal+ floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);


    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);


    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); 


    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}



bool UCreateTexture(const char* filename, GLuint &textureId0)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId0);
        glBindTexture(GL_TEXTURE_2D, textureId0);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }


    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{

    int success = 0;
    char infoLog[512];


    programId = glCreateProgram();

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);


    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);


    glCompileShader(vertexShaderId);

    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId);
    //error here
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }


    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


















