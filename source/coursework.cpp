#include <iostream>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Object struct
struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale    = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle        = 0.0f;
    std::string name;
};

// Function prototypes
void keyboardInput(GLFWwindow *window, const std::vector<Object>& objects);
void mouseInput(GLFWwindow* window);
bool checkCollision(const glm::vec3& position, const std::vector<Object>& objects);
bool objectCollision(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& boxCenter, const glm::vec3& boxScale);

// Frame timers
float previousTime = 0.0f;  // time of previous iteration of the loop
float deltaTime    = 0.0f;  // time elapsed since the previous frame

// Create Camera object
Camera camera(glm::vec3(0.0f, -4.0f, 4.5f), glm::vec3(2.0f, 2.0f, 0.0f));


// Collision boundaries (xMin, xMax, yMin, yMax, zMin, zMax)
struct boxBound 
{
    float xMin, xMax;
    float yMin, yMax;
    float zMin, zMax;
};

boxBound roomBounds[] = {
    { -5.0f,  5.0f, -5.0f, -4.5f, -5.0f,  5.0f },     // Floor
    { -5.0f,  5.0f,  2.5f,  3.0f, -5.0f,  5.0f },     // Ceiling
    { -5.0f,  5.0f, -4.5f,  2.5f, -5.0f, -4.5f },     // Back Wall
    { -5.0f,  5.0f, -4.5f,  2.5f,  4.5f,  5.0f },     // Front Wall
    { -5.0f, -4.5f, -4.5f,  2.5f, -5.0f,  5.0f },     // Left Wall
    {  4.5f,  5.0f, -4.5f,  2.5f, -5.0f,  5.0f }      // Right Wall
};

// Check collision with room bounds and objects
const float cameraRadius = 0.5f;

bool checkCollision(const glm::vec3& position, const std::vector<Object>& objects)
{
    // Check collision with room bounds 
    for (const auto& bound : roomBounds) {
        if (position.x >= bound.xMin && position.x <= bound.xMax &&
            position.y >= bound.yMin && position.y <= bound.yMax &&
            position.z >= bound.zMin && position.z <= bound.zMax) {
            return true;
        }
    }

    // Check collision with cubes using sphere-box collision
    for (const auto& obj : objects) {
        if (obj.name == "cube") {
            if (objectCollision(position, cameraRadius, obj.position, obj.scale)) {
                return true;
            }
        }
    }

    return false;
}


bool objectCollision(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& boxCenter, const glm::vec3& boxScale)
{
    glm::vec3 halfSize = boxScale * 0.5f;

    // Find closest point to the camera
    float x = std::max(boxCenter.x - halfSize.x, std::min(sphereCenter.x, boxCenter.x + halfSize.x));
    float y = std::max(boxCenter.y - halfSize.y, std::min(sphereCenter.y, boxCenter.y + halfSize.y));
    float z = std::max(boxCenter.z - halfSize.z, std::min(sphereCenter.z, boxCenter.z + halfSize.z));

    glm::vec3 closestPoint(x, y, z);

    // Calculate distance between sphere center and closest point
    float distance = glm::length(sphereCenter - closestPoint);

    return distance < sphereRadius;
}

