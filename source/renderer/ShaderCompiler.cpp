#include "ShaderCompiler.h"
#include "CommonDX.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <array>

ShaderCompiler::ShaderCompiler()
{
    slang::createGlobalSession(m_globalSession.writeRef());
    if (!m_globalSession)
        throw std::runtime_error("Failed to create Slang global session");
}

ShaderCompiler::~ShaderCompiler() = default;

void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
	    std::cerr << static_cast<const char*>(diagnosticsBlob->getBufferPointer()) << std::endl;
    }
}

ShaderCompiler::CompilationResult ShaderCompiler::Compile(const std::string& filePath) const
{
    CompilationResult result;

    slang::SessionDesc sessionDesc{};
    slang::TargetDesc targetDesc{};
    targetDesc.format = SLANG_DXIL;
    targetDesc.profile = m_globalSession->findProfile("lib_6_5");
    std::array<slang::CompilerOptionEntry, 1> options = {{{
    	slang::CompilerOptionName::GenerateWholeProgram,
		{
			.kind = slang::CompilerOptionValueKind::Int, .intValue0 = 1, .intValue1 = 0, .stringValue0 = nullptr,
			.stringValue1 = nullptr }
		    }
	    }
    };
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());
	sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    std::string directory = std::filesystem::path(filePath).parent_path().string();
    const char* searchPaths[] = { directory.c_str() };
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;

    Slang::ComPtr<slang::ISession> session;
    m_globalSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics;

    slang::IModule* module = session->loadModule(filePath.c_str(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics.get());
    if (!module) { return result; }

    std::vector<slang::IComponentType*> components;
    components.push_back(module);

    const char* entryPointNames[] = { "RayGen", "ClosestHit", "Miss" };
    std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints;
    for (const auto& name : entryPointNames)
    {
        Slang::ComPtr<slang::IEntryPoint> ep;
        module->findEntryPointByName(name, ep.writeRef());
        if (!ep)
        {
            result.errorLog = std::string("Entry point not found: ") + name;
            return result;
        }
        entryPoints.push_back(ep);
        components.push_back(ep.get());
    }

    Slang::ComPtr<slang::IComponentType> composed;
	session->createCompositeComponentType(components.data(),
        static_cast<SlangInt>(components.size()),
        composed.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics.get());

    Slang::ComPtr<slang::IComponentType> linked;
    composed->link(linked.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics.get());

    Slang::ComPtr<slang::IBlob> code;
    linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics.get());

    if (!code) { result.errorLog = "Failed to generate code"; return result; }

    result.blob.resize(code->getBufferSize());
    memcpy(result.blob.data(), code->getBufferPointer(), code->getBufferSize());
    result.success = true;
    return result;
}