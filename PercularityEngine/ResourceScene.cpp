#include "ResourceScene.h"
#include "Application.h"
#include "ModuleScene.h"

bool ResourceScene::LoadInMemory()
{
	App->scene->LoadScene(exported_file);
	return true;
}

void ResourceScene::ReleaseFromMemory() {}

// Save and Load
void ResourceScene::OnSave(const char* resourceNum, json &config) const
{
	config["Resources"]["Scene resources"][resourceNum]["UUID"] = UUID;
	config["Resources"]["Scene resources"][resourceNum]["File"] = file;
	config["Resources"]["Scene resources"][resourceNum]["Exported file"] = exported_file;
	config["Resources"]["Scene resources"][resourceNum]["Name"] = name;
}

void ResourceScene::OnLoad(const char* resourceNum, const json &config)
{
	UUID = config["Resources"]["Scene resources"][resourceNum]["UUID"];
	json s1 = config["Resources"]["Scene resources"][resourceNum]["Name"];
	json s2 = config["Resources"]["Scene resources"][resourceNum]["File"];
	json s3 = config["Resources"]["Scene resources"][resourceNum]["Exported file"];

	name = s1.get<std::string>();
	file = s2.get<std::string>();
	exported_file = s3.get<std::string>();
}