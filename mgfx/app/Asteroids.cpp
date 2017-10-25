#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/device/IDevice.h"
#include "file/media_manager.h"
#include "animation/timer.h"

#include "stb/stb_image.h"
#include "json/src/json.hpp"

#include "Asteroids.h"
#include <graphics3d/camera/camera.h>

#include <glm/gtx/vector_angle.hpp>

// This is a simple demo of showing sprites on the screen from a loaded sprite map
// A simple physical simulation manages the velocities of objects and their collisions
// TODO: 
// 'Game Levels'
// Oriented rectangle collisions
// Expose more game parameters in the UI
using namespace Mgfx;
using namespace nlohmann;

namespace
{
const float MaxAcceleration = 1000.0f;
const float MaxVelocity = 1000.0f;

struct ShooterWindowData : public BaseWindowData
{
    ShooterWindowData(Mgfx::Window* pWindow)
        : BaseWindowData(pWindow)
    {

    }

    virtual ~ShooterWindowData()
    {
    }

    // Cleanup window specific stuff
    virtual void Free() override
    {
        m_pWindow->GetDevice()->DestroyTexture(textureQuad);
        BaseWindowData::Free();
    }

    // Sprite texture
    uint32_t textureQuad = 0;
    glm::uvec2 textureSize;
};

struct Properties
{
    float UFOEasySpeed = 120.0f;
    float UFOHardSpeed = 200.0f;
    float UFOSpawnTime = 10.0f;
    float UFOLifeTime = 14.0f;
    float UFOFireTime = 1.f;
    float UFOLaserSpeed = 200.0f;
    float UFOLaserDuration = 2.0f;

    float BoulderSpeedRange = 80.0f;
    float BoulderMinSpeed = 10.0f;
};

Properties properties;
}

const char* Asteroids::Description() const
{
    return R"(A simple asteroids game, built using a sprite map.  Oriented rectangles are drawn on the screen using the GPU.  They may be blended and sorted in Z.
This is the basics of how you might implement a 2D game on graphics hardware, for example.
This demo shows how to do basic physics, parse a json file for tile coordinates, and handle basic collisions (which could be improved).
Keys: A,D Rotate, W Thrust, SPACE Shoot
)";
}

void Asteroids::CleanUp()
{
    m_allEntities.clear();
    m_spriteCoords.clear();
    m_bigBoulders.clear();
    m_mediumBoulders.clear();
    m_smallBoulders.clear();
    m_stars.clear();
    m_numbers.clear();
    m_boulders.clear();

    m_pShip = nullptr;
    m_pFire1 = nullptr;
    m_pFire2 = nullptr;
    m_pUFO = nullptr;
}

bool Asteroids::Init()
{
    m_spCamera = std::make_shared<Camera>(CameraMode::Ortho);

    auto spriteData = MediaManager::Instance().LoadAsset("shooter_sprites.json", MediaType::Texture);
    json parse = json::parse(spriteData);
    m_numbers.resize(10);
    for (auto& entry : parse["TextureAtlas"]["SubTexture"])
    {
        auto x = std::stoi(entry["x"].get<std::string>());
        auto y = std::stoi(entry["y"].get<std::string>());
        auto width = std::stoi(entry["width"].get<std::string>());
        auto height = std::stoi(entry["height"].get<std::string>());
        std::string name = entry["name"].get<std::string>();
        m_spriteCoords[name] = glm::uvec4(x, y, width, height);

        if (name.find("star") == 0)
        {
            m_stars.push_back(name);
        }
        else if (name.find("meteor") != std::string::npos)
        {
            if (name.find("big") != std::string::npos)
            {
                m_bigBoulders.push_back(name);
            }
            else if (name.find("small") != std::string::npos)
            {
                m_mediumBoulders.push_back(name);
            }
            else if (name.find("tiny") != std::string::npos)
            {
                m_smallBoulders.push_back(name);
            }
        }
        else if (name.find("playerShip1_blue") != std::string::npos)
        {
            m_shipName = name;
        }
        else if (name.find("fire02") != std::string::npos)
        {
            m_fire1 = name;
        }
        else if (name.find("fire01") != std::string::npos)
        {
            m_fire2 = name;
        }
        else if (name.find("laserBlue03") != std::string::npos)
        {
            m_laser = name;
        }
        else if (name.find("powerupRed") != std::string::npos)
        {
            m_ufoLaser = name;
        }
        else if (name.find("ufoGreen") != std::string::npos)
        {
            m_greenUFO = name;
        }
        else if (name.find("ufoRed") != std::string::npos)
        {
            m_redUFO = name;
        }
        else if (name.find("numeral") != std::string::npos)
        {
            auto prefix = strlen("numeral");
            if (name.size() > prefix)
            {
                auto strNum = name.substr(prefix, 1);
                if (strNum[0] >= '0' && strNum[0] <= '9')
                {
                    auto pNum = AddEntity(EntityType::Digit, name);
                    pNum->sizeScale = 2.0f;
                    m_numbers[std::stoi(strNum)] = pNum;

                }
            }
        }
    }

    return true;
}

