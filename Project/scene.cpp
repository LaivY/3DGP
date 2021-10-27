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
	// 메쉬, 셰이더, 텍스쳐들은 모두 ResourceManager에 있는 map에 담는다.
	// ResourceManager::Add??? 함수를 통해 리소스를 추가하고 ResourceManager::Get??? 함수를 통해 리소스를 불러올 수 있다.
	// class ResourceManager가 있으므로서 나중에 객체를 생성할 때 필요한 Mesh, Shader, Texture를 미리 생성해둘 수 있다.

	// 리소스들 담고있을 ResourceManager 생성
	m_resourceManager = make_unique<ResourceManager>();

	// 필요한 메쉬들 생성
	shared_ptr<CubeMesh> cubeMesh{ make_shared<CubeMesh>(device, commandList, 0.5f, 0.5f, 0.5f) };
	shared_ptr<CubeMesh> bulletMesh{ make_shared<CubeMesh>(device, commandList, 0.1f, 0.5f, 0.1f) };
	shared_ptr<TextureRectMesh> textureRectMesh{ make_shared<TextureRectMesh>(device, commandList, 10.0f, 0.0f, 10.0f, XMFLOAT3{}) };
	shared_ptr<Mesh> tankMesh{ make_shared<Mesh>(device, commandList, "resource/tank2.obj") };

	// 필요한 셰이더들 생성
	shared_ptr<Shader> colorShader{ make_shared<Shader>(device, rootSignature) };
	shared_ptr<TextureShader> textureShader{ make_shared<TextureShader>(device, rootSignature) };
	shared_ptr<TerrainShader> terrainShader{ make_shared<TerrainShader>(device, rootSignature) };
	shared_ptr<InstanceShader> instanceShader{ make_shared<InstanceShader>(device, rootSignature) };

	// 필요한 텍스쳐들 생성
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

	// 리소스 보관 객체에 저장
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

	// 지형 생성
	unique_ptr<HeightMapTerrain> terrain{
		make_unique<HeightMapTerrain>(device, commandList, TEXT("resource/heightMap.raw"), terrainShader, terrainTexture, 257, 257, 257, 257, XMFLOAT3{ 1.0f, 0.2f, 1.0f })
	};
	terrain->SetPosition(XMFLOAT3{ 0.0f, -300.0f, 0.0f });
	m_terrains.push_back(move(terrain));

	// 카메라 생성
	shared_ptr<ThirdPersonCamera> camera{ make_shared<ThirdPersonCamera>() };
	camera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	camera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	camera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	SetCamera(camera);

	// 카메라 투영 행렬 설정
	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 0.1f, 3000.0f));
	camera->SetProjMatrix(projMatrix);

	// 스카이박스 생성
	unique_ptr<Skybox> skybox{ make_unique<Skybox>(device, commandList, rootSignature) };
	skybox->SetCamera(camera);
	SetSkybox(skybox);

	// 플레이어 생성
	shared_ptr<Player> player{ make_shared<Player>() };
	player->SetMesh(m_resourceManager->GetMesh("TANK_MESH"));
	player->SetShader(m_resourceManager->GetShader("COLOR_SHADER"));
	player->SetTexture(m_resourceManager->GetTexture("ROCK_TEXTURE"));
	player->SetCamera(camera);

	// 씬, 카메라 플레이어 설정
	SetPlayer(player);
	camera->SetPlayer(m_player);

	// 빌보드 객체 10000개를 인스턴싱으로 생성
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
	// 화면 가운데 좌표 계산
	RECT rect; GetWindowRect(hWnd, &rect);
	POINT oldMousePosition{ rect.left + width / 2, rect.top + height / 2 };

	// 움직인 마우스 좌표
	POINT newMousePosition; GetCursorPos(&newMousePosition);

	// 움직인 정도에 비례해서 회전
	int dx = newMousePosition.x - oldMousePosition.x;
	int dy = newMousePosition.y - oldMousePosition.y;
	float sensitive{ 5.0f };
	if (m_player) m_player->Rotate(0.0f, dy * sensitive * deltaTime, dx * sensitive * deltaTime);

	// 마우스를 화면 가운데로 이동
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

	// m_isDeleted가 true인 객체들을 삭제한다.
	auto pred = [this, &willBeAdded](unique_ptr<GameObject>& object) {

		// 삭제한 오브젝트가 총알이라면 그 자리에 폭발이펙트 생성
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
	// 지형이 여러개일 수 있다는 것을 고려했기 때문에
	// 게임오브젝트들이 어느 지형 위에 있는지 설정해준다.

	XMFLOAT3 pos{};
	auto pred = [&pos](unique_ptr<HeightMapTerrain>& terrain) {
		XMFLOAT3 tPos{ terrain->GetPosition() };
		XMFLOAT3 scale{ terrain->GetScale() };
		float width{ terrain->GetWidth() * scale.x };
		float length{ terrain->GetLength() * scale.z };

		// 하늘에서 +z축을 머리쪽으로 두고 지형을 봤을 때를 기준
		float left{ tPos.x };
		float right{ tPos.x + width };
		float top{ tPos.z + length };
		float bot{ tPos.z };

		if ((left <= pos.x && pos.x <= right) &&
			(bot <= pos.z && pos.z <= top))
			return true;
		return false;
	};

	// 플레이어, 카메라, 오브젝트가 어느 지형 위에 있는지 설정
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
	// 카메라 셰이더 변수(뷰, 투영 변환 행렬) 최신화
	if (m_camera) m_camera->UpdateShaderVariable(commandList);

	// 스카이박스 렌더링, 가장 먼저 렌더링해야함
	// 스카이박스는 깊이버퍼에 영향을 미치지 않는다(SkyboxShader).
	if (m_skybox) m_skybox->Render(commandList);

	// 플레이어 렌더링
	if (m_player) m_player->Render(commandList);

	// 게임오브젝트 렌더링
	for (const auto& gameObject : m_gameObjects)
		gameObject->Render(commandList);

	// 인스턴스 렌더링
	for (const auto& instance : m_instances)
		instance->Render(commandList);

	// 지형 렌더링
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

	// 탱크 매쉬 피봇이 땅에 있기 때문에 조금 위에서 생성
	// 그리고 탱크 포신 길이도 생각해서 조정해줘야함
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

		// 지형을 하늘에서 밑으로 봤을 때
		float left{ pos.x };			// 왼쪽
		float right{ pos.x + width };	// 오른쪽
		float top{ pos.z + length };	// 위
		float bot{ pos.z };				// 밑

		if ((left <= x && x <= right) && (bot <= z && z <= top))
			return true;
		return false;
		});

	return terrain != m_terrains.end() ? terrain->get() : nullptr;
}