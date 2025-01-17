#include "Application.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceLoader.h"
#include "ModuleResourceManager.h"
#include "ModulePhysics.h"
#include "ModuleFileSystem.h"
#include "ModuleGui.h"
#include "ModuleInput.h"
#include "ModuleCamera3D.h"
#include "ModulePlayer.h"
#include "ModulePhysics.h"
#include "ModuleTaskManager.h"
#include "SceneActions.h"

#include "Time.h"
#include "Tree.h"
#include "OpenGL.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ComponentCamera.h"
#include "Debug.h"

#include <fstream>
#include <iomanip>

#define PAR_SHAPES_IMPLEMENTATION

#include "Par Shapes/par_shapes.h"
#include "Brofiler/Lib/Brofiler.h"
#include "mmgr/mmgr.h"

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	loadingTime.Start();
}

bool ModuleScene::Init() 
{
	root = new GameObject("World");
	selected = root;

	sceneAddress = "Assets/Scenes/"; 
	modelAddress = "Library/Models/";

	//frustumTest = new GameObject("Camera", root);
	//frustumTest->CreateComponent(COMPONENT_TYPE::CAMERA);

	sceneTree = new Tree(TREE_TYPE::OCTREE, AABB({ -80,-80,-80 }, { 80,80,80 }), 5);

	return true;
}

bool ModuleScene::Start()
{
	return true;
}

update_status ModuleScene::PreUpdate(float dt)
{
	BROFILER_CATEGORY("ScenePreUpdate", Profiler::Color::Orange);

	return UPDATE_CONTINUE;
}

update_status ModuleScene::Update(float dt)
{
	BROFILER_CATEGORY("SceneUpdate", Profiler::Color::LightSeaGreen);

	if (Debug::drawScenePlane)
		DrawSimplePlane();

	if (Debug::drawSceneAxis)
		DrawAxis();

	//if (sphereCount == 0) App->res_manager->GetResourceFromMap(sphereMesh_UUID)->ReleaseFromMemory();	

	// If the user wants to save the scene
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN
		&& ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
		|| (App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)))
		SaveScene(root, "Scene.json");

	// If the user wants to load another scene
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN
		&& ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
		|| (App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)))
		mustLoad = true;
		
	if (mustLoad) ImGui::OpenPopup("Loading new scene");	

	if (ImGui::BeginPopupModal("Loading new scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to load a new scene?");
		ImGui::Text("The current scene will be discarded.");
		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::Button("Yes", ImVec2(140, 0))) 
		{ 
			ImGui::CloseCurrentPopup(); 
			LoadScene(sceneToLoad); 
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(140, 0))) { ImGui::CloseCurrentPopup(); }

		mustLoad = false;
		ImGui::EndPopup();
	}

	// Update all GameObjects
	UpdateGameObjects(root);

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		for (int i = 0; i < root->children.size(); ++i)
			sceneTree->rootNode->Insert(root->children[i]);

	// If delete is pressed, destroy selected GameObjects
	if (App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN && selected != root)
	{
		GameObject* deletedObject = new GameObject(selected);
		DeleteGameObject* deleteAction = new DeleteGameObject(deletedObject, selected->parent);
		App->undo->StoreNewAction(deleteAction);
		RemoveGameObject(selected);
	}

	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate(float dt)
{
	BROFILER_CATEGORY("ScenePostUpdate", Profiler::Color::Yellow);

	if (Debug::drawSceneTree && sceneTree)
		sceneTree->Draw();

	//frustumTest->GetComponent<ComponentCamera>()->DrawFrustum();

	return UPDATE_CONTINUE;
}

void ModuleScene::UpdateGameObjects(GameObject* root)
{
	root->Update();

	for (uint i = 0; i < root->children.size(); ++i)
		UpdateGameObjects(root->children[i]);
}

GameObject* ModuleScene::GetGameObject(GameObject* root, uint uuid) const
{
	if (root->GetUUID() == uuid)
		return root;

	for (uint i = 0; i < root->children.size(); ++i)
	{
		GameObject* objective = nullptr;
		objective = GetGameObject(root->children[i], uuid);
		if (objective) return objective;
	}

	return nullptr;
}

