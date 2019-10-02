#pragma once

#include "Globals.h"
#include "Timer.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleGui.h"

#include "Json Parser/nlohmann/json.hpp"

#include <list>

using json = nlohmann::json;

class Application
{
public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;
	ModuleGui* gui;

private:

	Timer	ms_timer;
	float	dt;
	std::list<Module*> modules;

	//Save & load
	const char* settingsAdress;
	json		settingsFile;
	
	//Aditional data
	std::string engineName;
	std::string engineVersion;

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	//Save & load
	void LoadSettings();
	void SaveSettings();
};