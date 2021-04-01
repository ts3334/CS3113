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
#define ENEMY_COUNT 3
#define ROCK_COUNT 33
#define gravity -9.8f
#define playerSpeed 2.5f
struct GameState {
    Entity *player;
    Entity *rocks;
    Entity *enemies;
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
    displayWindow = SDL_CreateWindow("Textured!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 640, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 640);
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-6.0f, 6.0f, -5.0f, 7.0f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    
    // Initialize Game Objects

    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, gravity, 0);
    state.player->movement = glm::vec3(0);
    state.player->position.y = -3;
    state.player->position.x = -4;
    state.player->speed = 1.5f;
    GLuint playerTextureID = LoadTexture("george_0.png");
    state.player->textureID = playerTextureID;

    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    state.player->height = 0.8f;
    state.player->width = 0.5f;

    state.player->jumpPower = 6.5f;


    state.rocks = new Entity[ROCK_COUNT];
    GLuint rockTextureID = LoadTexture("platformPack_tile040.png");

    for (int i = 0; i < 11; i++) {
        state.rocks[i].textureID = rockTextureID;
        state.rocks[i].position = glm::vec3(i - 5, -4, 0);
    }
    for (int i = 11; i < 19; i++) {
        state.rocks[i].textureID = rockTextureID;
        state.rocks[i].position = glm::vec3(i - 16, 0, 0);
    }

     state.rocks[20].textureID = rockTextureID;
     state.rocks[20].position = glm::vec3(-2, -3, 0);

     state.rocks[21].textureID = rockTextureID;
     state.rocks[21].position = glm::vec3(3, -3, 0);

     state.rocks[19].textureID = rockTextureID;
     state.rocks[19].position = glm::vec3(4, -1, 0);

     state.rocks[22].textureID = rockTextureID;
     state.rocks[22].position = glm::vec3(5, -1, 0);

     state.rocks[30].textureID = rockTextureID;
     state.rocks[30].position = glm::vec3(-3, 2, 0);

     state.rocks[31].textureID = rockTextureID;
     state.rocks[31].position = glm::vec3(-4, 4, 0);

     state.rocks[32].textureID = rockTextureID;
     state.rocks[32].position = glm::vec3(-3, 6, 0);

     for (int i = 23; i < 30; i++) {
         state.rocks[i].textureID = rockTextureID;
         state.rocks[i].position = glm::vec3(i - 24, 3, 0);
     }

    for (int i = 0; i < ROCK_COUNT; i++) {
        state.rocks[i].entityType = ROCK;
        state.rocks[i].Update(0, NULL, NULL, 0, 0);
    }


    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemyTextureID = LoadTexture("ctg.png");

    state.enemies[0].textureID = enemyTextureID;
    state.enemies[0].position = glm::vec3(1, -3, 0);
    state.enemies[0].entityType = ENEMY;
    state.enemies[0].speed = 1;
    state.enemies[0].aiState = ATTACKING;
    state.enemies[0].aiType = PATROLER;

    state.enemies[1].textureID = enemyTextureID;
    state.enemies[1].position = glm::vec3(-1, 5, 0);
    state.enemies[1].movement = glm::vec3(0);
    state.enemies[1].entityType = ENEMY;
    state.enemies[1].speed = 1;
    state.enemies[1].aiState = ATTACKING;
    state.enemies[1].aiType = JUMPER;
    state.enemies[1].acceleration = glm::vec3(0, gravity, 0);

