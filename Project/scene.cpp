#include "scene.h"

void ResourceManager::ReleaseUploadBuffer() const
{
	for (auto [_, mesh] : m_meshes)
		mesh->ReleaseUploadBuffer();
	for (auto [_, texture] : m_textures)
		texture->ReleaseUploadBuffer();
}

shared_ptr<Mesh> ResourceManager::GetMesh(const string& key) const
{
	auto value{ m_meshes.find(key) };
	if (value == m_meshes.end())
		return nullptr;
	return value->second;
}

shared_ptr<Shader> ResourceManager::GetShader(const string& key) const
{
	auto value{ m_shaders.find(key) };
	if (value == m_shaders.end())
		return nullptr;
	return value->second;
}

shared_ptr<Texture> ResourceManager::GetTexture(const string& key) const
{
	auto value{ m_textures.find(key) };
	if (value == m_textures.end())
		return nullptr;
	return value->second;
}

// --------------------------------------

void Scene::OnInit(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12RootSignature>& rootSignature, FLOAT aspectRatio)
{
	// �޽�, ���̴�, �ؽ��ĵ��� ��� ResourceManager�� �ִ� map�� ��´�.
	// ResourceManager::Add??? �Լ��� ���� ���ҽ��� �߰��ϰ� ResourceManager::Get??? �Լ��� ���� ���ҽ��� �ҷ��� �� �ִ�.
	// class ResourceManager�� �����Ƿμ� ���߿� ��ü�� ������ �� �ʿ��� Mesh, Shader, Texture�� �̸� �����ص� �� �ִ�.

	// ���ҽ��� ������� ResourceManager ����
	m_resourceManager = make_unique<ResourceManager>();

	// �ʿ��� �޽��� ����
	shared_ptr<CubeMesh> cubeMesh{ make_shared<CubeMesh>(device, commandList, 0.5f, 0.5f, 0.5f) };
	shared_ptr<CubeMesh> bulletMesh{ make_shared<CubeMesh>(device, commandList, 0.1f, 0.5f, 0.1f) };
	shared_ptr<TextureRectMesh> textureRectMesh{ make_shared<TextureRectMesh>(device, commandList, 10.0f, 0.0f, 10.0f, XMFLOAT3{}) };
	shared_ptr<Mesh> tankMesh{ make_shared<Mesh>(device, commandList, "resource/tank2.obj") };

	// �ʿ��� ���̴��� ����
	shared_ptr<Shader> colorShader{ make_shared<Shader>(device, rootSignature) };
	shared_ptr<TextureShader> textureShader{ make_shared<TextureShader>(device, rootSignature) };
	shared_ptr<TerrainShader> terrainShader{ make_shared<TerrainShader>(device, rootSignature) };
	shared_ptr<InstanceShader> instanceShader{ make_shared<InstanceShader>(device, rootSignature) };

	// �ʿ��� �ؽ��ĵ� ����
	shared_ptr<Texture> rockTexture{ make_shared<Texture>() };
	rockTexture->LoadTextureFile(device, commandList, TEXT("resource/rock.dds"), 2);
	rockTexture->CreateSrvDescriptorHeap(device);
	rockTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> treeTexture{ make_shared<Texture>() };
	treeTexture->LoadTextureFile(device, commandList, TEXT("resource/tree.dds"), 2);
	treeTexture->CreateSrvDescriptorHeap(device);
	treeTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> terrainTexture{ make_shared<Texture>() };
	terrainTexture->LoadTextureFile(device, commandList, TEXT("resource/BaseTerrain.dds"), 2); // BaseTexture
	terrainTexture->LoadTextureFile(device, commandList, TEXT("resource/DetailTerrain.dds"), 3); // DetailTexture
	terrainTexture->CreateSrvDescriptorHeap(device);
	terrainTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> explosionTexture{ make_shared<Texture>() };
	for (int i = 1; i <= 33; ++i)
		explosionTexture->LoadTextureFile(device, commandList, TEXT("resource/explosion (") + to_wstring(i) + TEXT(").dds"), 2);
	explosionTexture->CreateSrvDescriptorHeap(device);
	explosionTexture->CreateShaderResourceView(device);

	// ���ҽ� ���� ��ü�� ����
	m_resourceManager->AddMesh("CUBE_MESH", cubeMesh);
	m_resourceManager->AddMesh("BULLET_MESH", bulletMesh);
	m_resourceManager->AddMesh("TEXTURE_RECT_MESH", textureRectMesh);
	m_resourceManager->AddMesh("TANK_MESH", tankMesh);

	m_resourceManager->AddShader("COLOR_SHADER", colorShader);
	m_resourceManager->AddShader("TEXTURE_SHADER", textureShader);
	m_resourceManager->AddShader("TERRAIN_SHADER", terrainShader);
	m_resourceManager->AddShader("INSTANCE_SHADER", instanceShader);

	m_resourceManager->AddTexture("ROCK_TEXTURE", rockTexture);
	m_resourceManager->AddTexture("TREE_TEXTURE", treeTexture);
	m_resourceManager->AddTexture("TERRAIN_TEXTURE", terrainTexture);
	m_resourceManager->AddTexture("EXPLOSION_TEXTURE", explosionTexture);

	// ���� ����
	unique_ptr<HeightMapTerrain> terrain{
		make_unique<HeightMapTerrain>(device, commandList, TEXT("resource/heightMap.raw"), terrainShader, terrainTexture, 257, 257, 257, 257, XMFLOAT3{ 1.0f, 0.2f, 1.0f })
	};
	terrain->SetPosition(XMFLOAT3{ 0.0f, -300.0f, 0.0f });
	m_terrains.push_back(move(terrain));

	// ī�޶� ����
	shared_ptr<ThirdPersonCamera> camera{ make_shared<ThirdPersonCamera>() };
	camera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	camera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	camera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	SetCamera(camera);

	// ī�޶� ���� ��� ����
	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 0.1f, 3000.0f));
	camera->SetProjMatrix(projMatrix);

	// ��ī�̹ڽ� ����
	unique_ptr<Skybox> skybox{ make_unique<Skybox>(device, commandList, rootSignature) };
	skybox->SetCamera(camera);
	SetSkybox(skybox);

	// �÷��̾� ����
	shared_ptr<Player> player{ make_shared<Player>() };
	player->SetMesh(m_resourceManager->GetMesh("TANK_MESH"));
	player->SetShader(m_resourceManager->GetShader("COLOR_SHADER"));
	player->SetTexture(m_resourceManager->GetTexture("ROCK_TEXTURE"));
	player->SetCamera(camera);

	// ��, ī�޶� �÷��̾� ����
	SetPlayer(player);
	camera->SetPlayer(m_player);

	// ������ ��ü 10000���� �ν��Ͻ����� ����
	unique_ptr<Instance> instance{ make_unique<Instance>(device, commandList, 100) };
	for (int i = 0; i < 10000; ++i)
	{
		float x{ static_cast<float>((i * 10) % 100) };
		float z{ (i / 10) * 10.0f };

		unique_ptr<BillboardObject> obj{ make_unique<BillboardObject>(camera) };
		obj->SetPosition(XMFLOAT3{ x, -270.0f, z });
		instance->AddGameObject(move(obj));
	}
	instance->SetMesh(m_resourceManager->GetMesh("TEXTURE_RECT_MESH"));
	instance->SetShader(m_resourceManager->GetShader("INSTANCE_SHADER"));
	instance->SetTexture(m_resourceManager->GetTexture("TREE_TEXTURE"));
	m_instances.push_back(move(instance));
}