// Deletes all the allocated data
bool ModuleScene::CleanUp()
{
	LOG("Releasing all the GameObjects");
	sceneTree->Clear();
	RELEASE(sceneTree);
	selectedNodes.clear();
	RecursiveCleanUp(root);
	root->children.clear();

	if (App->closingEngine) 
	{
		for (uint i = 0; i < root->components.size(); ++i) 
		{
			root->components[i]->CleanUp();
			RELEASE(root->components[i]);
		}

		root->components.clear();
		RELEASE(root);
		std::string name = "TemporalScene.json";
		std::string path = ASSETS_SCENE_FOLDER + name;
		App->file_system->Remove(path.c_str());
	}

	return true;
}

void ModuleScene::RecursiveCleanUp(GameObject* root) 
{
	for (int i = 0; i < root->children.size(); ++i)
		RecursiveCleanUp(root->children[i]);

	if (root != GetRoot()) 
	{
		root->CleanUp();
		RELEASE(root);
	}
}

void ModuleScene::RemoveGameObject(GameObject* gameObject)
{
	GameObject* parent = gameObject->parent;
	parent->children.erase(std::find(parent->children.begin(), parent->children.end(), gameObject));
	RecursiveCleanUp(gameObject);
	selected = parent;
}

// -----------------------------------------------------------------------------------------------
// GAME SCENE MANAGEMENT
// -----------------------------------------------------------------------------------------------

void ModuleScene::Play()
{
	if (!Time::running) 
	{
		if (Time::paused)
			Time::Resume();
		else
		{
			Time::Start();

			// Saves the current scene
			SaveScene(root, "TemporalScene.json", false, true);
			playMode = true;
		}
	}
}

void ModuleScene::Pause()
{
	Time::Pause();
}

void ModuleScene::ExitGame()
{
	if (Time::running) 
	{
		Time::Stop();
		CleanUp();
		sceneTree = new Tree(TREE_TYPE::OCTREE, AABB({ -80,-80,-80 }, { 80,80,80 }), 5);
		playMode = false;
		App->player->ResetCar();
		App->physics->ClearBalls();

		// Loads the former scene and then deletes the file
		std::string name = "TemporalScene.json";
		LoadScene(name, true);
		std::string path = ASSETS_SCENE_FOLDER + name;
		App->file_system->Remove(path.c_str());
	}
}

// -----------------------------------------------------------------------------------------------
// SAVE & LOAD METHODS
// -----------------------------------------------------------------------------------------------

void ModuleScene::LoadScene(const std::string scene_name, bool tempScene) 
{
	LOG("\nLoading scene: %s", scene_name.c_str());
	uint startTime = loadingTime.Read();

	json scene_file;
	std::string full_path;

	// First we delete the current scene
	App->undo->CleanUp();
	CleanUp();
	sceneTree = new Tree(TREE_TYPE::OCTREE, AABB({ -80, -80, -80 }, { 80, 80, 80 }), 5);
	full_path = sceneAddress + scene_name;

	// If the adress of the settings file is null, create  an exception
	assert(full_path.c_str() != nullptr);

	// Create a stream and open the file
	std::ifstream stream;
	stream.open(full_path);
	scene_file = json::parse(stream);

	// Close the file
	stream.close();

	// We load the resources
	if (!tempScene) App->res_manager->LoadResources(scene_file);

	// Then we load all the GameObjects
	go_counter = scene_file["Game Objects"]["Count"];
	RecursiveLoad(root, scene_file);	

	selected = root;
	loaded_go = 0;

	uint fullTime = loadingTime.Read() - startTime;
	LOG("Scene loaded. Time spent: %d ms", fullTime);
}

GameObject* ModuleScene::LoadModel(const std::string model_name, uint usedAsReference)
{
	LOG("\nLoading model: %s", model_name.c_str());
	uint startTime = loadingTime.Read();

	json scene_file;
	std::string full_path;
	full_path = modelAddress + model_name + ".json";

	// If the adress of the settings file is null, create  an exception
	assert(full_path.c_str() != nullptr);

	// Create a stream and open the file
	std::ifstream stream;
	stream.open(full_path);
	scene_file = json::parse(stream);

	// Close the file
	stream.close();

	// We load the resources
	App->res_manager->LoadResources(scene_file);

	// Then we load all the GameObjects
	go_counter = scene_file["Game Objects"]["Count"];

	GameObject* model = new GameObject("Model", nullptr, true);
	RecursiveLoad(model, scene_file);
	model->MakeChild(root);

	if (usedAsReference > 1)
	{
		char num[10];
		sprintf_s(num, 10, " (%d)", usedAsReference - 1);
		std::string full_name = model->name + num;
		model->name = full_name;

		// We reset the UUIDS of all the GameObjects of the model in case it is duplicated
		RecursiveReset(model);
	}

	selected = model;
	selectedNodes.clear();
	selectedNodes.push_back(selected);
	loaded_go = 0;

	uint fullTime = loadingTime.Read() - startTime;
	LOG("Scene loaded. Time spent: %d ms", fullTime);

	return model;
}

