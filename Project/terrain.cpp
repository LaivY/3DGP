#include "terrain.h"
#include "object.h"

HeightMapImage::HeightMapImage(const wstring& fileName, INT width, INT length, XMFLOAT3 scale)
	: m_width{ width }, m_length{ length }, m_scale{ scale }, m_pixels{ new BYTE[width * length] }
{
	// ���� �б�
	unique_ptr<BYTE[]> buffer{ new BYTE[m_width * m_length] };
	HANDLE hFile{ CreateFile(fileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL) };
	DWORD bytesRead;
	ReadFile(hFile, buffer.get(), m_width* m_length, &bytesRead, NULL);
	CloseHandle(hFile);

	// ���̸� �̹����� �»���� (0, 0)�̰� �츮�� ���ϴ� ��ǥ��� ���ϴ��� (0, 0)�̹Ƿ� ���ϴ�Ī ���Ѽ� �����Ѵ�.
	for (int y = 0; y < m_length; ++y)
		for (int x = 0; x < m_width; ++x)
			m_pixels[x + (y * m_width)] = buffer[x + ((m_length - y - 1) * m_width)];
}

XMFLOAT3 HeightMapImage::GetNormal(INT x, INT z) const
{
	// x, z��ǥ�� �̹����� ������ ��� ��� +y���� ��ȯ
	if (x < 0 || x >= m_width || z < 0 || z >= m_length)
		return XMFLOAT3{ 0.0f, 1.0f, 0.0f };

	// P1(x, z), P2(x+1, z), P3(x, z+1) �� 3���� �̿��ؼ� ���� ���͸� ����Ѵ�.
	int index{ x + z * m_width };
	int xAdd{ x < m_width - 1 ? 1 : -1 };				// x�� ���� ������ �ȼ��� ��� (x-1, z)�� �̿�
	int yAdd{ z < m_length - 1 ? m_width : -m_width };	// z�� ���� ���� ��� (x, z-1)�� �̿�

	BYTE* pixels{ m_pixels.get() };
	float y1{ pixels[index] * m_scale.y };			// P1�� y��
	float y2{ pixels[index + xAdd] * m_scale.y };	// P2�� y��
	float y3{ pixels[index + yAdd] * m_scale.y };	// P3�� y��

	XMFLOAT3 P1P2{ m_scale.x, y2 - y1, 0.0f }; // P1 -> P2 ����
	XMFLOAT3 P1P3{ 0.0f, y3 - y1, m_scale.z }; // P1 -> P3 ����

	// �� ���͸� ������ ���� ���� �����̴�.
	return Vector3::Normalize(Vector3::Cross(P1P3, P1P2));
}

FLOAT HeightMapImage::GetHeight(FLOAT x, FLOAT z) const
{
	// x, z��ǥ�� �̹����� ������ ��� ��� 0�� ��ȯ
	if (x < 0 || x >= m_width || z < 0 || z >= m_length)
		return 0.0f;

	int ix{ static_cast<int>(x) };	// x�� ���� �κ�
	int iz{ static_cast<int>(z) };	// z�� ���� �κ�
	float fx{ x - ix };	// x�� �Ҽ� �κ�
	float fz{ z - iz };	// z�� �Ҽ� �κ�

	BYTE* pixels{ m_pixels.get() };
	float LT{ (float)pixels[ix + ((iz + 1) * m_width)] };		// �»�� ����
	float RT{ (float)pixels[(ix + 1) + ((iz + 1) * m_width)] };	// ���� ����
	float LB{ (float)pixels[ix + (iz * m_width)] };				// ���ϴ� ����
	float RB{ (float)pixels[(ix + 1) + (iz * m_width)] };		// ���ϴ� ����

	// �簢���� �� ���� �����Ͽ� ���� ���� ��ȯ
	float topHeight{ LT * (1 - fx) + RT * fx };		// ������ ��� ����
	float botHeight{ LB * (1 - fx) + RB * fx };		// ������ �ϴ� ����
	return botHeight * (1 - fz) + topHeight * fz;	// ������ ���� ����
}

// --------------------------------------

