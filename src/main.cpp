#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//obavezno prvo ucitavanje glad pa onda glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//ucitavanje glm biblioteke za rad s transformacijama
//BITNO! skaliranje->rotacija->translacija
//u programu se pise obrnuto zbog glm fja i njihove implementacije, al to je to
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
//apstrakcije za kameru, modele i shadere
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(const char *path);
void modelRender(const Shader& ourShader,glm::vec3 modelPosition,float modelScale);

// sirina i visina prozora koji se prikazuje
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//kamera i njena pozicija
Camera camera(glm::vec3(2.5f,5.0f,20.0f));

bool ImGuiEnabled=false;
bool CameraMouseMovementUpdateEnabled=true;

//za pomeranje misa
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//pozicije modela u svetu i faktori skaliranja za iste
glm::vec3 blokPosition=glm::vec3(1.0f,1.0f,1.0f);
float blokScale=1.9f;
glm::vec3 creeperPosition=glm::vec3(3.5f,1.0f,1.4f);
float creeperScale=2.0f;
glm::vec3 krabPosition=glm::vec3(7.3f,1.0f,1.7f);
float krabScale=1.0f;
glm::vec3 scyphozoaPosition=glm::vec3(-3.0f,0.0f,1.8f);
float scyphozoaScale=0.8f;
//glm::vec3 sharkPosition=glm::vec3(1.5f,7.0f,1.6f);
glm::vec3 sharkPosition=glm::vec3(1.5f,7.0f,1.6f);
float sharkScale=1.8f;



//struct DirLight {
//    glm::vec3 direction;
//    glm::vec3 ambient;
//    glm::vec3 diffuse;
//    glm::vec3 specular;
//};


//struktura za pointlight
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


