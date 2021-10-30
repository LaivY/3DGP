#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "shader.h"
#include "terrain.h"
#include "texture.h"

class Camera;

enum class GameObjectType {
	DEFAULT, BILLBOARD, BULLET, BUILDING
};

class GameObject
{
public:
	GameObject();
	~GameObject() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void Update(FLOAT deltaTime);
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void SetWorldMatrix(const XMFLOAT4X4& worldMatrix) { m_worldMatrix = worldMatrix; }
	void SetWorldMatrix(const XMFLOAT3& right, const XMFLOAT3& up, const XMFLOAT3& look);
	void SetPosition(const XMFLOAT3& position);
	void SetTerrain(HeightMapTerrain* terrain) { m_terrain = terrain; }
	void SetMesh(const shared_ptr<Mesh>& mesh);
	void SetShader(const shared_ptr<Shader>& shader);
	void SetTexture(const shared_ptr<Texture>& texture);
	void SetTextureInfo(unique_ptr<TextureInfo>& textureInfo);
	void SetCheckTerrain(bool checkTerrain) { m_checkTerrain = checkTerrain; }
	void SetDelete(bool isDeleted) { m_isDeleted = isDeleted; }

	GameObjectType GetType() const { return m_type; }
	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRight() const { return m_right; }
	XMFLOAT3 GetUp() const { return m_up; }
	XMFLOAT3 GetFront() const { return m_front; }
	BoundingOrientedBox GetBoundingBox() const { return m_boundingBox; }
	HeightMapTerrain* GetTerrain() const { return m_terrain; }
	XMFLOAT3 GetNormal() const { return m_normal; }
	XMFLOAT3 GetLook() const { return m_look; }
	bool GetIsDeleted() const { return m_isDeleted; }
	bool GetCheckTerrain() const { return m_checkTerrain; }

protected:
	GameObjectType			m_type;			// �ʿ��� �� ĳ�����ϱ� ���� ��ü Ÿ��

	XMFLOAT4X4				m_worldMatrix;	// ���� ���
	XMFLOAT3				m_right;		// ���� x��
	XMFLOAT3				m_up;			// ���� y��
	XMFLOAT3				m_front;		// ���� z��

	FLOAT					m_roll;			// z�� ȸ����
	FLOAT					m_pitch;		// x�� ȸ����
	FLOAT					m_yaw;			// y�� ȸ����

	BoundingOrientedBox		m_boundingBox;	// �ǰ� �ڽ�

	bool					m_checkTerrain; // true�� ��� ��� ���� ���� �ִ��� Ȯ����
	HeightMapTerrain*		m_terrain;		// ���ִ� ������ ������
	XMFLOAT3				m_normal;		// ���ִ� ������ ��� ����
	XMFLOAT3				m_look;			// ������ ����� ����

	bool					m_isDeleted;	// true�� ��� ���� �����ӿ� ������

	shared_ptr<Mesh>		m_mesh;			// �޽�
	shared_ptr<Shader>		m_shader;		// ���̴�
	shared_ptr<Texture>		m_texture;		// �ؽ���
	unique_ptr<TextureInfo>	m_textureInfo;	// �ؽ��� �ִϸ��̼��� ���� ����ü
};

class BillboardObject : public GameObject
{
public:
	BillboardObject(const shared_ptr<Camera>& camera, const XMFLOAT3& offset = { 0.0f, 0.0f, 0.0f });
	~BillboardObject() = default;

	virtual void Update(FLOAT deltaTime);
	void SetCamera(const shared_ptr<Camera>& camera);
	XMFLOAT3 GetOffset() const { return m_offset; }

private:
	shared_ptr<Camera>	m_camera; // ������ �Ǵ� ī�޶�
	XMFLOAT3			m_offset; // �Ǻ� ��ġ���� �������� ���� ��ġ�� ��������
};

class Bullet : public GameObject
{
public:
	Bullet(const XMFLOAT3& position, const XMFLOAT3& direction, const XMFLOAT3& up, FLOAT speed = 30.0f, FLOAT damage = 1.0f);
	~Bullet() = default;

	virtual void Update(FLOAT deltaTime);

private:
	XMFLOAT3	m_origin;		 // �߻� ���� ��ġ
	XMFLOAT3	m_direction;	 // ���ư��� ����
	FLOAT		m_speed;		 // ���ư��� �ӵ�
	FLOAT		m_damage;		 // ���ط�
};

class Building : public GameObject
{
public:
	Building();
	~Building() = default;

	virtual void Update(FLOAT deltaTime);
};