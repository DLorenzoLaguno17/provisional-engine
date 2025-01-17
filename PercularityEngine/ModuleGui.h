#ifndef __ModuleGui_H__
#define __ModuleGui_H__

#include "Module.h"
#include "ConfigWindow.h"
#include "SceneWindow.h"
#include "ConsoleWindow.h"
#include "InspectorWindow.h"
#include "HierarchyWindow.h"
#include "ProjectWindow.h"
#include "MainMenuBar.h"
#include "ImGui/imgui.h"

class DebugWindow;

// ---------------------------------------------------
class ModuleGui : public Module
{
public:
	ModuleGui(Application* app, bool start_enabled = true);

	// Destructor
	virtual ~ModuleGui();

	// Called when before render is available
	bool Init();

	// Called before all Updates
	update_status PreUpdate(float dt);

	// Called after all Updates
	update_status PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	//Save & Load
	void Load(const nlohmann::json &config);
	void Save(nlohmann::json &config);

	// Draw all the GUI
	void DrawImGui(float dt);

public:
	ImGuiIO* io = nullptr;
	SceneWindow* scene_window = nullptr;
	ConfigWindow* settings = nullptr;

private:

	MainMenuBar* main_menu_bar = nullptr;
	ConsoleWindow* console = nullptr;
	InspectorWindow* inspector = nullptr;
	HierarchyWindow* hierarchy = nullptr;
	ProjectWindow* project = nullptr;
	DebugWindow* debugWindow = nullptr;

	std::vector<UIElement*> ui_elements_list;

	bool p_open = true;
};

#endif // __ModuleGui_H__