void Asteroids::RemoveEntity(Entity* spEntity)
{
    if (!spEntity)
        return;

    auto itrFound = m_allEntities.find(spEntity->type);
    if (itrFound != m_allEntities.end())
    {
        // Erase if a boulder
        if (spEntity->type == EntityType::BigBoulder ||
            spEntity->type == EntityType::MediumBoulder ||
            spEntity->type == EntityType::SmallBoulder)
        {
            m_boulders.erase(spEntity);
        }

        if (spEntity == m_pUFO)
        {
            m_pUFO = nullptr;
        }
        itrFound->second.erase(spEntity);
        delete spEntity;
    }
}

Entity* Asteroids::AddEntity(EntityType type, HashString spriteName)
{
    static uint32_t CurrentID = 0;

    auto spEntity = new Entity();
    spEntity->id = CurrentID++;
    spEntity->type = type;
    spEntity->spriteCoords = m_spriteCoords[spriteName];
    spEntity->spriteSize = glm::ivec2(m_spriteCoords[spriteName].z, m_spriteCoords[spriteName].w);
    spEntity->damage = 0;
    spEntity->position = glm::vec3(0.0f);
    spEntity->velocity = glm::vec3(0.0f);
    spEntity->acceleration = glm::vec3(0.0f);
    spEntity->angle = 0.0f;
    spEntity->rotationalVelocity = 0.0f;
    m_allEntities[type].insert(spEntity);
    return spEntity;
}

void Asteroids::SplitBoulder(Entity* boulder)
{
    std::vector<Entity*> newBoulders;
    switch (boulder->type)
    {
    case EntityType::BigBoulder:
        newBoulders.push_back(AddEntity(EntityType::MediumBoulder, *select_randomly(m_mediumBoulders.begin(), m_mediumBoulders.end())));
        newBoulders.push_back(AddEntity(EntityType::MediumBoulder, *select_randomly(m_mediumBoulders.begin(), m_mediumBoulders.end())));
        AddExplosion(boulder->position, .65f);
        break;
    case EntityType::MediumBoulder:
        newBoulders.push_back(AddEntity(EntityType::SmallBoulder, *select_randomly(m_smallBoulders.begin(), m_smallBoulders.end())));
        newBoulders.push_back(AddEntity(EntityType::SmallBoulder, *select_randomly(m_smallBoulders.begin(), m_smallBoulders.end())));
        AddExplosion(boulder->position, .45f);
        break;
    default:
        AddExplosion(boulder->position, .25f);
        break;
    }
    for (auto& child : newBoulders)
    {
        child->position = boulder->position;
        child->velocity = boulder->velocity;
        child->rotationalVelocity = boulder->rotationalVelocity * glm::linearRand(2.0f, 3.0f);
        child->angle = RandRange(0.0f, 360.0f);

        auto currentVelocityLength = length(boulder->velocity);
        auto vRand = glm::normalize(glm::linearRand(glm::vec2(-1.0f), glm::vec2(1.0f)));

        child->velocity = vRand * currentVelocityLength;
        child->sizeScale = 3.0f;
        m_boulders.insert(child);
    }
    RemoveEntity(boulder);
}


