﻿#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

// Windows 헤더 파일
#include <windows.h>

// C/C++
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>
#include <algorithm>
#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
using namespace std;
using Microsoft::WRL::ComPtr;

// DIRECT3D 12
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3d12sdklayers.h>
#include "d3dx12.h"
using namespace DirectX;

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception{};
        }
    }
}

namespace Vector3
{
    inline XMFLOAT3 Add(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return XMFLOAT3{ a.x + b.x, a.y + b.y, a.z + b.z };
    }
    inline XMFLOAT3 Sub(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return XMFLOAT3{ a.x - b.x, a.y - b.y, a.z - b.z };
    }
    inline XMFLOAT3 Mul(const XMFLOAT3& a, const FLOAT& scalar)
    {
        return XMFLOAT3{ a.x * scalar, a.y * scalar, a.z * scalar };
    }
    inline FLOAT Dot(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    inline XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b)
    {
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(&a), XMLoadFloat3(&b)));
        return result;
    }
    inline XMFLOAT3 Normalize(const XMFLOAT3& a)
    {
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&a)));
        return result;
    }
    inline FLOAT Length(const XMFLOAT3& a)
    {
        XMFLOAT3 result;
        XMVECTOR v{ XMVector3Length(XMLoadFloat3(&a)) };
        XMStoreFloat3(&result, v);
        return result.x;
    }
    inline XMFLOAT3 TransformCoord(const XMFLOAT3& a, const XMFLOAT4X4& b)
    {
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3TransformCoord(XMLoadFloat3(&a), XMLoadFloat4x4(&b)));
        return result;
    }
    inline XMFLOAT3 TransformNormal(const XMFLOAT3& a, const XMFLOAT4X4& b)
    {
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3TransformNormal(XMLoadFloat3(&a), XMLoadFloat4x4(&b)));
        return result;
    }
    inline void Print(const XMFLOAT3& a, BOOL newLine=TRUE)
    {
        cout << a.x << ", " << a.y << ", " << a.z;
        if (newLine) cout << endl;
    }
}

namespace Matrix
{
    inline XMFLOAT4X4 Mul(const XMFLOAT4X4& a, const XMFLOAT4X4& b)
    {
        XMFLOAT4X4 result;
        XMMATRIX x{ XMLoadFloat4x4(&a) };
        XMMATRIX y{ XMLoadFloat4x4(&b) };
        XMStoreFloat4x4(&result, x * y);
        return result;
    }

    inline XMFLOAT4X4 Transpose(const XMFLOAT4X4& a)
    {
        XMFLOAT4X4 result;
        XMMATRIX m{ XMMatrixTranspose(XMLoadFloat4x4(&a)) };
        XMStoreFloat4x4(&result, m);
        return result;
    }

    inline XMFLOAT4X4 Inverse(const XMFLOAT4X4& a)
    {
        XMFLOAT4X4 result;
        XMMATRIX m{ XMMatrixInverse(NULL, XMLoadFloat4x4(&a)) };
        XMStoreFloat4x4(&result, m);
        return result;
    }

    inline XMFLOAT4X4 Identity()
    {
        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, XMMatrixIdentity());
        return result;
    }
}

// --------------------------------------

extern UINT g_cbvSrvDescriptorIncrementSize; // 상수버퍼뷰, 셰이더리소스뷰 서술자 힙 크기

ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const void* data, UINT sizePerData, UINT dataCount,
    D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState, ComPtr<ID3D12Resource>& uploadBuffer);

string sPATH(const string& fileName);
wstring wPATH(const string& fileName);