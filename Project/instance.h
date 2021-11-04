#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "object.h"
#include "shader.h"

struct InstanceData
{
	XMFLOAT4X4 worldMatrix;
};

class Instance
{
public:
	Instance(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, UINT count);
	~Instance();

	void Update(FLOAT deltaTime);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void AddGameObject(unique_ptr<GameObject> gameObject);
	void SetMesh(const shared_ptr<Mesh>& mesh);
	void SetShader(const shared_ptr<Shader>& mesh);
	void SetTexture(const shared_ptr<Texture>& texture);

	vector<unique_ptr<GameObject>>& GetGameObjects() { return m_gameObjects; }

private:
	UINT							m_count;
	vector<unique_ptr<GameObject>>	m_gameObjects;

	shared_ptr<Mesh>				m_mesh;
	shared_ptr<Shader>				m_shader;
	shared_ptr<Texture>				m_texture;

	ComPtr<ID3D12Resource>			m_instanceBuffer;
	D3D12_VERTEX_BUFFER_VIEW		m_instanceBufferView;
	InstanceData*					m_instanceBufferPointer;
};