Entity* Asteroids::AddUFO(EntityType type)
{
    auto pUFO = AddEntity(type, type == EntityType::UFOEasy ? m_greenUFO : m_redUFO);
    pUFO->position = GetRandomEntryPoint(pUFO->spriteSize);
    if (type == EntityType::UFOEasy)
    {
        pUFO->velocity = glm::linearRand(glm::vec3(-1.0f) * properties.UFOEasySpeed, glm::vec3(1.0f) * properties.UFOEasySpeed);
    }
    else
    {
        pUFO->velocity = glm::linearRand(glm::vec3(-1.0f) * properties.UFOHardSpeed, glm::vec3(1.0f) * properties.UFOHardSpeed);
    }
    m_ufoTimer.Restart();
    m_nextUFOFireTime = properties.UFOFireTime;
    return pUFO;
}

void Asteroids::UFOFire()
{
    if (!m_pUFO)
    {
        return;
    }
    auto pLaser = AddEntity(EntityType::UFOLaser, m_ufoLaser);
    pLaser->position = m_pUFO->position;
    glm::vec2 laserDir = glm::normalize(m_pShip->position - m_pUFO->position);

    float dither = glm::linearRand(-90.0f, 90.0f);
    if (fabs(dither) < 10.0f)
    {
        dither = glm::linearRand(-60.0f, 60.0f);
    }
    laserDir = glm::rotate(laserDir, glm::radians(dither));

    laserDir *= properties.UFOLaserSpeed;

    pLaser->velocity = laserDir;
    pLaser->angle = glm::linearRand(0.0f, 360.0f);
    pLaser->rotationalVelocity = 100.0f;
    pLaser->sizeScale = .75f;
    pLaser->color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    pLaser->death = properties.UFOLaserDuration;

}

// Find a location around the edge of the world to add an enemy object
glm::vec3 Asteroids::GetRandomEntryPoint(const glm::ivec2& spriteSize) const
{
    auto maxSpriteSize = std::max(spriteSize.x, spriteSize.y);
    auto maxWorldSize = std::max(m_worldSize.x, m_worldSize.y);
    auto location = RandRange(0, int(maxWorldSize));
    auto entrySide = RandRange(0, 3);
    switch (entrySide)
    {
    default:
    case 0:
        return glm::vec3(location, 0.0f, glm::linearRand(.1f, .9f));
    case 1:
        return glm::vec3(0.0f, location, glm::linearRand(.1f, .9f));
    case 2:
        return glm::vec3(location, maxSpriteSize + m_worldSize.y, glm::linearRand(.1f, .9f));
    case 3:
        return glm::vec3(maxSpriteSize + m_worldSize.x, location, glm::linearRand(.1f, .9f));
    }
}

Entity* Asteroids::AddBoulder()
{
    auto pRock = AddEntity(EntityType::BigBoulder, *select_randomly(m_bigBoulders.begin(), m_bigBoulders.end()));

    pRock->position = GetRandomEntryPoint(pRock->spriteSize);
    pRock->rotationalVelocity = glm::linearRand(0.0f, 45.0f);
    while (fabs(pRock->velocity.x) < properties.BoulderMinSpeed || fabs(pRock->velocity.y) < properties.BoulderMinSpeed)
    {
        pRock->velocity = glm::linearRand(glm::vec3(-properties.BoulderSpeedRange), glm::vec3(properties.BoulderSpeedRange));
    }
    pRock->acceleration = glm::vec2(0.0f);
    pRock->angle = glm::linearRand(0.0f, 360.0f);
    pRock->sizeScale = 1.5f;
    m_boulders.insert(pRock);
    return pRock;
}

