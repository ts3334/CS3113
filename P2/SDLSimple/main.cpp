#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;

glm::mat4 viewMatrix, modelMatrix, projectionMatrix, playerMatrix1, playerMatrix2, ballMatrix;

glm::vec3 player_position1 = glm::vec3(4.5, 0, 0);
glm::vec3 player_position2 = glm::vec3(-4.5, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

float player_speed1 = 5.0f;
float player_speed2 = 5.0f;
float ball_speed = 3.0f;

glm::vec3 player_movement1 = glm::vec3(0, 0, 0);
glm::vec3 player_movement2 = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);


GLuint PlayerTextureID, BallTextureID;


GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("PONG!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    PlayerTextureID = LoadTexture("Player.png");
    BallTextureID = LoadTexture("Ball.png");
}

void ProcessInput() {

    player_movement1 = glm::vec3(0);
    player_movement2 = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT: 
                // Move the player left
                break;

            case SDLK_RIGHT:
                
                // Move the player right
                break;

            case SDLK_SPACE:
                // Some sort of action
                break;
            }
            break; // SDL_KEYDOWN
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_UP]) {
        if (player_position1.y < 3.25) {
            player_movement1.y = 1.0f;
        }
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        if (player_position1.y > -3.25) {
            player_movement1.y = -1.0f;
        }
    }



    if (keys[SDL_SCANCODE_W]) {
        if (player_position2.y < 3.25) {
            player_movement2.y = 1.0f;
        }
    }
    else if (keys[SDL_SCANCODE_S]) {
        if (player_position2.y > -3.25) {
            player_movement2.y = -1.0f;
        }
    }



    if (ball_position.x == 0 && ball_position.y == 0) {
        if (keys[SDL_SCANCODE_SPACE]) {
            ball_movement.y = -1.0f;
            ball_movement.x = 1.0f;
            ball_speed = 3.0f;
        }
    }

    if (glm::length(player_movement1) > 1.0f) {
        player_movement1 = glm::normalize(player_movement1);
    }

    if (glm::length(player_movement2) > 1.0f) {
        player_movement2 = glm::normalize(player_movement2);
    }

    if (glm::length(ball_movement) > 1.0f) {
        ball_movement = glm::normalize(ball_movement);
    }
}

float lastTicks = 0.0f;
 
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    // Add (direction * units per second * elapsed time)
    
 

    player_position1 += player_movement1 * player_speed1 * deltaTime;
    player_position2 += player_movement2 * player_speed2 * deltaTime;
    ball_position += ball_movement * ball_speed * deltaTime;

    if (ball_position.y > 3.25 || ball_position.y < -3.25) //check ball top and bottom 
    {
        ball_movement.y = ball_movement.y * -1;
    }

    if (ball_position.x > 4.5 || ball_position.x < -4.5) //check ball left and right
    {
        ball_position = glm::vec3(0, 0, 0);
        ball_speed = 3;
        ball_movement.y = 0;
        ball_movement.x = 0;
    }
    float xdist1 = fabs(ball_position.x - player_position1.x + 0.45) - (0.1 / 2.0f);
    float ydist1 = fabs(ball_position.y - player_position1.y) - (2.1 / 2.0f);

    float xdist2 = fabs(ball_position.x - player_position2.x - 0.45) - (0.1 / 2.0f);
    float ydist2 = fabs(ball_position.y - player_position2.y) - (2.1 / 2.0f);

    if ((xdist1 < 0 && ydist1 < 0) || (xdist2 < 0 && ydist2 < 0)) {
        ball_movement.x = ball_movement.x * -1;
    }

    if (ball_speed < 10) {
        ball_speed += 0.1 * deltaTime;
    }

    playerMatrix1 = glm::mat4(1.0f);
    playerMatrix1 = glm::translate(playerMatrix1, player_position1);

    playerMatrix2 = glm::mat4(1.0f);
    playerMatrix2 = glm::translate(playerMatrix2, player_position2);
    
    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::translate(ballMatrix, ball_position);
}

void DrawPlayer1() {
    program.SetModelMatrix(playerMatrix1);
    glBindTexture(GL_TEXTURE_2D, PlayerTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
} 

void DrawPlayer2() {
    program.SetModelMatrix(playerMatrix2);
    glBindTexture(GL_TEXTURE_2D, PlayerTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawBall() {
    program.SetModelMatrix(ballMatrix);
    glBindTexture(GL_TEXTURE_2D, BallTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    DrawPlayer1();
    DrawPlayer2();
    DrawBall();

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}