int main( void )
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Computer Graphics Coursework", NULL, NULL);
    
    if( window == NULL ){
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Capture mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Compile shader program
    unsigned int shaderID, lightShaderID;
    shaderID      = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    lightShaderID = LoadShaders("lightVertexShader.glsl", "lightFragmentShader.glsl");

    // Activate shader
    glUseProgram(shaderID);

    //Load Models
    Model cube("../assets/cube.obj"); 
    Model sphere("../assets/sphere.obj");

    // Load the textures
    cube.addTexture("../assets/Leather_Diffuse.jpg", "diffuse");
    cube.addTexture("../assets/Leather_Normal.jpg", "normal");
    cube.addTexture("../assets/Leather_Specular.jpg", "specular");

    // Define teapot object lighting properties
    cube.ka = 0.2f;
    cube.kd = 0.7f;
    cube.ks = 1.0f;
    cube.Ns = 20.0f;

    // Add light sources
    Light lightSources;
    lightSources.addPointLight(glm::vec3(2.0f, 1.0f, -2.0f),        // position
                               glm::vec3(1.0f, 1.0f, 1.0f),         // colour
                               1.0f, 0.1f, 0.02f);                  // attenuation

    lightSources.addPointLight(glm::vec3(-2.0f, 1.0f, 2.0f),        // position
                               glm::vec3(1.0f, 1.0f, 1.0f),         // colour
                               1.0f, 0.1f, 0.02f);                  // attenuation

    lightSources.addSpotLight(glm::vec3(0.0f, 2.0f, 0.0f),          // position
                              glm::vec3(0.0f, -1.0f, 0.0f),         // direction
                              glm::vec3(1.0f, 1.0f, 1.0f),          // colour
                              1.0f, 0.1f, 0.02f,                    // attenuation
                              std::cos(Maths::radians(45.0f)));     // cos(phi)

    lightSources.addDirectionalLight(glm::vec3(0.0f, -1.0f, 0.0f),  // direction
                                     glm::vec3(1.0f, 1.0f, 0.0f));  // colour

    // Cube positions
    glm::vec3 positions[] = {
         glm::vec3(-3.0f, -4.5f, -3.0f),
         glm::vec3(-1.5f, -4.5f, -2.0f),
         glm::vec3( 0.0f, -4.5f, -1.5f),
         glm::vec3( 1.5f, -4.5f, -1.0f),
         glm::vec3( 3.0f, -4.5f, -0.5f),
         glm::vec3(-3.0f, -4.5f,  0.5f),
         glm::vec3(-1.5f, -4.5f,  1.0f),
         glm::vec3( 0.0f, -4.5f,  1.5f),
         glm::vec3( 1.5f, -4.5f,  2.0f),
         glm::vec3(-1.0f, -4.5f,  0.0f),
         glm::vec3( 3.0f, -4.5f,  2.5f)
    };

    // Add cubes to objects vector
    std::vector<Object> objects;
    Object object;
    object.name = "cube";
    for (unsigned int i = 0; i < 11; i++)
    {
        object.position = positions[i];
        object.rotation = glm::vec3(1.0f, 1.0f, 1.0f);
        object.scale = glm::vec3(0.3f, 0.3f, 0.3f);
        object.angle = Maths::radians(0.0f);
        objects.push_back(object);
    }

    // Load a 2D plane model for the floor 
    Model floor("../assets/plane.obj");
    floor.addTexture("../assets/Floor_Diffuse.jpg", "diffuse");
    floor.addTexture("../assets/Floor_Normal.jpg", "normal");
    floor.addTexture("../assets/Floor_Specular.jpg", "specular");

    // Define floor light properties
    floor.ka = 0.0f;
    floor.kd = 0.7f;
    floor.ks = 0.0f;
    floor.Ns = 20.2f;

    // Add floor model to objects vector
    object.position = glm::vec3(0.0f, -5.0f, 0.0f);
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = 0.0f;
    object.name = "floor";
    objects.push_back(object);

    // Load a plane model for the ceiling
    Model ceiling("../assets/plane.obj");
    ceiling.addTexture("../assets/OfficeCeiling_Diffuse.jpg", "diffuse");
    ceiling.addTexture("../assets/OfficeCeiling_Normal.jpg", "normal");
    ceiling.addTexture("../assets/OfficeCeiling_Specular.jpg", "specular");

    // Define ceiling light properties
    ceiling.ka = 0.2f;
    ceiling.kd = 0.5f;
    ceiling.ks = 1.0f;
    ceiling.Ns = 20.0f;

    // Add ceiling model to objects vector
    object.position = glm::vec3(0.0f, 2.75f, 0.0f);
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = Maths::radians(180.0f);
    object.name = "ceiling";
    objects.push_back(object);

    // Load a plane model for the ceiling 
    Model wall("../assets/plane.obj"); 
    wall.addTexture("../assets/OfficeWall_Diffuse.jpg", "diffuse");
    wall.addTexture("../assets/OfficeWall_Normal.jpg", "normal");
    wall.addTexture("../assets/OfficeWall_Specular.jpg", "specular");

    // Define wall light properties
    wall.ka = 0.2f;
    wall.kd = 1.0f;
    wall.ks = 1.0f;
    wall.Ns = 20.0f;

    // Add wall model to objects vector
    // Back wall
    object.position = glm::vec3(0.0f, -0.5f, -5.0f); 
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = Maths::radians(90.0f);
    object.name = "wall";
    objects.push_back(object);

    // Right wall
    object.position = glm::vec3(5.0f, -0.5f, 0.0f); 
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(0.0f, 0.0f, 1.0f);
    object.angle = Maths::radians(90.0f);
    object.name = "wall";
    objects.push_back(object);

    // Left wall
    object.position = glm::vec3(-5.0f, -0.5f, 0.0f); 
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(0.0f, 0.0f, 1.0f);
    object.angle = Maths::radians(-90.0f);
    object.name = "wall";
    objects.push_back(object);

    // Front wall
    object.position = glm::vec3(-0.0f, -0.5f, 5.0f); 
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    object.angle = Maths::radians(-90.0f);
    object.name = "wall";
    objects.push_back(object);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Update timer
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        // Get inputs
        keyboardInput(window, objects);
        mouseInput(window);
        
        // Clear the window
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        camera.target = camera.eye + camera.front;
        camera.quaternionCamera();

        // Activate shader
        glUseProgram(shaderID);
        
        // Send light source properties to the shader
        lightSources.toShader(shaderID, camera.view);
        
        // Loop through objects
        for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            // Calculate the model matrix
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            glm::mat4 model = translate * rotate * scale;

            // Send MVP and MV matrices to the vertex shader         
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            // Draw the model
            if(objects[i].name == "cube")
                cube.draw(shaderID);

            if (objects[i].name == "floor")
                floor.draw(shaderID);

            if (objects[i].name == "ceiling")
                ceiling.draw(shaderID);

            if (objects[i].name == "wall")
                wall.draw(shaderID);
        }

        // Draw light sources
        lightSources.draw(lightShaderID, camera.view, camera.projection, sphere);      

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    cube.deleteBuffers();
    glDeleteProgram(shaderID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow *window, const std::vector<Object>& objects)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 originalPos = camera.eye;  // Stores original position

    // Move the camera using WSAD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.right;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.right;

    // Make the camera stick to low level
    camera.eye.y = -4.0f; 

    // Apply collision detection
    if (checkCollision(camera.eye, objects))
    {
        camera.eye = originalPos;
    }
}

void mouseInput(GLFWwindow* window)
{
    // Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Update yaw and pitch angles
    camera.yaw += 0.0005f * float(xPos - 1024 / 2);
    camera.pitch += 0.0005f * float(768 / 2 - yPos);

    // Calculate camera vectors from the yaw and pitch angles
    camera.calculateCameraVectors();
}