Entity* Asteroids::AddStar()
{
    auto pRock = AddEntity(EntityType::Star, *select_randomly(m_stars.begin(), m_stars.end()));

    auto maxSpriteSize = std::max(pRock->spriteSize.x, pRock->spriteSize.y);
    auto maxWorldSize = std::max(m_worldSize.x, m_worldSize.y);
    auto entrySide = RandRange(0, 3);
    auto location = RandRange(0, int(maxWorldSize));

    // A randomly placed star, with slow rotation, slow velocity and dim color
    pRock->rotationalVelocity = glm::linearRand(0.0f, 1.0f);
    pRock->position = glm::linearRand(glm::vec3(0.0f), glm::vec3(m_worldSize.x, m_worldSize.y, 0.0f));
    pRock->position.z = 0.0f;
    pRock->velocity = glm::linearRand(glm::vec3(-3.f), glm::vec3(3.f));
    pRock->angle = glm::linearRand(0.0f, 360.0f);
    pRock->sizeScale = .25f;
    pRock->color = glm::vec4(.75f);
    return pRock;
}

void Asteroids::AddExplosion(const glm::vec3& position, float duration)
{
    std::vector<Entity*> vecParticles;
    vecParticles.push_back(AddStar());
    vecParticles.push_back(AddStar());
    vecParticles.push_back(AddStar());
    vecParticles.push_back(AddStar());
    vecParticles.push_back(AddStar());

    // particles are fast, bright and don't live long 
    for (auto& particle : vecParticles)
    {
        particle->position = position;
        particle->death = duration;
        particle->color *= 1.5f;
        particle->velocity = glm::linearRand(glm::vec3(-500.0f), glm::vec3(500.0f));
    }
}

void Asteroids::Restart()
{
    // Entity management could be better!
    RemoveEntity(m_pShip);
    RemoveEntity(m_pUFO);

    while (!m_allEntities[EntityType::Star].empty())
    {
        RemoveEntity(*m_allEntities[EntityType::Star].begin());
    }

    while (!m_boulders.empty())
    {
        RemoveEntity(*m_boulders.begin());
    }

    m_pShip = AddEntity(EntityType::Ship, m_shipName);
    m_pShip->position = glm::vec3(m_worldSize.x / 2, m_worldSize.y / 2, 0.0f);

    for (int i = 0; i < 5; i++)
    {
        AddBoulder();
    }
    
    for (int i = 0; i < 20; i++)
    {
        AddStar();
    }

    m_pFire1 = AddEntity(EntityType::Exhaust, m_fire1);
    m_pFire2 = AddEntity(EntityType::Exhaust, m_fire2);

    m_gameState = GameState::Spawning;
    m_pUFO = nullptr;

    m_ufoTimer.Restart();
    m_spawnTimer.Restart();
    m_score = 0;
    m_lives = 3;
}

void Asteroids::ResizeWindow(Mgfx::Window* pWindow)
{
    /* Nothing */
}

void Asteroids::AddToWindow(Mgfx::Window* pWindow)
{
    auto data = MediaManager::Instance().LoadAsset("shooter_sprites.png", MediaType::Texture);
    uint32_t quad = pWindow->GetDevice()->CreateTexture();

    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load_from_memory((stbi_uc*)(data.c_str()), int(data.size()), &w, &h, &comp, 4);
    if (image)
    {
        auto textureData = pWindow->GetDevice()->ResizeTexture(quad, glm::uvec2(w, h));
        if (textureData.pData)
        {
            for (auto y = 0; y < h; y++)
            {
                memcpy(textureData.LinePtr(y, 0), image + y * sizeof(glm::u8vec4) * w, w * sizeof(glm::u8vec4));
            }
        }
        stbi_image_free(image);
    }
    pWindow->GetDevice()->UpdateTexture(quad);

    auto pWindowData = GetWindowData<ShooterWindowData>(pWindow);
    pWindowData->AddGeometry(100000 * sizeof(GeometryVertex), 100000 * sizeof(uint32_t));
    pWindowData->textureQuad = quad;
    pWindowData->textureSize = glm::uvec2(w, h);

    m_worldSize = pWindow->GetClientSize();

    Restart();
}

void Asteroids::RemoveFromWindow(Mgfx::Window* pWindow)
{
    FreeWindowData(pWindow);
}