int main() {
    //inicijalizujemo glfw
    glfwInit();

    //postavljamo verziju na 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //za MacOS se koristi sledece
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //kreiramo prozor
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Racunarska grafika projekat", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //naznaka da hocemo u ovom prozoru da crtamo
    glfwMakeContextCurrent(window);

    //fja koja sluzi za odredjivanje dimenzija za renderovanje
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //pomeranje misa
    glfwSetCursorPosCallback(window, mouse_callback);
    //skrolovanje
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    //za registrovanje misa
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //ucitavamo OpenGL fje sa glad bibliotekom
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //okrece ucitanu teksturu
    //stbi_set_flip_vertically_on_load(true);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);


    //imamo apstrakciju za shader, tj klasu Shader
    //pravljenje shader programa
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");
    Shader blendingShader("resources/shaders/blending.vs","resources/shaders/blending.fs");
    //ucitavanje teksture



    //ucitavanje modela
    Model blokModel("resources/objects/blok/scene.gltf");
    blokModel.SetShaderTextureNamePrefix("material.");
    Model creeperModel("resources/objects/creeper/scene.gltf");
    creeperModel.SetShaderTextureNamePrefix("material.");
    Model krabModel("resources/objects/krab/scene.gltf");
    krabModel.SetShaderTextureNamePrefix("material.");
    Model scyphozoaModel("resources/objects/scyphozoa/scene.gltf");
    scyphozoaModel.SetShaderTextureNamePrefix("material.");
    Model sharkModel("resources/objects/shark/scene.gltf");
    sharkModel.SetShaderTextureNamePrefix("material.");

    //stbi_set_flip_vertically_on_load(false);





    PointLight pointLight;
    pointLight.position = glm::vec3(0.0f, 1.0, 0.0);
    pointLight.ambient = glm::vec3(1.0f);
    pointLight.diffuse = glm::vec3(1.0f);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;


//    DirLight dirLight;
//    dirLight.direction=glm::vec3(0.5f,0.7f,0.4f);
//    dirLight.ambient=glm::vec3(0.2f);
//    dirLight.diffuse=glm::vec3(0.5f);
//    dirLight.specular=glm::vec3(0.1f);


    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/megatank.png").c_str());

    vector<glm::vec3> megatenkovi;
    megatenkovi.push_back(glm::vec3(-1.4f,  0.0f, -0.4f));
    megatenkovi.push_back(glm::vec3( 1.3f,  0.0f,  0.2f));
    megatenkovi.push_back(glm::vec3( 0.2f,  0.0f,  0.5f));
    megatenkovi.push_back(glm::vec3(-0.7f,  0.0f, -1.7f));
    megatenkovi.push_back(glm::vec3( 0.3f,  0.0f, -0.5f));

    blendingShader.use();
    blendingShader.setInt("texture1", 0);

    //skybox koordinate
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    //slanje podataka na GPU za skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO); //generisemo vertex array objekat
    glGenBuffers(1, &skyboxVBO);//generisemo vertex buffer objekat
    //vbo je u sustini bafer s podacima, a vao ima pokazivace na odredjene atribute u tim baferima

    glBindVertexArray(skyboxVAO);//vezujemo se za vao

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);//aktiviramo bafer
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW); //saljemo podatke

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); //aktivacija prethodnog(vidimo oznaku 0 ili ti location)
    //glBindVertexArray(0) za deaktivaciju

    //ove podatke na lokaciji 0 koristimo kao ulazne podatke za vertex shader

    stbi_set_flip_vertically_on_load(false);
    //niz slika za skybox
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/left.tga"),
                    FileSystem::getPath("resources/textures/skybox/right.tga"),
                    FileSystem::getPath("resources/textures/skybox/up.tga"),
                    FileSystem::getPath("resources/textures/skybox/down.tga"),
                    FileSystem::getPath("resources/textures/skybox/front.tga"),
                    FileSystem::getPath("resources/textures/skybox/back.tga")
            };
    //ucitavamo skybox
    unsigned int cubemapTexture = loadCubemap(faces);

    //petlja renderovanja, dok se ne zatvori prozor petlja se vrti
    while (!glfwWindowShouldClose(window)) {
        //za pomeranje na ekranu WASD
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //kao poll events i update
        processInput(window);


        //faza renderovanja

        //cistimo ekran,crna boja
        glClearColor(0.0f,0.0f,0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //ukljucujemo shader uvek
        ourShader.use();
        pointLight.position = glm::vec3(3.0 * cos(currentFrame), 3.0f, 3.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition",camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                               (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //directional light
//        ourShader.setVec3("dirLight.direction",dirLight.direction);
//        ourShader.setVec3("dirLight.ambient",dirLight.ambient);
//        ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
//        ourShader.setVec3("dirLight.specular",dirLight.specular)
        glDisable(GL_CULL_FACE);
        //mozemo uci u "unutrasnjost" bloka
        modelRender(ourShader,blokPosition,blokScale);
        blokModel.Draw(ourShader);
        glEnable(GL_CULL_FACE);
        modelRender(ourShader,creeperPosition,creeperScale);
        creeperModel.Draw(ourShader);
        modelRender(ourShader,krabPosition,krabScale);
        krabModel.Draw(ourShader);
        modelRender(ourShader,scyphozoaPosition,scyphozoaScale);
        scyphozoaModel.Draw(ourShader);
        modelRender(ourShader,sharkPosition,sharkScale);
        sharkModel.Draw(ourShader);
        glDisable(GL_CULL_FACE);


        blendingShader.use();
        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        for (unsigned int i = 0; i < megatenkovi.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, megatenkovi[i]);
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // draw skybox as last
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);

        // skybox kocka
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // set depth function back to default


        //zamenjuje trenutni bafer s onim koji sluzi za crtanje piksela
        glfwSwapBuffers(window);
        //registruje akcije kao sto je dodir na tastaturi i slicno
        glfwPollEvents();
    }

    //brisemo vao i vbo jer nam ne trebaju vise
    glDeleteVertexArrays(1,&skyboxVAO);
    glDeleteVertexArrays(1,&skyboxVBO);
    glDeleteVertexArrays(1,&transparentVAO);
    glDeleteVertexArrays(1,&transparentVBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //deinicijalizacija
    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    //odredjuje dimenzije za renderovanje
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        ImGuiEnabled=!ImGuiEnabled;
        if (ImGuiEnabled) {
            CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
//ucitavanje skybox
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); ++i)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
//ucitavanje teksture
unsigned int loadTexture(const char *path){
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void modelRender(const Shader& ourShader,glm::vec3 modelPosition,float modelScale){

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model,
                           glm::vec3(modelPosition)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(modelScale));    // it's a bit too big for our scene, so scale it down
    ourShader.setMat4("model", model);
}