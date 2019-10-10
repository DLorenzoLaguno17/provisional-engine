#ifndef __ModuleMeshLoader_H__
#define __ModuleMeshLoader_H__

#include "Module.h"
#include <vector>

struct MeshData {
	uint id_index = 0; // index in VRAM
	uint num_indices = 0;
	uint* indices = nullptr;

	uint id_vertex = 0; // Unique vertex in VRAM
	uint num_vertices = 0;
	float* vertices = nullptr;
};

// ---------------------------------------------------
class ModuleMeshLoader : public Module
{
public:

	ModuleMeshLoader(Application* app, bool start_enabled = true);

	// Destructor
	virtual ~ModuleMeshLoader();

	// Called when before render is available
	bool Awake();

	// Call before first frame
	bool Start();

	// Called before all Updates
	update_status PreUpdate(float dt);

	// Called every frame
	update_status Update(float dt);

	// Called after all Updates
	update_status PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	//Save & Load
	void Load(const nlohmann::json  &config);
	void Save(nlohmann::json &config);

	//Method to load a file
	void LoadFBX(const char* path, std::vector<MeshData*> meshList);
	void RenderFBX(std::vector<MeshData*> FBX);

public:
	std::vector<MeshData*> test_list;
};

#endif // __ModuleMeshLoader_H__