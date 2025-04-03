#include "PhysicsModule.h"

#include "Application.h"
#include "BulletDebugDraw.h"
#include "GameObject.h"
#include "SceneModule.h"
#include "Standalone/Physics/CubeColliderComponent.h"

#include "Math/float3.h"
#include "btBulletDynamicsCommon.h"

PhysicsModule::PhysicsModule()
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher             = new btCollisionDispatcher(collisionConfiguration);
    broadPhase             = new btDbvtBroadphase();
    solver                 = new btSequentialImpulseConstraintSolver;
    dynamicsWorld          = new btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfiguration);

    debugDraw              = new BulletDebugDraw();

    debugDraw->setDebugMode(2);

    dynamicsWorld->setGravity(btVector3(0, gravity, 0));
    dynamicsWorld->setDebugDrawer(debugDraw);

    colliderLayerConfig.assign(sizeof(ColliderLayerStrings) / sizeof(char*), LayerBitset().reset());
}

bool PhysicsModule::Init()
{
    LoadLayerData();
    return true;
}

update_status PhysicsModule::PreUpdate(float deltaTime)
{
    if (!App->GetSceneModule()->GetInPlayMode()) return UPDATE_CONTINUE;

    dynamicsWorld->stepSimulation(deltaTime, 10);

    // COLLISION CHECKS
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int currentManifold = 0; currentManifold < numManifolds; ++currentManifold)
    {
        // Collision Handler
        btPersistentManifold* contactManifold =
            dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(currentManifold);

        // CHECKING THAT THERE IS AN ACTUAL COLLISION
        if (contactManifold->getNumContacts() > 0)
        {
            // Getting own data structure to handle callbacks
            BulletUserPointer* firstUserPointer =
                static_cast<BulletUserPointer*>(contactManifold->getBody0()->getUserPointer());
            BulletUserPointer* secondUserPointer =
                static_cast<BulletUserPointer*>(contactManifold->getBody1()->getUserPointer());

            // Calculating normal
            float3 normal = float3(contactManifold->getContactPoint(0).m_normalWorldOnB);

            firstUserPointer->onCollisionCallback->Call(secondUserPointer->collider->GetParent(), normal);
            secondUserPointer->onCollisionCallback->Call(firstUserPointer->collider->GetParent(), -normal);
        }
    }

    return UPDATE_CONTINUE;
}

update_status PhysicsModule::Render(float deltaTime)
{
    if (debugDraw->getDebugMode()) dynamicsWorld->debugDrawWorld();

    return UPDATE_CONTINUE;
}

update_status PhysicsModule::PostUpdate(float deltaTime)
{
    if (updateGravity)
    {
        updateGravity = false;
        dynamicsWorld->setGravity(btVector3(0, gravity, 0));
    }

    // REMOVE RIGID BODIES
    for (btRigidBody* rigidBody : bodiesToRemove)
    {
        dynamicsWorld->removeRigidBody(rigidBody);
        btCollisionShape* shape = rigidBody->getCollisionShape();
        delete shape;
        delete rigidBody;
    }
    bodiesToRemove.clear();

    return UPDATE_CONTINUE;
}

bool PhysicsModule::ShutDown()
{
    EmptyWorld();

    delete dynamicsWorld;
    delete solver;
    delete broadPhase;
    delete dispatcher;
    delete collisionConfiguration;

    return true;
}

