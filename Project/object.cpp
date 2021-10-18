#include "object.h"
#include "camera.h"

GameObject::GameObject() : m_right{ 1.0f, 0.0f, 0.0f }, m_up{ 0.0f, 1.0f, 0.0f }, m_front{ 0.0f, 0.0f, 1.0f },
						   m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }, m_terrain{ nullptr }, m_normal{ 0.0f, 1.0f, 0.0f }
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

GameObject::~GameObject()
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
	if (m_texture) m_texture->ReleaseUploadBuffer();
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	// PSO 설정
	if (m_shader) commandList->SetPipelineState(m_shader->GetPipelineState().Get());

	// 셰이더 변수 최신화
	UpdateShaderVariable(commandList);
	if (m_texture) m_texture->UpdateShaderVariable(commandList);

	// 렌더링
	if (m_mesh) m_mesh->Render(commandList);
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
	XMFLOAT4X4 worldMatrix{ m_worldMatrix };
	commandList->SetGraphicsRoot32BitConstants(0, 16, &Matrix::Transpose(worldMatrix), 0);
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

XMFLOAT3 GameObject::GetPosition() const
{
	return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
}

void GameObject::ReleaseUploadBuffer() const
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
	if (m_texture) m_texture->ReleaseUploadBuffer();
}

// --------------------------------------

BillboardObject::BillboardObject(const shared_ptr<Camera>& camera) : GameObject{}, m_camera{ camera }
{

}

void BillboardObject::Update(FLOAT deltaTime)
{
	XMFLOAT3 pos{ GetPosition() };			// 빌보드 객체의 위치
	XMFLOAT3 target{ m_camera->GetEye() };	// 봐야할 곳

	XMFLOAT3 up{ GetUp() };
	XMFLOAT3 look{ Vector3::Normalize(Vector3::Sub(target, pos)) };
	XMFLOAT3 right{ Vector3::Normalize(Vector3::Cross(up, look)) };

	m_worldMatrix._11 = right.x;	m_worldMatrix._12 = right.y;	m_worldMatrix._13 = right.z;
	m_worldMatrix._21 = up.x;		m_worldMatrix._22 = up.y;		m_worldMatrix._23 = up.z;
	m_worldMatrix._31 = look.x;		m_worldMatrix._32 = look.y;		m_worldMatrix._33 = look.z;
	m_worldMatrix._41 = pos.x;		m_worldMatrix._42 = pos.y;		m_worldMatrix._43 = pos.z;

	if (m_terrain)
	{
		XMFLOAT3 pos{ GetPosition() };
		FLOAT height{ m_terrain->GetHeight(pos.x, pos.z) };
		SetPosition(XMFLOAT3{ pos.x, height + 5.0f, pos.z });
		m_normal = m_terrain->GetNormal(pos.x, pos.z);
	}
}

void BillboardObject::SetCamera(const shared_ptr<Camera>& camera)
{
	if (m_camera) m_camera.reset();
	m_camera = camera;
}