void Asteroids::DrawGUI(Mgfx::Window* pWindow)
{
    if (ImGui::Button("Restart (Some properties required this)"))
    {
        Restart();
    }
    ImGui::SliderFloat("UFO Easy Speed", &properties.UFOEasySpeed, 10.0f, 200.0f);
    ImGui::SliderFloat("UFO Hard Speed", &properties.UFOHardSpeed, 10.0f, 400.0f);
    ImGui::SliderFloat("UFO Spawn Time", &properties.UFOSpawnTime, 1.0f, 60.0f);
    ImGui::SliderFloat("UFO Life Time", &properties.UFOLifeTime, 2.0f, 60.0f);
    ImGui::SliderFloat("UFO Fire Time", &properties.UFOFireTime, .1f, 5.0f);
    ImGui::SliderFloat("UFO Laser Speed", &properties.UFOLaserSpeed, 1.0f, 400.0f);
    ImGui::SliderFloat("UFO Laser Duration", &properties.UFOLaserDuration, 1.0f, 10.0f);
    ImGui::SliderFloat("Boulder Speed Range", &properties.BoulderSpeedRange, 1.0f, 200.0f);
    ImGui::SliderFloat("Boulder Min Speed Range", &properties.BoulderMinSpeed, 1.0f, 100.0f);

    if ((properties.BoulderMinSpeed + 1.0f) >= properties.BoulderSpeedRange)
    {
        properties.BoulderMinSpeed = properties.BoulderSpeedRange - 1.0f;
    }
}

void Asteroids::WrapAtBorders()
{
    for (auto& entityType : m_allEntities)
    {
        for (auto& entity : entityType.second)
        {
            bool wrapped = false;
            // Sprite position is origin center, and sprites are double sized !
            auto spriteSize = glm::ceil(glm::vec2(entity->spriteSize) / 4.0f);
            if (entity->position.x < -spriteSize.x)
            {
                wrapped = true;
                entity->position.x = float(spriteSize.x + m_worldSize.x);
            }
            if (entity->position.y < -spriteSize.y)
            {
                wrapped = true;
                entity->position.y = float(spriteSize.y + m_worldSize.y);
            }
            if (entity->position.x > (m_worldSize.x + spriteSize.x))
            {
                wrapped = true;
                entity->position.x = float(-spriteSize.x);
            }
            if (entity->position.y > (m_worldSize.y + spriteSize.y))
            {
                wrapped = true;
                entity->position.y = float(-spriteSize.y);
            }

            // Special handling for UFO - kill it inside boundary
            if (wrapped && entity == m_pUFO)
            {
                if (m_ufoTimer.GetDelta() > properties.UFOLifeTime)
                {
                    // Force UFO death inside the boundary
                    m_pUFO->death = .1f;
                    m_ufoTimer.Restart();
                }
            }
        }
    }
}

void Asteroids::HandleInput()
{
    auto timeDelta = m_inputTimer.GetDelta();
    m_inputTimer.Restart();

    if (m_gameState != GameState::Playing &&
        m_gameState != GameState::Spawning)
    {
        return;
    }
    auto shipForward = glm::vec2(cos(glm::radians(m_pShip->angle - 90.0f)), sin(glm::radians(m_pShip->angle - 90.0f)));
    if (ImGui::GetIO().KeysDown[SDLK_w])
    {
        m_pShip->acceleration += shipForward * MaxAcceleration * timeDelta;
        m_pShip->acceleration = glm::clamp(m_pShip->acceleration, glm::vec2(-MaxAcceleration), glm::vec2(MaxAcceleration));
    }
    else
    {
        m_pShip->acceleration = glm::vec2(0.0f);
    }

    float angleDelta = std::min(m_keyholdTimer.GetDelta() * 2000.0f, 300.0f);
    if (ImGui::GetIO().KeysDown[SDLK_a])
    {
        m_pShip->angle -= angleDelta * timeDelta;
    }
    else if (ImGui::GetIO().KeysDown[SDLK_d])
    {
        m_pShip->angle += angleDelta * timeDelta;
    }
    else
    {
        m_keyholdTimer.Restart();
    }

    if (ImGui::GetIO().KeysDown[SDLK_SPACE])
    {
        if (m_bCanShoot)
        {
            m_bCanShoot = false;
            auto pLaser = AddEntity(EntityType::Laser, m_laser);
            pLaser->position = m_pShip->position + glm::vec3(shipForward * float(m_pShip->spriteSize.y *.2f), 0.0);
            pLaser->startPosition = pLaser->position;
            pLaser->angle = m_pShip->angle;
            pLaser->death = .9f;
            pLaser->color = glm::vec4(1.0f);
            pLaser->velocity = shipForward * 500.0f;
        }
    }
    else
    {
        m_bCanShoot = true;
    }
}

