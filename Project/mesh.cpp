#include "mesh.h"

Mesh::Mesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	void* vertexData, UINT sizePerVertexData, UINT vertexDataCount, void* indexData, UINT indexDataCount, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
	: m_nVertices{ vertexDataCount }, m_nIndices{ indexDataCount }, m_primitiveTopology{ primitiveTopology }
{
	if (vertexData) CreateVertexBuffer(device, commandList, vertexData, sizePerVertexData, vertexDataCount);
	if (indexData) CreateIndexBuffer(device, commandList, indexData, indexDataCount);
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const
{
	m_commandList->IASetPrimitiveTopology(m_primitiveTopology);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	if (m_nIndices)
	{
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else m_commandList->DrawInstanced(m_nVertices, 1, 0, 0);
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, const D3D12_VERTEX_BUFFER_VIEW& instanceBufferView, UINT count) const
{
	commandList->IASetPrimitiveTopology(m_primitiveTopology);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetVertexBuffers(1, 1, &instanceBufferView);
	if (m_nIndices)
	{
		commandList->IASetIndexBuffer(&m_indexBufferView);
		commandList->DrawIndexedInstanced(m_nIndices, count, 0, 0, 0);
	}
	else commandList->DrawInstanced(m_nVertices, count, 0, 0);
}

void Mesh::CreateVertexBuffer(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, void* data, UINT sizePerData, UINT dataCount)
{
	// ���� ���� ���� ����
	m_nVertices = dataCount;

	// ���� ���� ����
	m_vertexBuffer = CreateBufferResource(device, commandList, data, sizePerData, dataCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_vertexUploadBuffer);

	// ���� ���� �� ����
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizePerData * dataCount;
	m_vertexBufferView.StrideInBytes = sizePerData;
}

void Mesh::CreateIndexBuffer(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, void* data, UINT dataCount)
{
	// �ε��� ���� ���� ����
	m_nIndices = dataCount;

	// �ε��� ���� ����
	m_indexBuffer = CreateBufferResource(device, commandList, data, sizeof(UINT), dataCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_indexUploadBuffer);

	// �ε��� ���� �� ����
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * dataCount;
}

void Mesh::ReleaseUploadBuffer()
{
	if (m_vertexUploadBuffer) m_vertexUploadBuffer.Reset();
	if (m_indexUploadBuffer) m_indexUploadBuffer.Reset();
}

CubeMesh::CubeMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, FLOAT width, FLOAT length, FLOAT height)
{
	m_nIndices = 0;
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// ť�� ����, ����, ����
	FLOAT sx{ width }, sy{ length }, sz{ height };

	// �ո�
	vector<TextureVertex> vertices;
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, -sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, -sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ -sx, +sy, -sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, -sz }, XMFLOAT2{ 0.0f, 1.0f });

	// �����ʸ�
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, -sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, +sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, +sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ +sx, +sy, -sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, +sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, -sz }, XMFLOAT2{ 0.0f, 1.0f });

	// ���ʸ�
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, -sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ -sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, +sz }, XMFLOAT2{ 0.0f, 1.0f });

	// �޸�
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, +sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, +sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ +sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, +sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, +sz }, XMFLOAT2{ 0.0f, 1.0f });

	// ����
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, +sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ -sx, +sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, +sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, +sy, -sz }, XMFLOAT2{ 0.0f, 1.0f });

	// �ظ�
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, +sz }, XMFLOAT2{ 1.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ +sx, -sy, +sz }, XMFLOAT2{ 0.0f, 0.0f });
	vertices.emplace_back(XMFLOAT3{ -sx, -sy, -sz }, XMFLOAT2{ 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +sx, -sy, -sz }, XMFLOAT2{ 0.0f, 1.0f });

	CreateVertexBuffer(device, commandList, vertices.data(), sizeof(TextureVertex), vertices.size());
}

TextureRectMesh::TextureRectMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, FLOAT width, FLOAT length, FLOAT height, XMFLOAT3 position)
{
	m_nIndices = 0;
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vector<TextureVertex> vertices;
	FLOAT hx{ position.x + width / 2.0f }, hy{ position.y + height / 2.0f }, hz{ position.z + length / 2.0f };
	
	if (width == 0.0f)
	{
		// �б��ϴ� ������ ��ġ�� ���� �ﰢ�� ���ε� ������ �޶����� ������
		// YZ���
		if (position.x > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 1.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, -hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2

			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, -hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 0.0f, 1.0f }); // 3
		}
		else
		{
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 0.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 1.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 3

			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 0.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 3
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, -hz }, XMFLOAT2{ 0.0f, 1.0f }); // 2
		}
	}
	else if (length == 0.0f)
	{
		// XY���
		if (position.z > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 1.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2

			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2
			vertices.emplace_back(XMFLOAT3{ -hx, -hy, +hz }, XMFLOAT2{ 0.0f, 1.0f }); // 3
		}
		else
		{
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 1.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ -hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 3

			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ -hx, -hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 3
			vertices.emplace_back(XMFLOAT3{ +hx, -hy, +hz }, XMFLOAT2{ 0.0f, 1.0f }); // 2
		}
	}
	else if (height == 0.0f)
	{
		// XZ���
		if (position.y > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, -hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 1.0f, 0.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2

			vertices.emplace_back(XMFLOAT3{ -hx, +hy, -hz }, XMFLOAT2{ 0.0f, 0.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 1.0f, 1.0f }); // 2
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 0.0f, 1.0f }); // 3
		}
		else
		{
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 1.0f, 1.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, -hz }, XMFLOAT2{ 0.0f, 1.0f }); // 0
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 3

			vertices.emplace_back(XMFLOAT3{ +hx, +hy, -hz }, XMFLOAT2{ 1.0f, 1.0f }); // 1
			vertices.emplace_back(XMFLOAT3{ -hx, +hy, +hz }, XMFLOAT2{ 0.0f, 0.0f }); // 3
			vertices.emplace_back(XMFLOAT3{ +hx, +hy, +hz }, XMFLOAT2{ 1.0f, 0.0f }); // 2
		}
	}

	CreateVertexBuffer(device, commandList, vertices.data(), sizeof(TextureVertex), vertices.size());
}