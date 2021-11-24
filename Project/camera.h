#pragma once
#include "stdafx.h"
#include "player.h"

#define MAX_PITCH +60
#define MIN_PITCH -80

class Camera
{
public:
	Camera();
	~Camera() = default;

	virtual void Update(FLOAT deltaTime);
	void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void UpdateLocalAxis();

	void SetViewMatrix(const XMFLOAT4X4& viewMatrix) { m_viewMatrix = viewMatrix; }
	void SetProjMatrix(const XMFLOAT4X4& projMatrix) { m_projMatrix = projMatrix; }
	void SetEye(const XMFLOAT3& eye) { m_eye = eye; UpdateLocalAxis(); }
	void SetAt(const XMFLOAT3& at) { m_look = at;	UpdateLocalAxis(); }
	void SetUp(const XMFLOAT3& up) { m_up = up;		UpdateLocalAxis(); }
	void SetPlayer(const shared_ptr<Player>& player);
	void SetTerrain(HeightMapTerrain* terrain) { m_terrain = terrain; }

	XMFLOAT4X4 GetViewMatrix() const { return m_viewMatrix; }
	XMFLOAT4X4 GetProjMatrix() const { return m_projMatrix; }
	XMFLOAT3 GetEye() const { return m_eye; }
	XMFLOAT3 GetAt() const { return m_look; }
	XMFLOAT3 GetUp() const { return m_up; }
	shared_ptr<Player> GetPlayer() const { return m_player; }

protected:
	XMFLOAT4X4			m_viewMatrix;	// �亯ȯ ���
	XMFLOAT4X4			m_projMatrix;	// ������ȯ ���

	XMFLOAT3			m_eye;			// ī�޶� ��ġ
	XMFLOAT3			m_look;			// ī�޶� �ٶ󺸴� ����
	XMFLOAT3			m_up;			// ī�޶� Up����

	XMFLOAT3			m_u;			// ���� x��
	XMFLOAT3			m_v;			// ���� y��
	XMFLOAT3			m_n;			// ���� z��

	FLOAT				m_roll;			// x�� ȸ����
	FLOAT				m_pitch;		// y�� ȸ����
	FLOAT				m_yaw;			// z�� ȸ����

	shared_ptr<Player>	m_player;		// �÷��̾�
	HeightMapTerrain*	m_terrain;		// ī�޶� ��ġ���ִ� ����
};

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera();
	~ThirdPersonCamera() = default;

	virtual void Update(FLOAT deltaTime);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	XMFLOAT3 GetOffset() const { return m_offset; }
	FLOAT GetDistance() const { return m_distance; }

	void SetOffset(const XMFLOAT3& offset) { m_offset = offset; }
	void SetDistance(FLOAT distance) { m_distance = clamp(distance, 3.0f, 20.0f); }
	void SetDelay(FLOAT delay) { m_delay = delay; }

private:
	XMFLOAT3	m_offset;	// �÷��̾�κ��� �������ִ� ��ġ
	FLOAT		m_distance;	// ������ �������� ������ �Ÿ�
	FLOAT		m_delay;	// ������ ������ (0.0 ~ 1.0)
};