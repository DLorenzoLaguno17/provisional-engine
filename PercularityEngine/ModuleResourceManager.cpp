#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleResourceLoader.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
#include "ResourceModel.h"
#include "ResourceTexture.h"
#include "GameObject.h"
#include "OpenGL.h"

#include "Brofiler/Lib/Brofiler.h"
#include "ImGui/imgui.h"
#include "mmgr/mmgr.h"

void ImportFileTask::Execute()
{
	App->res_manager->ImportFile(filePath, type, force);
}

void ModuleResourceManager::OnTaskFinished(Task* task)
{
	RELEASE(task);
}

// Called before the first frame
bool ModuleResourceManager::Start()
{
	// Track files from the assets folder
	std::vector<std::string> tex_files;
	std::vector<std::string> tex_directories;

	App->file_system->DiscoverFiles(ASSETS_TEXTURE_FOLDER, tex_files, tex_directories);

	// We create resources from every asset
	for (int i = 0; i < tex_files.size(); ++i)
	{	
		std::string file = ASSETS_TEXTURE_FOLDER + tex_files[i];
		ImportFile(file.c_str(), RESOURCE_TYPE::TEXTURE);
	}

	currentFolder = ASSETS_FOLDER;
	return true;
}

void ModuleResourceManager::ReceiveExternalFile(const char* new_file)
{
	// We create the correct path
	std::string final_path;
	std::string extension;

	// If we don't have it already, we add the file to our Assets folder
	App->file_system->SplitFilePath(new_file, nullptr, &final_path, &extension);

	if (HasTextureExtension(extension.c_str()))
		final_path = ASSETS_TEXTURE_FOLDER + final_path;
	else if (HasModelExtension(extension.c_str()))
		final_path = ASSETS_MODEL_FOLDER + final_path;
	else if (strcmp(extension.c_str(), "json") == 0)
		final_path = ASSETS_SCENE_FOLDER + final_path;

	if (App->file_system->CopyFromOutsideFS(new_file, final_path.c_str())) 
	{
		RESOURCE_TYPE type = GetTypeFromExtension(extension.c_str());
		ImportFile(final_path.c_str(), type, true);
		//ImportFileTask* task = new ImportFileTask(final_path.c_str(), type, true);
		//App->task_manager->ScheduleTask(task, this);
	}
}

uint ModuleResourceManager::ImportFile(const char* new_file, RESOURCE_TYPE type, bool force)
{
	uint ret = 0;

	// Check is the file has already been exported
	if (force)
	{
		ret = FindFileInAssets(new_file);
		if (ret != 0) return ret;
	}

	bool success = false;
	std::string written_file;

	switch (type) 
	{
	case RESOURCE_TYPE::TEXTURE:
		success = App->res_loader->LoadTexture(new_file, written_file);
		break;

	case RESOURCE_TYPE::MODEL:
		success = App->res_loader->LoadModel(new_file, written_file);
		break;
	}

	// If the import was successful, we create a new resource
	if (success) 
	{
		Resource* res = CreateNewResource(type);
		res->file = new_file;
		App->file_system->NormalizePath(res->file);
		res->exported_file = written_file;
		res->name = App->res_loader->getNameFromPath(res->file);
		ret = res->GetUUID();		
	}

	return ret;
}

Resource* ModuleResourceManager::CreateNewResource(RESOURCE_TYPE type, uint specific_uuid)
{
	Resource* ret = nullptr;
	uint uuid;

	if (specific_uuid != 0 && GetResourceFromMap(specific_uuid) == nullptr) uuid = specific_uuid;
	else uuid = (uint)App->GetRandomGenerator().Int();

	switch (type)
	{
	case RESOURCE_TYPE::TEXTURE:
		ret = (Resource*) new ResourceTexture(uuid);
		break;

	case RESOURCE_TYPE::MODEL:
		ret = (Resource*) new ResourceModel(uuid);
		break;

	case RESOURCE_TYPE::MESH:
		ret = (Resource*) new ResourceMesh(uuid);
		break;
	}

	if (ret != nullptr)
		resources[uuid] = ret;

	return ret;
}

