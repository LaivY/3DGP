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
	// 리소스매니저 생성
	m_resourceManager = make_unique<ResourceManager>();

	// 메쉬 생성
	auto tankMesh{ make_shared<Mesh>(device, commandList, sPATH("Tank.obj")) };
	auto cubeMesh{ make_shared<CubeMesh>(device, commandList, 0.5f, 0.5f, 0.5f) };
	auto indoorMesh{ make_shared<ReverseCubeMesh>(device, commandList, 15.0f, 15.0f, 15.0f) };
	auto bulletMesh{ make_shared<CubeMesh>(device, commandList, 0.1f, 0.1f, 0.1f) };
	auto explosionMesh{ make_shared<TextureRectMesh>(device, commandList, 5.0f, 0.0f, 5.0f, XMFLOAT3{}) };
	auto smokeMesh{ make_shared<TextureRectMesh>(device, commandList, 5.0f, 0.0f, 5.0f, XMFLOAT3{}) };
	auto mirrorMesh{ make_shared<TextureRectMesh>(device, commandList, 15.0f, 0.0f, 15.0f, XMFLOAT3{ 0.0f, 0.0f, 0.1f }) };
	
	// 셰이더 생성
	auto colorShader{ make_shared<Shader>(device, rootSignature) };
	auto textureShader{ make_shared<TextureShader>(device, rootSignature) };
	auto terrainTessShader{ make_shared<TerrainTessShader>(device, rootSignature) };
	auto terrainTessWireShader{ make_shared<TerrainTessWireShader>(device, rootSignature) };
	auto blendingShader{ make_shared<BlendingShader>(device, rootSignature) };
	auto blendingDepthShader{ make_shared<BlendingDepthShader>(device, rootSignature) };
	auto stencilShader{ make_shared<StencilShader>(device, rootSignature) };
	auto mirrorShader{ make_shared<MirrorShader>(device, rootSignature) };
	auto mirrorTextureShader{ make_shared<MirrorTextureShader>(device, rootSignature) };

	// 텍스쳐 생성
	auto rockTexture{ make_shared<Texture>() };
	rockTexture->LoadTextureFile(device, commandList, 2, wPATH("Rock.dds"));
	rockTexture->CreateSrvDescriptorHeap(device);
	rockTexture->CreateShaderResourceView(device);

	auto terrainTexture{ make_shared<Texture>() };
	terrainTexture->LoadTextureFile(device, commandList, 2, wPATH("BaseTerrain.dds"));
	terrainTexture->LoadTextureFile(device, commandList, 3, wPATH("DetailTerrain.dds"));
	terrainTexture->CreateSrvDescriptorHeap(device);
	terrainTexture->CreateShaderResourceView(device);

	auto explosionTexture{ make_shared<Texture>() };
	for (int i = 1; i <= 33; ++i)
		explosionTexture->LoadTextureFile(device, commandList, 2, wPATH("explosion (" + to_string(i) + ").dds"));
	explosionTexture->CreateSrvDescriptorHeap(device);
	explosionTexture->CreateShaderResourceView(device);

	auto smokeTexture{ make_shared<Texture>() };
	for (int i = 1; i <= 91; ++i)
		smokeTexture->LoadTextureFile(device, commandList, 2, wPATH("smoke (" + to_string(i) + ").dds"));
	smokeTexture->CreateSrvDescriptorHeap(device);
	smokeTexture->CreateShaderResourceView(device);

	auto indoorTexture{ make_shared<Texture>() };
	indoorTexture->LoadTextureFile(device, commandList, 2, wPATH("Wall.dds"));
	indoorTexture->CreateSrvDescriptorHeap(device);
	indoorTexture->CreateShaderResourceView(device);

	auto mirrorTexture{ make_shared<Texture>() };
	mirrorTexture->LoadTextureFile(device, commandList, 2, wPATH("Mirror.dds"));
	mirrorTexture->CreateSrvDescriptorHeap(device);
	mirrorTexture->CreateShaderResourceView(device);

	// 리소스매니저에 리소스 추가
	m_resourceManager->AddMesh("TANK", tankMesh);
	m_resourceManager->AddMesh("CUBE", cubeMesh);
	m_resourceManager->AddMesh("INDOOR", indoorMesh);
	m_resourceManager->AddMesh("BULLET", bulletMesh);
	m_resourceManager->AddMesh("EXPLOSION", explosionMesh);
	m_resourceManager->AddMesh("SMOKE", smokeMesh);
	m_resourceManager->AddMesh("MIRROR", mirrorMesh);

	m_resourceManager->AddShader("COLOR", colorShader);
	m_resourceManager->AddShader("TEXTURE", textureShader);
	m_resourceManager->AddShader("TERRAINTESS", terrainTessShader);
	m_resourceManager->AddShader("TERRAINTESSWIRE", terrainTessWireShader);
	m_resourceManager->AddShader("BLENDING", blendingShader);
	m_resourceManager->AddShader("BLENDINGDEPTH", blendingDepthShader);
	m_resourceManager->AddShader("STENCIL", stencilShader);
	m_resourceManager->AddShader("MIRROR", mirrorShader);
	m_resourceManager->AddShader("MIRRORTEXTURE", mirrorTextureShader);

	m_resourceManager->AddTexture("ROCK", rockTexture);
	m_resourceManager->AddTexture("TERRAIN", terrainTexture);
	m_resourceManager->AddTexture("EXPLOSION", explosionTexture);
	m_resourceManager->AddTexture("SMOKE", smokeTexture);
	m_resourceManager->AddTexture("MIRROR", mirrorTexture);
	m_resourceManager->AddTexture("INDOOR", indoorTexture);

	// 카메라 생성
	auto camera{ make_shared<ThirdPersonCamera>() };
	SetCamera(camera);

	// 카메라 투영 행렬 설정
	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 1.0f, 5000.0f));
	camera->SetProjMatrix(projMatrix);

	// 플레이어 생성
	auto player{ make_shared<Player>() };
	player->SetMesh(m_resourceManager->GetMesh("TANK"));
	player->SetShader(m_resourceManager->GetShader("COLOR"));
	SetPlayer(player);

	// 카메라, 플레이어 서로 설정
	camera->SetPlayer(player);
	player->SetCamera(camera);

	// 스카이박스 생성
	auto skybox{ make_unique<Skybox>(device, commandList, rootSignature) };
	skybox->SetCamera(camera);
	SetSkybox(skybox);

	// 지형 생성
	// 한 블록 당 25개의 정점으로 이루어져있으므로 블록 너비, 길이는 4의 배수여야한다.
	XMFLOAT3 terrainScale{ 1.0f, 0.2f, 1.0f };
	auto terrain{ make_unique<HeightMapTerrain>(device, commandList, wPATH("HeightMap.raw"), m_resourceManager->GetShader("TERRAINTESS"), 
												m_resourceManager->GetTexture("TERRAIN"), 257, 257, 12, 12, terrainScale) };
	terrain->SetPosition({ -257.0f / 2.0f, 0.0f, -257.0f / 2.0f });
	m_terrains.push_back(move(terrain));

	// 실내 생성
	auto indoor{ make_unique<GameObject>() };
	indoor->SetPosition({ 0.0f, 500.0f, 0.0f });
	indoor->SetMesh(m_resourceManager->GetMesh("INDOOR"));
	indoor->SetShader(m_resourceManager->GetShader("TEXTURE"));
	indoor->SetTexture(m_resourceManager->GetTexture("INDOOR"));
	m_gameObjects.push_back(move(indoor));

	// 거울 생성
	auto mirror{ make_unique<GameObject>() };
	mirror->SetPosition(XMFLOAT3{ 0.0f, 500.0f -7.5f, 14.5f });
	mirror->SetMesh(m_resourceManager->GetMesh("MIRROR"));
	mirror->SetShader(m_resourceManager->GetShader("BLENDINGDEPTH"));
	mirror->SetTexture(m_resourceManager->GetTexture("MIRROR"));
	m_mirror = move(mirror);
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
	float sensitive{ 2.5f };
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
	case WM_MOUSEWHEEL:
	{
		if (!m_camera) break;
		ThirdPersonCamera* camera{ reinterpret_cast<ThirdPersonCamera*>(m_camera.get()) };
		if ((SHORT)HIWORD(wParam) > 0)
			camera->SetDistance(camera->GetDistance() - 1.0f);
		else
			camera->SetDistance(camera->GetDistance() + 1.0f);
		break;
	}
	}
}

