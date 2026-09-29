#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_internal.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <array>
#include <memory>
#include <functional>
#include <chrono>
#include <string>
#include <map>

enum ImShaderTex_v2 : unsigned int { ImShaderTex_Default_v2 = 0, ImShaderTex_WindowBg_v2, ImShaderTex_COUNT_v2 };

namespace shaderrt_v2 {

    using ImDrawFlags_v2 = int;

    inline ID3D11Device* g_device_v2{};
    inline ID3D11DeviceContext* g_ctx_v2{};
    inline ImColor g_main_col_v2{};
    inline bool g_reset_time_v2 = false;

    inline float g_scale_v2 = 0.1f;
    inline float g_in_octaves_v2 = 7.f;
    inline float g_in_persistence_v2 = 10.f;

    struct VS_INPUT_v2 { DirectX::XMFLOAT3 position; };
    struct PS_INPUT_v2 { DirectX::XMFLOAT4 position; };

    inline const char* kVSH_v2 = R"(
struct VS_INPUT{ float3 position:POSITION; };
struct PS_INPUT{ float4 position:SV_POSITION; };
PS_INPUT main(VS_INPUT input){ PS_INPUT o; o.position=float4(input.position,1.0); return o; }
)";

    inline const char* kPSH_v2 = R"(
cbuffer DATA:register(b0){
  float in_time; float2 in_resolution; float in_octaves;
  float in_persistence; float in_scale; float4 in_bg_color; float4 in_fg_color; float2 align_;
}
static float4 gl_FragCoord; static float4 f_color;
struct SPIRV_Cross_Input{ float4 gl_FragCoord:SV_Position; };
struct SPIRV_Cross_Output{ float4 f_color:SV_Target0; };
void frag_main(){
  float2 uv=(2.0*gl_FragCoord.xy-in_resolution.xy)/min(in_resolution.x,in_resolution.y);
  [loop] for(int i=1;i<10;++i){ float fi=(float)i; uv.x+=0.6/fi*cos(fi*2.5*uv.y+in_time); uv.y+=0.6/fi*cos(fi*1.5*uv.x+in_time); }
  float v=0.1/abs(sin(in_time-uv.y-uv.x));
  float t=saturate(v);
  float3 rgb=lerp(in_bg_color.rgb,in_fg_color.rgb,t);
  float a=lerp(in_bg_color.a,in_fg_color.a,t);
  f_color=float4(rgb,a);
}
SPIRV_Cross_Output main(SPIRV_Cross_Input i):SV_Target{
  gl_FragCoord=i.gl_FragCoord; gl_FragCoord.w=1.0/max(gl_FragCoord.w,1e-6); frag_main();
  SPIRV_Cross_Output o; o.f_color=f_color; return o;
}
)";

    // Text shader that combines pattern with font texture alpha
    inline const char* kPSH_Text_v2 = R"(
