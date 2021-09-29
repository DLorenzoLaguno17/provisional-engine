#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleResourceLoader.h"
#include "ModuleResourceManager.h"
#include "ResourceTexture.h"
#include "GameObject.h"
#include "ImGui/imgui.h"

ComponentMaterial::ComponentMaterial(GameObject* parent, bool active, ComponentMaterial* reference) :
	Component(COMPONENT_TYPE::MATERIAL, parent, active) 
{
	if (reference)
	{
		UUID = reference->UUID;
		parent_UUID = reference->parent_UUID;
		active = reference->active;
		resource_tex = (ResourceTexture*)App->res_manager->GetResourceFromMap(reference->resource_tex->GetUUID());
	}
}

void ComponentMaterial::OnEditor() 
{
	if (ImGui::CollapsingHeader("Material")) 
	{
		ImGui::Checkbox(resource_tex->name.c_str(), &active);
		ImGui::Text("Size: %dx%d", resource_tex->height, resource_tex->width);
		ImGui::NewLine();

		ImGui::Image((void*)resource_tex->texture, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureUUID"))
			{
				std::string test = *(std::string*)payload->Data;
				ResourceTexture* droppedTexture = (ResourceTexture*)App->res_manager->GetResourceByName(test.c_str());
				
				// Update texture resource references
				if (droppedTexture)
				{
					droppedTexture->IncreaseReferenceCount();
					resource_tex->DecreaseReferenceCount();
					resource_tex = droppedTexture;
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::NewLine();
	}
}

void ComponentMaterial::CleanUp() 
{
	//resource_tex->DecreaseReferenceCount();
}

// Load & Save 
void ComponentMaterial::OnLoad(const char* gameObjectNum, const nlohmann::json &scene_file) 
{	
	UUID = scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["UUID"];
	parent_UUID = scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Parent UUID"];
	active = scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Active"];

	uint uuid = scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Resource UUID"];
	resource_tex = (ResourceTexture*)App->res_manager->GetResourceFromMap(uuid);
}

void ComponentMaterial::OnSave(const char* gameObjectNum, nlohmann::json &scene_file) {
	scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["UUID"] = UUID;
	scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Parent UUID"] = parent_UUID;
	scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Active"] = active;
	scene_file["Game Objects"][gameObjectNum]["Components"]["Material"]["Resource UUID"] = resource_tex->GetUUID();
}