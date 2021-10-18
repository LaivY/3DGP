#pragma once
#include "stdafx.h"
#include "DDSTextureLoader12.h"

class Texture
{
public:
	Texture() = default;
	~Texture() = default;

	void LoadTextureFile(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName, UINT rootParameterIndex);
	void CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device);
	void CreateShaderResourceView(const ComPtr<ID3D12Device>& device);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void ReleaseUploadBuffer();

private:
	ComPtr<ID3D12DescriptorHeap>				m_srvHeap;
	vector<pair<ComPtr<ID3D12Resource>, UINT>>	m_textures;
	vector<ComPtr<ID3D12Resource>>				m_textureUploadBuffers;
};