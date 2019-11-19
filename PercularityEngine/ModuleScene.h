#ifndef _MODULE_SCENE_H__
#define _MODULE_SCENE_H__

#include "Module.h"
#include "Tree.h"


class GameObject;

class ModuleScene : public Module
{
public:

	ModuleScene(Application* app, bool start_enabled = true);
	~ModuleScene();

	bool Init();
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	//Save & Load
	void Load(const nlohmann::json &config) {}
	void Save(nlohmann::json &config) {}
	void LoadScene(const std::string scene_name);
	void SaveScene(std::string scene_name);
	void RecursiveLoad(const char* scene_name, GameObject* root, const nlohmann::json &scene_file);
	void RecursiveSave(const char* scene_name, GameObject* root, nlohmann::json &scene_file);

	//Methods to create primitives
	GameObject* CreateSphere(int slices, int stacks, float diameter);
	GameObject* CreateCube(float sizeX, float sizeY, float sizeZ);
	GameObject* CreatePlane(float length, float depth);
	GameObject* CreateDonut(int slices, int stacks, float radius);

	GameObject* GetRoot() const { return root; }

private:
	void DrawAxis() const;			//Draw XYZ axis of coordinates
	void DrawSimplePlane()const;	//Draw a plane with some lines

	// Recursive methods
	void RecursiveCleanUp(GameObject* root);
	void UpdateGameObjects(GameObject* root);

public:
	GameObject* selected = nullptr;

private:
	GameObject* root = nullptr;
	const char* sceneAddress;

	uint sphereCount = 0;
	uint donutCount = 0;
	uint planeCount = 0;
	uint cubeCount = 0;
};
#endif
