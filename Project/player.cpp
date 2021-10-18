#include "player.h"
#include "camera.h"

Player::Player() : GameObject{}, m_velocity{ 0.0f, 0.0f, 0.0f }, m_maxVelocity{ 10.0f }, m_friction{ 1.1f }
{

}

void Player::Update(FLOAT deltaTime)
{
	// 이동
	Move(m_velocity);

	// 플레이어가 어떤 지형 위에 있다면
	if (m_terrain)
	{
		// 플레이어가 지형 위를 이동하도록
		XMFLOAT3 pos{ GetPosition() };
		FLOAT height{ m_terrain->GetHeight(pos.x, pos.z) };
		SetPosition(XMFLOAT3{ pos.x, height + 0.5f, pos.z });

		// 플레이어가 서있는 곳의 노말을 저장
		m_normal = m_terrain->GetNormal(pos.x, pos.z);
	}

	// 마찰력 적용
	m_velocity = Vector3::Mul(m_velocity, 1 / m_friction * deltaTime);
}

void Player::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	// 회전각 제한
	if (m_pitch + pitch > MAX_PITCH)
		pitch = MAX_PITCH - m_pitch;
	else if (m_pitch + pitch < MIN_PITCH)
		pitch = MIN_PITCH - m_pitch;

	// 회전각 합산
	m_roll += roll; m_pitch += pitch; m_yaw += yaw;

	// 카메라는 x,y축으로 회전할 수 있다.
	// GameObject::Rotate에서 플레이어의 로컬 x,y,z축을 변경하므로 먼저 호출해야한다.
	m_camera->Rotate(0.0f, pitch, yaw);

	// 플레이어는 y축으로만 회전할 수 있다.
	GameObject::Rotate(0.0f, 0.0f, yaw);
}

void Player::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	XMFLOAT4X4 worldMatrix{ m_worldMatrix };

	// +y축과 지형의 normal벡터 사이각 계산
	float theta{ acosf(Vector3::Dot(m_up, m_normal)) };

	// +y축과 m_normal이 다를 때
	if (theta)
	{
		// +z축을 보고있을 때의 right벡터 계산
		XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(m_up, m_normal)) };
		if (m_normal.z < 0)
		{
			right = Vector3::Mul(right, -1);
			theta *= -1;
		}

		// 자전하기 위해 위치 정보 삭제
		worldMatrix._41 = 0.0f; worldMatrix._42 = 0.0f; worldMatrix._43 = 0.0f;

		// 지형의 각도에 맞게 회전
		XMFLOAT4X4 rotate;
		XMStoreFloat4x4(&rotate, XMMatrixRotationNormal(XMLoadFloat3(&right), theta));
		worldMatrix = Matrix::Mul(worldMatrix, rotate);

		// 위치 정보 복구
		XMFLOAT3 pos{ GetPosition() };
		worldMatrix._41 = pos.x; worldMatrix._42 = pos.y; worldMatrix._43 = pos.z;
	}

	commandList->SetGraphicsRoot32BitConstants(0, 16, &Matrix::Transpose(worldMatrix), 0);
}

void Player::AddVelocity(const XMFLOAT3& increase)
{
	m_velocity = Vector3::Add(m_velocity, increase);

	// 최대 속도에 걸린다면 해당 비율로 축소시킴
	FLOAT length{ Vector3::Length(m_velocity) };
	if (length > m_maxVelocity)
	{
		FLOAT ratio{ m_maxVelocity / length };
		m_velocity = Vector3::Mul(m_velocity, ratio);
	}
}