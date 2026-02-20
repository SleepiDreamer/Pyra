#pragma once
#include <d3dx12.h>
#include <glm/glm.hpp>
#include <ImReflect.hpp>
#include <iostream>
#include <stdexcept>

inline void ThrowError(const std::string& msg)
{
    std::cerr << msg << std::endl;
    __debugbreak();
}

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
        ThrowError(msg);
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

inline std::wstring ToWideString(const std::string& str)
{
    return ToWideString(str.c_str());
}

inline std::string ToNarrowString(const wchar_t* wstr)
{
    if (!wstr || !*wstr) return {};
    const int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string str(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str.data(), len, nullptr, nullptr);
    return str;
}

inline std::string ToNarrowString(const std::wstring& wstr)
{
    return ToNarrowString(wstr.c_str());
}

inline void tag_invoke(ImReflect::ImInput_t, const char* label, BOOL& value, ImSettings& settings, ImResponse& response) {
    auto& bool_response = response.get<BOOL>();

    bool temp = static_cast<bool>(value);
    if (ImGui::Checkbox(label, &temp)) {
        value = temp ? TRUE : FALSE;
        bool_response.changed();
    }

    ImReflect::Detail::check_input_states(bool_response);
}

inline void tag_invoke(ImReflect::ImInput_t, const char* label, glm::vec2& value, ImSettings& settings, ImResponse& response) {
    auto& vec2_response = response.get<glm::vec2>();

	bool changed = ImGui::DragFloat2(label, &value.x, 0.02f);
    if (changed) vec2_response.changed();

    ImReflect::Detail::check_input_states(vec2_response);
}

inline void tag_invoke(ImReflect::ImInput_t, const char* label, glm::vec3& value, ImSettings& settings, ImResponse& response) {
    auto& vec3_response = response.get<glm::vec3>();

    bool changed = ImGui::DragFloat3(label, &value.x, 0.02f);
    if (changed) vec3_response.changed();

    ImReflect::Detail::check_input_states(vec3_response);
}