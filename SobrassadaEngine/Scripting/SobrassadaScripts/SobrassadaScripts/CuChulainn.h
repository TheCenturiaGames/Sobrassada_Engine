#pragma once

#include "Character.h"

class GameObject;
class CharacterControllerComponent;
class CameraMovement;


enum class CharacterStates
{
    NONE,
    IDLE,
    RUN,
    DASH,
    BASIC_ATTACK,
    AIM
};

class CuChulainn : public Character
{
  public:
    CuChulainn(GameObject* parent);
    virtual ~CuChulainn() noexcept override { parent = nullptr; };

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void OnDeath() override;
    void OnDamageTaken(int amount) override;
    void OnHealed(int amount) override;
    void PerformAttack() override;
    void HandleState(float deltaTime) override;
    void SetAnimations();
    bool CanAttack(float deltaTime) override;
    bool CanAim() const;
    void GetInputs();
    void UpdateTimers(float deltaTime);
    void LookAtMouse();
    void ThrowSpear();

  private:
    std::string cameraName;
    CameraMovement* camera = nullptr;

    bool isDashing          = false;
    float dashCooldown      = 2.0f;
    bool desiredDash        = false;
    float dashBufferTimer   = 0;
    float dashBuffer        = 0.5f;

    bool desiredAttack      = false;
    float attackBufferTimer = 0;
    float attackBuffer      = 0.5f;

    bool desiredAim         = false;
    float throwTimer        = 0;
    float throwCooldown     = 1.0f;

    CharacterStates state   = CharacterStates::IDLE;
};

extern CharacterControllerComponent* character;
