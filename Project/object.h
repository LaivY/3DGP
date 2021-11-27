#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "shader.h"
#include "terrain.h"
#include "texture.h"

class Camera;

enum class GameObjectType {
	DEFAULT, BULLET
};

class GameObject
{
public:
	GameObject();
	~GameObject() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, const shared_ptr<Shader>& shader=nullptr) const;
	virtual void Update(FLOAT deltaTime);
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void SetWorldMatrix(const XMFLOAT4X4& worldMatrix) { m_worldMatrix = worldMatrix; }
	void SetPosition(const XMFLOAT3& position);
	void SetMesh(const shared_ptr<Mesh>& Mesh);
	void SetShader(const shared_ptr<Shader>& shader);
	void SetTexture(const shared_ptr<Texture>& texture);
	void SetTextureInfo(unique_ptr<TextureInfo>& textureInfo);
	void SetTerrain(HeightMapTerrain* terrain) { m_terrain = terrain; }

	GameObjectType GetType() const { return m_type; }
	bool isDeleted() const { return m_isDeleted; }
	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRight() const { return m_right; }
	XMFLOAT3 GetUp() const { return m_up; }
	XMFLOAT3 GetFront() const { return m_front; }
	XMFLOAT3 GetRollPitchYaw() const { return XMFLOAT3{ m_roll, m_pitch, m_yaw }; }

	HeightMapTerrain* GetTerrain() const { return m_terrain; }
	XMFLOAT3 GetNormal() const { return m_normal; }
	XMFLOAT3 GetLook() const { return m_look; }

protected:
	GameObjectType			m_type;				// ���ӿ�����Ʈ ���� Ư������ ���� Ÿ��
	bool					m_isDeleted;		// true�� ��� ���� �����ӿ� ������

	XMFLOAT4X4				m_worldMatrix;		// ���� ��ȯ ���
	XMFLOAT3				m_right;			// ���� x��
	XMFLOAT3				m_up;				// ���� y��
	XMFLOAT3				m_front;			// ���� z��
	FLOAT					m_roll;				// z�� ȸ����
	FLOAT					m_pitch;			// x�� ȸ����
	FLOAT					m_yaw;				// y�� ȸ����

	HeightMapTerrain*		m_terrain;			// ���ִ� ���� ��ü�� ������
	XMFLOAT3				m_normal;			// ���� ��ġ�� �븻 ����
	XMFLOAT3				m_look;				// ������ ����� ���� ����

	shared_ptr<Mesh>		m_mesh;				// �޽�
	shared_ptr<Shader>		m_shader;			// ���̴�
	shared_ptr<Texture>		m_texture;			// �ؽ���
	unique_ptr<TextureInfo>	m_textureInfo;		// �ؽ��� �ִϸ��̼� ���� ����ü
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