void Scene::OnKeyboardEvent(FLOAT deltaTime)
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

void Scene::OnKeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 지형 와이어프레임 ON, OFF
	static bool drawAsWireframe{ false };
	if (wParam == 'q' || wParam == 'Q')
	{
		drawAsWireframe = !drawAsWireframe;
		for (auto& terrain : m_terrains)
			if (drawAsWireframe)
				terrain->SetShader(m_resourceManager->GetShader("TERRAINTESSWIRE"));
			else
				terrain->SetShader(m_resourceManager->GetShader("TERRAINTESS"));
	}

	// 실내로 이동
	else if (wParam == 'i' || wParam == 'I')
	{
		// right up look 초기화
		XMFLOAT3 rpy{ m_player->GetRollPitchYaw() };
		m_player->Rotate(-rpy.x, -rpy.y, -rpy.z);
		
		XMFLOAT4X4 worldMatrix{ Matrix::Identity() };
		worldMatrix._11 = 1.0f; worldMatrix._12 = 0.0f; worldMatrix._13 = 0.0f;
		worldMatrix._21 = 0.0f; worldMatrix._22 = 1.0f; worldMatrix._23 = 0.0f;
		worldMatrix._31 = 0.0f; worldMatrix._32 = 0.0f; worldMatrix._33 = 1.0f;
		m_player->SetWorldMatrix(worldMatrix);
		m_player->SetPosition({ 0.0f, 500.0f - 15.0f, 0.0f });
	}

	// 지형 위로 이동
	else if (wParam == 'e' || wParam == 'E')
	{
		m_player->SetPosition({ 0.0f, 0.0f, 0.0f });
	}

	// 종료
	else if (wParam == VK_ESCAPE)
	{
		exit(0);
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
	for (auto& particle : m_particles)
		particle->Update(deltaTime);
}