void ModuleScene::RecursiveLoad(GameObject* root, const nlohmann::json &scene_file) 
{	
	// We load each GameObject excepte the world
	if (root != GetRoot()) 
	{
		loaded_go++;
		char name[50];
		sprintf_s(name, 50, "GameObject %d", loaded_go);
		root->OnLoad(name, scene_file);		
	}

	// Check for children of the current GameObject
	for (int i = 1; i <= go_counter; ++i) 
	{
		char n[50];
		sprintf_s(n, 50, "GameObject %d", i);
		uint aux = scene_file["Game Objects"][n]["Parent UUID"];

		if (aux == root->GetUUID())
			GameObject* newGo = new GameObject("Temp", root, true);		
	}

	for (int i = 0; i < root->children.size(); ++i)
		RecursiveLoad(root->children[i], scene_file);
}

void ModuleScene::RecursiveReset(GameObject* root) 
{
	if (root != GetRoot())
	{
		root->NewUUID();
		root->parent_UUID = root->parent->GetUUID();

		for (int i = 0; i < root->components.size(); ++i)
			root->components[i]->SetNewParentUUID(root->GetUUID());
	}

	for (int i = 0; i < root->children.size(); ++i)
		RecursiveReset(root->children[i]);
}

void ModuleScene::SaveScene(GameObject* root, std::string scene_name, bool savingModel, bool tempScene) 
{
	// Create auxiliar file
	json scene_file;
	std::string full_path;

	if (savingModel)
	{
		LOG("\nImporting model...");
		full_path = modelAddress + scene_name + ".json";
	}
	else
	{
		LOG("\nSaving scene..."); 
		full_path = sceneAddress + scene_name;
	}			

	// First we save the resources
	if (!tempScene) App->res_manager->SaveResources(scene_file);

	RecursiveSave(root, scene_file);
	scene_file["Game Objects"]["Count"] = saved_go;
	saved_go = 0;

	// Create the stream and open the file
	std::ofstream stream;
	stream.open(full_path);
	stream << std::setw(4) << scene_file << std::endl;
	stream.close();
	
	if (savingModel)	LOG("Model imported.")
	else				LOG("Scene saved.");
}

void ModuleScene::RecursiveSave(GameObject* root, nlohmann::json &scene_file) 
{
	if (root != GetRoot()) 
	{
		saved_go++;
		char name[50];
		sprintf_s(name, 50, "GameObject %d", saved_go);
		root->OnSave(name, scene_file);
	}

	for (int i = 0; i < root->children.size(); ++i)
		RecursiveSave(root->children[i], scene_file);
}

void ModuleScene::Load(const nlohmann::json &config)
{
	sphereMesh_UUID = config["Scene"]["Sphere resource UUID"];
	cubeMesh_UUID = config["Scene"]["Cube resource UUID"];
	planeMesh_UUID = config["Scene"]["Plane resource UUID"];
	donutMesh_UUID = config["Scene"]["Donut resource UUID"];
}

void ModuleScene::Save(nlohmann::json &config)
{
	config["Scene"]["Sphere resource UUID"] = sphereMesh_UUID;
	config["Scene"]["Cube resource UUID"] = cubeMesh_UUID;
	config["Scene"]["Plane resource UUID"] = planeMesh_UUID;
	config["Scene"]["Donut resource UUID"] = donutMesh_UUID;
}

// -----------------------------------------------------------------------------------------------
// GEOMETRY DRAWING METHODS
// -----------------------------------------------------------------------------------------------

