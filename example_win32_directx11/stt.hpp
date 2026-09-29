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

enum ImShaderTex : unsigned int { ImShaderTex_Default = 0, ImShaderTex_WindowBg, ImShaderTex_COUNT };

namespace shaderrt {

    using ImDrawFlags = int;

    inline ID3D11Device* g_device{};
    inline ID3D11DeviceContext* g_ctx{};
    inline ImColor g_main_col{};
    inline bool g_reset_time = false;

    // --- simple fullscreen VS ---
    inline const char* kVSH = R"(
struct VS_OUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
VS_OUT vs_main(uint id : SV_VertexID) {
    float2 p[3] = { float2(-1.0, -1.0), float2(-1.0, 3.0), float2(3.0, -1.0) };
    VS_OUT o; o.pos = float4(p[id], 0.0, 1.0); o.uv = (p[id] + 1.0) * 0.5; return o;
}
)";

    inline const char* kPSH = R"(
        cbuffer CB : register(b0) {
            float iTime;
            float iResolutionX;
            float iResolutionY;
            float pad;
        };

        SamplerState samp : register(s0);
        Texture2D dummy : register(t0);

        float rayStrength(float2 raySource, float2 rayRefDirection, float2 coord, float seedA, float seedB, float speed) {
            float2 sourceToCoord = coord - raySource;
            float cosAngle = dot(normalize(sourceToCoord), rayRefDirection);
    
            return clamp(
                (0.35 + 0.15 * sin(cosAngle * seedA + iTime * speed)) +
                (0.2 + 0.2 * cos(-cosAngle * seedB + iTime * speed)),
                0.0, 1.0) *
                clamp((iResolutionX - length(sourceToCoord)) / iResolutionX, 0.5, 1.0);
        }

        struct PS_IN { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

        float4 ps_main(PS_IN IN) : SV_Target {
            float2 fragCoord = float2(IN.pos.x, IN.pos.y);
            float2 uv = fragCoord / float2(iResolutionX, iResolutionY);
            float2 coord = float2(fragCoord.x, fragCoord.y);
    
            // Источники света сверху (отрицательные Y координаты)
            float2 rayPos1 = float2(iResolutionX * 0.7, iResolutionY * -0.4);
            float2 rayRefDir1 = normalize(float2(1.0, 0.116));
            float raySeedA1 = 52.2214;
            float raySeedB1 = 31.11349;
            float raySpeed1 = 2.5;
    
            float2 rayPos2 = float2(iResolutionX * 0.8, iResolutionY * -0.6);
            float2 rayRefDir2 = normalize(float2(1.0, -0.241));
            float raySeedA2 = 42.39910;
            float raySeedB2 = 28.0234;
            float raySpeed2 = 2.1;
    
            float4 rays1 = float4(1.0, 1.0, 1.0, 1.0) *
                rayStrength(rayPos1, rayRefDir1, coord, raySeedA1, raySeedB1, raySpeed1);
     
            float4 rays2 = float4(1.0, 1.0, 1.0, 1.0) *
                rayStrength(rayPos2, rayRefDir2, coord, raySeedA2, raySeedB2, raySpeed2);
    
            float4 fragColor = rays1 * 0.5 + rays2 * 0.4;
    
            // Градиент яркости - темнее внизу
            float brightness = 1.0 - (coord.y / iResolutionY);
            fragColor.x *= 0.1 + (brightness * 0.8);
            fragColor.y *= 0.3 + (brightness * 0.6);
            fragColor.z *= 0.5 + (brightness * 0.5);

            return float4(fragColor.rgb, 1.0);
        }
)";


    // --- helper DX11 wrappers ---
    struct dx11_draw_data {
        ID3D11VertexShader* vertex_shader{};
        ID3D11InputLayout* input_layout{};
        ID3D11PixelShader* pixel_shader{};
        ID3D11Buffer* pixel_constant_buffer{};
        ID3D11BlendState* blend_state{};
    };

    struct dx11_tex {
        ID3D11Texture2D* tex{};
        ID3D11RenderTargetView* rtv{};
        ID3D11ShaderResourceView* srv{};
        ImVec2 size{};
        ImVec2 requested_size{};

        void release() {
            if (rtv) { rtv->Release(); rtv = nullptr; }
            if (tex) { tex->Release(); tex = nullptr; }
            if (srv) { srv->Release(); srv = nullptr; }
        }

        void create_tex(ImVec2 req_size) {
            requested_size = req_size;
            size = req_size;
            D3D11_TEXTURE2D_DESC d = {};
            d.Width = (UINT)size.x;
            d.Height = (UINT)size.y;
            d.MipLevels = 1;
            d.ArraySize = 1;
            d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            d.SampleDesc.Count = 1;
            d.Usage = D3D11_USAGE_DEFAULT;
            d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(g_device->CreateTexture2D(&d, nullptr, &tex)))
                throw std::exception("CreateTexture2D");
        }

        void create_rtv() {
            if (FAILED(g_device->CreateRenderTargetView(tex, nullptr, &rtv)))
                throw std::exception("CreateRTV");
        }

        void create_srv() {
            if (FAILED(g_device->CreateShaderResourceView(tex, nullptr, &srv)))
                throw std::exception("CreateSRV");
        }

        void bind(ID3D11DeviceContext* ctx, ImVec2 req_size) {
            if ((int)req_size.x != (int)requested_size.x ||
                (int)req_size.y != (int)requested_size.y) {
                release();
                create_tex(req_size);
                create_rtv();
                create_srv();
            }
            ctx->OMSetRenderTargets(1, &rtv, nullptr);
        }

        ImTextureID texid() const { return srv; }
    };

    struct dx11_shader {
        dx11_draw_data dd{};
        dx11_tex tex;
        std::function<void(void*, ImVec2)> fill_cb;
        size_t cb_size{};
        ImVec2 render_size{};

        static void print_err(ID3DBlob* e) {}

        static void bind_vp(ID3D11DeviceContext* ctx, ImVec2 size) {
            D3D11_VIEWPORT vp{};
            vp.Width = size.x;
            vp.Height = size.y;
            vp.MinDepth = 0;
            vp.MaxDepth = 1;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            ctx->RSSetViewports(1, &vp);
        }

        void make_vs(const char* src) {
            ID3DBlob* err{}, * blob{};
            if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &blob, &err))) {
                print_err(err);
                throw std::exception("vs compile");
            }
            if (g_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &dd.vertex_shader) != S_OK) {
                blob->Release();
                print_err(err);
                throw std::exception("vs create");
            }
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                {"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
                {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D11_INPUT_PER_VERTEX_DATA,0}
            };
            if (g_device->CreateInputLayout(layout, 2, blob->GetBufferPointer(), blob->GetBufferSize(), &dd.input_layout) != S_OK) {
                blob->Release();
                throw std::exception("input layout");
            }
            blob->Release();
        }

        void make_ps(const char* src) {
            ID3DBlob* err{}, * blob{};
            if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &blob, &err))) {
                print_err(err);
                if (blob) blob->Release();
                throw std::exception("ps compile");
            }
            if (g_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &dd.pixel_shader) != S_OK) {
                blob->Release();
                throw std::exception("ps create");
            }
            blob->Release();
        }

        void make_cb() {
            D3D11_BUFFER_DESC b{};
            b.Usage = D3D11_USAGE_DYNAMIC;
            b.ByteWidth = (UINT)cb_size;
            b.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            b.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(g_device->CreateBuffer(&b, nullptr, &dd.pixel_constant_buffer)))
                throw std::exception("cb");
        }

        void make_blend() {
            D3D11_BLEND_DESC d{};
            d.AlphaToCoverageEnable = FALSE;
            d.RenderTarget[0].BlendEnable = TRUE;
            d.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            d.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            d.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            d.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            d.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            d.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            d.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            if (FAILED(g_device->CreateBlendState(&d, &dd.blend_state)))
                throw std::exception("blend");
        }

        dx11_shader(const char* vsh, const char* psh, size_t cb_sz, std::function<void(void*, ImVec2)> cb)
            : fill_cb(std::move(cb)), cb_size(cb_sz) {
            make_vs(vsh);
            make_ps(psh);
            make_cb();
            make_blend();
        }

        void bind(ID3D11DeviceContext* ctx, ImVec2 size) {
            render_size = size;
            tex.bind(ctx, size);
            bind_vp(ctx, size);
            ctx->IASetInputLayout(dd.input_layout);
            ctx->VSSetShader(dd.vertex_shader, nullptr, 0);
            ctx->PSSetShader(dd.pixel_shader, nullptr, 0);

            void* mem = malloc(cb_size);
            fill_cb(mem, size);

            D3D11_MAPPED_SUBRESOURCE m{};
            if (FAILED(ctx->Map(dd.pixel_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
                free(mem);
                throw std::exception("map cb");
            }
            memcpy(m.pData, mem, cb_size);
            free(mem);
            ctx->Unmap(dd.pixel_constant_buffer, 0);
            ctx->PSSetConstantBuffers(0, 1, &dd.pixel_constant_buffer);

            constexpr float bf[4] = { 0,0,0,0 };
            ctx->OMSetBlendState(dd.blend_state, bf, 0xffffffff);

            static ID3D11SamplerState* s = nullptr;
            if (!s) {
                D3D11_SAMPLER_DESC sd{};
                sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                sd.MaxLOD = D3D11_FLOAT32_MAX;
                g_device->CreateSamplerState(&sd, &s);
            }
            ctx->PSSetSamplers(0, 1, &s);
        }

        ImTextureID texid() const { return tex.texid(); }
    };

    struct dx11_vbo {
        ID3D11Buffer* vb{};
        dx11_vbo() {
            struct V { float px, py, ux, uy; };
            const V v[] = {
                {-1.f,  1.f, 0.f, 0.f}, { 1.f,  1.f, 1.f, 0.f}, {-1.f, -1.f, 0.f, 1.f},
                {-1.f, -1.f, 0.f, 1.f}, { 1.f,  1.f, 1.f, 0.f}, { 1.f, -1.f, 1.f, 1.f},
            };
            D3D11_BUFFER_DESC d{};
            d.Usage = D3D11_USAGE_DEFAULT;
            d.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            d.ByteWidth = sizeof(v);
            D3D11_SUBRESOURCE_DATA i{};
            i.pSysMem = v;
            if (FAILED(g_device->CreateBuffer(&d, &i, &vb)))
                throw std::exception("vb");
        }
        void draw(ID3D11DeviceContext* ctx) const {
            UINT stride = sizeof(float) * 4;
            UINT offset = 0;
            ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->Draw(6, 0);
        }
    };

    constexpr size_t kShadersCount = ImShaderTex_COUNT;
    inline std::array<std::unique_ptr<dx11_shader>, kShadersCount> g_shaders{};
    inline std::unique_ptr<dx11_vbo> g_vbo{};
    inline std::map<ImShaderTex, ImVec2> g_shader_sizes;

    struct alignas(16) shader_data {
        float in_time;
        float in_resolution[2];
        float pad;
    };
    static_assert(sizeof(shader_data) % 16 == 0, "cb size must be multiple of 16");

    inline void NewFrame(IDXGISwapChain* /*swap_chain*/, ID3D11Device* device, ID3D11DeviceContext* ctx, ImColor main_color) {
        g_device = device;
        g_ctx = ctx;
        g_main_col = main_color;

        if (!g_vbo) g_vbo = std::make_unique<dx11_vbo>();

        ImVec2 default_size(400, 300);

        if (!g_shaders[ImShaderTex_Default]) {
            g_shaders[ImShaderTex_Default] = std::make_unique<dx11_shader>(
                kVSH, kPSH, sizeof(shader_data),
                [](void* p, ImVec2 size) {
                    auto& d = *reinterpret_cast<shader_data*>(p);
                    d.in_time = (float)ImGui::GetTime();
                    d.in_resolution[0] = size.x;
                    d.in_resolution[1] = size.y;
                    d.pad = 0.0f;
                }
            );
        }

        if (!g_shaders[ImShaderTex_WindowBg]) {
            g_shaders[ImShaderTex_WindowBg] = std::make_unique<dx11_shader>(
                kVSH, kPSH, sizeof(shader_data),
                [](void* p, ImVec2 size) {
                    auto& d = *reinterpret_cast<shader_data*>(p);
                    d.in_time = (float)ImGui::GetTime();
                    d.in_resolution[0] = size.x;
                    d.in_resolution[1] = size.y;
                    d.pad = 0.0f;
                    g_reset_time = true;
                }
            );
        }

        for (size_t i = 0; i < g_shaders.size(); ++i) {
            if (!g_shaders[i]) continue;

            ImVec2 size = g_shader_sizes.count((ImShaderTex)i) > 0
                ? g_shader_sizes[(ImShaderTex)i]
                : default_size;

            g_shaders[i]->bind(ctx, size);
            g_vbo->draw(ctx);
        }
    }

    inline ImTextureID Get(ImShaderTex s) {
        if (!g_shaders[0]) return nullptr;
        return g_shaders[s]->texid();
    }

    inline void Draw(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding, float alpha, ImShaderTex s) {
        ImVec2 size(max.x - min.x, max.y - min.y);
        g_shader_sizes[s] = size;

        auto* t = Get(s);
        if (!t) return;

        const auto uv_min = ImVec2(0, 0);
        const auto uv_max = ImVec2(1, 1);

        dl->AddImageRounded(t, min, max, uv_min, uv_max,
            ImColor(g_main_col.Value.x, g_main_col.Value.y,
                g_main_col.Value.z, alpha), rounding);
    }

    inline void UI() {
        ImGui::Text("Sun rays shader");
    }

} // namespace shaderrt