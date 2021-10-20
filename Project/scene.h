#pragma once
#include "stdafx.h"
#include "camera.h"
#include "instance.h"
#include "object.h"
#include "player.h"
#include "skybox.h"
#include "terrain.h"

class Resources
{
public:
	Resources() = default;
	~Resources() = default;

	void AddMesh(const string& key, const shared_ptr<Mesh>& mesh) { m_meshes[key] = mesh; }
	void AddShader(const string& key, const shared_ptr<Shader>& shader) { m_shaders[key] = shader; }
	void AddTexture(const string& key, const shared_ptr<Texture>& texture) { m_textures[key] = texture; }

	shared_ptr<Mesh> GetMesh(const string& key) const;
	shared_ptr<Shader> GetShader(const string& key) const;
	shared_ptr<Texture> GetTexture(const string& key) const;

private:
	map<string, shared_ptr<Mesh>>		m_meshes;
	map<string, shared_ptr<Shader>>		m_shaders;
	map<string, shared_ptr<Texture>>	m_textures;
};

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void OnInit(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12RootSignature>& rootSignature, FLOAT aspectRatio);
	void OnMouseEvent(HWND hWnd, UINT width, UINT height, FLOAT deltaTime);
	void OnMouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void OnKeyboardEvent(FLOAT deltaTime) const;
	void OnUpdate(FLOAT deltaTime);

	void Update(FLOAT deltaTime);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void ReleaseUploadBuffer();

	void CreateBullet();

	void SetSkybox(unique_ptr<Skybox>& skybox);
	void SetPlayer(const shared_ptr<Player>& player);
	void SetCamera(const shared_ptr<Camera>& camera);

	Skybox* GetSkybox() const { return m_skybox.get(); }
	shared_ptr<Player> GetPlayer() const { return m_player; }
	shared_ptr<Camera> GetCamera() const { return m_camera; }
	HeightMapTerrain* GetTerrain(FLOAT x, FLOAT z) const;

private:
	unique_ptr<Resources>					m_resources; // 메쉬, 셰이더, 텍스쳐들을 보관하고 있는 클래스

	vector<unique_ptr<GameObject>>			m_gameObjects;
	vector<unique_ptr<Instance>>			m_instances;
	vector<unique_ptr<HeightMapTerrain>>	m_terrains;
	unique_ptr<Skybox>						m_skybox;
	shared_ptr<Player>						m_player;
	shared_ptr<Camera>						m_camera;
};