void ModuleScene::DrawSimplePlane() const
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(255, 255, 255);

	// Try to draw a plane
	for (float i = -100.0f; i < 100; ++i)
	{
		glVertex3f(i, 0.0f, -100.0f);
		glVertex3f(i, 0.0f, 100.0f);
	}

	for (float i = -100.0f; i < 100; ++i)
	{
		glVertex3f(-100, 0.0f, i);
		glVertex3f(100.f, 0.0f, i);
	}

	glEnd();
}

void ModuleScene::DrawAxis() const 
{
	glLineWidth(7.0f);
	glBegin(GL_LINES);

	// X axis
	glColor3f(1, 0, 0); //Red color

	glVertex3f(-0.1f, 0.1f, -0.1f);
	glVertex3f(5.1f, 0.1f, -0.1f);

	/// Draw an X
	glVertex3f(5.0f, 2.0f, -0.2f);
	glVertex3f(4.5f, 1.5f, -0.2f);
	glVertex3f(4.5f, 2.0f, -0.2f);
	glVertex3f(5.0f, 1.5f, -0.2f);

	// Y axis
	glColor3f(0, 1, 0); //Green color

	glVertex3f(-0.1f, 0.1f, -0.1f);
	glVertex3f(-0.1f, 5.1f, -0.1f);

	/// Draw an Y
	glVertex3f(0.0f, 5.5f, 0.0f);
	glVertex3f(0.0f, 6.0f, 0.0f);
	glVertex3f(0.0f, 6.0f, 0.0f);
	glVertex3f(-0.25f, 6.5f, 0.0f);
	glVertex3f(0.0f, 6.0f, 0.0f);
	glVertex3f(0.15f, 6.5f, 0.0f);

	// Z axis
	glColor3f(0, 0, 1); //Blue color

	glVertex3f(-0.1f, 0.1f, -0.1f);
	glVertex3f(-0.1f, 0.1f, 5.1f);

	/// Draw a Z
	glVertex3f(0.0f, 2.0f, 4.5f);
	glVertex3f(0.0f, 2.0f, 5.0f);
	glVertex3f(0.0f, 2.0f, 5.0f);
	glVertex3f(0.0f, 1.5f, 4.5f);
	glVertex3f(0.0f, 1.5f, 4.5f);
	glVertex3f(0.0f, 1.5f, 5.0f);

	glEnd();
	glColor3f(1, 1, 1); //Set color back to white
}

GameObject* ModuleScene::CreateSphere(int slices, int stacks, float diameter)
{
	GameObject* item = new GameObject();

	if (sphereCount > 0) 
	{
		char name[50];
		sprintf_s(name, 50, "Sphere (%d)", sphereCount);
		item->name = name;
	}
	else item->name = "Sphere";
	sphereCount++;

	ComponentMesh* mesh = (ComponentMesh*)item->CreateComponent(COMPONENT_TYPE::MESH);
	ComponentMaterial* material = (ComponentMaterial*)item->CreateComponent(COMPONENT_TYPE::MATERIAL);

	//Create the mesh with Par_Shapes
	par_shapes_mesh_s* newMesh;
	newMesh = par_shapes_create_parametric_sphere(slices,stacks);
	par_shapes_scale(newMesh, diameter, diameter, diameter);

	// If it is the first time we create a Sphere, we save its mesh as a resource
	if (sphereCount == 1) {
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->CreateNewResource(RESOURCE_TYPE::MESH, sphereMesh_UUID);
		mesh->LoadParShape(newMesh);
		std::string file;
		App->res_loader->ImportMeshToLibrary(mesh->resource_mesh, file, "Sphere");
		mesh->resource_mesh->name = "Sphere";
		mesh->resource_mesh->file = "Primitive";
		mesh->resource_mesh->exported_file = file;
	}
	else
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->GetResourceFromMap(sphereMesh_UUID);

	// Create the AABB
	mesh->gameObject->aabb.SetNegativeInfinity();
	mesh->gameObject->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);
	mesh->aabb.SetNegativeInfinity();
	mesh->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);

	// Free the Par_mesh
	par_shapes_free_mesh(newMesh);

	// Set default texture
	material->resource_tex = App->res_loader->default_material;

	Sphere sphere;
	sphere.r = diameter;
	App->physics->AddRigidBody(sphere, item, 0.5f);

	CreateGameObject* creationAction = new CreateGameObject(item);
	App->undo->StoreNewAction(creationAction);

	App->scene->selected = item;
	App->scene->selectedNodes.clear();
	App->scene->selectedNodes.push_back(App->scene->selected);
	return item;
}

