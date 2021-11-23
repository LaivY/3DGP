#pragma once
#include "stdafx.h"
#include "object.h"
#include "terrain.h"

#define ROLL_MAX +20
#define ROLL_MIN -10

class Camera;

class Player : public GameObject
{
public:
	Player();
	~Player() = default;

	virtual void Update(FLOAT deltaTime);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	void SetPlayerOnTerrain();

	void AddVelocity(const XMFLOAT3& increase);
	void SetVelocity(const XMFLOAT3& velocity) { m_velocity = velocity; }
	void SetCamera(const shared_ptr<Camera>& camera) { m_camera = camera; }

	XMFLOAT3 GetVelocity() const { return m_velocity; }

private:
	XMFLOAT3			m_velocity;		// �ӵ�
	FLOAT				m_maxVelocity;	// �ִ�ӵ�
	FLOAT				m_friction;		// ������
	shared_ptr<Camera>	m_camera;		// ī�޶�
};