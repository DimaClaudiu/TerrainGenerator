#include "TerrainGenerator.h"

#include <vector>
#include <string>
#include <iostream>
#include <time.h>
#include <stb/stb_image.h>
#include <Core/Engine.h>
#include "Camera.h"

using namespace std;

TerrainGenerator::TerrainGenerator()
{
	this->resolution = window->GetResolution();
	this->aspectRatio = glm::vec2(16, 9); // sorry ultrawide
}

TerrainGenerator::~TerrainGenerator()
{
}

bool gameStart;
float myrotation = 0;
bool birdEyeView = true;
Mesh* ground;
bool turn = true;

void TerrainGenerator::Init()
{
	srand(time(NULL));
	camera = new Camera();
	camera->Set(Player1CameraPosition, Player1CameraForward, Player1CameraRight, Player1CameraUp);

	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		ground = GenerateTerrain(256, 256);
		meshes[ground->GetMeshID()] = ground;
	}

	{
		Shader* shader = new Shader("ShaderGround");
		shader->AddShader("Source/Game/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Game/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	const string textureLoc = "Source/Game/Textures/";
	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "ground.jpg").c_str(), GL_REPEAT);
		mapTextures["earth"] = texture;
	}
	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "ground2.jpg").c_str(), GL_REPEAT);
		mapTextures["earth2"] = texture;
	}

	projectilePosition = glm::vec3(1.2, -10, -1.6);

	//Light & material properties
	{
		lightPosition = glm::vec3(0, 7, 0);
		lightDirection = glm::vec3(0, -1, 0);
		materialShininess = 0.2;
		materialKd = 0.5;
		materialKs = 3;
	}
}

void TerrainGenerator::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void TerrainGenerator::ShootProjectile(bool turn)
{
	int minSpeed = 5;
	int maxSpeed = 7;

	float modif = (rand() % (maxSpeed - minSpeed) * 1000) / 1000.0f + minSpeed;

	projectileVx = modif * camera->forward.x;
	projectileVz = modif * camera->forward.z;
	projectileVy = modif * camera->forward.y + maxSpeed - minSpeed;

	projectilePosition = camera->position;
	if (birdEyeView)
		camera->Set(OverviewCameraPosition, OverviewCameraForward, OverviewCameraRight, OverviewCameraUp);
}

bool colided = false;
int collisionArea = 0;
void TerrainGenerator::Update(float deltaTimeSeconds)
{
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
	RenderSimpleMesh(meshes["terrain"], shaders["ShaderGround"], modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f), mapTextures["earth"]);

	{
		// Render projectile
		glm::mat4 modelMatrix = glm::mat4(1);
		projectilePosition += glm::vec3(projectileVx * deltaTimeSeconds, projectileVy * deltaTimeSeconds, projectileVz * deltaTimeSeconds);
		projectileVy -= gravity * deltaTimeSeconds;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(projectilePosition.x, projectilePosition.y + 0.75, projectilePosition.z));
		for (int i = 0; i < ground->positions.size(); i++)
		{
			float myDistance = distance(ground->positions[i], projectilePosition);
			if (!colided && myDistance <= 0.1)
			{
				colided = true;
				break;
			}
			else if (colided && myDistance <= AOE)
			{
				ground->positions[i].y /= (2 - 2 * myDistance);
				collisionArea++;
			}
			if (collisionArea >= AOE * 200)
			{
				ground->InitFromData(ground->positions, ground->normals, ground->indices);
				meshes[ground->GetMeshID()] = ground;
				colided = false;
				collisionArea = 0;
				projectilePosition = glm::vec3(0, -10, 0);
				projectileVx = projectileVy = 0;
			}
		}
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25));
		RenderSimpleMesh(meshes["sphere"], shaders["VertexNormal"], modelMatrix, glm::vec3(0, 1, 0));
	}

	// Render the point light in the scene
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, lightPosition);
		modelMatrix = glm::rotate(modelMatrix, myrotation, glm::vec3(1, 0, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1));
		//RenderSimpleMesh(meshes["sphere"], shaders["VertexColor"], modelMatrix, glm::vec3(1, 0, 0));
	}

	// Rotate and draw the skybox
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(8, 0, 0));
		myrotation += deltaTimeSeconds / 20.0f;
		modelMatrix = glm::rotate(modelMatrix, myrotation, glm::vec3(1, 1, 1));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(160.0f));
		RenderTexturedMesh(meshes["sphere"], shaders["Simple"], modelMatrix, 2);
	}
}

void TerrainGenerator::FrameEnd()
{
	//DrawCoordinatSystem(camera->GetViewMatrix(), projectionMatrix);
}

void TerrainGenerator::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, Texture2D* texture1, Texture2D* texture2)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set shader uniforms for light & material properties
	// Set light position uniform
	int lighting_type_position = glGetUniformLocation(shader->program, "lighting_type");
	glUniform1i(lighting_type_position, lightingType);

	int cutoff_position = glGetUniformLocation(shader->program, "cut_off");
	glUniform1f(cutoff_position, cutoff);

	int light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

	int light_direction = glGetUniformLocation(shader->program, "light_direction");
	glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

	// Set eye position (camera position) uniform
	glm::vec3 eyePosition = camera->position;
	int eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	// Set material property uniforms (shininess, kd, ks, object color)
	int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	int object_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(object_color, color.r, color.g, color.b);

	GLint loc_projectile_position = glGetUniformLocation(shader->program, "ProjectilePosition");
	glUniform3f(loc_projectile_position, projectilePosition.x, projectilePosition.y, projectilePosition.z);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = camera->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	if (texture1)
	{
		// activate texture location 0
		// Bind the texture1 ID
		// Send texture uniform value
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);
	}

	if (texture2)
	{
		// activate texture location 1
		// Bind the texture2 ID
		// Send texture uniform value
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->program, "texture_2"), 1);
	}

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

