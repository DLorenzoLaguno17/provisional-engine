#ifndef __ModuleUndo_H__
#define __ModuleUndo_H__

#include "Module.h"

class Action 
{
public:
	virtual void Undo() {};
	virtual void Redo() {};
};

// ---------------------------------------------------
class ModuleUndo : public Module
{
public:
	ModuleUndo(Application* app, bool start_enabled = true) : Module(app, start_enabled) {}

	// Destructor
	virtual ~ModuleUndo() {}

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void Undo();
	void Redo();

	void StoreNewAction(Action* newAction);

private:
	std::vector<Action*> actions;
	uint cursor = 0;
};

#endif // __ModuleUndo_H__