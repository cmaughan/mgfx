#pragma once

#include "MgfxRender.h"
#include "animation/timer.h"
#include "graphics3d/device/IDevice.h"

enum class EntityType
{
    Unknown,
    Ship,
    BigBoulder,
    MediumBoulder,
    SmallBoulder,
    Star,
    Exhaust,
    Laser,
    Explosion,
    UFOEasy,
    UFOHard,
    UFOLaser,
    Digit
};

enum class GameState
{
    Start,
    Spawning,
    Playing,
    End
};

struct Entity
{
    uint32_t id = 0;
    EntityType type;
    glm::vec4 color = glm::vec4(1.0f);
    glm::ivec2 spriteCoords;
    glm::ivec2 spriteSize;
    float sizeScale = 1.0f;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    
    glm::vec3 position;
    float angle;
    float rotationalVelocity;
    float rotationalAcceleration = 0.0f;
    float age = 0.0f;
    float death = 0.0f;
    glm::vec3 startPosition;

    uint32_t damage;
};

namespace Mgfx
{
class Camera;
}

class Asteroids : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "Asteroids"; }
    virtual const char* Description() const override;

private:
    glm::vec3 GetRandomEntryPoint(const glm::ivec2& spriteSize) const;

    Entity* AddEntity(EntityType type, HashString spriteName);
    Entity* AddBoulder();
    Entity* AddStar();
    Entity* AddUFO(EntityType type);
    void AddExplosion(const glm::vec3& position, float duration);
    void RemoveEntity(Entity* spEntity);
    void SplitBoulder(Entity* boulder);

    void Restart();
    void StepPhysics();
    void HandleInput();
    void WrapAtBorders();
    void UpdateGameState();
    void UFOFire();

private:
    std::map<HashString, glm::uvec4> m_spriteCoords;
    std::vector<HashString> m_bigBoulders;
    std::vector<HashString> m_mediumBoulders;
    std::vector<HashString> m_smallBoulders;
    std::vector<HashString> m_stars;

    HashString m_fire1;
    HashString m_fire2;
    HashString m_shipName;
    HashString m_laser;
    HashString m_ufoLaser;
    HashString m_redUFO;
    HashString m_greenUFO;

    std::vector<Entity*> m_numbers;

    std::map<EntityType, std::set<Entity*>> m_allEntities;
    std::set<Entity*> m_boulders;
    Entity* m_pShip = nullptr;
    Entity* m_pFire1 = nullptr;
    Entity* m_pFire2 = nullptr;
    Entity* m_pUFO = nullptr;

    GameState m_gameState = GameState::Start;

    Timer m_inputTimer;
    Timer m_physicsTimer;
    Timer m_spawnTimer;
    Timer m_keyholdTimer;
    Timer m_ufoTimer;
    float m_nextUFOFireTime = 0.0f;
    uint32_t m_score = 0;
    int32_t m_lives = 3;

    glm::uvec2 m_worldSize = glm::uvec2(0);
    bool m_bCanShoot = true;
    std::shared_ptr<Mgfx::Camera> m_spCamera;
};
