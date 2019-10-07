#include "MainMenuBar.h"
#include "Application.h"

// Show main menu bar
void MainMenuBar::Update(Application* App) {
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Open");
			ImGui::MenuItem("Open recent", "Ctrl+0");
			ImGui::MenuItem("New");
			ImGui::Separator();
			ImGui::MenuItem("Save", " Ctrl+S");
			ImGui::MenuItem("Save as...", " Ctrl+Shift+S");
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ESC"))
				App->renderer3D->status = UPDATE_STOP;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Undo", "Ctrl+Z");
			ImGui::MenuItem("Redo", "Ctrl+Y");

			ImGui::Separator();
			ImGui::MenuItem("Cut", "Ctrl+X");
			ImGui::MenuItem("Copy", "Ctrl+C");
			ImGui::MenuItem("Paste", "Ctrl+V");

			ImGui::Separator();
			ImGui::MenuItem("Duplicate", "Ctrl+D");
			ImGui::MenuItem("Delete", "Supr");

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Demo window", NULL, &App->gui->show_demo_window);
			ImGui::MenuItem("Settings", NULL, &App->gui->show_settings);
			ImGui::MenuItem("Console", NULL, &App->gui->show_console);
			ImGui::MenuItem("Scene", NULL, &App->gui->show_scene);
			ImGui::MenuItem("Elements", NULL, &App->gui->show_elements);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Report a bug"))
				ShellExecuteA(NULL, "open", "https://github.com/DLorenzoLaguno17/PercularityEngine/issues/new/choose", NULL, NULL, SW_SHOWNORMAL);
			if (ImGui::MenuItem("Download the latest version"))
				ShellExecuteA(NULL, "open", "https://github.com/DLorenzoLaguno17/PercularityEngine/releases", NULL, NULL, SW_SHOWNORMAL);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			ImGui::Text("Percularity v0.1");
			ImGui::Text("3D engine developed for student purposes");
			ImGui::Text("By Joan Marin & Daniel Lorenzo");
			if (ImGui::Button("Go to our GitHub"))
				ShellExecuteA(NULL, "open", "https://github.com/DLorenzoLaguno17/PercularityEngine", NULL, NULL, SW_SHOWNORMAL);
			ImGui::NewLine();

			ImGui::Separator();
			ImGui::Text("3rd party libraries used:");
			ImGui::BulletText("SDL 2.0.6");
			ImGui::BulletText("STL 2.0");
			ImGui::BulletText("Dear ImGui 1.72b");
			ImGui::BulletText("MathGeoLib 1.5");
			ImGui::BulletText("Open GL 4.5");
			ImGui::NewLine();

			ImGui::Separator();
			ImGui::Text("MIT License");
			ImGui::NewLine();
			ImGui::Text("Copyright(c) 2019 Joan Marin & Dani Lorenzo");
			ImGui::NewLine();
			ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy");
			ImGui::Text("of this software and associated documentation files (the 'Software'), to deal");
			ImGui::Text("in the Software without restriction, including without limitation the rights");
			ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell");
			ImGui::Text("copies of the Software, and to permit persons to whom the Software is");
			ImGui::Text("furnished to do so, subject to the following conditions:");
			ImGui::NewLine();
			ImGui::Text("The above copyright notice and this permission notice shall be included in all");
			ImGui::Text("copies or substantial portions of the Software.");
			ImGui::NewLine();
			ImGui::Text("THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
			ImGui::Text("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
			ImGui::Text("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE");
			ImGui::Text("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
			ImGui::Text("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
			ImGui::Text("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE");
			ImGui::Text("SOFTWARE.");

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

bool MainMenuBar::CleanUp() {

	return true;
}