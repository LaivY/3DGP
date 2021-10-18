#pragma once
#include "stdafx.h"
#include "object.h"
#include "camera.h"

class Skybox
{
public:
	Skybox(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12RootSignature>& rootSignature);
	~Skybox() = default;

	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void Update();
	void SetCamera(const shared_ptr<Camera>& camera);
	void SetPosition(XMFLOAT3 position);

private:
	shared_ptr<Camera>			m_camera;
	unique_ptr<GameObject[]>	m_faces;
};