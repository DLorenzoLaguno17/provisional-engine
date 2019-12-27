#include "Application.h"
#include "ModulePhysics.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentRigidbody.h"
#include "ComponentTransform.h"
#include "Globals.h"
#include "stdio.h"

#include "Brofiler/Lib/Brofiler.h"
#include "mmgr/mmgr.h"

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
	#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
}

ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init() 
{
	LOG("Initializing physics module");
	return true;
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics environment");
	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	//world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);

	// Big plane as ground
	btCollisionShape* colShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

	btDefaultMotionState* myMotionState = new btDefaultMotionState();
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);
	btRigidBody* body = new btRigidBody(rbInfo);
	world->addRigidBody(body);

	return true;
}

update_status ModulePhysics::PreUpdate(float dt)
{
	BROFILER_CATEGORY("PhysicsPreUpdate", Profiler::Color::Orange);

	world->stepSimulation(dt, 15);

	/*int manifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < manifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* object_A = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* object_B = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if (numContacts > 0)
		{
			PhysBody* physbody_A = (PhysBody*)object_A->getUserPointer();
			PhysBody* physbody_B = (PhysBody*)object_B->getUserPointer();

			if (physbody_A && physbody_B)
			{
				for (int i = 0; i < physbody_A->collision_listeners.size(); ++i)
					physbody_A->collision_listeners[i]->OnCollision(physbody_A, physbody_B);

				for (int i = 0; i < physbody_B->collision_listeners.size(); ++i)
					physbody_B->collision_listeners[i]->OnCollision(physbody_B, physbody_A);
			}
		}
	}*/

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::Update(float dt)
{
	BROFILER_CATEGORY("PhysicsUpdate", Profiler::Color::LightSeaGreen);

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate(float dt)
{
	BROFILER_CATEGORY("PhysicsPostUpdate", Profiler::Color::Yellow);

	return UPDATE_CONTINUE;
}

void ModulePhysics::AddRigidBody() 
{
	// First we create the Component Rigidbody and the collision shape
	float mass = 30.0f;
	ComponentRigidbody* rigidbody = (ComponentRigidbody*)App->scene->selected->CreateComponent(COMPONENT_TYPE::RIGIDBODY);
	btCollisionShape* colShape = new btBoxShape(btVector3(4 * 0.5f, 4 * 0.5f, 4 * 0.5f));
	shapes.push_back(colShape);

	// The we set its transform
	ComponentTransform* transform = (ComponentTransform*)App->scene->selected->GetComponent(COMPONENT_TYPE::TRANSFORM);
	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&transform->renderTransform);

	// We calculate the local inertia
	btVector3 localInertia(0, 0, 0);
	if (mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	// To finish we create the Physbody and add it to the world
	btRigidBody* body = new btRigidBody(rbInfo);
	rigidbody->CreateBody(body);
	world->addRigidBody(body);
}

bool ModulePhysics::CleanUp()
{
	LOG("Destroying Physics module");

	// Remove from the world all collision bodies
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	// Clear all the lists
	for (int i = 0; i < motions.size(); ++i)
		RELEASE(motions[i])

	motions.clear();

	for (int i = 0; i < shapes.size(); ++i)
		RELEASE(shapes[i])

	shapes.clear();

	// Delete the pointers
	RELEASE(vehicle_raycaster);
	RELEASE(world);
	RELEASE(solver);
	RELEASE(broad_phase);
	RELEASE(dispatcher);
	RELEASE(collision_conf);

	return true;
}