#include "camera.h"

Camera::Camera() :
	m_eye{ 0.0f, 0.0f, 0.0f }, m_look{ 0.0f, 0.0f, 1.0f }, m_up{ 0.0f, 1.0f, 0.0f },
	m_u{ 1.0f, 0.0f, 0.0f }, m_v{ 0.0f, 1.0f, 0.0f }, m_n{ 0.0f, 0.0f, 1.0f },
	m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }, m_terrain{ nullptr }
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());
}

void Camera::Update(FLOAT deltaTime)
{
	// 카메라가 지형 밑으로 내려가지 않도록함
	if (m_terrain)
	{
		XMFLOAT3 pos{ GetEye() };
		FLOAT height{ m_terrain->GetHeight(pos.x, pos.z) };
		if (pos.y < height + 0.5f)
			SetEye(XMFLOAT3{ pos.x, height + 0.5f, pos.z });
	}

	// 카메라 뷰 변환 행렬 최신화
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&Vector3::Add(m_eye, m_look)), XMLoadFloat3(&m_up)));
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	// DIRECTX는 행우선(row-major), HLSL는 열우선(column-major)
	// 행렬이 셰이더로 넘어갈 때 자동으로 전치 행렬로 변환된다.
	// 그래서 셰이더에 전치 행렬을 넘겨주면 DIRECTX의 곱셈 순서와 동일하게 계산할 수 있다.
	XMFLOAT4X4 transViewMatrix, transProjMatrix;
	XMStoreFloat4x4(&transViewMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_viewMatrix)));
	XMStoreFloat4x4(&transProjMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_projMatrix)));

	commandList->SetGraphicsRoot32BitConstants(1, 16, &transViewMatrix, 0);
	commandList->SetGraphicsRoot32BitConstants(1, 16, &transProjMatrix, 16);
}

void Camera::UpdateLocalAxis()
{
	m_n = Vector3::Normalize(m_look);					 // 로컬 z축
	m_u = Vector3::Normalize(Vector3::Cross(m_up, m_n)); // 로컬 x축
	m_v = Vector3::Cross(m_n, m_u);						 // 로컬 y축
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
		// z축(roll)으로는 회전할 수 없음
	}
	if (pitch != 0.0f)
	{
		// x축(pitch)의 경우 MIN_PITCH ~ MAX_PITCH
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
		// y축(yaw)의 경우 제한 없음
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

ThirdPersonCamera::ThirdPersonCamera() : Camera{}, m_offset{ 0.0f, 1.0f, -5.0f }, m_delay{ 0.01f }
{

}

void ThirdPersonCamera::Update(FLOAT deltaTime)
{
	XMFLOAT3 destination{ Vector3::Add(m_player->GetPosition(), m_offset) };
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

	// 항상 플레이어를 바라보도록 설정
	XMFLOAT3 look{ Vector3::Sub(m_player->GetPosition(), m_eye) };
	if (Vector3::Length(look))
		SetAt(look);
}