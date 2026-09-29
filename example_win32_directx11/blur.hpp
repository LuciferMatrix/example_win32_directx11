#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"
#include <DirectXMath.h>
#include <array>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>
#include <wrl/client.h>
#include <cmath>
#include <type_traits>
#include <exception>

namespace img_blur {

    using Microsoft::WRL::ComPtr;

    inline ID3D11Device* g_device{};
    inline IDXGISwapChain* g_swap{};
    inline ID3D11RenderTargetView* g_rtv{};
    inline bool g_effects_enabled = true;

    struct ImGui_ImplDX11_Data {
        ID3D11Device* pd3dDevice;
        ID3D11DeviceContext* pd3dDeviceContext;
        IDXGIFactory* pFactory;
        ID3D11Buffer* pVB;
        ID3D11Buffer* pIB;
        ID3D11VertexShader* pVertexShader;
        ID3D11InputLayout* pInputLayout;
        ID3D11Buffer* pVertexConstantBuffer;
        ID3D11PixelShader* pPixelShader;
        ID3D11SamplerState* pFontSampler;
        ID3D11ShaderResourceView* pFontTextureView;
        ID3D11RasterizerState* pRasterizerState;
        ID3D11BlendState* pBlendState;
        ID3D11DepthStencilState* pDepthStencilState;
        int VertexBufferSize;
        int IndexBufferSize;
        ImGui_ImplDX11_Data() { memset(this, 0, sizeof(*this)); VertexBufferSize = 5000; IndexBufferSize = 10000; }
    };

    struct VTX_CBUFFER { float mvp[4][4]; };

    inline ImGui_ImplDX11_Data* GetBD() {
        return ImGui::GetCurrentContext()
            ? (ImGui_ImplDX11_Data*)ImGui::GetIO().BackendRendererUserData
            : nullptr;
    }

    inline const char psh_x_data[] = R"(
SamplerState sampler0;
Texture2D texture0;
struct PS_INPUT { float4 pos:SV_POSITION; float4 col:COLOR0; float2 uv:TEXCOORD0; };
cbuffer ConstantBuffer:register(b0){ float texelWidth; }
float4 main(PS_INPUT input):SV_Target{
    float2 uv=input.uv;
    float3 c=texture0.SampleLevel(sampler0,uv,0).rgb*0.2270270270f;
    float2 d1=float2(texelWidth*2.7692307692f,0);
    float2 d2=float2(texelWidth*6.4615384616f,0);
    float3 p1=texture0.SampleLevel(sampler0,uv+d1,0).rgb+texture0.SampleLevel(sampler0,uv-d1,0).rgb;
    float3 p2=texture0.SampleLevel(sampler0,uv+d2,0).rgb+texture0.SampleLevel(sampler0,uv-d2,0).rgb;
    c+=p1*0.3162162162f;
    c+=p2*0.0702702703f;
    float a=texture0.SampleLevel(sampler0,uv,0).a;
    return float4(c,a);
}
)";

    inline const char psh_y_data[] = R"(
