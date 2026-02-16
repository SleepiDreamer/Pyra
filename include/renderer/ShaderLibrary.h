#pragma once
#include <d3d12.h>
#include <filesystem>
#include <string>
#include <vector>

class ShaderCompiler;

class ShaderLibrary
{
public:
    ShaderLibrary(ShaderCompiler& compiler, std::string filePath, const std::vector<std::string>& entryPoints);
    ~ShaderLibrary();

    bool NeedsReload() const;
    bool Reload();

    [[nodiscard]] D3D12_SHADER_BYTECODE GetBytecode() const;
    [[nodiscard]] const std::vector<uint8_t>& GetBlob() const { return m_blob; }
    [[nodiscard]] bool IsValid() const { return !m_blob.empty(); }

private:
    ShaderCompiler& m_compiler;
    std::string m_filePath;
    std::vector<std::string> m_entryPoints;
    std::vector<uint8_t> m_blob;
    std::filesystem::file_time_type m_lastWriteTime;
};