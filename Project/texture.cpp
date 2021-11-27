#include "texture.h"

void Texture::LoadTextureFile(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, UINT rootParameterIndex, const wstring& fileName)
{
	ComPtr<ID3D12Resource> textureBuffer, textureUploadBuffer;

	// DDS �ؽ��� �ε�
	unique_ptr<uint8_t[]> ddsData;
	vector<D3D12_SUBRESOURCE_DATA> subresources;
	DDS_ALPHA_MODE ddsAlphaMode{ DDS_ALPHA_MODE_UNKNOWN };
	DX::ThrowIfFailed(LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &textureBuffer, ddsData, subresources, &ddsAlphaMode));

	// ����Ʈ ������ ������ �����ϱ� ���� ���ε� �� ����
	UINT nSubresources{ (UINT)subresources.size() };
	UINT64 nBytes{ GetRequiredIntermediateSize(textureBuffer.Get(), 0, nSubresources) };
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nBytes),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&textureUploadBuffer)
	));

	// subresources�� �ִ� �����͸� textureBuffer�� ����
	UpdateSubresources(commandList.Get(), textureBuffer.Get(), textureUploadBuffer.Get(), 0, 0, nSubresources, subresources.data());

	// ���ҽ� ������ ����
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// ����
	m_textures.push_back(make_pair(textureBuffer, rootParameterIndex));
	m_textureUploadBuffers.push_back(textureUploadBuffer);
}

void Texture::CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device)
{
	if (m_srvHeap) m_srvHeap.Reset();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.NumDescriptors = m_textures.size(); // SRV ����
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
}

void Texture::CreateShaderResourceView(const ComPtr<ID3D12Device>& device)
{
	D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptorHandle{ m_srvHeap->GetCPUDescriptorHandleForHeapStart() };
	for (const auto& [texture, _] : m_textures)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texture->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		device->CreateShaderResourceView(texture.Get(), &srvDesc, srvDescriptorHandle);
		srvDescriptorHandle.ptr += g_cbvSrvDescriptorIncrementSize;
	}
}

void Texture::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvDescriptorHandle{ m_srvHeap->GetGPUDescriptorHandleForHeapStart() };

	if (m_textureInfo)
	{
		srvDescriptorHandle.Offset(m_textureInfo->frame, g_cbvSrvDescriptorIncrementSize);
		commandList->SetGraphicsRootDescriptorTable(m_textures[m_textureInfo->frame].second, srvDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_textures.size(); ++i)
		{
			auto [_, rootParameterIndex] = m_textures[i];
			commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, srvDescriptorHandle);
			srvDescriptorHandle.Offset(g_cbvSrvDescriptorIncrementSize);
		}
	}
}

void Texture::ReleaseUploadBuffer()
{
	for (auto& textureUploadBuffer : m_textureUploadBuffers)
		textureUploadBuffer.Reset();
}