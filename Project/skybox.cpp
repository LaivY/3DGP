#include "skybox.h"

Skybox::Skybox(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12RootSignature>& rootSignature) : m_faces{ new GameObject[6] }
{
	// 메쉬 생성
	shared_ptr<TextureRectMesh> frontMesh{ make_shared<TextureRectMesh>(device, commandList, 20.0f, 0.0f, 20.0f, XMFLOAT3{ 0.0f, 0.0f, 10.0f }) };
	shared_ptr<TextureRectMesh> leftMesh{ make_shared<TextureRectMesh>(device, commandList, 0.0f, 20.0f, 20.0f, XMFLOAT3{ -10.0f, 0.0f, 0.0f }) };
	shared_ptr<TextureRectMesh> rightMesh{ make_shared<TextureRectMesh>(device, commandList, 0.0f, 20.0f, 20.0f, XMFLOAT3{ 10.0f, 0.0f, 0.0f }) };
	shared_ptr<TextureRectMesh> backMesh{ make_shared<TextureRectMesh>(device, commandList, 20.0f, 0.0f, 20.0f, XMFLOAT3{ 0.0f, 0.0f, -10.0f }) };
	shared_ptr<TextureRectMesh> topMesh{ make_shared<TextureRectMesh>(device, commandList, 20.0f, 20.0f, 0.0f, XMFLOAT3{ 0.0f, 10.0f, 0.0f }) };
	shared_ptr<TextureRectMesh> botMesh{ make_shared<TextureRectMesh>(device, commandList, 20.0f, 20.0f, 0.0f, XMFLOAT3{ 0.0f, -10.0f, 0.0f }) };

	// 셰이더 생성
	shared_ptr<SkyboxShader> skyboxShader{ make_shared<SkyboxShader>(device, rootSignature) };

	// 텍스쳐 생성
	shared_ptr<Texture> frontTexture{ make_shared<Texture>() };
	frontTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxFront.dds"));
	frontTexture->CreateSrvDescriptorHeap(device);
	frontTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> leftTexture{ make_shared<Texture>() };
	leftTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxLeft.dds"));
	leftTexture->CreateSrvDescriptorHeap(device);
	leftTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> rightTexture{ make_shared<Texture>() };
	rightTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxRight.dds"));
	rightTexture->CreateSrvDescriptorHeap(device);
	rightTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> backTexture{ make_shared<Texture>() };
	backTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxBack.dds"));
	backTexture->CreateSrvDescriptorHeap(device);
	backTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> topTexture{ make_shared<Texture>() };
	topTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxTop.dds"));
	topTexture->CreateSrvDescriptorHeap(device);
	topTexture->CreateShaderResourceView(device);

	shared_ptr<Texture> botTexture{ make_shared<Texture>() };
	botTexture->LoadTextureFile(device, commandList, 2, wPATH("SkyboxBot.dds"));
	botTexture->CreateSrvDescriptorHeap(device);
	botTexture->CreateShaderResourceView(device);

	// 면 생성
	m_faces[0].SetMesh(frontMesh);	m_faces[0].SetShader(skyboxShader); m_faces[0].SetTexture(frontTexture);	// 앞
	m_faces[1].SetMesh(leftMesh);	m_faces[1].SetShader(skyboxShader); m_faces[1].SetTexture(leftTexture);		// 왼쪽
	m_faces[2].SetMesh(rightMesh);	m_faces[2].SetShader(skyboxShader); m_faces[2].SetTexture(rightTexture);	// 오른쪽
	m_faces[3].SetMesh(backMesh);	m_faces[3].SetShader(skyboxShader); m_faces[3].SetTexture(backTexture);		// 뒤
	m_faces[4].SetMesh(topMesh);	m_faces[4].SetShader(skyboxShader); m_faces[4].SetTexture(topTexture);		// 위
	m_faces[5].SetMesh(botMesh);	m_faces[5].SetShader(skyboxShader); m_faces[5].SetTexture(botTexture);		// 아래
}

void Skybox::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	for (int i = 0; i < 6; ++i)
		m_faces[i].Render(commandList);
}

void Skybox::Update()
{
	if (m_camera) SetPosition(m_camera->GetEye());
}

void Skybox::SetCamera(const shared_ptr<Camera>& camera)
{
	if (m_camera) m_camera.reset();
	m_camera = camera;
}

void Skybox::SetPosition(XMFLOAT3 position)
{
	for (int i = 0; i < 6; ++i)
		m_faces[i].SetPosition(position);
}