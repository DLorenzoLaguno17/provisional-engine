#include "Application.h"
#include "ModuleMeshLoader.h"

#include "OpenGL.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "Brofiler/Lib/Brofiler.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/lib86/assimp.lib")

ModuleMeshLoader::ModuleMeshLoader(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleMeshLoader::~ModuleMeshLoader()
{}

// Called before render is available
bool ModuleMeshLoader::Awake()
{
	LOG("Preparing mesh loader");

	return true;
}

// Called before the first frame
bool ModuleMeshLoader::Start()
{
	// Stream log messages to Assimp debug window
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	// Loading FBX
	LoadFBX("Assets/warrior.FBX", test_list);

	return true;
}

// Called before Update
update_status ModuleMeshLoader::PreUpdate(float dt)
{
	BROFILER_CATEGORY("MeshLoaderPreUpdate", Profiler::Color::Orange)

	return UPDATE_CONTINUE;
}

// Called every frame
update_status ModuleMeshLoader::Update(float dt)
{
	BROFILER_CATEGORY("MeshLoaderUpdate", Profiler::Color::LightSeaGreen)	
		
	RenderFBX(test_list);

	return UPDATE_CONTINUE;
}

// Called after Update
update_status ModuleMeshLoader::PostUpdate(float dt)
{
	BROFILER_CATEGORY("MeshLoaderPostUpdate", Profiler::Color::Yellow)
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleMeshLoader::CleanUp()
{
	LOG("Freeing mesh loader");
	// Detach Assimp log stream
	aiDetachAllLogStreams();

	return true;
}

void  ModuleMeshLoader::LoadFBX(const char* path, std::vector<MeshData*> meshList) {	
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	
	if (scene != nullptr && scene->HasMeshes())
	{
		// Use scene->mNumMeshes to iterate on scene->mMeshes array
		for (int i = 0; i < scene->mNumMeshes; ++i) {

			MeshData* m = new MeshData();

			// Copy vertices
			m->num_vertices = scene->mMeshes[i]->mNumVertices;
			m->vertices = new float[m->num_vertices * 3];
			memcpy(m->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * m->num_vertices * 3);
			LOG("New mesh with %d vertices", m->num_vertices);

			// Copy faces
			if (scene->mMeshes[i]->HasFaces())
			{
				m->num_indices = scene->mMeshes[i]->mNumFaces * 3;
				m->indices = new uint[m->num_indices]; // Assume each face is a triangle
				for (uint j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
				{
					if (scene->mMeshes[i]->mFaces[j].mNumIndices != 3) 
						LOG("WARNING, geometry face with != 3 indices!")
					else 
						memcpy(&m->indices[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}

			meshList.push_back(m);
		}

		aiReleaseImport(scene);
	}
	else LOG("Error loading scene %s", path);
}

void ModuleMeshLoader::RenderFBX(std::vector<MeshData*> FBX) {
	/*glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	for (int i = 0; i < FBX.size(); ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, FBX[i]->num_vertices);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FBX[i]->vertices), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FBX[i]->vertices), (const GLvoid*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FBX[i]->vertices), (const GLvoid*)20);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FBX[i]->num_indices);

		glDrawElements(GL_TRIANGLES, FBX[i]->num_indices, GL_UNSIGNED_INT, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);*/
}

void ModuleMeshLoader::Load(const json &config)
{
}

void ModuleMeshLoader::Save(json &config)
{
}