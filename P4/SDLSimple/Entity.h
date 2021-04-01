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
enum EntityType{PLAYER, ROCK, PLATFORM, ENEMY};
enum AIType { WALKER, PATROLER, JUMPER};
enum AIState {IDLE, ATTACKING, DEFEATED};
class Entity {
public:
    EntityType entityType;
    AIType aiType;
    AIState aiState;
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    float width = 1;
    float height = 1;

    bool jump = false;
    float jumpPower = 0;

    float speed;

    GLuint textureID;

    glm::mat4 modelMatrix;

    int* animRight = NULL;
    int* animLeft = NULL;
    int* animUp = NULL;
    int* animDown = NULL;

    int* animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    bool isActive = true;

    Entity();
   
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;
    bool collided = false;

    bool game_over = false;
    bool game_success = false;

    
    bool CheckCollision(Entity* other);
    void Update(float deltaTime, Entity* rocks, Entity* enemies, int rockCount, int ENEMY_COUNT);
    void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
    void CheckCollisionsY(Entity* objects, int objectCount, EntityType entityType);
    void CheckCollisionsX(Entity* objects, int objectCount, EntityType entityType);

    void AI(Entity* enemies);
    void AIWalker(Entity* player);
    void AIPatroler();
    void AIJumper();
};