void Scene::OnMouseEvent(HWND hWnd, UINT width, UINT height, FLOAT deltaTime)
{
	// ȭ�� ��� ��ǥ ���
	RECT rect; GetWindowRect(hWnd, &rect);
	POINT oldMousePosition{ rect.left + width / 2, rect.top + height / 2 };

	// ������ ���콺 ��ǥ
	POINT newMousePosition; GetCursorPos(&newMousePosition);

	// ������ ������ ����ؼ� ȸ��
	int dx = newMousePosition.x - oldMousePosition.x;
	int dy = newMousePosition.y - oldMousePosition.y;
	float sensitive{ 5.0f };
	if (m_player) m_player->Rotate(0.0f, dy * sensitive * deltaTime, dx * sensitive * deltaTime);

	// ���콺�� ȭ�� ����� �̵�
	SetCursorPos(oldMousePosition.x, oldMousePosition.y);
}

void Scene::OnMouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		CreateBullet();
		break;
	}
}

void Scene::OnKeyboardEvent(FLOAT deltaTime) const
{
	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetFront(), deltaTime * 10.0f));
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetRight(), deltaTime * -10.0f));
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetFront(), deltaTime * -10.0f));
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetRight(), deltaTime * 10.0f));
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetUp(), deltaTime * 10.0f));
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetUp(), deltaTime * -10.0f));
	}
}