void PhysicsModule::CreateCubeRigidBody(CubeColliderComponent* colliderComponent)
{
    // Collision shape
    btCollisionShape* collisionShape = new btBoxShape(btVector3(
        btScalar(colliderComponent->size.x), btScalar(colliderComponent->size.y), btScalar(colliderComponent->size.z)
    ));

    // DYNAMIC -> THIS WILL COME FROM ENUM INSIDE COMPONENT
    bool isDynamic                   = (colliderComponent->mass != 0.f);

    // Inertia
    btVector3 localInertia(1, 0, 0);
    if (isDynamic) collisionShape->calculateLocalInertia(colliderComponent->mass, localInertia);

    // MotionState for RENDER AND
    colliderComponent->motionState = BulletMotionState(
        colliderComponent, colliderComponent->centerOffset, colliderComponent->centerRotation,
        colliderComponent->freezeRotation
    );

    // Creating final RigidBody
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        btScalar(colliderComponent->mass), &colliderComponent->motionState, collisionShape, localInertia
    );
    btRigidBody* newRigidBody = new btRigidBody(rbInfo);

    newRigidBody->setUserPointer(&colliderComponent->userPointer);

    colliderComponent->rigidBody = newRigidBody;

    AddRigidBody(newRigidBody, colliderComponent->GetColliderType(), colliderComponent->GetLayer());
}

void PhysicsModule::UpdateCubeRigidBody(CubeColliderComponent* colliderComponent)
{
    DeleteCubeRigidBody(colliderComponent);
    CreateCubeRigidBody(colliderComponent);
}

void PhysicsModule::DeleteCubeRigidBody(CubeColliderComponent* colliderComponent)
{
    bodiesToRemove.push_back(colliderComponent->rigidBody);
    colliderComponent->rigidBody = nullptr;
}

// TODO UPDATE WITH CHANNELS
void PhysicsModule::AddRigidBody(btRigidBody* rigidBody, ColliderType colliderType, ColliderLayer layerType)
{
    switch (colliderType)
    {
    case ColliderType::DYNAMIC:
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_DYNAMIC_OBJECT);
        break;
    case ColliderType::KINEMATIC:
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        break;
    case ColliderType::TRIGGER:
        rigidBody->setCollisionFlags(
            rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT |
            btCollisionObject::CF_NO_CONTACT_RESPONSE
        );
        break;
    case ColliderType::STATIC:
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
        break;
    default:
        break;
    }

    int group                     = 1 << (int)layerType;

    int mask                      = 0;
    const LayerBitset& maskBitset = colliderLayerConfig[(int)layerType];
    for (int i = 0; i < maskBitset.size(); ++i)
    {
        if (maskBitset[i]) mask |= 1 << i;
    }

    dynamicsWorld->addRigidBody(rigidBody, group, mask);
}

// TODO READ FROM CONFIG FILE
void PhysicsModule::LoadLayerData()
{
    // STATIC OBJECT
    int config =
        1 << (int)ColliderLayer::PLAYER | 1 << (int)ColliderLayer::ENEMY | 1 << (int)ColliderLayer::WORLD_OBJECTS;
    colliderLayerConfig[0] |= config;

    // TRIGGER
    config                  = 1 << (int)ColliderLayer::PLAYER;
    colliderLayerConfig[1] |= config;

    // ENEMY
    config                  = 1 << (int)ColliderLayer::PLAYER | 1 << (int)ColliderLayer::WORLD_OBJECTS;
    colliderLayerConfig[2] |= config;

    // PLAYER
    config =
        1 << (int)ColliderLayer::ENEMY | 1 << (int)ColliderLayer::WORLD_OBJECTS | 1 << (int)ColliderLayer::TRIGGERS;
    colliderLayerConfig[3] |= config;
}

void PhysicsModule::EmptyWorld()
{
    // REMOVE RIGID BODIES
    for (btRigidBody* rigidBody : bodiesToRemove)
    {
        dynamicsWorld->removeRigidBody(rigidBody);
        btCollisionShape* shape = rigidBody->getCollisionShape();
        delete shape;
        delete rigidBody;
    }

    bodiesToRemove.clear();

    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }
}

void PhysicsModule::RebuildWorld()
{
    const auto& allGameObjects = App->GetSceneModule()->GetScene()->GetAllGameObjects();

    for (const auto& gameObject : allGameObjects)
    {
        gameObject.second->UpdateComponents();
    }
}