void Asteroids::StepPhysics()
{
    const float MaxAcceleration = 1000.0f;
    const float MaxVelocity = 1000.0f;

    auto timeDelta = m_physicsTimer.GetDelta();

    // Update physics 50fps
    if (timeDelta < 0.02f)
    {
        return;
    }
    m_physicsTimer.Restart();

    // Slow ship over time
    m_pShip->velocity += -m_pShip->velocity * std::min(timeDelta, 1.0f) * .5f;

    std::set<Entity*> victims;
    for (auto& entityType : m_allEntities)
    {
        for (auto& entity : entityType.second)
        {
            entity->age += timeDelta;
            entity->velocity += entity->acceleration * timeDelta;
            entity->angle += (entity->rotationalVelocity * timeDelta);
            entity->position += glm::vec3(entity->velocity, 0.0f) * timeDelta;

            if (entity->death != 0.0f && entity->age >= entity->death)
            {
                victims.insert(entity);
            }
        }
    }

    if (m_gameState != GameState::Playing &&
        m_gameState != GameState::Spawning)
    {
        // No colllisions at the end of the game
        return;
    }

    struct Collision
    {
        Entity* spEntity1;
        Entity* spEntity2;
    };
    std::vector<Collision> collisions;
    std::set<Entity*> collidedEntities;
    auto HasCollided = [&](Entity* pEntity) { return collidedEntities.find(pEntity) != collidedEntities.end(); };
    auto CheckCollision = [&](Entity* spEntity1, Entity* spEntity2)
    {
        if (HasCollided(spEntity1) || HasCollided(spEntity2))
            return false;

        auto minSize = std::min(spEntity1->spriteSize.x, spEntity1->spriteSize.y) * .5f;
        minSize += std::min(spEntity2->spriteSize.x, spEntity2->spriteSize.y) * .5f;
        if (glm::distance(spEntity1->position, spEntity2->position) < minSize)
        {
            collisions.push_back(Collision{ spEntity1, spEntity2 });
            collidedEntities.insert(spEntity1);
            collidedEntities.insert(spEntity2);
            return true;
        }
        return false;
    };

    // Bullet with boulder and UFO
    for (auto& bullet : m_allEntities[EntityType::Laser])
    {
        for (auto& boulder : m_boulders)
        {
            CheckCollision(bullet, boulder);
        }

        if (m_pUFO)
        {
            CheckCollision(bullet, m_pUFO);
        }
    }

    if (m_gameState != GameState::Spawning)
    {
        // Ship with a boulder
        for (auto& boulder : m_boulders)
        {
            CheckCollision(boulder, m_pShip);
        }

        for (auto& ufoLaser : m_allEntities[EntityType::UFOLaser])
        {
            CheckCollision(ufoLaser, m_pShip);
        }
    }

    if (m_pUFO)
    {
        // Ship with UFO
        CheckCollision(m_pUFO, m_pShip);
    }

    auto HandleCollision = [&](Entity* spEntity)
    {
        switch (spEntity->type)
        {
        case EntityType::SmallBoulder:
            m_score += 5;
        case EntityType::MediumBoulder:
            m_score += 5;
        case EntityType::BigBoulder:
            m_score += 5;
            SplitBoulder(spEntity);
            break;
        case EntityType::UFOHard:
            m_score += 20;
        case EntityType::UFOEasy:
            m_score += 20;
            AddExplosion(spEntity->position, .3f);
            RemoveEntity(spEntity);
            m_ufoTimer.Restart();
            break;
        case EntityType::Ship:
        {
            AddExplosion(spEntity->position, 1.0f);
            m_gameState = GameState::Spawning;
            m_pShip->color.a = 0.0f;
            m_pShip->velocity = glm::vec2(0.0f);
            m_pShip->position = glm::vec3(m_worldSize.x / 2, m_worldSize.y / 2, 0.0f);
            m_spawnTimer.Restart();
            m_lives--;
            if (m_lives <= 0)
            {
                m_gameState = GameState::End;
            }
        }
        break;
        default:
        {
            RemoveEntity(spEntity);
        }
        break;

        }
    };

    for (auto& collision : collisions)
    {
        HandleCollision(collision.spEntity1);
        HandleCollision(collision.spEntity2);
    }

    for (auto& victim : victims)
    {
        RemoveEntity(victim);
    }

    WrapAtBorders();
}