void Scene::Update(FLOAT deltaTime)
{
	RemoveDeletedObjects();
	UpdateObjectsTerrain();
}

void Scene::RemoveDeletedObjects()
{
	vector<unique_ptr<GameObject>> willBeAdded;

	auto pred = [&](unique_ptr<GameObject>& object) {
		if (object->isDeleted() && object->GetType() == GameObjectType::BULLET)
		{
			// 폭발 이펙트 생성
			auto textureInfo{ make_unique<TextureInfo>() };
			textureInfo->frameInterver *= 1.5f;
			textureInfo->isFrameRepeat = false;

			auto explosion{ make_unique<BillboardObject>(m_camera) };
			explosion->SetPosition(object->GetPosition());
			explosion->SetMesh(m_resourceManager->GetMesh("EXPLOSION"));
			explosion->SetShader(m_resourceManager->GetShader("BLENDING"));
			explosion->SetTexture(m_resourceManager->GetTexture("EXPLOSION"));
			explosion->SetTextureInfo(textureInfo);
			willBeAdded.push_back(move(explosion));

			// 연기 이펙트 생성
			textureInfo = make_unique<TextureInfo>();
			textureInfo->frameInterver *= 3.0f;
			textureInfo->isFrameRepeat = false;

			auto smoke{ make_unique<BillboardObject>(m_camera) };
			smoke->SetPosition(object->GetPosition());
			smoke->SetMesh(m_resourceManager->GetMesh("SMOKE"));
			smoke->SetShader(m_resourceManager->GetShader("BLENDING"));
			smoke->SetTexture(m_resourceManager->GetTexture("SMOKE"));
			smoke->SetTextureInfo(textureInfo);
			willBeAdded.push_back(move(smoke));
		}
		return object->isDeleted();
	};
	m_gameObjects.erase(remove_if(m_gameObjects.begin(), m_gameObjects.end(), pred), m_gameObjects.end());
	m_particles.erase(remove_if(m_particles.begin(), m_particles.end(), pred), m_particles.end());

	// 총알 삭제될 때 생기는 이펙트는 파티클 객체에 추가한다.
	for (auto& object : willBeAdded)
		m_particles.push_back(move(object));
}

