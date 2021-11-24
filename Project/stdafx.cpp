#include "stdafx.h"

UINT g_cbvSrvDescriptorIncrementSize{ 0 };

ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const void* data, UINT sizePerData, UINT dataCount,
	D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> buffer;
	const UINT bufferSize{ sizePerData * dataCount };

	// ����Ʈ ���ۿ� �����͸� ���� ��� ���ε� ���۰� �ʿ���
	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
	{
		// ����Ʈ �� ����
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			NULL,
			IID_PPV_ARGS(&buffer)));

		// ���ε� �� ����
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			NULL,
			IID_PPV_ARGS(&uploadBuffer)));

		// ���ε� ������ ����Ʈ ������ ����
		D3D12_SUBRESOURCE_DATA bufferData{};
		bufferData.pData = data;
		bufferData.RowPitch = bufferSize;
		bufferData.SlicePitch = bufferData.RowPitch;
		UpdateSubresources<1>(commandList.Get(), buffer.Get(), uploadBuffer.Get(), 0, 0, 1, &bufferData);

		// ���� ���ҽ� ������ ����
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState));
		return buffer;
	}

	// ���ε� ���ۿ� �����͸� ���� ��� �ٷ� ������
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		// ���ε� �� ����
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			NULL,
			IID_PPV_ARGS(&buffer)));

		// ������ ����
		UINT8* pBufferDataBegin{ NULL };
		CD3DX12_RANGE readRange{ 0, 0 };
		DX::ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&pBufferDataBegin)));
		memcpy(pBufferDataBegin, data, bufferSize);
		buffer->Unmap(0, NULL);

		// ���ҽ� ������ ����
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceState));
		return buffer;
	}
	return NULL;
}

string sPATH(const string& filePath)
{
	return "resource/" + filePath;
}

wstring wPATH(const string& filePath)
{
	wstring wStr{ filePath.begin(), filePath.end() };
	return TEXT("resource/") + wStr;
}