void TerrainGenerator::RenderTexturedMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int textureID)
{
	if (!mesh || !shader || !shader->program)
		return;

	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	mesh->Render(textureID);
}

Mesh* TerrainGenerator::GenerateTerrain(int width, int length)
{
	int imgW, imgH, channels;
	unsigned char* data = stbi_load((RESOURCE_PATH::TEXTURES + "heightmap.jpg").c_str(), &imgW, &imgH, &channels, 0);

	const int size = width * length;
	vector<glm::vec3> verts(size), norms(size);
	vector<unsigned short> indices;

	const float w = 15.0f, l = 15.0f, h = 3.0f;
	int i = 0;
	for (int z = 0; z < length; z++)
		for (int x = 0; x < width; x++)
		{
			glm::vec3 v = glm::vec3(x / (float)width, 0, z / (float)length);

			int ix = roundf(v.x * imgW);
			int iy = roundf(v.z * imgH);

			int pixel = (int)data[(ix * length + iy) * channels];
			int pixelRight = pixel;
			int pixelUp = pixel;

			if (iy + 1 < width)
				pixelRight = (int)data[(ix * length + iy + 1) * channels];
			if (ix - 1 >= 0)
				pixelUp = (int)data[((ix - 1) * length + iy) * channels];

			float height = (float)pixel / 255.0f;

			v.y = height * h;
			v.x *= w;
			v.z *= l;
			v.x -= w / 2;
			v.z -= l / 2;

			verts[i] = v;

			float heightRight = (float)pixelRight / 255.0f;
			float heightUp = (float)pixelUp / 255.0f;

			float Hx = height - heightRight;
			float Hz = height = heightUp;

			norms[i] = glm::normalize(glm::vec3(Hx, 1, Hz));

			if ((i + 1) % width != 0 && z + 1 < length)
			{
				indices.push_back(i);
				indices.push_back(i + width);
				indices.push_back(i + width + 1);

				indices.push_back(i);
				indices.push_back(i + width + 1);
				indices.push_back(i + 1);
			}

			i++;
		}

	Mesh* mesh = new Mesh("terrain");
	mesh->InitFromData(verts, norms, indices);

	return mesh;
}

void TerrainGenerator::OnInputUpdate(float deltaTime, int mods)
{
	float speed = 2;

	float cameraSpeed = 2.0f;

	// move the camera only if MOUSE_RIGHT button is pressed
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float distance = cameraSpeed * deltaTime;

		if (window->KeyHold(GLFW_KEY_W)) {
			//  translate the camera forward
			camera->MoveForward(distance);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			//  translate the camera to the left
			camera->TranslateRight(-distance);
		}

		if (window->KeyHold(GLFW_KEY_S)) {
			//  translate the camera backwards
			camera->MoveForward(-distance);
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			//  translate the camera to the right
			camera->TranslateRight(distance);
		}

		if (window->KeyHold(GLFW_KEY_Q)) {
			//  translate the camera down
			camera->TranslateUpword(-distance);
		}

		if (window->KeyHold(GLFW_KEY_E)) {
			//  translate the camera up
			camera->TranslateUpword(distance);
		}
	}

	if (window->KeyHold(GLFW_KEY_O) && matrix == "proj") {
		projectionMatrix = glm::ortho(-width, width, -height, height, zNear, zFar);
		matrix = "orth";
	}

	if (window->KeyHold(GLFW_KEY_P) && matrix == "orth") {
		projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, zNear, zFar);
		matrix = "proj";
	}

	if (window->KeyHold(GLFW_KEY_X) && matrix == "proj") {
		fov += deltaTime * cameraSpeed * 50.0f;
		projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, zNear, zFar);
	}

	if (window->KeyHold(GLFW_KEY_Z) && matrix == "proj") {
		fov -= deltaTime * cameraSpeed * 50.0f;
		projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, zNear, zFar);
	}

	if (window->KeyHold(GLFW_KEY_V) && matrix == "orth") {
		width += deltaTime * cameraSpeed * 5.0f;
		height += deltaTime * cameraSpeed * 5.0f;
		projectionMatrix = glm::ortho(-width, width, -height, height, zNear, zFar);
	}

	if (window->KeyHold(GLFW_KEY_C) && matrix == "orth") {
		width -= deltaTime * cameraSpeed * 5.0f;
		height -= deltaTime * cameraSpeed * 5.0f;
		projectionMatrix = glm::ortho(-width, width, -height, height, zNear, zFar);
	}
}

void TerrainGenerator::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE)
	{
		cout << camera->position << "  " << camera->forward << " " << camera->right << " " << camera->up << endl;

		if (!turn)
			camera->Set(Player1CameraPosition, Player1CameraForward, Player1CameraRight, Player1CameraUp);
		else
			camera->Set(Player2CameraPosition, Player2CameraForward, Player2CameraRight, Player2CameraUp);
		turn = !turn;
	}
	if (key == GLFW_KEY_P)
	{
		ShootProjectile(turn);
	}
	if (key == GLFW_KEY_B)
		birdEyeView = !birdEyeView;
}

void TerrainGenerator::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void TerrainGenerator::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float sensivityOX = 0.001f;
		float sensivityOY = 0.001f;

		renderCameraTarget = true;
		//  rotate the camera in Third-person mode around OX and OY using deltaX and deltaY
		camera->RotateThirdPerson_OX(-sensivityOX * deltaY);
		camera->RotateThirdPerson_OY(-sensivityOY * deltaX);
	}
}

void TerrainGenerator::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void TerrainGenerator::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void TerrainGenerator::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void TerrainGenerator::OnWindowResize(int width, int height)
{
}