void Asteroids::UpdateGameState()
{
    m_pFire1->color = glm::vec4(0.0f);
    m_pFire2->color = glm::vec4(0.0f);

    // Update visibility of ships thruster 
    float shipAcceleration = glm::length(m_pShip->acceleration);
    if (shipAcceleration != 0.0f)
    {
        auto shipForward = glm::vec2(cos(glm::radians(m_pShip->angle - 90.0f)), sin(glm::radians(m_pShip->angle - 90.0f)));
        // Over a certain speed, change the ship's thruster
        if (shipAcceleration > (MaxAcceleration / 5))
        {
            m_pFire2->position = m_pShip->position - glm::vec3(shipForward * float(m_pShip->spriteSize.y *.4f), 0.0);
            m_pFire2->angle = m_pShip->angle;
            m_pFire2->color = glm::vec4(1.0f);
        }
        else
        {
            m_pFire1->position = m_pShip->position - glm::vec3(shipForward * float(m_pShip->spriteSize.y *.4f), 0.0);
            m_pFire1->angle = m_pShip->angle;
            m_pFire1->color = glm::vec4(1.0f);
        }
    }

    switch (m_gameState)
    {
    case GameState::End:
        m_pShip->color.a = 0.0f;
        m_pFire1->color.a = 0.0f;
        m_pFire2->color.a = 0.0f;
        return;
        break;
    case GameState::Spawning:
    {
        auto delta = m_spawnTimer.GetDelta();
        if (delta > 3.0f)
        {
            m_gameState = GameState::Playing;
            m_pShip->color.a = 1.0f;
        }
        else
        {
            if (delta < .75f)
            {
                m_pShip->color.a = 0.0f;
            }
            else
            {
                m_pShip->color.a = std::cos(glm::radians(delta * 500.0f)) * .5f + .5f;
            }
        }
    }
    break;
    default:
        m_pShip->color.a = 1.0f;
        break;
    }

    // Spawn the UFO if appropriate
    auto ufoTime = m_ufoTimer.GetDelta();
    if (m_pUFO == nullptr)
    {
        if (ufoTime > properties.UFOSpawnTime)
        {
            m_pUFO = AddUFO(EntityType::UFOEasy);
        }
    }
    else
    {
        if (ufoTime >= m_nextUFOFireTime)
        {
            m_nextUFOFireTime = ufoTime + properties.UFOFireTime;
            UFOFire();
        }
    }
}