void ModuleResourceManager::DrawProjectExplorer()
{
	// Data from the assets
	std::vector<std::string> tex_files, tex_directories;
	std::vector<std::string> mod_files, mod_directories;
	std::vector<std::string> sce_files, sce_directories;

	App->file_system->DiscoverFiles(ASSETS_TEXTURE_FOLDER, tex_files, tex_directories);
	App->file_system->DiscoverFiles(ASSETS_MODEL_FOLDER, mod_files, mod_directories);
	App->file_system->DiscoverFiles(ASSETS_SCENE_FOLDER, sce_files, sce_directories);

	std::vector<std::string> files, directories;
	App->file_system->DiscoverFiles(currentFolder.c_str(), files, directories);

	static int columns = 5;
	//ImGui::SliderInt("Columns", &columns, 5, 10);
	ImGui::Columns(columns, "project_columns", false);

	ImVec4 baseColor = ImVec4(255, 255, 255, 255);
	ImVec4 selectedColor = ImVec4(0, 125, 255, 255);

	/*// Show folders
	for (int i = 0; i < directories.size(); ++i)
	{
		ImGui::PushID(i);

		if (selected == directories[i])
			ImGui::Image((void*)App->res_loader->folder_tex->texture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0), selectedColor);
		else
			ImGui::Image((void*)App->res_loader->folder_tex->texture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0), baseColor);

		if (ImGui::IsItemClicked())
			selected = directories[i];

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			currentFolder += directories[i];
			selected = "";
		}

		ImGui::Text(directories[i].c_str());
		ImGui::NextColumn();

		ImGui::PopID();
	}

	// Show files
	for (int i = 0; i < files.size(); ++i)
	{
		ImGui::PushID(i);

		if (selected == files[i])
			ImGui::Image((void*)App->res_loader->folder_tex->texture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0), selectedColor);
		else
			ImGui::Image((void*)App->res_loader->folder_tex->texture, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0), baseColor);

		if (ImGui::IsItemClicked())
			selected = files[i];

		//if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			//ShellExecuteA(NULL, "open", "https://github.com/DLorenzoLaguno17/PercularityEngine", NULL, NULL, SW_SHOWNORMAL);

		ImGui::Text(files[i].c_str());
		ImGui::NextColumn();

		ImGui::PopID();
	}*/

	// Show scenes in the folder
	for (int i = 0; i < sce_files.size(); ++i)
	{
		ImGui::PushID(i);

		// Clicking a scene resource loads it
		if (ImGui::ImageButton((void*)App->res_loader->scene_icon_tex->texture, ImVec2(50, 50)))
		{
			App->scene->mustLoad = true;
			App->scene->sceneToLoad = sce_files[i];
		}

		ImGui::Text(sce_files[i].c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	// Show models in the folder
	for (int i = 0; i < mod_files.size(); ++i)
	{
		ImGui::PushID(i);
		ImGui::ImageButton((void*)App->res_loader->model_icon_tex->texture, ImVec2(50, 50));

		// Dragging a model to the screen window generates an instance
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			draggedModel = mod_files[i];
			ImGui::SetDragDropPayload("ModelUUID", &draggedModel, sizeof(std::string));
			ImGui::Image((void*)App->res_loader->model_icon_tex->texture, ImVec2(50, 50));
			ImGui::EndDragDropSource();
		}

		ImGui::Text(mod_files[i].c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	// Show textures in the folder
	for (int i = 0; i < tex_files.size(); ++i)
	{
		ImGui::PushID(i);

		// Show each texture as a thumbnail
		std::string name = App->res_loader->getNameFromPath(tex_files[i]);
		ResourceTexture* image = (ResourceTexture*)GetResourceByName(name.c_str());
		image->IncreaseReferenceCount();
		ImGui::ImageButton((void*)image->texture, ImVec2(50, 50));

		// Prepare drag and drop to carry texture's name
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("TextureUUID", &image->name, sizeof(std::string));
			ImGui::Image((void*)image->texture, ImVec2(50, 50));
			ImGui::EndDragDropSource();
		}

		ImGui::Text(name.c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	ImGui::Columns(1);
}

// Load & Save
void ModuleResourceManager::LoadResources(const json &scene_file) 
{
	//if (!loadingModel) CleanUp();

	uint cnt = scene_file["Resources"]["Count"];
	for (int i = 1; i <= cnt; ++i) 
	{
		char name[50];
		sprintf_s(name, 50, "Resource %d", i);

		uint UUID = scene_file["Resources"][name]["UUID"];
		Resource* newResource = GetResourceFromMap(UUID);

		if (newResource) 
			newResource->IncreaseReferenceCount();
		else
		{
			json js = scene_file["Resources"][name]["Type"];
			std::string type = js.get<std::string>();
			
			if (strcmp(type.c_str(), "Mesh") == 0) 
			{
				ResourceMesh* res = (ResourceMesh*)CreateNewResource(RESOURCE_TYPE::MESH, UUID);
				res->OnLoad(name, scene_file);
				res->IncreaseReferenceCount();
			}
			if (strcmp(type.c_str(), "Texture") == 0) 
			{
				ResourceTexture* res = (ResourceTexture*)CreateNewResource(RESOURCE_TYPE::TEXTURE, UUID);
				res->OnLoad(name, scene_file);
				res->IncreaseReferenceCount();
			}
		}
	}
}

void ModuleResourceManager::SaveResources(json &scene_file) 
{
	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->file.compare("None")) 
		{
			saved_res++;
			char name[50];
			sprintf_s(name, 50, "Resource %d", saved_res);
			it->second->OnSave(name, scene_file);
		}

		scene_file["Resources"]["Count"] = saved_res;
	}

	saved_res = 0;
}

// Checks if a file already exists in Assets folder, and returns the UUID of the resource if it does
uint ModuleResourceManager::FindFileInAssets(const char* existing_file) const 
{
	std::string file = existing_file;
	App->file_system->NormalizePath(file);

	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->file.compare(file) == 0)
			return it->first;
	}

	return 0;
}