void Scene::OnUpdate(FLOAT deltaTime)
{
	Update(deltaTime);
	if (m_player) m_player->Update(deltaTime);
	if (m_camera) m_camera->Update(deltaTime);
	if (m_skybox) m_skybox->Update();
	for (auto& object : m_gameObjects)
		object->Update(deltaTime);
	for (auto& instance : m_instances)
		instance->Update(deltaTime);
}

void Scene::Update(FLOAT deltaTime)
{
	RemoveDeletedGameObjects();
	UpdateGameObjectsTerrain();
}

void Scene::RemoveDeletedGameObjects()
{
	vector<unique_ptr<GameObject>> willBeAdded;

	// m_isDeleted�� true�� ��ü���� �����Ѵ�.
	auto pred = [this, &willBeAdded](unique_ptr<GameObject>& object) {

		// ������ ������Ʈ�� �Ѿ��̶�� �� �ڸ��� ��������Ʈ ����
		if (object->GetIsDeleted() && object->GetType() == GameObjectType::BULLET)
		{
			unique_ptr<BillboardObject> explosion{ make_unique<BillboardObject>(m_camera) };
			explosion->SetMesh(m_resourceManager->GetMesh("TEXTURE_RECT_MESH"));
			explosion->SetShader(m_resourceManager->GetShader("TEXTURE_SHADER"));
			explosion->SetTexture(m_resourceManager->GetTexture("EXPLOSION_TEXTURE"));
			unique_ptr<TextureInfo> textureInfo{ make_unique<TextureInfo>() };
			textureInfo->frameInterver *= 1.5f;
			textureInfo->isFrameRepeat = false;
			explosion->SetTextureInfo(textureInfo);
			explosion->SetPosition(object->GetPosition());
			explosion->SetCheckTerrain(false);
			willBeAdded.push_back(move(explosion));
		}

		return object->GetIsDeleted();
	};
	m_gameObjects.erase(remove_if(m_gameObjects.begin(), m_gameObjects.end(), pred), m_gameObjects.end());
	
	for (auto& instance : m_instances)
	{
		vector<unique_ptr<GameObject>>& gameObjects{ instance->GetGameObjects() };
		gameObjects.erase(remove_if(gameObjects.begin(), gameObjects.end(), pred), gameObjects.end());
	}

	for (auto& object : willBeAdded)
		m_gameObjects.push_back(move(object));
}

