#include "camera.h"

Camera::Camera() :
	m_eye{ 0.0f, 0.0f, 0.0f }, m_look{ 0.0f, 0.0f, 1.0f }, m_up{ 0.0f, 1.0f, 0.0f },
	m_u{ 1.0f, 0.0f, 0.0f }, m_v{ 0.0f, 1.0f, 0.0f }, m_n{ 0.0f, 0.0f, 1.0f },
	m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());
}

void Camera::Update(FLOAT deltaTime)
{
	// ī�޶� ���� ������ �������� �ʵ�����
	if (m_terrain)
	{
		XMFLOAT3 pos{ GetEye() };
		FLOAT height{ m_terrain->GetHeight(pos.x, pos.z) };
		if (pos.y < height + 0.5f)
			SetEye(XMFLOAT3{ pos.x, height + 0.5f, pos.z });
	}

	// ī�޶� �� ��ȯ ��� �ֽ�ȭ
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&Vector3::Add(m_eye, m_look)), XMLoadFloat3(&m_up)));
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	// DIRECTX�� ��켱(row-major), HLSL�� ���켱(column-major)
	// ����� ���̴��� �Ѿ �� �ڵ����� ��ġ ��ķ� ��ȯ�ȴ�.
	// �׷��� ���̴��� ��ġ ����� �Ѱ��ָ� DIRECTX�� ���� ������ �����ϰ� ����� �� �ִ�.
	commandList->SetGraphicsRoot32BitConstants(1, 16, &Matrix::Transpose(m_viewMatrix), 0);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &Matrix::Transpose(m_projMatrix), 16);
	commandList->SetGraphicsRoot32BitConstants(1, 3, &GetEye(), 32);
}

void Camera::UpdateLocalAxis()
{
	m_n = Vector3::Normalize(m_look);					 // ���� z��
	m_u = Vector3::Normalize(Vector3::Cross(m_up, m_n)); // ���� x��
	m_v = Vector3::Cross(m_n, m_u);						 // ���� y��
}

void Camera::Move(const XMFLOAT3& shift)
{
	m_eye = Vector3::Add(m_eye, shift);
}

void Camera::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	XMMATRIX rotate{ XMMatrixIdentity() };
	if (roll != 0.0f)
	{
		// z��(roll)���δ� ȸ���� �� ����
	}
	if (pitch != 0.0f)
	{
		// x��(pitch)�� ��� MIN_PITCH ~ MAX_PITCH
		if (m_pitch + pitch > MAX_PITCH)
		{
			rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(MAX_PITCH - m_pitch));
			m_pitch = MAX_PITCH;
		}
		else if (m_pitch + pitch < MIN_PITCH)
		{
			rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(MIN_PITCH - m_pitch));
			m_pitch = MIN_PITCH;
		}
		else
		{
			rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(pitch));
			m_pitch += pitch;
		}
	}
	if (yaw != 0.0f)
	{
		// y��(yaw)�� ��� ���� ����
		rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_v), XMConvertToRadians(yaw));
		m_yaw += yaw;
	}
	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), rotate));
	UpdateLocalAxis();
}

void Camera::SetPlayer(const shared_ptr<Player>& player)
{
	if (m_player) m_player.reset();
	m_player = player;
	SetEye(m_player->GetPosition());
}

// --------------------------------------

ThirdPersonCamera::ThirdPersonCamera() : Camera{}, m_distance{ 5.0f }, m_delay{ 0.01f }
{
	m_offset = Vector3::Normalize(XMFLOAT3{ 0.0f, 1.0f, -5.0f });
}

void ThirdPersonCamera::Update(FLOAT deltaTime)
{
	XMFLOAT3 destination{ Vector3::Add(m_player->GetPosition(), Vector3::Mul(m_offset, m_distance)) };
	XMFLOAT3 direction{ Vector3::Sub(destination, GetEye()) };
	XMFLOAT3 shift{ Vector3::Mul(direction, fmax((1.0f - m_delay) * deltaTime * 10.0f, 0.01f)) };
	SetEye(Vector3::Add(GetEye(), shift));
	Camera::Update(deltaTime);
}

void ThirdPersonCamera::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	XMMATRIX rotate{ XMMatrixIdentity() };
	if (roll != 0.0f)
	{
		rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetFront()), XMConvertToRadians(roll));
		m_roll += roll;
	}
	if (pitch != 0.0f)
	{
		rotate *= XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetRight()), XMConvertToRadians(pitch));
		m_pitch += pitch;
	}
	if (yaw != 0.0f)
	{
		XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
		rotate *= XMMatrixRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(yaw));
		m_yaw += yaw;
	}
	XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));

	// �׻� �÷��̾ �ٶ󺸵��� ����
	XMFLOAT3 look{ Vector3::Sub(m_player->GetPosition(), m_eye) };
	if (Vector3::Length(look))
	{
		m_look = look;
		UpdateLocalAxis();
	}
}