void Scene::UpdateObjectsTerrain()
{
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
	for (auto& object : m_particles)
	{
		pos = object->GetPosition();
		auto terrain = find_if(m_terrains.begin(), m_terrains.end(), pred);
		object->SetTerrain(terrain != m_terrains.end() ? terrain->get() : nullptr);
	}
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle) const
{
	// 카메라 셰이더 변수(뷰, 투영 변환 행렬) 최신화
	if (m_camera) m_camera->UpdateShaderVariable(commandList);

	// 반사상, 거울 렌더링
	if (m_mirror && m_player)
	{
		// 스텐실 버퍼 초기화
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		// 스텐실 참조값 설정
		commandList->OMSetStencilRef(1);

		// 거울 위치를 스텐실 버퍼에 표시
		m_mirror->Render(commandList, m_resourceManager->GetShader("STENCIL"));

		// 반사 행렬
		XMVECTOR mirrorPlane{ XMVectorSet(0.0f, 0.0f, -1.0f, m_mirror->GetPosition().z) };
		XMFLOAT4X4 reflectMatrix;
		XMStoreFloat4x4(&reflectMatrix, XMMatrixReflect(mirrorPlane));

		// 실내 반사상 렌더링
		for (const auto& object : m_gameObjects)
		{
			XMFLOAT4X4 temp{ object->GetWorldMatrix() };
			object->SetWorldMatrix(Matrix::Mul(temp, reflectMatrix));
			object->Render(commandList, m_resourceManager->GetShader("MIRRORTEXTURE"));
			object->SetWorldMatrix(temp);
		}

		// 플레이어 반사상 렌더링
		XMFLOAT4X4 originWorldMatrix{ m_player->GetWorldMatrix() };
		m_player->SetWorldMatrix(Matrix::Mul(originWorldMatrix, reflectMatrix));
		m_player->Render(commandList, m_resourceManager->GetShader("MIRROR"));
		m_player->SetWorldMatrix(originWorldMatrix);

		// 스텐실 참조값 초기화
		commandList->OMSetStencilRef(0);

		// 거울 렌더링
		m_mirror->Render(commandList);
	}

	// 스카이박스 렌더링
	if (m_skybox) m_skybox->Render(commandList);

	// 플레이어 렌더링
	if (m_player) m_player->Render(commandList);

	// 게임오브젝트 렌더링
	for (const auto& gameObject : m_gameObjects)
		gameObject->Render(commandList);

	// 지형 렌더링
	for (const auto& terrain : m_terrains)
		terrain->Render(commandList);

	// 파티클 렌더링
	for (const auto& particle : m_particles)
		particle->Render(commandList);
}

void Scene::ReleaseUploadBuffer()
{
	if (m_resourceManager) m_resourceManager->ReleaseUploadBuffer();
}

void Scene::CreateBullet()
{
	unique_ptr<Bullet> bullet{ make_unique<Bullet>(m_player->GetPosition(), m_player->GetLook(), m_player->GetNormal(), 100.0f) };
	bullet->SetPosition(Vector3::Add(m_player->GetPosition(), XMFLOAT3{ 0.0f, 0.5f, 0.0f }));
	bullet->SetMesh(m_resourceManager->GetMesh("BULLET"));
	bullet->SetShader(m_resourceManager->GetShader("BLENDING"));
	bullet->SetTexture(m_resourceManager->GetTexture("ROCK"));
	m_particles.push_back(move(bullet));
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