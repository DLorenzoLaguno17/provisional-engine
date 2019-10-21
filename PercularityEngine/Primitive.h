#ifndef _PRIMITIVE_H__
#define _PRIMITIVE_H__

#include "Globals.h"

struct par_shapes_mesh_s;

class Primitive {
public:

	//Constructor & destructor
	Primitive();
	~Primitive();
	void CleanUp();

	//Render the figure
	void Render() const;

protected:

	//Send data to vram buffers
	/*IMPORTANT: This function MUST be called in all the constructors of 
	Primitive subclasses after creating the mesh, otherwise, they won't 
	be drawn*/
	void StoreData();

protected:

	//Mesh
	par_shapes_mesh_s* mesh = nullptr;

	uint verticesId		= 0;	//Vertices buffer id
	uint indicesId		= 0;	//Indices buffer id
	
};

class Sphere :public Primitive
{
public:
	//Constructors & destructor
	Sphere(int slices=10, int stacks=10);

	~Sphere();

};

class Cube :public Primitive
{
public:
	//Constructors & destructor
	Cube();
	~Cube();
};

class Cone:public Primitive
{
public:
	//Constructors & destructor
	Cone(int slices=10, int stacks=10);

	~Cone();
};
#endif