cbuffer DATA : register(b0) {
  float in_time; float2 in_resolution; float in_octaves;
  float in_persistence; float in_scale; float4 in_bg_color; float4 in_fg_color; float2 align_;
}
struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);
float4 main(PS_INPUT input) : SV_Target {
  float2 uv=(2.0*input.pos.xy-in_resolution.xy)/min(in_resolution.x,in_resolution.y);
  [loop] for(int i=1;i<10;++i){ float fi=(float)i; uv.x+=0.6/fi*cos(fi*2.5*uv.y+in_time); uv.y+=0.6/fi*cos(fi*1.5*uv.x+in_time); }
  float v=0.1/abs(sin(in_time-uv.y-uv.x));
  float t=saturate(v);
  float3 rgb=lerp(in_bg_color.rgb,in_fg_color.rgb,t);
  float a=lerp(in_bg_color.a,in_fg_color.a,t);
  float4 font_col = texture0.Sample(sampler0, input.uv);
  // Combine pattern color with font alpha and vertex alpha
  return float4(rgb, a * font_col.a * input.col.a);
}
)";

    struct dx11_draw_data_v2 {
        ID3D11VertexShader* vertex_shader{};
        ID3D11InputLayout* input_layout{};
        ID3D11PixelShader* pixel_shader{};
        ID3D11Buffer* pixel_constant_buffer{};
        ID3D11BlendState* blend_state{};
    };

    struct dx11_tex_v2 {
        ID3D11Texture2D* tex{};
        ID3D11RenderTargetView* rtv{};
        ID3D11ShaderResourceView* srv{};
        ImVec2 size{};
        void release() { if (rtv) { rtv->Release(); rtv = nullptr; } if (tex) { tex->Release(); tex = nullptr; } if (srv) { srv->Release(); srv = nullptr; } }
        void create_tex() {
            size = ImGui::GetMainViewport()->Size;
            D3D11_TEXTURE2D_DESC d = {};
            d.Width = (UINT)size.x; d.Height = (UINT)size.y; d.MipLevels = 1; d.ArraySize = 1;
            d.Format = DXGI_FORMAT_R8G8B8A8_UNORM; d.SampleDesc.Count = 1;
            d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(g_device_v2->CreateTexture2D(&d, nullptr, &tex))) throw std::exception("CreateTexture2D");
        }
        void create_rtv() { if (FAILED(g_device_v2->CreateRenderTargetView(tex, nullptr, &rtv))) throw std::exception("CreateRTV"); }
        void create_srv() { if (FAILED(g_device_v2->CreateShaderResourceView(tex, nullptr, &srv))) throw std::exception("CreateSRV"); }
        void bind(ID3D11DeviceContext* ctx) {
            if (ImGui::GetMainViewport()->Size.x != size.x || ImGui::GetMainViewport()->Size.y != size.y) {
                release(); create_tex(); create_rtv(); create_srv();
            }
            ctx->OMSetRenderTargets(1, &rtv, nullptr);
        }
        ImTextureID texid() const { return srv; }
    };

    struct dx11_shader_v2 {
        dx11_draw_data_v2 dd{};
        dx11_tex_v2 tex;
        std::function<void(void*)> fill_cb;
        size_t cb_size{};
        static void print_err(ID3DBlob* e) {
            fprintf(stderr, "Shader compilation failed!\n%s\n", e ? (const char*)e->GetBufferPointer() : "no msg");
            fflush(stderr);
        }
        static void bind_vp(ID3D11DeviceContext* ctx) {
            const auto s = ImGui::GetMainViewport()->Size;
            D3D11_VIEWPORT vp{}; vp.Width = s.x; vp.Height = s.y; vp.MinDepth = 0; vp.MaxDepth = 1; vp.TopLeftX = 0; vp.TopLeftY = 0;
            ctx->RSSetViewports(1, &vp);
        }
        void make_vs(const char* src) {
            ID3DBlob* err{}, * blob{};
            if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &blob, &err))) throw std::exception("vs compile");
            if (g_device_v2->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &dd.vertex_shader) != S_OK) { blob->Release(); print_err(err); throw std::exception("vs create"); }
            D3D11_INPUT_ELEMENT_DESC layout[] = { {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0} };
            if (g_device_v2->CreateInputLayout(layout, 1, blob->GetBufferPointer(), blob->GetBufferSize(), &dd.input_layout) != S_OK) { blob->Release(); throw std::exception("input layout"); }
            blob->Release();
        }
        void make_ps(const char* src) {
            ID3DBlob* err{}, * blob{};
            if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &blob, &err))) { if (blob) blob->Release(); print_err(err); throw std::exception("ps compile"); }
            if (g_device_v2->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &dd.pixel_shader) != S_OK) { blob->Release(); throw std::exception("ps create"); }
            blob->Release();
        }
        void make_cb() {
            D3D11_BUFFER_DESC b{}; b.Usage = D3D11_USAGE_DYNAMIC; b.ByteWidth = (UINT)cb_size; b.BindFlags = D3D11_BIND_CONSTANT_BUFFER; b.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(g_device_v2->CreateBuffer(&b, nullptr, &dd.pixel_constant_buffer))) throw std::exception("cb");
        }
        void make_blend() {
            D3D11_BLEND_DESC d{}; d.AlphaToCoverageEnable = FALSE; d.RenderTarget[0].BlendEnable = TRUE;
            d.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; d.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; d.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            d.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; d.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA; d.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            d.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            if (FAILED(g_device_v2->CreateBlendState(&d, &dd.blend_state))) throw std::exception("blend");
        }
        dx11_shader_v2(const char* vsh, const char* psh, size_t cb_sz, std::function<void(void*)> cb) :fill_cb(std::move(cb)), cb_size(cb_sz) {
            make_vs(vsh); make_ps(psh); make_cb(); make_blend();
        }
        void bind(ID3D11DeviceContext* ctx) {
            tex.bind(ctx);
            bind_vp(ctx);
            ctx->IASetInputLayout(dd.input_layout);
            ctx->VSSetShader(dd.vertex_shader, nullptr, 0);
            ctx->PSSetShader(dd.pixel_shader, nullptr, 0);
            void* mem = malloc(cb_size);
            fill_cb(mem);
            D3D11_MAPPED_SUBRESOURCE m{};
            if (FAILED(ctx->Map(dd.pixel_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) throw std::exception("map cb");
            memcpy(m.pData, mem, cb_size); free(mem);
            ctx->Unmap(dd.pixel_constant_buffer, 0);
            ctx->PSSetConstantBuffers(0, 1, &dd.pixel_constant_buffer);
            constexpr float bf[4] = { 0,0,0,0 };
            ctx->OMSetBlendState(dd.blend_state, bf, 0xffffffff);
        }
        ImTextureID texid() const { return tex.texid(); }
    };

    struct dx11_vbo_v2 {
        ID3D11Buffer* vb{};
        dx11_vbo_v2() {
            struct V { DirectX::XMFLOAT3 p; };
            const V v[] = {
                {{-1.f, 1.f,0.f}}, {{ 1.f, 1.f,0.f}}, {{-1.f,-1.f,0.f}},
                {{-1.f,-1.f,0.f}}, {{ 1.f, 1.f,0.f}}, {{ 1.f,-1.f,0.f}},
            };
            D3D11_BUFFER_DESC d{}; d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_VERTEX_BUFFER; d.ByteWidth = sizeof(v);
            D3D11_SUBRESOURCE_DATA i{}; i.pSysMem = v;
            if (FAILED(g_device_v2->CreateBuffer(&d, &i, &vb))) throw std::exception("vb");
        }
        void draw(ID3D11DeviceContext* ctx) const {
            UINT stride = sizeof(DirectX::XMFLOAT3); UINT offset = 0;
            ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->Draw(6, 0);
        }
    };

    constexpr size_t kShadersCount_v2 = ImShaderTex_COUNT_v2;
    inline std::array<std::unique_ptr<dx11_shader_v2>, kShadersCount_v2> g_shaders_v2{};
    inline std::unique_ptr<dx11_vbo_v2> g_vbo_v2{};

    struct alignas(16) shader_data_v2 {
        float in_time; float in_resolution[2]; float in_octaves;
        float in_persistence; float in_scale; float _pad_c1[2];
        float in_bg_color[4];
        float in_fg_color[4];
        float align_[2]; float _pad_c4[2];
    };
    static_assert(sizeof(shader_data_v2) == 80, "bad size");
    static_assert(alignof(shader_data_v2) == 16, "bad align");

    inline ID3D11PixelShader* g_text_ps_v2 = nullptr;
    inline ID3D11Buffer* g_text_cb_v2 = nullptr;

    inline void CreateTextShader(ID3D11Device* device) {
        if (g_text_ps_v2) return;
        ID3DBlob* blob = nullptr;
        ID3DBlob* err = nullptr;
        if (FAILED(D3DCompile(kPSH_Text_v2, strlen(kPSH_Text_v2), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &blob, &err))) {
            if (err) err->Release();
            return;
        }
        device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &g_text_ps_v2);
        blob->Release();

        D3D11_BUFFER_DESC b{};
        b.Usage = D3D11_USAGE_DYNAMIC;
        b.ByteWidth = sizeof(shader_data_v2);
        b.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        b.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&b, nullptr, &g_text_cb_v2);
    }

    inline void BindTextShaderCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        if (!g_ctx_v2 || !g_text_ps_v2 || !g_text_cb_v2) return;

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE m{};
        if (SUCCEEDED(g_ctx_v2->Map(g_text_cb_v2, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            shader_data_v2 d{};
            const auto s = ImGui::GetMainViewport()->Size;
            d.in_time = (float)ImGui::GetTime() / 0.6f;
            d.in_resolution[0] = s.x / g_in_persistence_v2; d.in_resolution[1] = s.y / g_in_persistence_v2 / 1.2f;
            d.in_octaves = g_in_octaves_v2; d.in_persistence = g_in_persistence_v2; d.in_scale = g_scale_v2;
            d.in_fg_color[0] = 1.f; d.in_fg_color[1] = 1.f; d.in_fg_color[2] = 1.f; d.in_fg_color[3] = 1.f;
            d.in_bg_color[0] = 0.f; d.in_bg_color[1] = 0.f; d.in_bg_color[2] = 0.f; d.in_bg_color[3] = 1.f;
            memcpy(m.pData, &d, sizeof(d));
            g_ctx_v2->Unmap(g_text_cb_v2, 0);
        }

        g_ctx_v2->PSSetConstantBuffers(0, 1, &g_text_cb_v2);
        g_ctx_v2->PSSetShader(g_text_ps_v2, nullptr, 0);
    }

    inline void UnbindTextShaderCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        if (!g_ctx_v2) return;
        g_ctx_v2->PSSetShader(nullptr, nullptr, 0); // Reset to default (ImGui will set its own if needed, but nullptr might be unsafe if ImGui expects its shader. Ideally we should restore ImGui's shader but we don't know it easily. However, ImGui sets shader on every draw call usually, or we can just rely on the next ImGui command to set it.)
        // Actually, ImGui DX11 backend sets the shader when it renders. 
        // But since we are inside a callback, the next command might be another text draw that expects the default shader.
        // We can't easily restore the "previous" shader without querying it or knowing it.
        // BUT, ImGui's callback system is designed such that if we change state, we should restore it OR ImGui will reset it if we trigger a state change.
        // The safest way is to do nothing in Unbind if we assume ImGui resets state for next draw.
        // However, to be safe, we can set it to NULL, which forces ImGui to re-bind? No.
        // Let's just set it to NULL for now. 
        // Better yet: We don't need an Unbind callback if we assume the next draw call (if any) will set the correct shader.
        // But wait, if we have multiple text calls, we want them all to use this shader? No, just the title.
        // So we need to switch BACK to default ImGui shader.
        // Since we don't have the default ImGui shader handle here, we can't restore it.
        // TRICK: ImGui_ImplDX11_RenderDrawData sets the shader at the start.
        // If we change it, subsequent draw calls in the same batch will use our shader unless we change it back.
        // Since we can't change it back easily, we should ensure our text draw is isolated or we use the ResetCallback provided by ImGui? No such thing.
        // We will just leave it. If the next item is standard ImGui, it might render with our shader! This is bad.
        // SOLUTION: We can ask ImGui to reset render state? No.
        // We will rely on the fact that we only use this for the title, and we can try to find the default shader?
        // Actually, we can just save the PS before setting ours and restore it!
    }
    
    // Improved Bind with State Saving
    inline ID3D11PixelShader* g_saved_ps = nullptr;
    inline void BindTextShaderCallback_WithSave(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        if (!g_ctx_v2 || !g_text_ps_v2 || !g_text_cb_v2) return;
        
        g_ctx_v2->PSGetShader(&g_saved_ps, nullptr, nullptr); // Save current PS

        // Update CB
        D3D11_MAPPED_SUBRESOURCE m{};
        if (SUCCEEDED(g_ctx_v2->Map(g_text_cb_v2, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            shader_data_v2 d{};
            const auto s = ImGui::GetMainViewport()->Size;
            // Increased speed for "more" animation
            d.in_time = (float)ImGui::GetTime() / 0.3f; 
            
            // Higher persistence = denser pattern (More "electric")
            d.in_resolution[0] = s.x / (g_in_persistence_v2 * 1.5f); 
            d.in_resolution[1] = s.y / (g_in_persistence_v2 * 1.5f) / 1.2f;

            d.in_octaves = g_in_octaves_v2; d.in_persistence = g_in_persistence_v2; d.in_scale = g_scale_v2;
            
            // FG = Active Menu Color (Dynamic)
            d.in_fg_color[0] = g_main_col_v2.Value.x; 
            d.in_fg_color[1] = g_main_col_v2.Value.y; 
            d.in_fg_color[2] = g_main_col_v2.Value.z; 
            d.in_fg_color[3] = 1.f;
            // BG = White (Visible & Clean)
            d.in_bg_color[0] = 1.f; 
            d.in_bg_color[1] = 1.f; 
            d.in_bg_color[2] = 1.f; 
            d.in_bg_color[3] = 1.f;
            
            memcpy(m.pData, &d, sizeof(d));
            g_ctx_v2->Unmap(g_text_cb_v2, 0);
        }

        g_ctx_v2->PSSetConstantBuffers(0, 1, &g_text_cb_v2);
        g_ctx_v2->PSSetShader(g_text_ps_v2, nullptr, 0);
    }

    inline void UnbindTextShaderCallback_WithRestore(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        if (!g_ctx_v2) return;
        g_ctx_v2->PSSetShader(g_saved_ps, nullptr, 0);
        if (g_saved_ps) { g_saved_ps->Release(); g_saved_ps = nullptr; }
    }

    inline void NewFrame_v2(IDXGISwapChain* swap_chain, ID3D11Device* device, ID3D11DeviceContext* ctx, ImColor main_color) {
        g_device_v2 = device; g_ctx_v2 = ctx; g_main_col_v2 = main_color;
        
        // Ensure text shader is created
        CreateTextShader(device);

        struct B {
            UINT ViewportsCount;
            D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
            ID3D11BlendState* BlendState; FLOAT BlendFactor[4]; UINT SampleMask; UINT StencilRef;
            ID3D11DepthStencilState* DepthStencilState;
            ID3D11ShaderResourceView* PSShaderResource;
            ID3D11PixelShader* PS; ID3D11VertexShader* VS;
            UINT PSInstancesCount, VSInstancesCount;
            ID3D11ClassInstance* PSInstances[256], * VSInstances[256];
            D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
            ID3D11Buffer* VertexBuffer, * PSConstantBuffer; UINT VertexBufferStride, VertexBufferOffset; ID3D11InputLayout* InputLayout;
        } old{};
        old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
        ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
        ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
        ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
        old.PSInstancesCount = old.VSInstancesCount = 256;
        ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
        ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
        ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
        ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
        ctx->IAGetInputLayout(&old.InputLayout);

        if (!g_vbo_v2 || !g_reset_time_v2) {
            g_vbo_v2 = std::make_unique<dx11_vbo_v2>();
            g_shaders_v2[ImShaderTex_Default_v2] = std::make_unique<dx11_shader_v2>(
                kVSH_v2, kPSH_v2, sizeof(shader_data_v2),
                [](void* p) {
                    auto& d = *reinterpret_cast<shader_data_v2*>(p);
                    const auto s = ImGui::GetMainViewport()->Size;
                    d.in_time = (float)ImGui::GetTime();
                    d.in_resolution[0] = s.x; d.in_resolution[1] = s.y;
                    d.in_octaves = 7.f; d.in_persistence = 50.f; d.in_scale = 0.15f;
                    d.in_bg_color[0] = 0.02f; d.in_bg_color[1] = 0.02f; d.in_bg_color[2] = 0.02f; d.in_bg_color[3] = 1.f;
                    d.in_fg_color[0] = 0.f; d.in_fg_color[1] = 1.f; d.in_fg_color[2] = 0.f; d.in_fg_color[3] = 1.f;
                }
            );
            g_shaders_v2[ImShaderTex_WindowBg_v2] = std::make_unique<dx11_shader_v2>(
                kVSH_v2, kPSH_v2, sizeof(shader_data_v2),
                [](void* p) {
                    auto& d = *reinterpret_cast<shader_data_v2*>(p);
                    const auto s = ImGui::GetMainViewport()->Size;
                    d.in_time = (float)ImGui::GetTime() / 0.6f;
                    d.in_resolution[0] = s.x / g_in_persistence_v2; d.in_resolution[1] = s.y / g_in_persistence_v2 / 1.2f;
                    d.in_octaves = g_in_octaves_v2; d.in_persistence = g_in_persistence_v2; d.in_scale = g_scale_v2;
                    d.in_fg_color[0] = 1.f; d.in_fg_color[1] = 1.f; d.in_fg_color[2] = 1.f; d.in_fg_color[3] = 1.f;
                    d.in_bg_color[0] = 0.f; d.in_bg_color[1] = 0.f; d.in_bg_color[2] = 0.f; d.in_bg_color[3] = 1.f;
                    g_reset_time_v2 = true;
                }
            );
        }

        for (auto& s : g_shaders_v2) { if (s) { s->bind(ctx); g_vbo_v2->draw(ctx); } }

        ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
        ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);
        if (old.BlendState) old.BlendState->Release();
        ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);
        if (old.DepthStencilState) old.DepthStencilState->Release();
        ctx->PSSetShaderResources(0, 1, &old.PSShaderResource);
        if (old.PSShaderResource) old.PSShaderResource->Release();
        ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount);
        if (old.PS) old.PS->Release();
        for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
        ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount);
        if (old.VS) old.VS->Release();
        for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
        ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
        ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
        if (old.VertexBuffer) old.VertexBuffer->Release();
        ctx->IASetInputLayout(old.InputLayout);
        if (old.InputLayout) old.InputLayout->Release();
    }

    inline ImTextureID Get_v2(ImShaderTex_v2 s) {
        if (!g_shaders_v2[0]) return nullptr;
        return g_shaders_v2[s]->texid();
    }

    inline void Draw_v2(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding, float alpha, ImShaderTex_v2 s) {
        auto* t = Get_v2(s);
        if (!t) return;
        const auto vp = ImGui::GetMainViewport()->Size;
        const auto uv_min = ImVec2(min.x / vp.x, min.y / vp.y);
        const auto uv_max = ImVec2(max.x / vp.x, max.y / vp.y);
        const auto col = ImGui::GetColorU32(0xFFFFFFFF, alpha);
        dl->AddImageRounded(t, min, max, uv_min, uv_max, ImColor(g_main_col_v2.Value.x, g_main_col_v2.Value.y, g_main_col_v2.Value.z, alpha), rounding);
    }

    inline void UI_v2() {
        ImGui::SliderFloat("Scale", &g_scale_v2, 0.f, 100.f);
        ImGui::SliderFloat("in_persistence", &g_in_persistence_v2, 0.f, 100.f);
        ImGui::SliderFloat("in_octaves", &g_in_octaves_v2, 0.f, 60.f);

    }

} // namespace shaderrt_v2
