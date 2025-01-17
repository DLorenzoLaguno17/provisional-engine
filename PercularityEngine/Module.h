#ifndef _MODULE_H__
#define _MODULE_H__

#include "Globals.h"

class Application;
class ComponentRigidBody;
class Task;

class Module
{
public:
	Application* App;

	Module(Application* parent, bool start_enabled = true) : App(parent) {}

	virtual ~Module() {}

	virtual bool Init() { return true; }
	virtual bool Start() { return true;	}
	virtual bool CleanUp() { return true; }

	virtual update_status PreUpdate(float dt) {	return UPDATE_CONTINUE;	}
	virtual update_status Update(float dt) { return UPDATE_CONTINUE; }
	virtual update_status PostUpdate(float dt) { return UPDATE_CONTINUE; }

	// Physics
	virtual void OnCollision(ComponentRigidBody* body1, ComponentRigidBody* body2)	{}

	// Save & Load
	virtual void Load(const nlohmann::json &config)	{}
	virtual void Save(nlohmann::json &config) {}

	// Multi-threading
	virtual void OnTaskFinished(Task* task) {}

private:
	bool enabled = false;
};

#endif