#pragma once
#include <d3dx12.h>
#include <iostream>
#include <stdexcept>

inline void ThrowIfFailed(const HRESULT hr, const char* msg = "")
{
    if (FAILED(hr))
    {
	    char* hrCstr = nullptr;
    	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		0,
		reinterpret_cast<LPSTR>(&hrCstr),
		0,
		nullptr);

    	std::cerr << "HRESULT: " << hrCstr << std::endl;
    	throw std::runtime_error(msg);
    }
}

inline void TransitionResource(ID3D12GraphicsCommandList2* commandList, ID3D12Resource* resource,
							   const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState)
{
    const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

inline std::wstring ToWideString(const char* str)
{
    if (!str || !*str) return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.data(), len);
    return wstr;
}

inline void ThrowError(const std::string& msg)
{
	std::cerr << msg << std::endl;
    __debugbreak();
}