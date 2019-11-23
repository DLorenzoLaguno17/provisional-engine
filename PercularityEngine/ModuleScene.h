#ifndef _MODULE_SCENE_H__
#define _MODULE_SCENE_H__

#include "Module.h"
#include "Tree.h"
#include "Timer.h"

class GameObject;
class SceneWindow;

class ModuleScene : public Module
{
	friend class SceneWindow;
public:

	ModuleScene(Application* app, bool start_enabled = true);
	~ModuleScene() {}

	bool Init();
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	//Save & Load
	void Load(const nlohmann::json &config);
	void Save(nlohmann::json &config);
	void LoadModel(const std::string model_name);
	void LoadScene(GameObject* root, const std::string scene_name);
	void SaveScene(GameObject* root, std::string scene_name);
	void RecursiveLoad(GameObject* root, const json &scene_file);
	void RecursiveSave(GameObject* root, json &scene_file);

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
	bool mustLoad = false;
	//FRUSTUM TESTER

	GameObject* frustumTest;

	//FRUSTUM TEST

	Tree* sceneTree;

	GameObject* selected = nullptr;
	const char* sceneAddress;
	uint numGameObjectsInScene = 0;

private:
	GameObject* root = nullptr;

	Timer loadingTime;
	uint saved_go = 0;
	uint loaded_go = 0;

	uint sphereCount = 0;
	uint donutCount = 0;
	uint planeCount = 0;
	uint cubeCount = 0;

	// Primitive's resources UUID
	uint sphereMesh_UUID = 0;
	uint cubeMesh_UUID = 0;
	uint planeMesh_UUID = 0;
	uint donutMesh_UUID = 0;

	uint go_counter = 0;
};
#endif