GameObject* ModuleScene::CreateCube(float sizeX, float sizeY, float sizeZ)
{
	GameObject* item = new GameObject();

	if (cubeCount > 0) 
	{
		char name[50];
		sprintf_s(name, 50, "Cube (%d)", cubeCount);
		item->name = name;
	}
	else item->name = "Cube";
	cubeCount++;

	ComponentMesh* mesh = (ComponentMesh*)item->CreateComponent(COMPONENT_TYPE::MESH);
	ComponentMaterial* material = (ComponentMaterial*)item->CreateComponent(COMPONENT_TYPE::MATERIAL);

	//Create the mesh with Par_Shapes
	//create 6 faces of the cube (we don't use par_shapes_create_cube
	//because it doesn't have normals or uvs
	par_shapes_mesh_s* mesh_front = par_shapes_create_plane(1,1);
	par_shapes_mesh_s* back = par_shapes_create_plane(1, 1);
	par_shapes_mesh_s* right = par_shapes_create_plane(1, 1);
	par_shapes_mesh_s* left = par_shapes_create_plane(1, 1);
	par_shapes_mesh_s* top = par_shapes_create_plane(1, 1);
	par_shapes_mesh_s* bottom = par_shapes_create_plane(1, 1);

	par_shapes_translate(back, -0.5f, -0.5f, 0.5f);

	par_shapes_rotate(mesh_front, PAR_PI, (float*)&float3::unitY);
	par_shapes_translate(mesh_front, 0.5f, -0.5f, -0.5f);

	par_shapes_rotate(left, PAR_PI/2, (float*)&float3::unitY);
	par_shapes_translate(left, 0.5f, -0.5f, 0.5f);

	par_shapes_rotate(right, -PAR_PI / 2, (float*)&float3::unitY);
	par_shapes_translate(right, -0.5f, -0.5f, -0.5f);

	par_shapes_rotate(bottom, PAR_PI / 2, (float*)&float3::unitX);
	par_shapes_translate(bottom, -0.5f, -0.5f, -0.5f);

	par_shapes_rotate(top, -PAR_PI / 2, (float*)&float3::unitX);
	par_shapes_translate(top, -0.5f, 0.5f, 0.5f);

	par_shapes_merge_and_free(mesh_front, back);
	par_shapes_merge_and_free(mesh_front, left);
	par_shapes_merge_and_free(mesh_front,right);
	par_shapes_merge_and_free(mesh_front, bottom);
	par_shapes_merge_and_free(mesh_front, top);

	par_shapes_scale(mesh_front, sizeX, sizeY, sizeZ);

	// If it is the first time we create a Cube, we save its mesh as a resource
	if (cubeCount == 1) 
	{
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->CreateNewResource(RESOURCE_TYPE::MESH, cubeMesh_UUID);
		mesh->LoadParShape(mesh_front);
		std::string file;
		App->res_loader->ImportMeshToLibrary(mesh->resource_mesh, file, "Cube");
		mesh->resource_mesh->name = "Cube";
		mesh->resource_mesh->file = "Primitive";
		mesh->resource_mesh->exported_file = file;
	}
	else
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->GetResourceFromMap(cubeMesh_UUID);

	// Create the AABB
	mesh->gameObject->aabb.SetNegativeInfinity();
	mesh->gameObject->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);
	mesh->aabb.SetNegativeInfinity();
	mesh->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);

	// Free the Par_mesh
	par_shapes_free_mesh(mesh_front);

	// Set default texture
	material->resource_tex = App->res_loader->default_material;

	App->physics->AddRigidBody(OBB(item->aabb), item, 0.5f);

	CreateGameObject* creationAction = new CreateGameObject(item);
	App->undo->StoreNewAction(creationAction);

	App->scene->selected = item;
	App->scene->selectedNodes.clear();
	App->scene->selectedNodes.push_back(App->scene->selected);
	return item;
}