HeightMapGridMesh::HeightMapGridMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	HeightMapImage* heightMapImage, INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale)
{
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	// ���� ������ ����, ���� ���� ����
	vector<Texture2Vertex> vertices;
	for (int z = zStart; z < zStart + length; ++z)
		for (int x = xStart; x < xStart + width; ++x)
			vertices.emplace_back(
				XMFLOAT3{ x * scale.x, heightMapImage->GetHeight(x, z) * scale.y, z * scale.z },
				XMFLOAT2{ (float)x / (float)heightMapImage->GetWidth(), 1.0f - ((float)z / (float)heightMapImage->GetLength()) },
				XMFLOAT2{ (float)x / (float)scale.x * 1.5f, (float)z / (float)scale.z * 1.5f }
			);
	CreateVertexBuffer(device, commandList, vertices.data(), sizeof(Texture2Vertex), vertices.size());

	// �ε��� ������ ����, �ε��� ���� ����
	vector<UINT> indices;
	for (int z = 0; z < length - 1; ++z) // ������ �� ° ���� �� �ʿ� ����
	{
		// Ȧ�� ��° �� (z = 0, 2, 4, 6, ...) �ε��� ���� ������ ���� -> ������
		if (z % 2 == 0)
			for (int x = 0; x < width; ++x)
			{
				// ù��° ���� �ƴϰ� ���� �ٲ� �� (x, z)�� �߰�
				if (x == 0 && z > 0) indices.push_back(x + (z * width));
				indices.push_back(x + (z * width));			// (x, z)
				indices.push_back(x + (z * width) + width);	// (x, z+1)
			}

		// ¦�� ��° �� (z = 1, 3, 5, 7, ...) �ε��� ���� ������ ���� <- ������
		else
			for (int x = width - 1; x >= 0; --x)
			{
				// ���� �ٲ� �� (x, z)�� �߰�
				if (x == width - 1) indices.push_back(x + (z * width));
				indices.push_back(x + (z * width));			// (x, z)
				indices.push_back(x + (z * width) + width);	// (x, z+1)
			}
	}
	CreateIndexBuffer(device, commandList, indices.data(), indices.size());
}

// --------------------------------------

HeightMapGridTessMesh::HeightMapGridTessMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	HeightMapImage* heightMapImage, INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale)
{
	// �ε��� ����
	m_nIndices = 0;

	// �� ���� ���� 25���� �̷��������
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;

	// ���� ���� ����, ���� �Ÿ�
	int widthStride{ width / 4 };
	int lengthStride{ length / 4 };

	float heightMapImageWidth{ static_cast<float>(heightMapImage->GetWidth()) };
	float heightMapImageLength{ static_cast<float>(heightMapImage->GetLength()) };

	// (-x, +z)(=�������)�������� (+x, -z)(=�����ϴ�)����
	vector<Texture2Vertex> vertices;
	for (int z = zStart + length; z >= zStart; z -= lengthStride)
		for (int x = xStart; x <= xStart + width; x += widthStride)
			vertices.emplace_back(
				XMFLOAT3{ x * scale.x, heightMapImage->GetHeight(x, z) * scale.y, z * scale.z },
				XMFLOAT2{ (float)x / heightMapImageWidth, 1.0f - ((float)z / heightMapImageLength) },
				XMFLOAT2{ (float)x / scale.x * 1.5f, (float)z / scale.z * 1.5f }
			);

	CreateVertexBuffer(device, commandList, vertices.data(), sizeof(Texture2Vertex), vertices.size());
}
// --------------------------------------

