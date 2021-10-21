#pragma once
#include "stdafx.h"
#include "DDSTextureLoader12.h"

// Texture ��ü�� �������� ��ü�� �Բ� ����Ѵ�.
// ������ �ִϸ��̼� ������, ���͹� ���� �� ��ü���� �ٸ��Ƿ� �������Ҷ� �ش� ��ü�� �ִϸ��̼� ������ �̿��ؼ� �������ϵ����Ѵ�.
struct TextureInfo
{
	INT		frame{ 0 };						// �� ��° ����������
	FLOAT	frameTimer{ 0.0f };				// ���� ���������� �Ѿ�� �ð��� �� �ð�
	FLOAT	frameInterver{ 1.0f / 60.0f };	// ������ �� �ð� ����
	BOOL	isFrameRepeat{ TRUE };			// TRUE�� ��� ������ ������ �� 0���������� �ݺ���
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