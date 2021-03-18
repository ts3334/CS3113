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
#include <vector>
#include "Entity.h"
#define PLATFORM_COUNT 1
#define ROCK_COUNT 15
#define gravity -0.1f
#define playerSpeed 0.6f
struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *rocks;
};

GameState state;
enum GameMode { GAME_LEVEL, GAME_OVER, GAME_SUCCESS };
GameMode mode = GAME_LEVEL;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;


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

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
 offset + (-0.5f * size), 0.5f * size,
 offset + (-0.5f * size), -0.5f * size,
 offset + (0.5f * size), 0.5f * size,
 offset + (0.5f * size), -0.5f * size,
 offset + (0.5f * size), 0.5f * size,
 offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
        u, v,
        u, v + height,
        u + width, v,
        u + width, v + height,
        u + width, v,
        u, v + height,
            });

    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Textured!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Initialize Game Objects

    // Initialize Player
    state.player = new Entity();
    state.player->position = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, gravity, 0);
    state.player->movement = glm::vec3(0);
    state.player->position.y = 3;
    state.player->speed = 1.0f;
    GLuint playerTextureID = LoadTexture("playerShip2_blue.png");
    state.player->textureID = playerTextureID;

    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };

    state.player->animIndices = NULL;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    state.player->height = 1;
    state.player->width = 1;

    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("platformPack_tile065.png");
    state.platforms[0].textureID = platformTextureID;
    state.platforms[0].position = glm::vec3(-3, -3, 0);
    state.platforms[0].height = 0.1;
    state.platforms[0].width = 0.8;
    state.platforms[0].Update(0, NULL, NULL, 0, 0);

    state.rocks = new Entity[ROCK_COUNT];
    GLuint rockTextureID = LoadTexture("platformPack_tile040.png");

    for (int i = 0; i < 11; i++) {
        state.rocks[i].textureID = rockTextureID;
        state.rocks[i].position = glm::vec3(i - 5, -4, 0);
    }

     state.rocks[11].textureID = rockTextureID;
     state.rocks[11].position = glm::vec3(-3, 2, 0);

     state.rocks[12].textureID = rockTextureID;
     state.rocks[12].position = glm::vec3(2, 1, 0);

     state.rocks[13].textureID = rockTextureID;
     state.rocks[13].position = glm::vec3(1, 0, 0);

     state.rocks[14].textureID = rockTextureID;
     state.rocks[14].position = glm::vec3(-2, -1, 0);

    for (int i = 0; i < ROCK_COUNT; i++) {
        state.rocks[i].Update(0, NULL, NULL, 0, 0);
    }


}


void ProcessInputGameLevel() {

    state.player->movement = glm::vec3(0);

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
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_A]) {
        state.player->acceleration.x = -playerSpeed;
    }
    else if (keys[SDL_SCANCODE_D]) {
        state.player->acceleration.x = playerSpeed;
    }
    

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void ProcessInputGameOver() {

    state.player->movement = glm::vec3(0);

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
                break;
            }
            break; // SDL_KEYDOWN
        }
    }


    const Uint8* keys = SDL_GetKeyboardState(NULL);
     if (keys[SDL_SCANCODE_SPACE]) {
         mode = GAME_LEVEL;
         state.player->movement.x = 0.0f;
         state.player->position.x = 0;
         state.player->position.y = 3;
         state.player->acceleration.y = gravity;
         state.player->game_over = false;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void ProcessInputGameSuccess() {

    state.player->movement = glm::vec3(0);

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
                break;
            }
            break; // SDL_KEYDOWN
        }
    }


    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_SPACE]) {
        mode = GAME_LEVEL;
        state.player->movement.x = 0.0f;
        state.player->position.x = 0;
        state.player->position.y = 3;
        state.player->acceleration.y = gravity;
        state.player->game_success = false;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void ProcessInput() {
    switch (mode) {
    case GAME_LEVEL:
        ProcessInputGameLevel();
        break;
    case GAME_OVER:
        ProcessInputGameOver();
        break;
    case GAME_SUCCESS:
        ProcessInputGameSuccess();
        break;

    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP, state.platforms, state.rocks, PLATFORM_COUNT, ROCK_COUNT);
        deltaTime -= FIXED_TIMESTEP;
    }
    if (state.player->game_over == true) {
        mode = GAME_OVER;
    }
    if (state.player->game_success == true) {
        mode = GAME_SUCCESS;
    }
    accumulator = deltaTime;
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < ROCK_COUNT; i++) {
        state.rocks[i].Render(&program);
    }
    state.platforms[0].Render(&program);

    state.player->Render(&program);

    GLuint fontTextureID = LoadTexture("font1.png");
    if(state.player->game_over){
    DrawText(&program, fontTextureID, "GAME OVER", 0.5f, -0.25f, glm::vec3(-4.5, 3.5, 0));
    }
    else if (state.player->game_success) {
    DrawText(&program, fontTextureID, "SUCCESS", 0.5f, -0.25f, glm::vec3(-4.5, 3.5, 0));
    }
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