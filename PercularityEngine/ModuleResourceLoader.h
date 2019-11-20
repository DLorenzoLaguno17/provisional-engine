#ifndef __ModuleResourceLoader_H__
#define __ModuleResourceLoader_H__

#include "Module.h"

#define CHECKERS_WIDTH 150
#define CHECKERS_HEIGHT 150

class GameObject;
class ComponentMaterial;

class ResourceMesh;
class ResourceTexture;
struct aiMesh;

// ---------------------------------------------------
class ModuleResourceLoader : public Module
{
public:
	ModuleResourceLoader(Application* app, bool start_enabled = true) : Module(app, start_enabled) {}

	// Destructor
	virtual ~ModuleResourceLoader() {}

	// Called before render is available
	bool Init();

	// Call before first frame
	bool Start();

	// Called before quitting
	bool CleanUp();

	// Save & Load
	void Load(const nlohmann::json &config);
	void Save(nlohmann::json &config);

	// Loading methods
	bool LoadModel(const char* path, std::string& output_file);
	bool LoadTexture(const char* path, std::string& output_file);
	bool LoadMesh(ResourceMesh* mesh, aiMesh* currentMesh, std::string& output_file, const char* name);

	// Importing methods 
	bool ImportTextureToLibrary(const char* path, std::string& output_file);
	bool ImportMeshToLibrary(ResourceMesh* mesh, std::string& output_file, const char* name);
	bool LoadMeshFromLibrary(const char* path, ResourceMesh* mesh);
	
	// Useful methods
	void CreateDefaultTexture();
	void CreateDefaultMaterial();
	std::string getNameFromPath(std::string path, bool withExtension = false);

public:
	ComponentMaterial* icon_tex = nullptr;
	ResourceTexture* default_material = nullptr;
	uint default_tex = 0;

private:
	uint defaultMat_UUID = 0;
};

#endif // __ModuleResourceLoader_H__