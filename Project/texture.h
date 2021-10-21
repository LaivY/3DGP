#pragma once
#include "stdafx.h"
#include "DDSTextureLoader12.h"

// Texture 객체는 여러개의 객체가 함께 사용한다.
// 하지만 애니메이션 프레임, 인터벌 등은 각 객체마다 다르므로 렌더링할때 해당 객체의 애니메이션 정보를 이용해서 렌더링하도록한다.
struct TextureInfo
{
	INT		frame{ 0 };						// 몇 번째 프레임인지
	FLOAT	frameTimer{ 0.0f };				// 다음 프레임으로 넘어가는 시간을 잴 시간
	FLOAT	frameInterver{ 1.0f / 60.0f };	// 프레임 당 시간 간격
	BOOL	isFrameRepeat{ TRUE };			// TRUE일 경우 마지막 프레임 후 0프레임으로 반복함
};

class Texture
{
public:
	Texture() : m_textureInfo{ nullptr } { }
	~Texture() = default;

	void LoadTextureFile(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName, UINT rootParameterIndex);
	void CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device);
	void CreateShaderResourceView(const ComPtr<ID3D12Device>& device);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void ReleaseUploadBuffer();

	void SetTextureInfo(TextureInfo* textureInfo) { m_textureInfo = textureInfo; }

	UINT GetTextureCount() const { return m_textures.size(); }

protected:
	ComPtr<ID3D12DescriptorHeap>				m_srvHeap;
	vector<pair<ComPtr<ID3D12Resource>, UINT>>	m_textures;
	vector<ComPtr<ID3D12Resource>>				m_textureUploadBuffers;

	TextureInfo*								m_textureInfo;
};