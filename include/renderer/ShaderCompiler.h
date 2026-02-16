#pragma once
#include "CommonDX.h"

#include <slang.h>
#include <slang-com-ptr.h>
#include <string>
#include <vector>

class ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler();
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    struct CompilationResult
    {
        std::vector<uint8_t> blob;
        bool success = false;
        std::string errorLog;
    };

    CompilationResult Compile(const std::string& filePath) const;

private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
};