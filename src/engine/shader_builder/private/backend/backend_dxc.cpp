#include "backend_dxc.h"

#include <OleCtl.h>
#include <cpputils/logger.hpp>
#include <d3dcompiler.h>
#include <dxcapi.h>

namespace shader_builder::dxc_backend
{
std::vector<uint32_t> DxcCompiler::build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage)
{
    std::string code;
    for (const auto& block : shader_code)
    {
        code += block.text;
    }


    std::vector<LPWSTR> arguments;
    //-E for the entry point (eg. PSMain)
    arguments.push_back(const_cast<LPWSTR>(L"-E"));
    arguments.push_back(const_cast<LPWSTR>(L"main"));

    //-T for the target profile (eg. ps_6_2)
    arguments.push_back(const_cast<LPWSTR>(L"-T"));
    arguments.push_back(target);

    // Strip reflection data and pdbs (see later)
    arguments.push_back(const_cast<LPWSTR>(L"-Qstrip_debug"));
    arguments.push_back(const_cast<LPWSTR>(L"-Qstrip_reflect"));

    arguments.push_back(const_cast<LPWSTR>(DXC_ARG_WARNINGS_ARE_ERRORS));   //-WX
    arguments.push_back(const_cast<LPWSTR>(DXC_ARG_DEBUG));                 //-Zi
    arguments.push_back(const_cast<LPWSTR>(DXC_ARG_PACK_MATRIX_ROW_MAJOR)); //-Zp

    /*
    for (const std::wstring& define : defines)
    {
        arguments.push_back(const_cast<LPWSTR>(L"-D"));
        arguments.push_back(define.c_str());
    }
    */

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr      = code.data();
    sourceBuffer.Size     = code.size();
    sourceBuffer.Encoding = 0;

    ComPtr<IDxcResult> pCompileResult;
    HR(pCompiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(pCompileResult.GetAddressOf())));

    // Error Handling
    ComPtr<IDxcBlobUtf8> pErrors;
    pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr);
    if (pErrors && pErrors->GetStringLength() > 0)
    {
        LOG_FATAL("compilation failed : %s", (char*)pErrors->GetBufferPointer());
    }
    
    return {};
}
}


/*


HRESULT CompileShader( _In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob )
{
    if ( !srcFile || !entryPoint || !profile || !blob )
       return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    const D3D_SHADER_MACRO defines[] =
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile( srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     entryPoint, profile,
                                     flags, 0, &shaderBlob, &errorBlob );
    if ( FAILED(hr) )
    {
        if ( errorBlob )
        {
            OutputDebugStringA( (char*)errorBlob->GetBufferPointer() );
            errorBlob->Release();
        }

        if ( shaderBlob )
           shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}

int main()
{
    // Compile vertex shader shader
    ID3DBlob *vsBlob = nullptr;
    HRESULT hr = CompileShader( L"BasicHLSL11_VS.hlsl", "VSMain", "vs_4_0_level_9_1", &vsBlob );
    if ( FAILED(hr) )
    {
        printf("Failed compiling vertex shader %08X\n", hr );
        return -1;
    }

    // Compile pixel shader shader
    ID3DBlob *psBlob = nullptr;
    hr = CompileShader( L"BasicHLSL11_PS.hlsl", "PSMain", "ps_4_0_level_9_1", &psBlob );
    if ( FAILED(hr) )
    {
        vsBlob->Release();
        printf("Failed compiling pixel shader %08X\n", hr );
        return -1;
    }

    printf("Success\n");

    // Clean up
    vsBlob->Release();
    psBlob->Release();

    return 0;
}
 
 */