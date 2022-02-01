/*
#include "backend_dxc.h"

#include <atlbase.h> // Common COM helpers
#include <cpputils/logger.hpp>
#include <dxcapi.h>
#include <string>

namespace shader_builder::dxc_backend
{
std::vector<uint32_t> DxcCompiler::build_to_spirv(const std::vector<ShaderBlock>& shader_code, [[maybe_unused]] EShaderLanguage source_language, EShaderStage shader_stage)
{
    std::string code;
    for (const auto& block : shader_code)
    {
        code += block.text;
    }
    CComPtr<IDxcUtils>          utils;
    CComPtr<IDxcCompiler3>      compiler;
    CComPtr<IDxcIncludeHandler> include_handler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    utils->CreateDefaultIncludeHandler(&include_handler);

    std::vector<LPCWSTR> args = {
        // L"myshader.hlsl", // Optional shader source file name for error reporting and for PIX shader source view.
        L"-E",
        L"main", // Entry point.
        // L"-Zs",  // Enable debug information (slim format)
        // L"-D",
        // L"MYDEFINE=1",
        // L"-Fo",
        // L"myshader.bin", // Optional. Stored in the pdb.
        // L"-Fd",
        // L"myshader.pdb",    // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
        L"-Qstrip_debug",
        L"-Qstrip_reflect", // Strip reflection into a separate blob.
        L"-spirv",          // generate a spirv code
        L"-WX",
        L"-Zpr",
    };

    args.emplace_back(L"-T");
    if (shader_stage == EShaderStage::Vertex)
        args.emplace_back(L"vs_6_0");
    else
        args.emplace_back(L"ps_6_0");

    DxcBuffer source_code{code.c_str(), code.size(), DXC_CP_UTF8};

    CComPtr<IDxcResult> results;
    compiler->Compile(&source_code,                       // Source get.
                      args.data(),                        // Array of pointers to arguments.
                      static_cast<uint32_t>(args.size()), // Number of arguments.
                      include_handler,                  // User-provided interface to handle #include directives (optional).
                      IID_PPV_ARGS(&results)              // Compiler output status, get, and errors.
    );

    //
    // Print errors if present.
    //
    CComPtr<IDxcBlobUtf8> pErrors = nullptr;
    results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    // Note that d3dcompiler would return null if no errors or warnings are present.
    // IDxcCompiler3::Compile will always return an error get, but its length will be zero if there are no warnings or errors.
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        wprintf(L"Warnings and Errors:\n%S\n", pErrors->GetStringPointer());

    //
    // Quit if the compilation failed.
    //
    HRESULT hrStatus;
    results->GetStatus(&hrStatus);
    if (FAILED(hrStatus))
    {
        wprintf(L"Compilation Failed\n");
        LOG_FATAL("error 3");
    }

    CComPtr<IDxcBlob>      shader      = nullptr;
    CComPtr<IDxcBlobUtf16> shader_name = nullptr;
    results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), &shader_name);

    if (shader->GetBufferSize() % 4 != 0)
        LOG_FATAL("size error");

    return std::vector(static_cast<uint32_t*>(shader->GetBufferPointer()), static_cast<uint32_t*>(shader->GetBufferPointer()) + shader->GetBufferSize() / 4);
}
} // namespace shader_builder::dxc_backend
*/