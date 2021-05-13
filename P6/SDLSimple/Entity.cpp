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



void Entity::CheckCollisionsX(Entity* objects, int objectCount)
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

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {

            collided = true;
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedBottom = true;

            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedTop = true;

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
                    if (collidedTop || collidedBottom) {
                        game_over = true;
                    }
                }
            }
        }
    }
}


void Entity::CheckCollisionsX(Map* map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeft = true;
    }

    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRight = true;
    }
    if (map->Win(left))
    {
        game_success = true;
    }
    if (map->Win(right))
    {
        game_success = true;
    }
}

void Entity::CheckCollisionsY(Map* map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y - (width / 2), position.z);
    glm::vec3 bottom = glm::vec3(position.x , position.y + (width / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }

    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }

    if (map->Win(top))
    {
        game_success = true;
    }

    if (map->Win(bottom))
    {
        game_success = true;
    }
}


void Entity::AIWalker(Entity* enemies) {
    int x, y;
    int xdiff = enemies->position.x - position.x;
    int ydiff = enemies->position.y - position.y;
    if (enemies->position.x > position.x) {
        x = 1;

    }
    else {
        x = -1;

    }
    if (enemies->position.y > position.y) {
        y = 1;
    }
    else {
        y = -1;

    }
    if (abs(xdiff) > abs(ydiff)) {
        if (xdiff > 0) {
            animIndices = animRight;
        }
        else {
            animIndices = animLeft;
        }
    }
    else {
        if (ydiff > 0) {
            animIndices = animUp;
        }
        else {
            animIndices = animDown;
        }
    }

    if (game_success) {
        x = 0;
        y = 0;
    }
    movement = glm::vec3(x * 0.5, y * 0.5, 0);

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





void Entity::Update(float deltaTime, Map *map, Entity* objects, int objectCount)
{
    if(isActive == false) {
        return;
    }

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY && aiState != DEFEATED) {
        AI(objects);
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
    
    velocity = movement * speed;
    velocity += acceleration * deltaTime;


    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    CheckCollisionsY(objects, objectCount); // Fix if needed

    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(map);
    CheckCollisionsX(objects, objectCount); // Fix if needed



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