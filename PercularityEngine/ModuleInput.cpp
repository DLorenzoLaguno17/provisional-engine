#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleResourceLoader.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "UIElement.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "Brofiler/Lib/Brofiler.h"

#include "mmgr/mmgr.h"

#define MAX_KEYS 300
#define MAX_INPUTS 20

ModuleInput::ModuleInput(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	delete[] keyboard;
}

// Called before render is available
bool ModuleInput::Init()
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called every draw update
update_status ModuleInput::PreUpdate(float dt)
{
	BROFILER_CATEGORY("InputPreUpdate", Profiler::Color::Orange)

	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1)
		{
			input_list.push_back((SDL_Scancode)i);
			if (input_list.size() > MAX_INPUTS) input_list.pop_front();

			if (keyboard[i] == KEY_IDLE) {
				keyboard[i] = KEY_DOWN;

				input_state_list.push_back(KEY_DOWN);
				if (input_state_list.size() > MAX_INPUTS) input_state_list.pop_front();
			}
			else {
				keyboard[i] = KEY_REPEAT;
				input_state_list.push_back(KEY_REPEAT);
				if (input_state_list.size() > MAX_INPUTS) input_state_list.pop_front();
			}			
		}
		else
		{
			input_list.push_back((SDL_Scancode)i);
			if (input_list.size() > MAX_INPUTS) input_list.pop_front();

			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN) {
				keyboard[i] = KEY_UP;

				input_state_list.push_back(KEY_UP);
				if (input_state_list.size() > MAX_INPUTS) input_state_list.pop_front();
			}
			else
				keyboard[i] = KEY_IDLE;
		}		
	}
	
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= SCREEN_SIZE;
	mouse_y /= SCREEN_SIZE;
	mouse_z = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i))
		{
			if(mouse_buttons[i] == KEY_IDLE)
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if(mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	bool quit = false;
	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		switch(e.type)
		{
			case SDL_MOUSEWHEEL:
			mouse_z = e.wheel.y;
			break;

			case SDL_MOUSEMOTION:
			mouse_x = e.motion.x / SCREEN_SIZE;
			mouse_y = e.motion.y / SCREEN_SIZE;

			mouse_x_motion = e.motion.xrel / SCREEN_SIZE;
			mouse_y_motion = e.motion.yrel / SCREEN_SIZE;
			break;

			case SDL_QUIT:
			quit = true;
			break;

			case SDL_WINDOWEVENT:
			if(e.window.event == SDL_WINDOWEVENT_RESIZED)
					App->renderer3D->OnResize(e.window.data1, e.window.data2);
			break;

			case SDL_DROPFILE:
			// We check if its a .png (texture) or an FBX
			if (strstr(e.drop.file, ".png") != nullptr || strstr(e.drop.file, ".dds") != nullptr) {
				if (App->scene->selected) App->scene->selected->c_texture->texture = App->res_loader->CreateTexture(e.drop.file);
			}
			else if (strstr(e.drop.file, ".fbx") != nullptr || strstr(e.drop.file, ".FBX") != nullptr) {
				App->res_loader->LoadFBX(e.drop.file);
				App->scene->selected = App->scene->game_objects.back();
				App->scene->selected_id = App->scene->game_objects.size() - 1;
			}
			// Free dropped_filedir memory
			SDL_free((void*)e.drop.file);   

			break;			
		}

		ImGui_ImplSDL2_ProcessEvent(&e);
	}

	if(quit == true || keyboard[SDL_SCANCODE_ESCAPE] == KEY_UP)
		return UPDATE_STOP;

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

bool ModuleInput::IsMouseInsideWindow(UIElement* window) const {

	return (ImGui::GetMousePos().x > window->windowPosition.x && ImGui::GetMousePos().x < window->windowSize.x + window->windowPosition.x
		&& ImGui::GetMousePos().y > window->windowPosition.y && ImGui::GetMousePos().y < window->windowSize.y + window->windowPosition.y);
}