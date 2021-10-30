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
	GameObjectType			m_type;			// 필요할 때 캐스팅하기 위한 객체 타입

	XMFLOAT4X4				m_worldMatrix;	// 월드 행렬
	XMFLOAT3				m_right;		// 로컬 x축
	XMFLOAT3				m_up;			// 로컬 y축
	XMFLOAT3				m_front;		// 로컬 z축

	FLOAT					m_roll;			// z축 회전각
	FLOAT					m_pitch;		// x축 회전각
	FLOAT					m_yaw;			// y축 회전각

	BoundingOrientedBox		m_boundingBox;	// 피격 박스

	bool					m_checkTerrain; // true일 경우 어느 지형 위에 있는지 확인함
	HeightMapTerrain*		m_terrain;		// 서있는 지형의 포인터
	XMFLOAT3				m_normal;		// 서있는 지형의 노멀 벡터
	XMFLOAT3				m_look;			// 지형이 적용된 정면

	bool					m_isDeleted;	// true일 경우 다음 프레임에 삭제됨

	shared_ptr<Mesh>		m_mesh;			// 메쉬
	shared_ptr<Shader>		m_shader;		// 셰이더
	shared_ptr<Texture>		m_texture;		// 텍스쳐
	unique_ptr<TextureInfo>	m_textureInfo;	// 텍스쳐 애니메이션을 위한 구조체
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
	shared_ptr<Camera>	m_camera; // 기준이 되는 카메라
	XMFLOAT3			m_offset; // 피봇 위치에서 오프셋을 더한 위치에 렌더링함
};

class Bullet : public GameObject
{
public:
	Bullet(const XMFLOAT3& position, const XMFLOAT3& direction, const XMFLOAT3& up, FLOAT speed = 30.0f, FLOAT damage = 1.0f);
	~Bullet() = default;

	virtual void Update(FLOAT deltaTime);

private:
	XMFLOAT3	m_origin;		 // 발사 시작 위치
	XMFLOAT3	m_direction;	 // 날아가는 방향
	FLOAT		m_speed;		 // 날아가는 속도
	FLOAT		m_damage;		 // 피해량
};

class Building : public GameObject
{
public:
	Building();
	~Building() = default;

	virtual void Update(FLOAT deltaTime);
};