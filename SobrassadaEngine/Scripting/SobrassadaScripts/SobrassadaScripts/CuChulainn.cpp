#include "pch.h"

#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "Standalone/CharacterControllerComponent.h"

CuChulainn::CuChulainn(GameObject* parent) : Character(parent, 5, 1, 2.0f, 1.0f, 1.0f)
{
}

bool CuChulainn::Init()
{
    GLOG("Initiating CuChulainn");

    Component* agent = parent->GetComponentByType(COMPONENT_CHARACTER_CONTROLLER);
    if (!agent)
    {
        GLOG("Component CharacterController not found for CuChulainn");
        return false;
    }

    character = dynamic_cast<CharacterControllerComponent*>(agent);

    SetMainCharacter(character);

    return true;
}

void CuChulainn::Update(float deltaTime)
{

}

void CuChulainn::OnDeath()
{
}

void CuChulainn::OnDamageTaken(int amount)
{
}

void CuChulainn::OnHealed(int amount)
{
}

void CuChulainn::PerformAttack()
{
}