GameObject* ModuleScene::CreatePlane(float length, float depth)
{
	GameObject* item = new GameObject();

	if (planeCount > 0) 
	{
		char name[50];
		sprintf_s(name, 50, "Plane (%d)", planeCount);
		item->name = name;
	}
	else item->name = "Plane";
	planeCount++;

	ComponentMesh* mesh = (ComponentMesh*)item->CreateComponent(COMPONENT_TYPE::MESH);
	ComponentMaterial* material = (ComponentMaterial*)item->CreateComponent(COMPONENT_TYPE::MATERIAL);

	//Convert the par_Mesh into a regular mesh
	par_shapes_mesh_s* plane = par_shapes_create_plane(2,2);

	par_shapes_rotate(plane, -PAR_PI / 2, (float*)&float3::unitX);

	par_shapes_scale(plane,length,0.0f, depth);

	// If it is the first time we create a Plane, we save its mesh as a resource
	if (planeCount == 1) 
	{
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->CreateNewResource(RESOURCE_TYPE::MESH, planeMesh_UUID);
		mesh->LoadParShape(plane);
		std::string file;
		App->res_loader->ImportMeshToLibrary(mesh->resource_mesh, file, "Plane");
		mesh->resource_mesh->name = "Plane";
		mesh->resource_mesh->file = "Primitive";
		mesh->resource_mesh->exported_file = file;
	}
	else
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->GetResourceFromMap(planeMesh_UUID);

	// Create the AABB
	mesh->gameObject->aabb.SetNegativeInfinity();
	mesh->gameObject->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);
	mesh->aabb.SetNegativeInfinity();
	mesh->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);

	// Free the Par_mesh
	par_shapes_free_mesh(plane);

	// Set default texture
	material->resource_tex = App->res_loader->default_material;

	App->physics->AddRigidBody(OBB(item->aabb), item, 0.5f);

	CreateGameObject* creationAction = new CreateGameObject(item);
	App->undo->StoreNewAction(creationAction);

	App->scene->selected = item;
	App->scene->selectedNodes.clear();
	App->scene->selectedNodes.push_back(App->scene->selected);
	return item;
}

GameObject* ModuleScene::CreateDonut(int slices, int stacks, float radius)
{
	GameObject* item = new GameObject();

	if (donutCount > 0) 
	{
		char name[50];
		sprintf_s(name, 50, "Donut (%d)", donutCount);
		item->name = name;
	}
	else item->name = "Donut";
	donutCount++;

	ComponentMesh* mesh = (ComponentMesh*)item->CreateComponent(COMPONENT_TYPE::MESH);
	ComponentMaterial* material = (ComponentMaterial*)item->CreateComponent(COMPONENT_TYPE::MATERIAL);

	//Create the mesh with Par_Shapes
	par_shapes_mesh_s* newMesh = par_shapes_create_torus(slices, stacks, 0.5f);

	par_shapes_rotate(newMesh, -PAR_PI / 2, (float*)&float3::unitX);

	par_shapes_scale(newMesh, radius, radius, radius);

	// If it is the first time we create a Sphere, we save its mesh as a resource
	if (donutCount == 1) 
	{
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->CreateNewResource(RESOURCE_TYPE::MESH, donutMesh_UUID);
		mesh->LoadParShape(newMesh);
		std::string file;
		App->res_loader->ImportMeshToLibrary(mesh->resource_mesh, file, "Donut");
		mesh->resource_mesh->name = "Donut";
		mesh->resource_mesh->file = "Primitive";
		mesh->resource_mesh->exported_file = file;
	}
	else
		mesh->resource_mesh = (ResourceMesh*)App->res_manager->GetResourceFromMap(donutMesh_UUID);

	// Create the AABB
	mesh->gameObject->aabb.SetNegativeInfinity();
	mesh->gameObject->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);
	mesh->aabb.SetNegativeInfinity();
	mesh->aabb = AABB::MinimalEnclosingAABB(mesh->resource_mesh->vertices, mesh->resource_mesh->num_vertices);

	// Free the Par_mesh
	par_shapes_free_mesh(newMesh);

	// Set default texture
	material->resource_tex = App->res_loader->default_material;

	App->physics->AddRigidBody(OBB(item->aabb), item, 0.5f);

	CreateGameObject* creationAction = new CreateGameObject(item);
	App->undo->StoreNewAction(creationAction);

	App->scene->selected = item;
	App->scene->selectedNodes.clear();
	App->scene->selectedNodes.push_back(App->scene->selected);
	return item;
}