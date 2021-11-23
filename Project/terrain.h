#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class GameObject;

class HeightMapImage
{
public:
	HeightMapImage(const wstring& fileName, INT width, INT length, XMFLOAT3 scale);
	~HeightMapImage() = default;

	BYTE* GetPixels() const { return m_pixels.get(); }
	XMFLOAT3 GetNormal(INT x, INT z) const;
	FLOAT GetHeight(FLOAT x, FLOAT z) const;
	INT GetWidth() const { return m_width; }
	INT GetLength() const { return m_length; }
	XMFLOAT3 GetScale() const { return m_scale; }
	
private:
	unique_ptr<BYTE[]>	m_pixels;	// �ȼ����� 2���� �迭(�� ���Ҵ� 0~255�� ��)
	INT					m_width;	// �̹����� ���� ����
	INT					m_length;	// �̹����� ���� ����
	XMFLOAT3			m_scale;	// Ȯ�� ����
};

class HeightMapGridMesh : public Mesh
{
public:
	HeightMapGridMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, 
		HeightMapImage* heightMapImage, INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale);
	~HeightMapGridMesh() = default;
};

class HeightMapGridTessMesh : public Mesh
{
public:
	HeightMapGridTessMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
		HeightMapImage* heightMapImage, INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale);
	~HeightMapGridTessMesh() = default;
};

class HeightMapTerrain
{
public:
	HeightMapTerrain(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
		const shared_ptr<Shader>& shader, const shared_ptr<Texture>& texture, INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale);
	~HeightMapTerrain() = default;

	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void Move(const XMFLOAT3& shift);
	void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	void SetPosition(const XMFLOAT3& position);
	void SetShader(const shared_ptr<Shader>& shader);

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetBlockPosition(FLOAT x, FLOAT z);
	FLOAT GetHeight(FLOAT x, FLOAT z) const;
	XMFLOAT3 GetNormal(FLOAT x, FLOAT z) const;
	INT GetWidth() const { return m_width; }
	INT GetLength() const { return m_length; }
	INT GetBlockWidth() const { return m_blockWidth; }
	INT GetBlockLength() const { return m_blockLength; }
	XMFLOAT3 GetScale() const { return m_scale; }

private:
	unique_ptr<HeightMapImage>		m_heightMapImage;	// ���̸� �̹���
	vector<unique_ptr<GameObject>>	m_blocks;			// ��ϵ�
	INT								m_width;			// �̹����� ���� ����
	INT								m_length;			// �̹����� ���� ����
	INT								m_blockWidth;		// ����� ���� ����
	INT								m_blockLength;		// ����� ���� ����
	XMFLOAT3						m_scale;			// Ȯ�� ����
};