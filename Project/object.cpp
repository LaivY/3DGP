#include "object.h"
#include "camera.h"

GameObject::GameObject() : m_type{ GameObjectType::DEFAULT }, m_isDeleted{ false }, m_right{ 1.0f, 0.0f, 0.0f }, m_up{ 0.0f, 1.0f, 0.0f }, m_front{ 0.0f, 0.0f, 1.0f },
						   m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }, m_terrain{ nullptr }, m_normal{ 0.0f, 1.0f, 0.0f }, m_look{ 0.0f, 0.0f, 1.0f }, m_textureInfo{ nullptr }
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, const shared_ptr<Shader>& shader) const
{
	// PSO 설정
	if (shader) commandList->SetPipelineState(shader->GetPipelineState().Get());
	else if (m_shader) commandList->SetPipelineState(m_shader->GetPipelineState().Get());

	// 셰이더 변수 최신화
	UpdateShaderVariable(commandList);

	// 텍스쳐
	if (m_texture)
	{
		if (m_textureInfo) m_texture->SetTextureInfo(m_textureInfo.get());
		m_texture->UpdateShaderVariable(commandList);
	}

	// 메쉬 렌더링
	if (m_mesh) m_mesh->Render(commandList);
}

void GameObject::Update(FLOAT deltaTime)
{
	if (!m_texture || !m_textureInfo)
		return;

	m_textureInfo->frameTimer += deltaTime;
	if (m_textureInfo->frameTimer > m_textureInfo->frameInterver)
	{
		m_textureInfo->frame += static_cast<int>(m_textureInfo->frameTimer / m_textureInfo->frameInterver);
		m_textureInfo->frameTimer = fmod(m_textureInfo->frameTimer, m_textureInfo->frameInterver);
	}

	if (m_textureInfo->frame >= m_texture->GetTextureCount())
	{
		if (m_textureInfo->isFrameRepeat) m_textureInfo->frame = 0;
		else
		{
			m_textureInfo->frame = m_texture->GetTextureCount() - 1;
			m_isDeleted = true;
		}
	}
}

void GameObject::Move(const XMFLOAT3& shift)
{
	SetPosition(Vector3::Add(GetPosition(), shift));
}

void GameObject::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	// 회전
	XMMATRIX rotate{ XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)) };
	XMMATRIX worldMatrix{ rotate * XMLoadFloat4x4(&m_worldMatrix) };
	XMStoreFloat4x4(&m_worldMatrix, worldMatrix);

	// 로컬 x,y,z축 최신화
	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotate));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotate));
	XMStoreFloat3(&m_front, XMVector3TransformNormal(XMLoadFloat3(&m_front), rotate));
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	// 게임오브젝트의 월드 변환 행렬 최신화
	commandList->SetGraphicsRoot32BitConstants(0, 16, &Matrix::Transpose(m_worldMatrix), 0);
}

void GameObject::SetPosition(const XMFLOAT3& position)
{
	m_worldMatrix._41 = position.x;
	m_worldMatrix._42 = position.y;
	m_worldMatrix._43 = position.z;
}

void GameObject::SetMesh(const shared_ptr<Mesh>& mesh)
{
	if (m_mesh) m_mesh.reset();
	m_mesh = mesh;
}

void GameObject::SetShader(const shared_ptr<Shader>& shader)
{
	if (m_shader) m_shader.reset();
	m_shader = shader;
}

void GameObject::SetTexture(const shared_ptr<Texture>& texture)
{
	if (m_texture) m_texture.reset();
	m_texture = texture;
}

void GameObject::SetTextureInfo(unique_ptr<TextureInfo>& textureInfo)
{
	m_textureInfo = move(textureInfo);
}

XMFLOAT3 GameObject::GetPosition() const
{
	return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
}

// --------------------------------------

BillboardObject::BillboardObject(const shared_ptr<Camera>& camera, const XMFLOAT3& offset) : GameObject{}, m_camera{ camera }, m_offset{ offset }
{
	m_type == GameObjectType::BILLBOARD;
}

void BillboardObject::Update(FLOAT deltaTime)
{
	GameObject::Update(deltaTime);

	XMFLOAT3 target{ m_camera->GetEye() };	// 봐야할 곳
	XMFLOAT3 pos{ GetPosition() };			// 객체의 위치

	XMFLOAT3 up{ GetUp() };
	XMFLOAT3 look{ Vector3::Normalize(Vector3::Sub(target, pos)) };
	XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(up, look)) };

	m_worldMatrix._11 = right.x;	m_worldMatrix._12 = right.y;	m_worldMatrix._13 = right.z;
	m_worldMatrix._21 = up.x;		m_worldMatrix._22 = up.y;		m_worldMatrix._23 = up.z;
	m_worldMatrix._31 = look.x;		m_worldMatrix._32 = look.y;		m_worldMatrix._33 = look.z;
	m_worldMatrix._41 = pos.x;		m_worldMatrix._42 = pos.y;		m_worldMatrix._43 = pos.z;
}

void BillboardObject::SetCamera(const shared_ptr<Camera>& camera)
{
	if (m_camera) m_camera.reset();
	m_camera = camera;
}

// --------------------------------------

Bullet::Bullet(const XMFLOAT3& position, const XMFLOAT3& direction, const XMFLOAT3& up, FLOAT speed, FLOAT damage)
	: m_origin{ position }, m_direction{ direction }, m_speed{ speed }, m_damage{ damage }
{
	m_type = GameObjectType::BULLET;

	SetPosition(position);

	XMFLOAT3 look{ Vector3::Normalize(m_direction) };
	XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(up, look)) };
	m_worldMatrix._11 = right.x;	m_worldMatrix._12 = right.y;	m_worldMatrix._13 = right.z;
	m_worldMatrix._21 = up.x;		m_worldMatrix._22 = up.y;		m_worldMatrix._23 = up.z;
	m_worldMatrix._31 = look.x;		m_worldMatrix._32 = look.y;		m_worldMatrix._33 = look.z;
}

void Bullet::Update(FLOAT deltaTime)
{
	GameObject::Update(deltaTime);

	// 일정 거리 날아가면 삭제
	if (Vector3::Length(Vector3::Sub(GetPosition(), m_origin)) > 100.0f)
		m_isDeleted = true;

	// 지형에 닿으면 삭제
	if (m_terrain)
	{
		XMFLOAT3 position{ GetPosition() };
		if (position.y < m_terrain->GetHeight(position.x, position.z))
			m_isDeleted = true;
	}

	// 삭제될 객체는 업데이트할 필요 없음
	if (m_isDeleted) return;

	// 총알 진행 방향으로 이동
	Move(Vector3::Mul(m_direction, m_speed * deltaTime));
}