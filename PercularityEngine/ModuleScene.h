#ifndef _MODULE_SCENE_H__
#define _MODULE_SCENE_H__

#include "Timer.h"

class GameObject;
class SceneWindow;
class Tree;

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

	void Play();
	void Pause();
	void ExitGame();

	// Save & Load
	void Load(const nlohmann::json &config);
	void Save(nlohmann::json &config);
	void LoadScene(const std::string scene_name, bool tempScene = false);
	void SaveScene(GameObject* root, std::string scene_name, bool savingModel = false, bool tempScene = false);
	void RecursiveLoad(GameObject* root, const json &scene_file);
	void RecursiveSave(GameObject* root, json &scene_file);

	GameObject* LoadModel(const std::string model_name, uint usedAsReference = 0);

	// Returns GameObject with a specific uuid
	GameObject* GetGameObject(GameObject* root, uint uuid) const;
	GameObject* GetRoot() const { return root; }

	void RemoveGameObject(GameObject* gameObject);
	void RecursiveCleanUp(GameObject* root);

	// Methods to create primitives
	GameObject* CreateSphere(int slices, int stacks, float diameter);
	GameObject* CreateCube(float sizeX, float sizeY, float sizeZ);
	GameObject* CreatePlane(float length, float depth);
	GameObject* CreateDonut(int slices, int stacks, float radius);

private:
	// Assigns a new UUID to all the GameObjects without loosing the hierarchy
	void RecursiveReset(GameObject* root);
	void UpdateGameObjects(GameObject* root);

	void DrawAxis() const;			// Draw XYZ axis of coordinates
	void DrawSimplePlane()const;	// Draw a plane with some lines

public:
	bool playMode = false;
	bool mustLoad = false;
	std::string sceneToLoad = "Scene.json";

	// Frustum
	GameObject* frustumTest = nullptr;
	Tree* sceneTree = nullptr;

	const char* sceneAddress = nullptr;
	const char* modelAddress = nullptr;

	GameObject* selected = nullptr;
	std::vector<GameObject*> selectedNodes;
	std::vector<GameObject*> nonStaticObjects;

private:
	GameObject* root = nullptr;
	Timer loadingTime;

	uint saved_go = 0;
	uint loaded_go = 0;
	uint go_counter = 0;

	uint sphereCount = 0;
	uint donutCount = 0;
	uint planeCount = 0;
	uint cubeCount = 0;

	// Primitive's resources UUID
	uint sphereMesh_UUID = 0;
	uint cubeMesh_UUID = 0;
	uint planeMesh_UUID = 0;
	uint donutMesh_UUID = 0;
};

#endif