#pragma once
#include "stdafx.h"
#include "texture.h"

class Shader
{
public:
	Shader() = default;
	Shader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) { CreatePipelineState(device, rootSignature); }
	~Shader() = default;

	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }

protected:
	ComPtr<ID3D12PipelineState> m_pipelineState;
};

class TextureShader : public Shader
{
public:
	TextureShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) { CreatePipelineState(device, rootSignature); }
	~TextureShader() = default;

private:
	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
};

class TerrainShader : public Shader
{
public:
	TerrainShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) { CreatePipelineState(device, rootSignature); }
	~TerrainShader() = default;

	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
};

class SkyboxShader : public Shader
{
public:
	SkyboxShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) { CreatePipelineState(device, rootSignature); }
	~SkyboxShader() = default;

	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
};

class InstanceShader : public Shader
{
public:
	InstanceShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) { CreatePipelineState(device, rootSignature); }
	~InstanceShader() = default;

	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
};