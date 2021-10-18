#include "player.h"
#include "camera.h"

Player::Player() : GameObject{}, m_velocity{ 0.0f, 0.0f, 0.0f }, m_maxVelocity{ 10.0f }, m_friction{ 1.1f }
{

}

void Player::Update(FLOAT deltaTime)
{
	// �̵�
	Move(m_velocity);

	// �÷��̾ � ���� ���� �ִٸ�
	if (m_terrain)
	{
		// �÷��̾ ���� ���� �̵��ϵ���
		XMFLOAT3 pos{ GetPosition() };
		FLOAT height{ m_terrain->GetHeight(pos.x, pos.z) };
		SetPosition(XMFLOAT3{ pos.x, height + 0.5f, pos.z });

		// �÷��̾ ���ִ� ���� �븻�� ����
		m_normal = m_terrain->GetNormal(pos.x, pos.z);
	}

	// ������ ����
	m_velocity = Vector3::Mul(m_velocity, 1 / m_friction * deltaTime);
}

void Player::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	// ȸ���� ����
	if (m_pitch + pitch > MAX_PITCH)
		pitch = MAX_PITCH - m_pitch;
	else if (m_pitch + pitch < MIN_PITCH)
		pitch = MIN_PITCH - m_pitch;

	// ȸ���� �ջ�
	m_roll += roll; m_pitch += pitch; m_yaw += yaw;

	// ī�޶�� x,y������ ȸ���� �� �ִ�.
	// GameObject::Rotate���� �÷��̾��� ���� x,y,z���� �����ϹǷ� ���� ȣ���ؾ��Ѵ�.
	m_camera->Rotate(0.0f, pitch, yaw);

	// �÷��̾�� y�����θ� ȸ���� �� �ִ�.
	GameObject::Rotate(0.0f, 0.0f, yaw);
}

void Player::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	XMFLOAT4X4 worldMatrix{ m_worldMatrix };

	// +y��� ������ normal���� ���̰� ���
	float theta{ acosf(Vector3::Dot(m_up, m_normal)) };

	// +y��� m_normal�� �ٸ� ��
	if (theta)
	{
		// +z���� �������� ���� right���� ���
		XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(m_up, m_normal)) };
		if (m_normal.z < 0)
		{
			right = Vector3::Mul(right, -1);
			theta *= -1;
		}

		// �����ϱ� ���� ��ġ ���� ����
		worldMatrix._41 = 0.0f; worldMatrix._42 = 0.0f; worldMatrix._43 = 0.0f;

		// ������ ������ �°� ȸ��
		XMFLOAT4X4 rotate;
		XMStoreFloat4x4(&rotate, XMMatrixRotationNormal(XMLoadFloat3(&right), theta));
		worldMatrix = Matrix::Mul(worldMatrix, rotate);

		// ��ġ ���� ����
		XMFLOAT3 pos{ GetPosition() };
		worldMatrix._41 = pos.x; worldMatrix._42 = pos.y; worldMatrix._43 = pos.z;
	}

	commandList->SetGraphicsRoot32BitConstants(0, 16, &Matrix::Transpose(worldMatrix), 0);
}

void Player::AddVelocity(const XMFLOAT3& increase)
{
	m_velocity = Vector3::Add(m_velocity, increase);

	// �ִ� �ӵ��� �ɸ��ٸ� �ش� ������ ��ҽ�Ŵ
	FLOAT length{ Vector3::Length(m_velocity) };
	if (length > m_maxVelocity)
	{
		FLOAT ratio{ m_maxVelocity / length };
		m_velocity = Vector3::Mul(m_velocity, ratio);
	}
}