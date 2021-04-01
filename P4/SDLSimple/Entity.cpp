#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;


    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other) {
    if (isActive == false || other->isActive == false){
        return false;
    }
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) { 
        return true; 
    }
    return false;
}



void Entity::CheckCollisionsX(Entity* objects, int objectCount, EntityType entityType)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {

            collided = true;
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;

            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;

                /*
                    if (objectCount == 1) {
                        game_success = true;

                    }
                    else {
                        game_over = true;
                    */

            }
            if (object->entityType == ENEMY) {
                if (object->aiType != DEFEATED) {
                    if (collidedLeft || collidedRight) {
                        game_over = true;
                    }
                }
            }
        }
    }
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount, EntityType entityType)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            collided = true;

            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;

            }
            /*
                if (objectCount == 1) {
                    game_success = true;

                }
                else {
                    game_over = true;
                }
                */
            if (object->entityType == ENEMY) {
                if (collidedBottom) {
                    object->aiState = DEFEATED;
                    jump = true;
                }
                else if (collidedTop) {
                    game_over = true;
                }
            }
        }
    }
}


void Entity::AIWalker(Entity *enemies) {
    if (enemies->position.x > position.x) {
        movement = glm::vec3(1, 0, 0);
    }
    else {
        movement = glm::vec3(-1, 0, 0);
    }

}

void Entity::AIJumper() {
    movement = glm::vec3(0);
}

void Entity::AIPatroler() {
    movement = glm::vec3(-1, 0, 0);
}


void Entity::AI(Entity* enemies) {
    switch (aiType) {
        case WALKER:
            AIWalker(enemies);
            break;
        case JUMPER:
            AIJumper();
            break;
        case PATROLER:    
            AIPatroler();
            break;
    }
}





void Entity::Update(float deltaTime, Entity* rocks, Entity* enemies, int rockCount, int ENEMY_COUNT)
{
    if(isActive == false) {
        return;
    }

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY && aiState != DEFEATED) {
        AI(enemies);
    }


    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndex = 0;
        }
    }

    if (jump) {
        jump = false;
        velocity.y += jumpPower;
    }
    
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    if (aiState == DEFEATED){
        velocity.y = -9.81f;
        position.y += velocity.y * deltaTime; // Move on Y

    }
    else{
        position.y += velocity.y * deltaTime; // Move on Y

        CheckCollisionsY(rocks, rockCount, entityType);

        if(entityType == PLAYER){
        CheckCollisionsY(enemies, ENEMY_COUNT, entityType);
        }
        position.x += velocity.x * deltaTime; // Move on X
        CheckCollisionsX(rocks, rockCount, entityType);
        if (entityType == PLAYER) {
            CheckCollisionsX(enemies, ENEMY_COUNT, entityType);
        }
        if (entityType == ENEMY) {
            if (aiType == PATROLER) {
                if (collidedLeft) {
                    speed = speed * -1;
                    collidedLeft = false;
                }
                if (collidedRight) {
                    speed = speed * -1;
                    collidedRight = false;
                }
            }
            if (aiType == JUMPER) {
                if (collidedBottom) {
                    speed = speed * -1;
                    velocity.y += 6;
                }
            }
        }
    }
   


    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {

    if (isActive == false) {
        return;
    }

    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}