#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "Primitive.h"
#include "ComponentCamera.h"
#include "Time.h"
#include "PhysVehicle.h"

ModulePlayer::ModulePlayer(Application* app, bool start_enabled) : Module(app, start_enabled), vehicle(NULL)
{
	turn = acceleration = brake = 0.0f;
}

ModulePlayer::~ModulePlayer() {}

// Load assets
bool ModulePlayer::Start() 
{ 
	LOG("Creating the player");

	// Creating car
	VehicleInfo car;

	// Car properties ----------------------------------------
	car.chassis_size.Set(2, 1.5, 5);
	car.chassis_offset.Set(0, 1.2, 0);
	car.mass = 500.0f;
	car.suspensionStiffness = 15.88f;
	car.suspensionCompression = 0.83f;
	car.suspensionDamping = 0.88f;
	car.maxSuspensionTravelCm = 1000.0f;
	car.frictionSlip = 50.5;
	car.maxSuspensionForce = 6000.0f;

	// Wheel properties ---------------------------------------
	float connection_height = 1.2f;
	float wheel_radius = 0.6f;
	float wheel_width = 0.5f;
	float suspensionRestLength = 1.2f;

	// Don't change anything below this line ------------------

	float half_width = car.chassis_size.x * 0.5f;
	float half_length = car.chassis_size.z * 0.5f;

	vec3 direction(0, -1, 0);
	vec3 axis(-1, 0, 0);

	car.num_wheels = 4;
	car.wheels = new Wheel[4];

	// FRONT-LEFT ------------------------
	car.wheels[0].connection.Set(half_width - 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[0].direction = direction;
	car.wheels[0].axis = axis;
	car.wheels[0].suspensionRestLength = suspensionRestLength;
	car.wheels[0].radius = wheel_radius;
	car.wheels[0].width = wheel_width;
	car.wheels[0].front = true;
	car.wheels[0].drive = true;
	car.wheels[0].brake = false;
	car.wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car.wheels[1].connection.Set(-half_width + 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[1].direction = direction;
	car.wheels[1].axis = axis;
	car.wheels[1].suspensionRestLength = suspensionRestLength;
	car.wheels[1].radius = wheel_radius;
	car.wheels[1].width = wheel_width;
	car.wheels[1].front = true;
	car.wheels[1].drive = true;
	car.wheels[1].brake = false;
	car.wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car.wheels[2].connection.Set(half_width - 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[2].direction = direction;
	car.wheels[2].axis = axis;
	car.wheels[2].suspensionRestLength = suspensionRestLength;
	car.wheels[2].radius = wheel_radius;
	car.wheels[2].width = wheel_width;
	car.wheels[2].front = false;
	car.wheels[2].drive = false;
	car.wheels[2].brake = true;
	car.wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car.wheels[3].connection.Set(-half_width + 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[3].direction = direction;
	car.wheels[3].axis = axis;
	car.wheels[3].suspensionRestLength = suspensionRestLength;
	car.wheels[3].radius = wheel_radius;
	car.wheels[3].width = wheel_width;
	car.wheels[3].front = false;
	car.wheels[3].drive = false;
	car.wheels[3].brake = true;
	car.wheels[3].steering = false;

	vehicle = App->physics->AddVehicle(car);
	vehicle->SetPos(0, 3, 10);
	initialCarPosition = vehicle->GetPosition();
	initialForwardVector = vehicle->GetForwardVector();

	return true;
}

update_status ModulePlayer::Update(float dt)
{
	if (App->scene->playMode) {

		turn = acceleration = brake = 0.0f;

		// Controls of the car
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		{
			if (abs(vehicle->GetKmh()) > 1 && !goingForward) {
				brake = BRAKE_POWER / 4;
				startedEngine = false;
			}
			else {
				acceleration = MAX_ACCELERATION;
				goingForward = true;

				if (!startedEngine) {
					startedEngine = true;
				}
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		{
			if (turn < TURN_DEGREES)
				turn += TURN_DEGREES;
		}

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		{
			if (turn > -TURN_DEGREES)
				turn -= TURN_DEGREES;
		}

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		{
			if (abs(vehicle->GetKmh()) > 1 && goingForward) {
				brake = BRAKE_POWER;
				startedEngine = false;
			}
			else {
				acceleration = -MAX_ACCELERATION;
				goingForward = false;

				if (!startedEngine) {
					startedEngine = true;
				}
			}
		}

		// To free the camera
		if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
			freeCamera = !freeCamera;

		vehicle->ApplyEngineForce(acceleration);
		vehicle->Turn(turn);
		vehicle->Brake(brake);

		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			App->physics->ShootBall();

		vehicle->Render();

		// Camera from the car's point of view
		/*if (!freeCamera) {
			App->camera->LookAt(vehicle->GetPosition());
			vec3 temp = (vehicle->GetPosition() - vehicle->GetForwardVector() * 10) + vec3(0, 4, 0);
			float3 finalPos = { temp.x, temp.y, temp.z };
			App->camera->camera->frustum.pos = finalPos;
		}*/
	}

	return UPDATE_CONTINUE;
}

// Resets the car
void ModulePlayer::ResetCar()
{
	freeCamera = false;
	vehicle->vehicle->getRigidBody()->setAngularVelocity({ 0, 0, 0 });
	vehicle->vehicle->getRigidBody()->setLinearVelocity({ 0, 0, 0 });
	vehicle->SetPos(initialCarPosition.x, initialCarPosition.y, initialCarPosition.z);
	vehicle->RotateBody({ 0, initialForwardVector.x, initialForwardVector.y, initialForwardVector.z });
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	return true;
}