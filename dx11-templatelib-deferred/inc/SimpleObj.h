#pragma once

// DirectX includes
#include <DirectXMath.h>

// DirectXTK includes
#include "SimpleMath.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "CommonStates.h"
#include "Effects.h"
#include "DebugDraw.h"
#include "DirectXHelpers.h" // for CreateInputLayoutFromEffect()

// imgui includes
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#define VGM_USES_LEFT_HAND_AXES
#include "imGuIZMOquat.h"

// DirectXTemplateLib includes
#include "Game.h"
#include "Camera.h"
#include "Mesh.h"

// our includes
#include "Scene.h"
#include "Entity.h"
#include "Type.h"

class SimpleObj final : public Game
{
public:
    typedef Game base;

    SimpleObj(Window& window);
    virtual ~SimpleObj();

    /**
     *  Load content required for the demo.
     */
    virtual bool LoadContent();

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    virtual void UnloadContent();

protected:
    // Don't allow copying of the demo.
    SimpleObj(const SimpleObj& copy);

    virtual void OnUpdate(UpdateEventArgs& e);
    virtual void OnRender(RenderEventArgs& e);

    virtual void OnKeyPressed(KeyEventArgs& e);
    virtual void OnKeyReleased(KeyEventArgs& e);

    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
    virtual void OnMouseMoved(MouseMotionEventArgs& e);

    virtual void OnMouseWheel(MouseWheelEventArgs& e);

    virtual void OnResize(ResizeEventArgs& e);
    virtual void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);

private:

    // Functions
    void LoadShaderResources();
    void LoadDebugDraw();
    void LoadTexture();
    void LoadLight();

    void RenderScene();
    void RenderDebug();

    void SetupImgui();
    void RenderImgui(RenderEventArgs& e);

    void RenderScene_Forward();
    void RenderScene_Deferred();

    void RenderScene_Deferred_GeometryPass();
    void RenderScene_Deferred_DebugPass();
    void RenderScene_Deferred_LightingPass();

    bool ResizeSwapChain(int width, int height);

    // Variables
    Camera m_Camera;

    __declspec(align(16)) struct AlignedData
    {
        DirectX::XMVECTOR m_InitialCameraPos;
        DirectX::XMVECTOR m_InitialCameraRot;
    };
    AlignedData* pData;

    // For panning the camera.
    int m_W, m_A, m_S, m_D;
    int m_Q, m_E;
    bool m_bShift;
    float m_Pitch, m_Yaw;

    DirectX::XMINT2 m_PreviousMousePosition = {0, 0};

    // Shader data
    __int64 m_d3dVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dInputLayout = nullptr;

    __int64 m_d3dPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;
    
    __int64 m_d3dInstancedVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dInstancedVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dInstancedInputLayout = nullptr;

    __int64 m_d3dInstancedPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dInstancedPixelShader = nullptr;

    __int64 m_d3dDeferredGeometryVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredGeometryVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dDeferredGeometryInputLayout = nullptr;

    __int64 m_d3dDeferredGeometryPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredGeometryPixelShader = nullptr;

    __int64 m_d3dDebugVertexShaderSize = 0;
    __int64 m_d3dDebugPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDebugVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDebugPixelShader = nullptr;

    __int64 m_d3dDeferredLightingVertexShaderSize = 0;
    __int64 m_d3dDeferredLightingPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredLightingVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredLightingPixelShader = nullptr;

    // Primitive Batch
    std::unique_ptr<DirectX::CommonStates> m_d3dStates = nullptr;
    std::unique_ptr<DirectX::BasicEffect> m_d3dEffect;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_d3dPrimitiveBatch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dPrimitiveBatchInputLayout;

    // Texture data
    std::unique_ptr<DirectX::EffectFactory> m_d3dEffectFactory = nullptr;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_d3dSamplerState = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GridTexture = nullptr;

    struct ObjectConstantBuffer m_ObjectConstantBuffer;
    struct FrameConstantBuffer m_FrameConstantBuffer;
    struct MaterialProperties m_MaterialPropertiesConstantBuffer;
    struct LightProperties m_LightPropertiesConstantBuffer;
    struct DebugProperties m_DebugPropertiesConstantBuffer;
    struct ScreenToViewParams m_ScreenToViewParamsConstantBuffer;
    
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dConstantBuffers[NumConstantBuffers];

    // Deferred Render target views
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView_lightAccumulation;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3dRenderTargetView_lightAccumulation_SRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dRenderTargetView_lightAccumulation_tex;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView_depth;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3dDepthStencilView_depth_SRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dDepthStencilView_depth_tex;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView_diffuse;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3dRenderTargetView_diffuse_SRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dRenderTargetView_diffuse_tex;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView_specular;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3dRenderTargetView_specular_SRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dRenderTargetView_specular_tex;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView_normal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3dRenderTargetView_normal_SRV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3dRenderTargetView_normal_tex;

    struct Material defaultMaterial;
    struct Material diffuseMaterial = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4::One, // Diffuse
        Vector4::Zero, // Specular
    };

    struct Material bunny1Material = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4(115, 165, 245, 255) / 255.0, // Diffuse
        Vector4::One, // Specular
    };

    struct Material bunny2Material = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4(245, 197, 115, 255) / 255.0, // Diffuse
        Vector4::One, // Specular
    };

    struct Material boxMaterial = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4::One, // Diffuse
        Vector4::Zero, // Specular
        true // use texture
    };

    Scene m_Scene;

    const float fovInDegree = 45.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;

    RenderMode m_RenderMode = RenderMode::Forward;
    Deferred_DebugMode m_DeferredDebugMode = Deferred_DebugMode::None;
    float m_DeferredDepthPower = 500.0f;

    Vector2 m_ScreenDimensions;

    // UI Flags
    bool m_ShowGizmoWindow = false;
    std::function<quat(void)> m_GizmoWindowQuaternionGetter = nullptr;
    std::function<void(quat)> m_GizmoWindowQuaternionSetter = nullptr;
    std::function<const char* (void)> m_GizmoWindowNameGetter = nullptr;

    bool m_ShowDirectionWindow = false;
    std::function<vec3(void)> m_DirectionWindowVec3Getter = nullptr;
    std::function<void(vec3)> m_DirectionWindowVec3Setter = nullptr;
    std::function<const char* (void)> m_DirectionWindowNameGetter = nullptr;
};