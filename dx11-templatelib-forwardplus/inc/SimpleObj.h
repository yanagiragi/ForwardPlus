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

struct BufType
{
    int i;
    float f;
};

class SimpleObj final : public Game
{
    // aliases
    typedef Game base;

public:
    // Construct and Destructor
    SimpleObj(Window& window);
    virtual ~SimpleObj();

    // Load content required for the demo.
    virtual bool LoadContent();

    // Unload demo specific content that was loaded in LoadContent.
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
    #pragma region Functions
    void LoadShaderResources();
    void LoadDebugDraw();
    void LoadTexture();
    void LoadLight();

    void RenderScene(RenderEventArgs& e);
    void RenderDebug(RenderEventArgs& e);

    void SetupImgui();
    void RenderImgui(RenderEventArgs& e);

    void RenderScene_Forward(RenderEventArgs& e);
    void RenderScene_Deferred(RenderEventArgs& e);

    void RenderScene_Deferred_GeometryPass();
    void RenderScene_Deferred_DebugPass();
    void RenderScene_Deferred_LightingPass_Loop();
    void RenderScene_Deferred_LightingPass_Single();
    void RenderScene_Deferred_LightingPass_Stencil();
    void DrawLightVolume(Light* type);

    bool ResizeSwapChain(int width, int height);

    // Wrap to native API
    void Draw(UINT VertexCount, UINT StartVertexLocation);
    void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);

#pragma endregion

    #pragma region Variables

    // Input Flags
    // For panning the camera.
    int m_W, m_A, m_S, m_D;
    int m_Q, m_E;
    bool m_bShift;
    float m_Pitch, m_Yaw;
    DirectX::XMINT2 m_PreviousMousePosition = {0, 0};

    // Shader data
    __int64 m_d3dRegularVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dRegularVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dRegularInputLayout = nullptr;

    __int64 m_d3dForward_LoopLight_PixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dForward_LoopLight_PixelShader = nullptr;
    
    __int64 m_d3dInstancedVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dInstancedVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dInstancedInputLayout = nullptr;

    __int64 m_d3dForward_LoopLight_InstancedPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dForward_LoopLight_InstancedPixelShader = nullptr;

    __int64 m_d3dForward_SingleLight_PixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dForward_SingleLight_PixelShader = nullptr;

    __int64 m_d3dForward_SingleLight_InstancedPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dForward_SingleLight_InstancedPixelShader = nullptr;

    __int64 m_d3dDeferredGeometry_RegularVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredGeometry_RegularVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dDeferredGeometry_RegularInputLayout = nullptr;

    __int64 m_d3dDeferredGeometry_RegularPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredGeometry_RegularPixelShader = nullptr;

    __int64 m_d3dDeferredGeometry_InstancedVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredGeometry_InstancedVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dDeferredGeometry_InstancedInputLayout = nullptr;

    __int64 m_d3dDeferredGeometry_InstancedPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredGeometry_InstancedPixelShader = nullptr;

    __int64 m_d3dDebugVertexShaderSize = 0;
    __int64 m_d3dDebugPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDebugVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDebugPixelShader = nullptr;

    __int64 m_d3dDeferredLightingVertexShaderSize = 0;
    __int64 m_d3dDeferredLighting_LoopLight_PixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredLightingVertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredLighting_LoopLight_PixelShader = nullptr;

    __int64 m_d3dDeferredLighting_SingleLight_PixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dDeferredLighting_SingleLight_PixelShader = nullptr;

    __int64 m_d3dDeferredLighting_LightVolume_VertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dDeferredLighting_LightVolume_VertexShader = nullptr;

    __int64 m_d3dUnlitPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dUnlitPixelShader = nullptr;

    __int64 m_d3dFowrardPlus_ComputeShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_d3dFowrardPlus_ComputeShader = nullptr;

    // Primitive Batch
    std::unique_ptr<DirectX::CommonStates> m_d3dStates = nullptr;
    std::unique_ptr<DirectX::BasicEffect> m_d3dEffect;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_d3dPrimitiveBatch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dPrimitiveBatchInputLayout;

    // Texture data
    std::unique_ptr<DirectX::EffectFactory> m_d3dEffectFactory = nullptr;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_d3dSamplerState = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_GridTexture = nullptr;

    // Constant Buffers
    struct ObjectConstantBuffer m_ObjectConstantBuffer;
    struct FrameConstantBuffer m_FrameConstantBuffer;
    struct MaterialProperties m_MaterialPropertiesConstantBuffer;
    struct LightProperties m_LightPropertiesConstantBuffer;
    struct DebugProperties m_DebugPropertiesConstantBuffer;
    struct ScreenToViewParams m_ScreenToViewParamsConstantBuffer;
    struct LightingCalculationOptions m_LightingCalculationOptionsConstrantBuffer;
    
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

    // Blend / Depth / Stencil States
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_d3dBlendState_Add;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState_DisableDepthTest;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState_Overlay;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState_UnmarkPixels;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState_ShadePixels;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState_Debug;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_d3dCullFrontRasterizerState;

    // Materials
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

    // Light Volume Settings
    Model* m_lightVolume_sphere;
    const float m_lightVolume_coneSpotAngle = 26.565f * 2.0f;
    Model* m_lightVolume_cone;

    // Camera Settings
    Camera m_Camera;
    Vector4 m_InitialCameraPos;
    Vector4 m_InitialCameraRot;
    const float fovInDegree = 45.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;

    // Debug options
    RenderMode m_RenderMode = RenderMode::Forward;
    Deferred_DebugMode m_DeferredDebugMode = Deferred_DebugMode::None;
    float m_DeferredDepthPower = 500.0f;
    LightingSpace m_LightingSpace = LightingSpace::World;
    int m_LightCalculationCount = MAX_LIGHTS;
    LightCalculationMode m_LightCalculationMode = LightCalculationMode::Single;

    // Others
    Scene m_Scene;
    int m_DrawCallCount = 0;
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

    #pragma endregion

    // test variables

    struct BufType g_vBuf0[1] = { { 1, 0.4 } };
    struct BufType g_vBuf1[1] = { { 2, 0.3 } };
    struct BufType g_vBufResult[1] = { { 0, 0 } };

    ID3D11Buffer* g_pBuf0 = nullptr;
    ID3D11Buffer* g_pBuf1 = nullptr;
    ID3D11Buffer* g_pBufResult = nullptr;

    ID3D11ShaderResourceView* g_pBuf0SRV = nullptr;
    ID3D11ShaderResourceView* g_pBuf1SRV = nullptr;
    ID3D11UnorderedAccessView* g_pBufResultUAV = nullptr;
};