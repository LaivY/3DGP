#include "player.h"
#include "camera.h"

Player::Player() : GameObject{}, m_velocity{ 0.0f, 0.0f, 0.0f }, m_maxVelocity{ 10.0f }, m_friction{ 1.1f }
{

}

void Player::Update(FLOAT deltaTime)
{
	GameObject::Update(deltaTime);

	// �̵�
	Move(m_velocity);

	// �÷��̾ � ���� ���� �ִٸ�
	if (m_terrain)
	{
		// �÷��̾ ���� ���� �̵��ϵ���
		XMFLOAT3 position{ GetPosition() };
		FLOAT height{ m_terrain->GetHeight(position.x, position.z) };
		SetPosition(XMFLOAT3{ position.x, height, position.z });

		// �ش� ��ġ�� �븻 ����
		m_normal = m_terrain->GetNormal(position.x, position.z);

		// ������ ����� ���� ���͸� ���
		float theta{ acosf(Vector3::Dot(m_up, m_normal)) };
		if (theta) // +y��� m_normal�� �ٸ� ��
		{
			XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(m_up, m_normal)) };
			if (m_normal.z < 0)
			{
				right = Vector3::Mul(right, -1);
				theta *= -1;
			}
			XMFLOAT4X4 rotate; XMStoreFloat4x4(&rotate, XMMatrixRotationNormal(XMLoadFloat3(&right), theta));
			m_look = Vector3::TransformNormal(XMFLOAT3{ m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33 }, rotate);
		}
		else m_look = m_front;
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
	XMFLOAT3 up{ m_normal };
	XMFLOAT3 look{ m_look };
	XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(up, look)) };
	worldMatrix._11 = right.x;	worldMatrix._12 = right.y;	worldMatrix._13 = right.z;
	worldMatrix._21 = up.x;		worldMatrix._22 = up.y;		worldMatrix._23 = up.z;
	worldMatrix._31 = look.x;	worldMatrix._32 = look.y;	worldMatrix._33 = look.z;
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