    state.enemies[2].textureID = enemyTextureID;
    state.enemies[2].position = glm::vec3(-5, 2, 0);
    state.enemies[2].entityType = ENEMY;
    state.enemies[2].speed = 1;
    state.enemies[2].aiState = ATTACKING;
    state.enemies[2].aiType = WALKER;
    state.enemies[2].acceleration = glm::vec3(0, gravity, 0);
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
                        if(state.player->collidedBottom == true){
                            state.player->jump = true;
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_A]) {
        state.player->movement.x = -playerSpeed;
        state.player->animIndices = state.player->animLeft;
    }
    else if (keys[SDL_SCANCODE_D]) {
        state.player->movement.x = playerSpeed;
        state.player->animIndices = state.player->animRight;
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
         state.player->position.x = -3;
         state.player->position.y = -4;
         state.player->acceleration.y = gravity;
         state.player->game_over = false;

         GLuint enemyTextureID = LoadTexture("ctg.png");
         state.enemies[0].textureID = enemyTextureID;
         state.enemies[0].position = glm::vec3(1, -3, 0);
         state.enemies[0].entityType = ENEMY;
         state.enemies[0].speed = 1;
         state.enemies[0].aiState = ATTACKING;
         state.enemies[0].aiType = PATROLER;

         state.enemies[1].textureID = enemyTextureID;
         state.enemies[1].position = glm::vec3(-1, 4, 0);
         state.enemies[1].entityType = ENEMY;
         state.enemies[1].speed = 1;
         state.enemies[1].aiState = ATTACKING;
         state.enemies[1].aiType = JUMPER;
         state.enemies[1].acceleration = glm::vec3(0, gravity, 0);

         state.enemies[2].textureID = enemyTextureID;
         state.enemies[2].position = glm::vec3(-4, 2, 0);
         state.enemies[2].entityType = ENEMY;
         state.enemies[2].speed = 0.7;
         state.enemies[2].aiState = ATTACKING;
         state.enemies[2].aiType = WALKER;
         state.enemies[2].acceleration = glm::vec3(0, gravity, 0);
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
        state.player->position.x = -3;
        state.player->position.y = -4;
        state.player->acceleration.y = gravity;
        state.player->game_success = false;

        GLuint enemyTextureID = LoadTexture("ctg.png");
        state.enemies[0].textureID = enemyTextureID;
        state.enemies[0].position = glm::vec3(1, -3, 0);
        state.enemies[0].entityType = ENEMY;
        state.enemies[0].speed = 1;
        state.enemies[0].aiState = ATTACKING;
        state.enemies[0].aiType = PATROLER;

        state.enemies[1].textureID = enemyTextureID;
        state.enemies[1].position = glm::vec3(-1, 4, 0);
        state.enemies[1].entityType = ENEMY;
        state.enemies[1].speed = 1;
        state.enemies[1].aiState = ATTACKING;
        state.enemies[1].aiType = JUMPER;
        state.enemies[1].acceleration = glm::vec3(0, gravity, 0);

        state.enemies[2].textureID = enemyTextureID;
        state.enemies[2].position = glm::vec3(-3.5, 2, 0);
        state.enemies[2].entityType = ENEMY;
        state.enemies[2].speed = 0.7;
        state.enemies[2].aiState = ATTACKING;
        state.enemies[2].aiType = WALKER;
        state.enemies[2].acceleration = glm::vec3(0, gravity, 0);
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
        state.player->Update(FIXED_TIMESTEP, state.rocks, state.enemies, ROCK_COUNT, ENEMY_COUNT);
        int enemiesDefeated = 0;
        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.rocks, state.player, ROCK_COUNT, ENEMY_COUNT);
            if (state.enemies[i].aiState == DEFEATED) {
                enemiesDefeated += 1;
            }
        }
        if (enemiesDefeated == ENEMY_COUNT) {
            state.player->game_success = true;
           mode = GAME_SUCCESS;
        }
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


    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].Render(&program);
    }

    state.player->Render(&program);

    GLuint fontTextureID = LoadTexture("font1.png");
    if(state.player->game_over){
    DrawText(&program, fontTextureID, "GAME OVER", 0.5f, -0.25f, glm::vec3(-5, 5, 0));
    }
    else if (state.player->game_success) {
    DrawText(&program, fontTextureID, "SUCCESS", 0.5f, -0.25f, glm::vec3(-5, 5, 0));
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