SamplerState sampler0;
Texture2D texture0;
cbuffer ConstantBuffer:register(b0){ float texelHeight; }
struct PS_INPUT { float4 pos:SV_POSITION; float4 col:COLOR0; float2 uv:TEXCOORD0; };
float4 main(PS_INPUT input):SV_Target{
    float2 uv=input.uv;
    float3 c=texture0.SampleLevel(sampler0,uv,0).rgb*0.2270270270f;
    float2 d1=float2(0,texelHeight*2.7692307692f);
    float2 d2=float2(0,texelHeight*6.4615384616f);
    float3 p1=texture0.SampleLevel(sampler0,uv+d1,0).rgb+texture0.SampleLevel(sampler0,uv-d1,0).rgb;
    float3 p2=texture0.SampleLevel(sampler0,uv+d2,0).rgb+texture0.SampleLevel(sampler0,uv-d2,0).rgb;
    c+=p1*0.3162162162f;
    c+=p2*0.0702702703f;
    float a=texture0.SampleLevel(sampler0,uv,0).a;
    return float4(c,a);
}
)";

    class pixel_shader {
        ComPtr<ID3D11PixelShader> ps_;
        ComPtr<ID3D11Buffer> cbuf_;
        struct CB { float v; float pad[3]; } data_{};
        void make_ps(const char* src) {
            ID3DBlob* bin{}; ID3DBlob* err{};
            if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &bin, &err))) {
                fprintf(stderr, "PS compile fail\n%s\n", err ? (char*)err->GetBufferPointer() : ""); fflush(stderr);
                throw std::exception("ps compile");
            }
            auto* dev = GetBD()->pd3dDevice;
            if (FAILED(dev->CreatePixelShader(bin->GetBufferPointer(), bin->GetBufferSize(), nullptr, ps_.GetAddressOf()))) { bin->Release(); throw std::exception("ps create"); }
            bin->Release();
        }
        void make_cb() {
            auto* dev = GetBD()->pd3dDevice;
            D3D11_BUFFER_DESC d{}; d.Usage = D3D11_USAGE_DYNAMIC; d.ByteWidth = sizeof(CB); d.BindFlags = D3D11_BIND_CONSTANT_BUFFER; d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(dev->CreateBuffer(&d, nullptr, cbuf_.GetAddressOf()))) throw std::exception("cb create");
        }
    public:
        bool ready() const { return ps_.Get() != nullptr; }
        void init(const char* src) { make_ps(src); make_cb(); }
        void use(ID3D11DeviceContext* ctx, float v) {
            ctx->PSSetShader(ps_.Get(), nullptr, 0);
            data_.v = v;
            D3D11_MAPPED_SUBRESOURCE m{};
            auto* b = cbuf_.Get();
            if (FAILED(ctx->Map(b, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) throw std::exception("cb map");
            memcpy(m.pData, &data_, sizeof(CB));
            ctx->Unmap(b, 0);
            ctx->PSSetConstantBuffers(0, 1, &b);
        }
    };

    template<class T>
    void rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

    class blur {
    public:
        float w{}, h{};
        ID3D11Texture2D* tex1{}; ID3D11RenderTargetView* tex1_rtv{}; ID3D11ShaderResourceView* tex1_srv{};
        ID3D11Texture2D* tex2{}; ID3D11RenderTargetView* tex2_rtv{}; ID3D11ShaderResourceView* tex2_srv{};
        ID3D11Buffer* vtx_cb{};
        pixel_shader psx, psy;
        ~blur() { rel(tex1); rel(tex1_rtv); rel(tex1_srv); rel(tex2); rel(tex2_rtv); rel(tex2_srv); rel(vtx_cb); }
        void new_frame() {
            auto s = ImGui::GetIO().DisplaySize;
            if (s.x == w && s.y == h) return;
            w = s.x; h = s.y;
            rel(tex1); rel(tex1_rtv); rel(tex1_srv);
            rel(tex2); rel(tex2_rtv); rel(tex2_srv);
        }
        void draw(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding, float alpha, ImDrawFlags flags, bool reuse, float angle_rad = 0.0f) {
            if (!reuse) {
                auto* dev = GetBD()->pd3dDevice;
                if (!tex1) create_tex(w, h, &tex1, &tex1_rtv, &tex1_srv);
                if (!tex2) create_tex(w, h, &tex2, &tex2_rtv, &tex2_srv);
                if (!psx.ready()) psx.init(psh_x_data);
                if (!psy.ready()) psy.init(psh_y_data);
                if (!vtx_cb) create_vtx_cb(dev, &vtx_cb);
                static constexpr ImDrawCallback cbs[4] = {
                    [](const ImDrawList*,const ImDrawCmd*) { begin_static(); },
                    [](const ImDrawList*,const ImDrawCmd*) { fp_static(); },
                    [](const ImDrawList*,const ImDrawCmd*) { sp_static(); },
                    [](const ImDrawList*,const ImDrawCmd*) { end_static(); }
                };
                dl->AddCallback(cbs[0], nullptr);
                constexpr ImRect quad{ -1,-1,1,1 };
                for (int i = 0; i < 8; ++i) {
                    dl->AddCallback(cbs[1], nullptr);
                    dl->AddImage(tex1_srv, quad.Min, quad.Max);
                    dl->AddCallback(cbs[2], nullptr);
                    dl->AddImage(tex2_srv, quad.Min, quad.Max);
                }
                dl->AddCallback(cbs[3], nullptr);
                dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
            }
            auto sz = ImGui::GetIO().DisplaySize;
            auto uv_min = ImVec2(min.x, sz.y - min.y) / sz;
            auto uv_max = ImVec2(max.x, sz.y - max.y) / sz;
            if (fabsf(angle_rad) < 1e-6f) {
                dl->AddImageRounded(tex2_srv, min, max, uv_min, uv_max, IM_COL32(255, 255, 255, (int)(255 * alpha)), rounding, flags);
            }
            else {
                const float s = sinf(angle_rad);
                const float c = cosf(angle_rad);
                const ImVec2 center = (min + max) * 0.5f;
                ImVec2 tl = ImVec2(min.x, min.y);
                ImVec2 tr = ImVec2(max.x, min.y);
                ImVec2 br = ImVec2(max.x, max.y);
                ImVec2 bl = ImVec2(min.x, max.y);
                auto rot = [&](const ImVec2& p)->ImVec2 {
                    ImVec2 r = ImVec2(p.x - center.x, p.y - center.y);
                    ImVec2 q = ImVec2(r.x * c - r.y * s, r.x * s + r.y * c);
                    return ImVec2(center.x + q.x, center.y + q.y);
                    };
                ImVec2 p0 = rot(tl);
                ImVec2 p1 = rot(tr);
                ImVec2 p2 = rot(br);
                ImVec2 p3 = rot(bl);
                ImVec2 uv_tl = ImVec2(uv_min.x, uv_min.y);
                ImVec2 uv_tr = ImVec2(uv_max.x, uv_min.y);
                ImVec2 uv_br = ImVec2(uv_max.x, uv_max.y);
                ImVec2 uv_bl = ImVec2(uv_min.x, uv_max.y);
                dl->AddImageQuad((ImTextureID)tex2_srv,
                    p0, p1, p2, p3,
                    uv_tl, uv_tr, uv_br, uv_bl,
                    IM_COL32(255, 255, 255, (int)(255 * alpha))
                );
            }
        }
    private:
        static void copy_backbuffer(ID3D11DeviceContext* ctx, ID3D11Texture2D* tex) {
            ComPtr<ID3D11Resource> bb;
            if (FAILED(g_swap->GetBuffer(0, IID_PPV_ARGS(&bb)))) throw std::exception("get backbuffer");
            ctx->CopySubresourceRegion(tex, 0, 0, 0, 0, bb.Get(), 0, nullptr);
        }
        static void push_mvp(ID3D11DeviceContext* ctx, ID3D11Buffer* cb) {
            D3D11_MAPPED_SUBRESOURCE m{};
            if (ctx->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &m) != S_OK) throw std::exception("vtx map");
            auto s = ImGui::GetIO().DisplaySize;
            float x = s.x, y = s.y;
            static float mvp[4][4] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, -1.f / x,1.f / y,0,1 };
            memcpy(m.pData, mvp, sizeof(VTX_CBUFFER));
            ctx->Unmap(cb, 0);
            ctx->VSSetConstantBuffers(0, 1, &cb);
        }
        static void begin_static() {
            auto* bd = GetBD();
            auto* ctx = bd->pd3dDeviceContext;
            instance().copy_backbuffer(ctx, instance().tex1);
            instance().push_mvp(ctx, instance().vtx_cb);
        }
        static void end_static() {
            auto* ctx = GetBD()->pd3dDeviceContext;
            ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        }
        static void fp_static() {
            auto* ctx = GetBD()->pd3dDeviceContext;
            ctx->OMSetRenderTargets(1, &instance().tex2_rtv, nullptr);
            instance().psx.use(ctx, 1.f / instance().w);
        }
        static void sp_static() {
            auto* ctx = GetBD()->pd3dDeviceContext;
            ctx->OMSetRenderTargets(1, &instance().tex1_rtv, nullptr);
            instance().psy.use(ctx, 1.f / instance().h);
        }
        static void create_tex(float w, float h, ID3D11Texture2D** t, ID3D11RenderTargetView** r, ID3D11ShaderResourceView** s) {
            auto* dev = GetBD()->pd3dDevice;
            D3D11_TEXTURE2D_DESC d{}; d.Width = (UINT)w; d.Height = (UINT)h; d.MipLevels = 1; d.ArraySize = 1; d.Format = DXGI_FORMAT_R8G8B8A8_UNORM; d.SampleDesc.Count = 1; d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(dev->CreateTexture2D(&d, nullptr, t))) throw std::exception("tex");
            if (FAILED(dev->CreateRenderTargetView(*t, nullptr, r))) throw std::exception("rtv");
            if (FAILED(dev->CreateShaderResourceView(*t, nullptr, s))) throw std::exception("srv");
        }
        static void create_vtx_cb(ID3D11Device* dev, ID3D11Buffer** out) {
            D3D11_BUFFER_DESC d{}; d.ByteWidth = sizeof(VTX_CBUFFER); d.Usage = D3D11_USAGE_DYNAMIC; d.BindFlags = D3D11_BIND_CONSTANT_BUFFER; d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(dev->CreateBuffer(&d, nullptr, out))) throw std::exception("vtx cb");
        }
    public:
        static blur& instance() { static blur s; return s; }
    };

    inline void NewFrame(ID3D11Device* device, ID3D11RenderTargetView* rtv, IDXGISwapChain* swap) {
        g_swap = swap;
        g_rtv = rtv;
        g_device = device;
        if (g_swap && g_rtv && g_device)
            blur::instance().new_frame();
    }
    inline void Before(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding, float alpha, ImDrawFlags flags, bool reuse, float angle_rad = 0.0f) {
        if (!g_effects_enabled || !g_swap || !g_rtv || !GetBD() || !GetBD()->pd3dDevice)
            return;
        blur::instance().draw(dl, min, max, rounding, alpha, flags, reuse, angle_rad);
    }

} // namespace img_blur
