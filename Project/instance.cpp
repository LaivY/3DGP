#include "instance.h"

Instance::Instance(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, UINT count) : m_count{ count }
{
	// 벡터 공간 미리 할당
	m_gameObjects.reserve(m_count);

	// 버퍼 생성
	ComPtr<ID3D12Resource> dumy;
	m_instanceBuffer = CreateBufferResource(device, commandList, nullptr, sizeof(InstanceData), count, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, dumy);
	m_instanceBuffer->Map(0, NULL, reinterpret_cast<void**>(&m_instanceBufferPointer));

	// 버퍼 뷰 설정
	m_instanceBufferView.BufferLocation = m_instanceBuffer->GetGPUVirtualAddress();
	m_instanceBufferView.SizeInBytes = sizeof(InstanceData) * m_count;
	m_instanceBufferView.StrideInBytes = sizeof(InstanceData);
}

Instance::~Instance()
{
	if (m_instanceBuffer) m_instanceBuffer->Unmap(0, NULL);
}

void Instance::Update(FLOAT deltaTime)
{
	for (auto& object : m_gameObjects)
		object->Update(deltaTime);
}

void Instance::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (!m_mesh) return;

	if (m_shader) commandList->SetPipelineState(m_shader->GetPipelineState().Get());
	if (m_texture) m_texture->UpdateShaderVariable(commandList);
	for (int i = 0; i < m_gameObjects.size(); ++i)
	{
		if (m_gameObjects[i]->GetType() == GameObjectType::BILLBOARD)
		{
			BillboardObject* object{ reinterpret_cast<BillboardObject*>(m_gameObjects[i].get()) };
			XMFLOAT4X4 objectWorldMatrix{ object->GetWorldMatrix() };
			XMFLOAT3 offset{ object->GetOffset() };
			objectWorldMatrix._41 += offset.x;
			objectWorldMatrix._42 += offset.y;
			objectWorldMatrix._43 += offset.z;
			m_instanceBufferPointer[i].worldMatrix = Matrix::Transpose(objectWorldMatrix);
		}
		else m_instanceBufferPointer[i].worldMatrix = Matrix::Transpose(m_gameObjects[i]->GetWorldMatrix());
	}
	m_mesh->Render(commandList, m_instanceBufferView, m_count);
}

void Instance::AddGameObject(unique_ptr<GameObject> gameObject)
{
	if (m_gameObjects.size() < m_count)
		m_gameObjects.push_back(move(gameObject));
}

void Instance::SetMesh(const shared_ptr<Mesh>& mesh)
{
	if (m_mesh) m_mesh.reset();
	m_mesh = mesh;
}

void Instance::SetShader(const shared_ptr<Shader>& shader)
{
	if (m_shader) m_shader.reset();
	m_shader = shader;
}

void Instance::SetTexture(const shared_ptr<Texture>& texture)
{
	if (m_texture) m_texture.reset();
	m_texture = texture;
}