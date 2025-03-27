#include "PhysicsModule.h"

#include "Application.h"
#include "BulletDebugDraw.h"
#include "SceneModule.h"

#include "Algorithm/Random/LCG.h"
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
}

bool PhysicsModule::Init()
{
    return true;
}

update_status PhysicsModule::PreUpdate(float deltaTime)
{
    if (!App->GetSceneModule()->GetInPlayMode()) return UPDATE_CONTINUE;
    
    dynamicsWorld->stepSimulation(deltaTime, 10);

    // print positions of all objects
    for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body      = btRigidBody::upcast(obj);
        btTransform trans;
        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
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

    return UPDATE_CONTINUE;
}

bool PhysicsModule::ShutDown()
{
    DeleteWorld();

    delete dynamicsWorld;
    delete solver;
    delete broadPhase;
    delete dispatcher;
    delete collisionConfiguration;

    return true;
}

void PhysicsModule::CreateWorld()
{
    DeleteWorld();

    // TODO, REPLACE WITH CURRENT SCENE LOADED DYNAMIC OBJECTS

    // the ground is a cube of side 100 at position y = -56.
    // the sphere will hit it at y = -6, with center at -5
    {
        btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

        collisionShapes.push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -56, 0));

        btScalar mass(0.);

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic) groundShape->calculateLocalInertia(mass, localInertia);

        // using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        // add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);
    }

    LCG randomGen;

    for (int i = 0; i < 1000; i++)
    {
        // create a dynamic rigidbody
        btCollisionShape* colShape = new btBoxShape(btVector3(1, 1, 1));
        // btCollisionShape* colShape = new btSphereShape(btScalar(1.));
        collisionShapes.push_back(colShape);

        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        float x = randomGen.Float(-75, 75);
        float y = randomGen.Float(1, 100);
        float z = randomGen.Float(-75, 75);

        startTransform.setOrigin(btVector3(x, y, z));

        btScalar mass(1.f);

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic) colShape->calculateLocalInertia(mass, localInertia);

        // using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active'
        // objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        dynamicsWorld->addRigidBody(body);
    }
}

void PhysicsModule::DeleteWorld()
{
    // TODO REMOVE, JUST FOR TESTING
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j]      = 0;
        delete shape;
    }
    // -------

    collisionShapes.clear();

    // EMPTY CURRENT WORLD
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
}