void Scene::UpdateGameObjectsTerrain()
{
	// ������ �������� �� �ִٴ� ���� ����߱� ������
	// ���ӿ�����Ʈ���� ��� ���� ���� �ִ��� �������ش�.

	XMFLOAT3 pos{};
	auto pred = [&pos](unique_ptr<HeightMapTerrain>& terrain) {
		XMFLOAT3 tPos{ terrain->GetPosition() };
		XMFLOAT3 scale{ terrain->GetScale() };
		float width{ terrain->GetWidth() * scale.x };
		float length{ terrain->GetLength() * scale.z };

		// �ϴÿ��� +z���� �Ӹ������� �ΰ� ������ ���� ���� ����
		float left{ tPos.x };
		float right{ tPos.x + width };
		float top{ tPos.z + length };
		float bot{ tPos.z };

		if ((left <= pos.x && pos.x <= right) &&
			(bot <= pos.z && pos.z <= top))
			return true;
		return false;
	};

	// �÷��̾�, ī�޶�, ������Ʈ�� ��� ���� ���� �ִ��� ����
	if (m_player)
	{
		pos = m_player->GetPosition();
		auto terrain = find_if(m_terrains.begin(), m_terrains.end(), pred);
		m_player->SetTerrain(terrain != m_terrains.end() ? terrain->get() : nullptr);
	}
	if (m_camera)
	{
		pos = m_camera->GetEye();
		auto terrain = find_if(m_terrains.begin(), m_terrains.end(), pred);
		m_camera->SetTerrain(terrain != m_terrains.end() ? terrain->get() : nullptr);
	}
	for (auto& object : m_gameObjects)
	{
		if (!object->GetCheckTerrain()) continue;

		pos = object->GetPosition();
		auto terrain = find_if(m_terrains.begin(), m_terrains.end(), pred);
		object->SetTerrain(terrain != m_terrains.end() ? terrain->get() : nullptr);
	}
	for (auto& instance : m_instances)
		for (auto& object : instance->GetGameObjects())
		{
			pos = object->GetPosition();
			auto terrain = find_if(m_terrains.begin(), m_terrains.end(), pred);
			object->SetTerrain(terrain != m_terrains.end() ? terrain->get() : nullptr);
		}
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	// ī�޶� ���̴� ����(��, ���� ��ȯ ���) �ֽ�ȭ
	if (m_camera) m_camera->UpdateShaderVariable(commandList);

	// ��ī�̹ڽ� ������, ���� ���� �������ؾ���
	// ��ī�̹ڽ��� ���̹��ۿ� ������ ��ġ�� �ʴ´�(SkyboxShader).
	if (m_skybox) m_skybox->Render(commandList);

	// �÷��̾� ������
	if (m_player) m_player->Render(commandList);

	// ���ӿ�����Ʈ ������
	for (const auto& gameObject : m_gameObjects)
		gameObject->Render(commandList);

	// �ν��Ͻ� ������
	for (const auto& instance : m_instances)
		instance->Render(commandList);

	// ���� ������
	for (const auto& terrain : m_terrains)
		terrain->Render(commandList);
}

void Scene::ReleaseUploadBuffer()
{
	if (m_resourceManager) m_resourceManager->ReleaseUploadBuffer();
}

void Scene::CreateBullet()
{
	unique_ptr<Bullet> bullet{ make_unique<Bullet>(m_player->GetPosition(), m_player->GetLook(), m_player->GetNormal(), 100.0f) };
	bullet->SetMesh(m_resourceManager->GetMesh("BULLET_MESH"));
	bullet->SetShader(m_resourceManager->GetShader("TEXTURE_SHADER"));
	bullet->SetTexture(m_resourceManager->GetTexture("ROCK_TEXTURE"));

	// ��ũ �Ž� �Ǻ��� ���� �ֱ� ������ ���� ������ ����
	// �׸��� ��ũ ���� ���̵� �����ؼ� �����������
	XMFLOAT3 position{ m_player->GetPosition() };
	position.y += 0.45f;
	position = Vector3::Add(position, Vector3::Mul(m_player->GetLook(), 2.5f));
	bullet->SetPosition(position);
	m_gameObjects.push_back(move(bullet));
}

void Scene::SetSkybox(unique_ptr<Skybox>& skybox)
{
	if (m_skybox) m_skybox.reset();
	m_skybox = move(skybox);
}

void Scene::SetPlayer(const shared_ptr<Player>& player)
{
	if (m_player) m_player.reset();
	m_player = player;
}

void Scene::SetCamera(const shared_ptr<Camera>& camera)
{
	if (m_camera) m_camera.reset();
	m_camera = camera;
}

HeightMapTerrain* Scene::GetTerrain(FLOAT x, FLOAT z) const
{
	auto terrain = find_if(m_terrains.begin(), m_terrains.end(), [&x, &z](const unique_ptr<HeightMapTerrain>& t) {
		XMFLOAT3 scale{ t->GetScale() };
		XMFLOAT3 pos{ t->GetPosition() };
		float width{ t->GetWidth() * scale.x };
		float length{ t->GetLength() * scale.z };

		// ������ �ϴÿ��� ������ ���� ��
		float left{ pos.x };			// ����
		float right{ pos.x + width };	// ������
		float top{ pos.z + length };	// ��
		float bot{ pos.z };				// ��

		if ((left <= x && x <= right) && (bot <= z && z <= top))
			return true;
		return false;
		});

	return terrain != m_terrains.end() ? terrain->get() : nullptr;
}