void Asteroids::Render(Mgfx::Window* pWindow)
{
    m_worldSize = pWindow->GetClientSize();

    HandleInput();

    StepPhysics();

    UpdateGameState();

    auto size = pWindow->GetClientSize();
    auto pWindowData = GetWindowData<ShooterWindowData>(pWindow);

    m_spCamera->SetFilmSize(size);
    m_spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pWindow->GetDevice()->SetCamera(m_spCamera.get());

    uint32_t numTextures = RandRange(100, 500);

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    auto pVertices = (GeometryVertex*)pWindowData->GetVB()->Map(10000, sizeof(GeometryVertex), vertexOffset);
    auto pIndices = (uint32_t*)pWindowData->GetIB()->Map(10000, sizeof(uint32_t), indexOffset);

    auto pStartIndices = pIndices;
    auto pStartVertices = pVertices;

    uint32_t currentVertex = 0;

    auto drawSprite = [&](const Entity& entity)
    {
        if (!pVertices || !pIndices)
        {
            return;
        }
        glm::vec3 right = glm::vec3(cos(glm::radians(entity.angle)), sin(glm::radians(entity.angle)), 0.0f);
        glm::vec3 down = glm::vec3(cos(glm::radians(entity.angle + 90.0f)), sin(glm::radians(entity.angle + 90.0f)), 0.0f);
        glm::vec3 entitySize = glm::vec3(entity.spriteSize.x, entity.spriteSize.y, 0.0f) * .25f * entity.sizeScale;

        glm::vec3 quadTopLeft = entity.position - (right * entitySize.x) - (down * entitySize.y);
        glm::vec3 quadTopRight = entity.position + (right * entitySize.x) - (down * entitySize.y);
        glm::vec3 quadBottomLeft = entity.position - (right * entitySize.x) + (down * entitySize.y);
        glm::vec3 quadBottomRight = entity.position + (right * entitySize.x) + (down * entitySize.y);

        glm::vec4 spriteTexCoords(entity.spriteCoords.x, entity.spriteCoords.y, (entity.spriteCoords.x + entity.spriteSize.x), (entity.spriteCoords.y + entity.spriteSize.y));
        spriteTexCoords += glm::vec4(.5f, .5f, -.5f, -.5f);
        spriteTexCoords.x /= pWindowData->textureSize.x;
        spriteTexCoords.z /= pWindowData->textureSize.x;
        spriteTexCoords.y /= pWindowData->textureSize.y;
        spriteTexCoords.w /= pWindowData->textureSize.y;

        *pVertices++ = GeometryVertex(quadTopLeft, glm::vec2(spriteTexCoords.x, spriteTexCoords.y), entity.color);
        *pVertices++ = GeometryVertex(quadTopRight, glm::vec2(spriteTexCoords.z, spriteTexCoords.y), entity.color);
        *pVertices++ = GeometryVertex(quadBottomLeft, glm::vec2(spriteTexCoords.x, spriteTexCoords.w), entity.color);
        *pVertices++ = GeometryVertex(quadBottomRight, glm::vec2(spriteTexCoords.z, spriteTexCoords.w), entity.color);

        // Quad
        *pIndices++ = 0 + currentVertex;
        *pIndices++ = 1 + currentVertex;
        *pIndices++ = 2 + currentVertex;
        *pIndices++ = 2 + currentVertex;
        *pIndices++ = 1 + currentVertex;
        *pIndices++ = 3 + currentVertex;
        currentVertex += 4;
    };


    // Draw the world 
    for (auto& entityType : m_allEntities)
    {
        if (entityType.first == EntityType::Digit)
        {
            continue;
        }
        for (auto& entity : entityType.second)
        {
            drawSprite(*entity);
        }
    }

    auto drawNumber = [&](int number, glm::vec2& pos)
    {
        std::string text = std::to_string(number);
        for (int digit = 0; digit < text.length(); digit++)
        {
            int index = text[digit] - '0';
            // sanity
            if (index < 0 || index > 9)
                continue;

            auto pEntity = m_numbers[index];
            pEntity->position = glm::vec3(pos, 0.0f);
            pos.x += pEntity->spriteSize.x + 5;
            drawSprite(*pEntity);
        }
    };

    glm::vec2 scorePos(15, 15);
    drawNumber(m_score, scorePos);

    scorePos.x += 80;
    drawNumber(m_lives, scorePos);

    pWindowData->GetIB()->UnMap();
    pWindowData->GetVB()->UnMap();

    uint32_t numVertices = uint32_t(pVertices - pStartVertices);
    uint32_t numIndices = uint32_t(pIndices - pStartIndices);

    pWindow->GetDevice()->BeginGeometry(pWindowData->textureQuad, pWindowData->GetVB(), pWindowData->GetIB());
    pWindow->GetDevice()->DrawTriangles(vertexOffset, indexOffset, numVertices, numIndices);
    pWindow->GetDevice()->EndGeometry();
}