HeightMapTerrain::HeightMapTerrain(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const wstring& fileName, const shared_ptr<Shader>& shader, const shared_ptr<Texture>& texture, INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale)
	: m_width{ width }, m_length{ length }, m_blockWidth{ blockWidth }, m_blockLength{ blockLength }, m_scale{ scale }
{
	// ���̸��̹��� �ε�
	m_heightMapImage = make_unique<HeightMapImage>(fileName, m_width, m_length, m_scale);

	// ����, ���� ����� ����
	int widthBlockCount{ m_width / m_blockWidth };
	int lengthBlockCount{ m_length / m_blockLength };

	// ��� ����
	for (int z = 0; z < lengthBlockCount; ++z)
		for (int x = 0; x < widthBlockCount; ++x)
		{
			int xStart{ x * m_blockWidth };
			int zStart{ z * m_blockLength };
			unique_ptr<GameObject> block{ make_unique<GameObject>() };
			shared_ptr<HeightMapGridTessMesh> mesh{
				make_shared<HeightMapGridTessMesh>(device, commandList, m_heightMapImage.get(), xStart, zStart, m_blockWidth, m_blockLength, m_scale)
			};
			block->SetMesh(mesh);
			block->SetShader(shader);
			block->SetTexture(texture);
			m_blocks.push_back(move(block));
		}
}

void HeightMapTerrain::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	for (const auto& block : m_blocks)
		block->Render(commandList);
}

void HeightMapTerrain::Move(const XMFLOAT3& shift)
{
	for (auto& block : m_blocks)
		block->Move(shift);
}

void HeightMapTerrain::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	for (auto& block : m_blocks)
		block->Rotate(roll, pitch, yaw);
}

void HeightMapTerrain::SetPosition(const XMFLOAT3& position)
{
	for (auto& block : m_blocks)
		block->SetPosition(position);
}

void HeightMapTerrain::SetShader(const shared_ptr<Shader>& shader)
{
	for (auto& block : m_blocks)
		block->SetShader(shader);
}

FLOAT HeightMapTerrain::GetHeight(FLOAT x, FLOAT z) const
{
	// �Ķ���ͷ� ���� (x, z)�� �÷��̾��� ��ġ�̴�.
	// (x, z)�� �����ϴ� �̹��� ��ǥ�� �ٲ�����Ѵ�.

	// ������ ������ �ݿ�
	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x;
	z -= pos.z;

	// ������ ������ �ݿ�
	x /= m_scale.x;
	z /= m_scale.z;

	return pos.y + m_heightMapImage->GetHeight(x, z) * m_scale.y;
}

XMFLOAT3 HeightMapTerrain::GetNormal(FLOAT x, FLOAT z) const
{
	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x; x /= m_scale.x;
	z -= pos.z; z /= m_scale.z;

	// (x, z) �ֺ��� ��� 4���� �����ؼ� ���
	int ix{ static_cast<int>(x) };
	int iz{ static_cast<int>(z) };
	float fx{ x - ix };
	float fz{ z - iz };

	XMFLOAT3 LT{ m_heightMapImage->GetNormal(ix, iz + 1) };
	XMFLOAT3 LB{ m_heightMapImage->GetNormal(ix, iz) };
	XMFLOAT3 RT{ m_heightMapImage->GetNormal(ix + 1, iz + 1) };
	XMFLOAT3 RB{ m_heightMapImage->GetNormal(ix + 1, iz) };

	XMFLOAT3 bot{ Vector3::Add(Vector3::Mul(LB, 1.0f - fx), Vector3::Mul(RB, fx)) };
	XMFLOAT3 top{ Vector3::Add(Vector3::Mul(LT, 1.0f - fx), Vector3::Mul(RT, fx)) };
	return Vector3::Normalize(Vector3::Add(Vector3::Mul(bot, 1.0f - fz), Vector3::Mul(top, fz)));
}

XMFLOAT3 HeightMapTerrain::GetPosition() const
{
	return m_blocks.front()->GetPosition();
}

XMFLOAT3 HeightMapTerrain::GetBlockPosition(FLOAT x, FLOAT z)
{
	// (x, z) ��ǥ�� �����ϴ� ����� �����ϴ� ��ǥ�� ��ȯ�Ѵ�. y���� �׻� 0�̴�.
	XMFLOAT3 pos{ GetPosition() };
	int bx{ static_cast<int>((x - pos.x) / m_blockWidth) };
	int bz{ static_cast<int>((z - pos.z) / m_blockLength) };
	return { pos.x + bx * m_blockWidth, 0.0f, pos.z + bz * m_blockLength };
}