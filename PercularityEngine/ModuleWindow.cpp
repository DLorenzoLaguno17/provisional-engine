#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"

ModuleWindow::ModuleWindow(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	window = NULL;
	screen_surface = NULL;
}

// Destructor
ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init()
{
	//Set the title of the screen
	winTitle.append(App->engineName);
	winTitle.append("  ");
	winTitle.append(App->engineVersion);

	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Create window
		int width = winWidth* SCREEN_SIZE;
		int height = winHeight* SCREEN_SIZE;
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		if(winFullscreen== true)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if(winResizable == true)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if(winBorderless== true)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if(winFullscreenDesktop== true)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		window = SDL_CreateWindow(winTitle.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		LOG("%d %d", width, height);

		if(window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			screen_surface = SDL_GetWindowSurface(window);
		}
	}


	return ret;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

void ModuleWindow::SetFullscreen(bool goingFullscreen) {
	if (goingFullscreen) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	else SDL_SetWindowFullscreen(window, SDL_WINDOW_MINIMIZED);
}

void ModuleWindow::SetResizable(bool goingResizable) {
	if (goingResizable) SDL_SetWindowResizable(window, SDL_TRUE);
	else SDL_SetWindowResizable(window, SDL_FALSE);
}

void ModuleWindow::SetBorderless(bool goingBorderless) {
	if (goingBorderless) SDL_SetWindowBordered(window, SDL_FALSE);
	else SDL_SetWindowBordered(window, SDL_TRUE);
}

void ModuleWindow::SetFulldesktop(bool goingFulldesktop) {
	if (goingFulldesktop) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else SDL_SetWindowFullscreen(window, SDL_WINDOW_MINIMIZED);
}

void ModuleWindow::Load(const nlohmann::json &config) {
	
	winResizable = config["Window"]["Resizable"];
	winBorderless= config["Window"]["Borderless"];
	winFullscreen=config["Window"]["Full Screen"];
	winFullscreenDesktop = config["Window"]["Full Screen Desktop"];
	winWidth = config["Window"]["Width"];
	winHeight = config["Window"]["Height"];

}

void ModuleWindow::Save(nlohmann::json &config) {

	config["Window"]["Resizable"] = winResizable;
	config["Window"]["Borderless"]=winBorderless;
	config["Window"]["Full Screen"]=winFullscreen;
	config["Window"]["Full Screen Desktop"]=winFullscreenDesktop;
	config["Window"]["Width"] = winWidth;
	config["Window"]["Height"]=winHeight;
}