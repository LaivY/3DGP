#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "shader.h"
#include "terrain.h"
#include "texture.h"

class Camera;

class GameObject
{
public:
	GameObject();
	~GameObject();

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void Update(FLOAT deltaTime) { }
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void ReleaseUploadBuffer() const;

	void SetFront(const XMFLOAT3& front);
	void SetPosition(const XMFLOAT3& position);
	void SetMesh(const shared_ptr<Mesh>& mesh);
	void SetShader(const shared_ptr<Shader>& shader);
	void SetTexture(const shared_ptr<Texture>& texture);
	void SetTerrain(HeightMapTerrain* terrain) { m_terrain = terrain; }

	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRight() const { return m_right; }
	XMFLOAT3 GetUp() const { return m_up; }
	XMFLOAT3 GetFront() const { return m_front; }
	HeightMapTerrain* GetTerrain() const { return m_terrain; }
	XMFLOAT3 GetNormal() const { return m_normal; }

protected:
	XMFLOAT4X4				m_worldMatrix;		// 월드 변환

	XMFLOAT3				m_right;			// 로컬 x축
	XMFLOAT3				m_up;				// 로컬 y축
	XMFLOAT3				m_front;			// 로컬 z축

	FLOAT					m_roll;				// z축 회전각
	FLOAT					m_pitch;			// x축 회전각
	FLOAT					m_yaw;				// y축 회전각

	HeightMapTerrain*		m_terrain;			// 서있는 지형의 포인터
	XMFLOAT3				m_normal;			// 서있는 지형의 노멀 벡터

	shared_ptr<Mesh>		m_mesh;				// 메쉬
	shared_ptr<Shader>		m_shader;			// 셰이더
	shared_ptr<Texture>		m_texture;			// 텍스쳐
};

class BillboardObject : public GameObject
{
public:
	BillboardObject(const shared_ptr<Camera>& camera);
	~BillboardObject() = default;

	virtual void Update(FLOAT deltaTime);
	void SetCamera(const shared_ptr<Camera>& camera);

private:
	shared_ptr<Camera> m_camera; // 기준이 되는 카메라
};

class Bullet : public GameObject
{
public:
	Bullet(const XMFLOAT3& position, const XMFLOAT3& direction, FLOAT speed = 10.0f, FLOAT damage = 1.0f);
	~Bullet() = default;

	virtual void Update(FLOAT deltaTime);

private:
	XMFLOAT3	m_direction;	// 날아가는 방향
	FLOAT		m_speed;		// 날아가는 속도
	FLOAT		m_damage;		// 피해량
};