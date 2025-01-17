#ifndef __Component_H__
#define __Component_H__

#include "Globals.h"

enum class COMPONENT_TYPE 
{
	NONE,
	TRANSFORM,
	MATERIAL,
	MESH,
	CAMERA,
	RIGIDBODY
};

class GameObject;

class Component 
{
public:
	// Constructors
	Component() {}
	Component(COMPONENT_TYPE type, GameObject* parent, bool active);

	// Destructor
	virtual ~Component() {}

	// Methods
	virtual void Update() {}
	virtual void OnEditor() {}
	virtual void CleanUp() {}

	virtual void OnUpdateTransform(){}

	// Load & Save 
	virtual void OnLoad(const char* gameObjectNum, const nlohmann::json &scene_file) {}
	virtual void OnSave(const char* gameObjectNum, nlohmann::json &scene_file) {}

	virtual void Pause() {}
	virtual void Stop() {}
	virtual void Update(float dt) {}

	uint GetUUID() const { return UUID; }
	void SetNewParentUUID(uint new_UUID) { parent_UUID = new_UUID; }
	bool IsActive() { return active; }

public:
	COMPONENT_TYPE type;
	GameObject* gameObject = nullptr;

protected:
	uint UUID = 0;
	uint parent_UUID = 0;
	bool active;
};

#endif // __Component_H__