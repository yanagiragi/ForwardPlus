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

#include "Game.h"
#include "Camera.h"
#include "Mesh.h"

#include "Material.h"
#include "Light.h"
#include "Entity.h"

#define MAX_LIGHTS 8

enum ConstantBuffer
{
    CB_Application,
    CB_Frame,
    CB_Object,
    NumConstantBuffers
};

struct ApplicationConstantBuffer
{
    DirectX::SimpleMath::Matrix projectionMatrix;
};

struct FrameConstantBuffer
{
    DirectX::SimpleMath::Matrix viewMatrix;
    DirectX::SimpleMath::Vector4 eyePosition;
    struct Light lights[MAX_LIGHTS];
};

struct ObjectConstantBuffer
{
    DirectX::SimpleMath::Matrix WorldMatrix;
    DirectX::SimpleMath::Matrix NormalMatrix;
    struct Material Material;
};

class SimpleObj : public Game
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

private:

    // Functions
    void LoadShaderResources();
    void LoadEffect();
    void LoadTexture();
    void LoadLight();

    void RenderScene();
    void RenderDebug();

    void SetupImgui();
    void RenderImgui(RenderEventArgs& e);

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

    // Vertex buffer data
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dInputLayout = nullptr;

    // Shader data
    __int64 m_d3dVertexShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;
    __int64 m_d3dPixelShaderSize = 0;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

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
    struct ApplicationConstantBuffer m_ApplicationConstantBuffer;
    struct FrameConstantBuffer m_FrameConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dConstantBuffers[NumConstantBuffers];

    struct Material defaultMaterial;
    struct Material diffuseMaterial = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4::One, // Diffuse
        Vector4::Zero, // Specular
    };

    struct Material boxMaterial = {
        Vector4::Zero, // Emissive
        Vector4::One, // Ambient
        Vector4::One, // Diffuse
        Vector4::Zero, // Specular
        true // use texture
    };

    std::vector<Entity*> m_Scene;

    const float fov = DirectX::XM_PI / 4.f;
    const float nearPlane = 0.1f;
    const float farPlane = 100.f;

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