uint ModuleResourceManager::FindFileInLibrary(const char* exported_file) const 
{
	std::string file = exported_file;
	App->file_system->NormalizePath(file);

	for (std::map<uint, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->file.compare(file) == 0)
			return it->first;
	}

	return 0;
}

const Resource* ModuleResourceManager::GetResourceFromMap(uint uuid) const
{
	if (resources.find(uuid) != resources.end())
		return resources.at(uuid);

	return nullptr;
}

// Returns a resource with certain uuid from the map
Resource* ModuleResourceManager::GetResourceFromMap(uint uuid)
{
	std::map<uint, Resource*>::iterator it = resources.find(uuid);
	if (it != resources.end())
		return it->second;

	return nullptr;
}

// Returns a resource with certain name from the map
Resource* ModuleResourceManager::GetResourceByName(const char* name) 
{
	for (std::map<uint, Resource*>::iterator item = resources.begin(); item != resources.end(); ++item) 
	{
		if (item->second->name.compare(name) == 0)
			return item->second;
	}

	return nullptr;
}

// Called before quitting
bool ModuleResourceManager::CleanUp()
{
	LOG("Freeing resource manager");
	for (std::map<uint, Resource*>::iterator item = resources.begin(); item != resources.end(); ++item) {
		
		if (strcmp(item._Ptr->_Myval.second->file.c_str(), "None") != 0) 
		{
			item._Ptr->_Myval.second->ReleaseFromMemory();
			RELEASE(item._Ptr->_Myval.second);
		}
	}

	resources.clear();

	return true;
}

// Methods to check the extension of a file
RESOURCE_TYPE ModuleResourceManager::GetTypeFromExtension(const char* extension) 
{	
	if (HasModelExtension(extension)) return RESOURCE_TYPE::MODEL;
	else if (HasTextureExtension(extension)) return RESOURCE_TYPE::TEXTURE;
	else return RESOURCE_TYPE::UNKNOWN;
}

bool ModuleResourceManager::HasTextureExtension(const char* extension) 
{
	return (strcmp(extension, "dds") == 0 || strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0);
}

bool ModuleResourceManager::HasModelExtension(const char* extension) 
{
	return (strcmp(extension, "fbx") == 0 || strcmp(extension, "FBX") == 0 || strcmp(extension, "obj") == 0);
}