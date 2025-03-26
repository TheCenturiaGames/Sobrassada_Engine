#include "PhysicsModule.h"

#include "btBulletDynamicsCommon.h"

bool PhysicsModule::Init()
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher             = new btCollisionDispatcher(collisionConfiguration);
    broadPhase             = new btDbvtBroadphase();
    solver                 = new btSequentialImpulseConstraintSolver;
    dynamicsWorld          = new btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, gravity, 0));

    return true;
}

update_status PhysicsModule::PreUpdate(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status PhysicsModule::Render(float deltaTime)
{
    return UPDATE_CONTINUE;
}

update_status PhysicsModule::PostUpdate(float deltaTime)
{
    if (updateGravity)
    {
        updateGravity = false;
        dynamicsWorld->setGravity(btVector3(0, gravity, 0));
    }

    return UPDATE_CONTINUE;
}

bool PhysicsModule::ShutDown()
{
    DeleteWorld();

    return true;
}

void PhysicsModule::DeleteWorld()
{
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body      = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    delete dynamicsWorld;
    delete solver;
    delete broadPhase;
    delete dispatcher;
    delete collisionConfiguration;
}
