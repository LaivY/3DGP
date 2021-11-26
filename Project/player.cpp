#include "player.h"
#include "camera.h"

Player::Player() : GameObject{}, m_velocity{ 0.0f, 0.0f, 0.0f }, m_maxVelocity{ 10.0f }, m_friction{ 1.1f }
{

}

void Player::Update(FLOAT deltaTime)
{
	Move(m_velocity);

	m_look = m_front;

	// �ǳ�, ���� ������ �������� ����
	SetPlayerInArea();

	// �÷��̾ � ���� ���� �ִٸ�
	if (m_terrain)
	{
		// �ǳ��� �ִ°� �ƴ϶��
		if (GetPosition().y < 300.0f)
		{
			// �÷��̾ ���� ���� �����̵��� ����
			SetPlayerOnTerrain();

			// �÷��̾��� �븻�� �� ���� ����
			SetPlayerNormalAndLook();
		}
	}
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

void Player::SetPlayerInArea()
{
	XMFLOAT3 pos{ GetPosition() };

	// �ǳ� ������ ��������
	if (GetPosition().y > 300.0f)
	{
		pos.x = clamp(pos.x, -14.0f, 14.0f);
		pos.z = clamp(pos.z, -14.0f, 14.0f);
		SetPosition(pos);
	}

	// ���� ������ ��������
	else if (m_terrain)
	{
		XMFLOAT3 tPos{ m_terrain->GetPosition() };
		XMFLOAT3 scale{ m_terrain->GetScale() };
		pos.x = clamp(pos.x, tPos.x, tPos.x + m_terrain->GetWidth() * scale.x);
		pos.z = clamp(pos.z, tPos.z, tPos.z + m_terrain->GetLength() * scale.z);
		SetPosition(pos);
	}
}

void Player::SetPlayerOnTerrain()
{
	/*
	1. �÷��̾ ��ġ�� ����� �����ϴ� ��ǥ�� ���Ѵ�.
	2. �ش� ��ǥ�� �̿��Ͽ� ��� �޽��� ���� 25���� ���Ѵ�.
	3. ���� 25���� �̿��Ͽ� 4�� ������ ����� ���Ѵ�.
	4. �÷��̾��� ��ġ t�� ���ؼ� ������ ��� ���� ���̸� ���Ѵ�.
	*/

	XMFLOAT3 pos{ GetPosition() };
	XMFLOAT3 LB{ m_terrain->GetBlockPosition(pos.x, pos.z) };
	int blockWidth{ m_terrain->GetBlockWidth() };
	int blockLength{ m_terrain->GetBlockLength() };
	XMFLOAT3 scale{ m_terrain->GetScale() };

	auto CubicBezierSum = [](const array<XMFLOAT3, 25>& patch, XMFLOAT2 t) {

		// 4�� ������ � ���
		array<float, 5> uB, vB;
		float txInv{ 1.0f - t.x };
		uB[0] = txInv * txInv * txInv * txInv;
		uB[1] = 4.0f * t.x * txInv * txInv * txInv;
		uB[2] = 6.0f * t.x * t.x * txInv * txInv;
		uB[3] = 4.0f * t.x * t.x * t.x * txInv;
		uB[4] = t.x * t.x * t.x * t.x;

		float tyInv{ 1.0f - t.y };
		vB[0] = tyInv * tyInv * tyInv * tyInv;
		vB[1] = 4.0f * t.y * tyInv * tyInv * tyInv;
		vB[2] = 6.0f * t.y * t.y * tyInv * tyInv;
		vB[3] = 4.0f * t.y * t.y * t.y * tyInv;
		vB[4] = t.y * t.y * t.y * t.y;

		// 4�� ������ ��� ���
		XMFLOAT3 sum{ 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < 5; ++i)
		{
			XMFLOAT3 subSum{ 0.0f, 0.0f, 0.0f };
			for (int j = 0; j < 5; ++j)
			{
				XMFLOAT3 temp{ Vector3::Mul(patch[(i * 5) + j], uB[j]) };
				subSum = Vector3::Add(subSum, temp);
			}
			subSum = Vector3::Mul(subSum, vB[i]);
			sum = Vector3::Add(sum, subSum);
		}
		return sum;
	};

	// ������ ��� ������ 25��
	array<XMFLOAT3, 25> vertices;
	for (int i = 0, z = 4; z >= 0; --z)
		for (int x = 0; x < 5; ++x)
		{
			vertices[i].x = LB.x + (x * blockWidth / 4 * scale.x);
			vertices[i].z = LB.z + (z * blockLength / 4 * scale.z);
			vertices[i].y = m_terrain->GetHeight(vertices[i].x, vertices[i].z);
			++i;
		}

	// �÷��̾��� ��ġ t
	XMFLOAT2 uv{ (pos.x - LB.x) / (blockWidth * scale.x), 1.0f - ((pos.z - LB.z) / (blockLength * scale.z)) };
	XMFLOAT3 posOnBazier{ CubicBezierSum(vertices, uv) };
	SetPosition(XMFLOAT3{ pos.x, posOnBazier.y, pos.z });
}

void Player::SetPlayerNormalAndLook()
{
	XMFLOAT3 pos{ GetPosition() };

	// �븻 ����
	m_normal = m_terrain->GetNormal(pos.x, pos.z);

	// �� ����
	if (float theta = acosf(Vector3::Dot(XMFLOAT3{ 0.0f, 1.0f, 0.0f }, m_normal)))
	{
		XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(XMFLOAT3{ 0.0f, 1.0f, 0.0f }, m_normal)) };
		XMFLOAT4X4 rotate; XMStoreFloat4x4(&rotate, XMMatrixRotationNormal(XMLoadFloat3(&right), theta));
		m_look = Vector3::TransformNormal(GetFront(), rotate);
		right = Vector3::Normalize(Vector3::Cross(m_normal, m_look));

		m_worldMatrix._11 = right.x;	m_worldMatrix._12 = right.y;	m_worldMatrix._13 = right.z;
		m_worldMatrix._21 = m_normal.x;	m_worldMatrix._22 = m_normal.y;	m_worldMatrix._23 = m_normal.z;
		m_worldMatrix._31 = m_look.x;	m_worldMatrix._32 = m_look.y;	m_worldMatrix._33 = m_look.z;
	}
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