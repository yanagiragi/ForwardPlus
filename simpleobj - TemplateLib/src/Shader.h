#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <string>

#include "Common.h"

using namespace Microsoft::WRL;

template<class ShaderClass>
std::string GetLatestProfile(ComPtr<ID3D11Device> device);

template<>
std::string GetLatestProfile<ComPtr<ID3D11VertexShader>>(ComPtr<ID3D11Device> device);

template<>
std::string GetLatestProfile<ComPtr<ID3D11PixelShader>>(ComPtr<ID3D11Device> device);

template<class ShaderClass>
ShaderClass CreateShader(ComPtr<ID3D11Device> device, ComPtr<ID3DBlob> pShaderBlob, ID3D11ClassLinkage* pClassLinkage);

/// <summary>
/// Get latest d3d profile of vertex shader
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
template<>
std::string GetLatestProfile<ComPtr<ID3D11VertexShader>>(ComPtr<ID3D11Device> device)
{
    AssertIfNull(device, "GetLatestProfile", "Device is null");

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "vs_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "vs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "vs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "vs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "vs_4_0_level_9_1";
    }
    break;
    } // switch( featureLevel )

    return "";
}

/// <summary>
/// Get latest d3d profile of pixel shader
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
template<>
std::string GetLatestProfile<ComPtr<ID3D11PixelShader>>(ComPtr<ID3D11Device> device)
{
    AssertIfNull(device, "GetLatestProfile", "Device is null");

    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "ps_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "ps_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "ps_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "ps_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "ps_4_0_level_9_1";
    }
    break;
    }
    return "";
}

/// <summary>
/// Create vertex shader
/// </summary>
/// <param name="device"></param>
/// <param name="pShaderBlob"></param>
/// <param name="pClassLinkage"></param>
/// <returns>pointer to ID3D11VertexShader instance</returns>
template<>
ComPtr<ID3D11VertexShader> CreateShader<ComPtr<ID3D11VertexShader>>(
    ComPtr<ID3D11Device> device, 
    ComPtr<ID3DBlob> pShaderBlob, 
    ID3D11ClassLinkage* pClassLinkage
    )
{
    AssertIfNull(device, "GetLatestProfile", "Device is null");
    AssertIfNull(pShaderBlob, "GetLatestProfile", "shader plob is null");

    ComPtr<ID3D11VertexShader> pVertexShader = nullptr;
    device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader);

    return pVertexShader;
}

/// <summary>
/// Create pixel shader
/// </summary>
/// <param name="device"></param>
/// <param name="pShaderBlob"></param>
/// <param name="pClassLinkage"></param>
/// <returns>pointer to ID3D11VertexShader instance</returns>
template<>
ComPtr<ID3D11PixelShader> CreateShader<ComPtr<ID3D11PixelShader>>(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3DBlob> pShaderBlob,
    ID3D11ClassLinkage* pClassLinkage
    )
{
    AssertIfNull(device, "GetLatestProfile", "Device is null");
    AssertIfNull(pShaderBlob, "GetLatestProfile", "shader plob is null");

    ComPtr<ID3D11PixelShader> pPixelShader = nullptr;
    device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader);

    return pPixelShader;
}

/// <summary>
/// Load Shader and compile it
/// </summary>
/// <typeparam name="ShaderClass">ID3D11VertexShader or ID3D11PixelShader</typeparam>
/// <param name="fileName"></param>
/// <param name="entryPoint"></param>
/// <param name="_profile"></param>
/// <returns>pointer to ID3DBlob instance</returns>
template<class ShaderClass>
ComPtr<ID3DBlob> LoadShader(ComPtr<ID3D11Device> d3dDevice, const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile)
{
    ComPtr<ID3DBlob> pShaderBlob = nullptr;
    ComPtr<ID3DBlob> pErrorBlob = nullptr;

    std::string profile = _profile;
    if (profile == "latest")
    {
        profile = GetLatestProfile<ShaderClass>(d3dDevice);
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompileFromFile(
        fileName.c_str(),                       // name of the shader file
        nullptr,                                // optional array of shader macros
        D3D_COMPILE_STANDARD_FILE_INCLUDE,      // optional pointer to include files, D3D_COMPILE_STANDARD_FILE_INCLUDE implies it include files that are relative to the current directory
        entryPoint.c_str(),                     // entry point of the shader
        profile.c_str(),                        // shader target
        flags,                                  // shader compile options of a HLSL code
        0,                                      // shader compile options of a effect
        &pShaderBlob,                           // pointer to the compiled code
        &pErrorBlob                             // pointer to the error messages
    );

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            std::string errorMessage = (char*)pErrorBlob->GetBufferPointer();
            OutputDebugStringA(errorMessage.c_str());

            SafeRelease(pShaderBlob);
            SafeRelease(pErrorBlob);
        }

        return nullptr;
    }

    SafeRelease(pErrorBlob);

    return pShaderBlob;
}
