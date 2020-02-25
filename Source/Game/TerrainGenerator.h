#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include "Camera.h"

class TerrainGenerator : public SimpleScene
{
private:
	// Camera
	Camera* camera;
	glm::mat4 projectionMatrix;
	bool renderCameraTarget;
	float upwardTranslation = 0;
	float fov = 60;
	float height = 5.0f, width = 5.0f;
	float zNear = 0.01f, zFar = 200.0f;

	std::string matrix = "proj";
	glm::vec2 resolution;
	glm::vec2 aspectRatio;

	// Light
	std::unordered_map<std::string, Texture2D*> mapTextures;
	glm::vec3 lightPosition;
	glm::vec3 lightDirection;

	unsigned int materialShininess;
	float materialKd;
	float materialKs;
	int lightingType = 0;
	float cutoff = 3.1415 / 3;

	// Projectile
	glm::vec3 projectilePosition;
	float projectileVx;
	float projectileVy;
	float projectileVz;
	float AOE = 0.6; // area of effect
	float gravity = 9.81;

	// Players
	glm::vec3 Player1CameraPosition = glm::vec3(-1.75623, 3.77513, 3.12959);
	glm::vec3 Player1CameraForward = glm::vec3(0.181743, 0.0246984, -0.983036);
	glm::vec3 Player1CameraRight = glm::vec3(0.983336, 0, 0.181798);
	glm::vec3 Player1CameraUp = glm::vec3(-0.00449012, 0.999695, 0.0242861);

	glm::vec3 Player2CameraPosition = glm::vec3(-1.73131, 3.81362, -4.31426);
	glm::vec3 Player2CameraForward = glm::vec3(0.132023, -0.0522753, 0.989867);
	glm::vec3 Player2CameraRight = glm::vec3(-0.991222, 0, 0.132205);
	glm::vec3 Player2CameraUp = glm::vec3(0.00691134, 0.998633, 0.051817);

	glm::vec3 OverviewCameraPosition = glm::vec3(1.09107, 13.5623, 0.174076);
	glm::vec3 OverviewCameraForward = glm::vec3(-0.109278, -0.994011, 0.000260204);
	glm::vec3 OverviewCameraRight = glm::vec3(-0.00238948, 0, -0.999997);
	glm::vec3 OverviewCameraUp = glm::vec3(-0.994008, 0.109278, 0.00237444);

public:
	TerrainGenerator();
	~TerrainGenerator();

	void Init() override;

private:
	void FrameStart() override;
	void ShootProjectile(bool turn);
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, Texture2D* texture1 = NULL, Texture2D* texture2 = NULL);
	void TerrainGenerator::RenderTexturedMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int textureID);

	Mesh* GenerateTerrain(int width, int height);

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;
};
