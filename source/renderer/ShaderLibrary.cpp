#include "ShaderLibrary.h"
#include "ShaderCompiler.h"
#include "HelpersDX.h"

#include <iostream>

ShaderLibrary::ShaderLibrary(ShaderCompiler& compiler, std::string filePath, const std::vector<std::string>& entryPoints)
    : m_compiler(compiler), m_filePath(std::move(filePath)), m_entryPoints(entryPoints)
{
    if (std::filesystem::exists(m_filePath))
    {
	    m_lastWriteTime = std::filesystem::last_write_time(m_filePath);
    }

    Reload();
}

ShaderLibrary::~ShaderLibrary() = default;

bool ShaderLibrary::NeedsReload() const
{
    if (!std::filesystem::exists(m_filePath))
    {
	    return false;
    }

    return std::filesystem::last_write_time(m_filePath) > m_lastWriteTime;
}

bool ShaderLibrary::Reload()
{
    auto result = m_compiler.Compile(m_filePath);

    if (!result.success)
    {
        ThrowError("Failed to compile shader: " + m_filePath + "\n" + result.errorLog);
    }

    m_blob = std::move(result.blob);
    m_lastWriteTime = std::filesystem::last_write_time(m_filePath);
    return true;
}

D3D12_SHADER_BYTECODE ShaderLibrary::GetBytecode() const
{
    return { m_blob.data(), m_blob.size() };
}