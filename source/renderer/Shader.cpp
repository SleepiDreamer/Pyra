#include "Shader.h"
#include "ShaderCompiler.h"
#include "HelpersDX.h"

#include <iostream>

Shader::Shader(ShaderCompiler& compiler, std::string filePath, const std::vector<std::string>& entryPoints)
    : m_compiler(compiler), m_filePath(std::move(filePath)), m_entryPoints(entryPoints)
{
    if (std::filesystem::exists(m_filePath))
    {
	    m_lastWriteTime = std::filesystem::last_write_time(m_filePath);
    }

    Reload();
}

Shader::~Shader() = default;

bool Shader::NeedsReload() const
{
    try
    {
        return std::filesystem::last_write_time(m_filePath) > m_lastWriteTime;
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return false;
    }
}

bool Shader::Reload()
{
    auto result = m_compiler.Compile(m_filePath);

    if (!result.success)
    {
        std::cerr << "Failed to compile shader: " << m_filePath << "\n" << result.errorLog << "\n";
        m_lastWriteTime = std::filesystem::last_write_time(m_filePath);
		m_lastCompileFailed = true;
        return false;
    }

    m_blob = std::move(result.blob);
    m_lastWriteTime = std::filesystem::last_write_time(m_filePath);
	m_lastCompileFailed = false;
    return true;
}

D3D12_SHADER_BYTECODE Shader::GetBytecode() const
{
    return { m_blob.data(), m_blob.size() };
}