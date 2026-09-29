#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_internal.h>
#include <d3d11.h>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <windows.h>

namespace crr
{
    inline void Log(const char* fmt, ...)
    {
        /*char buf[2048];
        va_list ap; va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        std::cout << "[ClipRectR] " << buf << std::endl;
        fprintf(stderr, "[ClipRectR] %s\n", buf);
        OutputDebugStringA("[ClipRectR] "); OutputDebugStringA(buf); OutputDebugStringA("\n");*/
    }

    inline ID3D11Device* gDev = nullptr;
    inline ID3D11DeviceContext* gCtx = nullptr;
    inline ImVec2               gTexSize{};
    inline bool                 gInited = false;

    inline bool  gFlipUV = false;                 // если картинка вверх ногами — переключи
    inline float gClear[4]{ 0, 0, 0, 0 };  // было {1,0,1,1}


    struct RT { ID3D11Texture2D* tex{}; ID3D11RenderTargetView* rtv{}; ID3D11ShaderResourceView* srv{}; };
    struct Props { ImVec2 min{}, max{}; float rounding{}; };

    struct SavedState
    {
        ID3D11RenderTargetView* rtv{};
        ID3D11DepthStencilView* dsv{};
        UINT vp_count{}; D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
        UINT sc_count{}; D3D11_RECT     scs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
        ID3D11BlendState* blend{}; FLOAT blend_factor[4]{}; UINT sample_mask{};
        ID3D11DepthStencilState* depthst{}; UINT stencil_ref{};
        ID3D11RasterizerState* rast{};
        ID3D11VertexShader* vs{}; ID3D11PixelShader* ps{}; ID3D11HullShader* hs{}; ID3D11DomainShader* ds{}; ID3D11GeometryShader* gs{};
        ID3D11ClassInstance* vs_ci[256]{}; UINT vs_cic{};
        ID3D11ClassInstance* ps_ci[256]{}; UINT ps_cic{};
        ID3D11ClassInstance* hs_ci[256]{}; UINT hs_cic{};
        ID3D11ClassInstance* ds_ci[256]{}; UINT ds_cic{};
        ID3D11ClassInstance* gs_ci[256]{}; UINT gs_cic{};
        ID3D11InputLayout* il{}; D3D11_PRIMITIVE_TOPOLOGY topo{};
        ID3D11Buffer* vb{}; UINT vb_stride{}; UINT vb_offset{};
        ID3D11Buffer* ib{}; DXGI_FORMAT ib_format{}; UINT ib_offset{};
        ID3D11Buffer* vs_cb[8]{}; ID3D11Buffer* ps_cb[8]{}; ID3D11SamplerState* ps_samp[8]{};
        ID3D11ShaderResourceView* ps_srv[8]{};
    };

    struct StackElem
    {
        RT rt{};
        Props props{};
        SavedState saved{};
        bool began{};
    };

    inline std::array<StackElem, 8> gStack{};
    inline size_t gSP = 0;

    inline void Release(RT& r) { if (r.srv) { r.srv->Release(); r.srv = nullptr; } if (r.rtv) { r.rtv->Release(); r.rtv = nullptr; } if (r.tex) { r.tex->Release(); r.tex = nullptr; } }
    inline void Create(RT& r) {
        D3D11_TEXTURE2D_DESC td{}; td.Width = (UINT)gTexSize.x; td.Height = (UINT)gTexSize.y; td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1; td.Usage = D3D11_USAGE_DEFAULT; td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (!gDev) { Log("Create: no device"); throw std::runtime_error("No device"); }
        if (FAILED(gDev->CreateTexture2D(&td, nullptr, &r.tex))) { Log("CreateTexture2D failed"); throw std::runtime_error("CreateTexture2D"); }
        if (FAILED(gDev->CreateRenderTargetView(r.tex, nullptr, &r.rtv))) { Log("CreateRTV failed"); throw std::runtime_error("CreateRTV"); }
        if (FAILED(gDev->CreateShaderResourceView(r.tex, nullptr, &r.srv))) { Log("CreateSRV failed"); throw std::runtime_error("CreateSRV"); }
        Log("RT created %ux%u", (unsigned)td.Width, (unsigned)td.Height);
    }
    inline void Ensure(RT& r) { if (!(r.tex && r.rtv && r.srv)) Create(r); }

    inline void Save(SavedState& s) {
        gCtx->OMGetRenderTargets(1, &s.rtv, &s.dsv);
        s.vp_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE; gCtx->RSGetViewports(&s.vp_count, s.vps);
        s.sc_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE; gCtx->RSGetScissorRects(&s.sc_count, s.scs);
        gCtx->OMGetBlendState(&s.blend, s.blend_factor, &s.sample_mask);
        gCtx->OMGetDepthStencilState(&s.depthst, &s.stencil_ref);
        gCtx->RSGetState(&s.rast);
        s.vs_cic = s.ps_cic = s.hs_cic = s.ds_cic = s.gs_cic = 256;
        gCtx->VSGetShader(&s.vs, s.vs_ci, &s.vs_cic);
        gCtx->PSGetShader(&s.ps, s.ps_ci, &s.ps_cic);
        gCtx->HSGetShader(&s.hs, s.hs_ci, &s.hs_cic);
        gCtx->DSGetShader(&s.ds, s.ds_ci, &s.ds_cic);
        gCtx->GSGetShader(&s.gs, s.gs_ci, &s.gs_cic);
        gCtx->IAGetInputLayout(&s.il);
        gCtx->IAGetPrimitiveTopology(&s.topo);
        gCtx->IAGetVertexBuffers(0, 1, &s.vb, &s.vb_stride, &s.vb_offset);
        gCtx->IAGetIndexBuffer(&s.ib, &s.ib_format, &s.ib_offset);
        gCtx->VSGetConstantBuffers(0, 8, s.vs_cb);
        gCtx->PSGetConstantBuffers(0, 8, s.ps_cb);
        gCtx->PSGetSamplers(0, 8, s.ps_samp);
        gCtx->PSGetShaderResources(0, 8, s.ps_srv);
    }
    inline void Restore(SavedState& s) {
        gCtx->OMSetRenderTargets(1, &s.rtv, s.dsv); if (s.rtv) s.rtv->Release(); if (s.dsv) s.dsv->Release();
        if (s.vp_count == 0) s.vp_count = 1; gCtx->RSSetViewports(s.vp_count, s.vps);
        if (s.sc_count == 0) s.sc_count = 1; gCtx->RSSetScissorRects(s.sc_count, s.scs);
        gCtx->OMSetBlendState(s.blend, s.blend_factor, s.sample_mask); if (s.blend) s.blend->Release();
        gCtx->OMSetDepthStencilState(s.depthst, s.stencil_ref); if (s.depthst) s.depthst->Release();
        gCtx->RSSetState(s.rast); if (s.rast) s.rast->Release();
        gCtx->VSSetShader(s.vs, s.vs_ci, s.vs_cic); if (s.vs) s.vs->Release(); for (UINT i = 0; i < s.vs_cic; i++) if (s.vs_ci[i]) s.vs_ci[i]->Release();
        gCtx->PSSetShader(s.ps, s.ps_ci, s.ps_cic); if (s.ps) s.ps->Release(); for (UINT i = 0; i < s.ps_cic; i++) if (s.ps_ci[i]) s.ps_ci[i]->Release();
        gCtx->HSSetShader(s.hs, s.hs_ci, s.hs_cic); if (s.hs) s.hs->Release(); for (UINT i = 0; i < s.hs_cic; i++) if (s.hs_ci[i]) s.hs_ci[i]->Release();
        gCtx->DSSetShader(s.ds, s.ds_ci, s.ds_cic); if (s.ds) s.ds->Release(); for (UINT i = 0; i < s.ds_cic; i++) if (s.ds_ci[i]) s.ds_ci[i]->Release();
        gCtx->GSSetShader(s.gs, s.gs_ci, s.gs_cic); if (s.gs) s.gs->Release(); for (UINT i = 0; i < s.gs_cic; i++) if (s.gs_ci[i]) s.gs_ci[i]->Release();
        gCtx->IASetInputLayout(s.il); if (s.il) s.il->Release();
        gCtx->IASetPrimitiveTopology(s.topo);
        gCtx->IASetVertexBuffers(0, 1, &s.vb, &s.vb_stride, &s.vb_offset); if (s.vb) s.vb->Release();
        gCtx->IASetIndexBuffer(s.ib, s.ib_format, s.ib_offset); if (s.ib) s.ib->Release();
        gCtx->VSSetConstantBuffers(0, 8, s.vs_cb);
        gCtx->PSSetConstantBuffers(0, 8, s.ps_cb);
        gCtx->PSSetSamplers(0, 8, s.ps_samp);
        gCtx->PSSetShaderResources(0, 8, s.ps_srv);
        for (int i = 0; i < 8; i++) { if (s.vs_cb[i]) s.vs_cb[i]->Release(); if (s.ps_cb[i]) s.ps_cb[i]->Release(); if (s.ps_samp[i]) s.ps_samp[i]->Release(); if (s.ps_srv[i]) s.ps_srv[i]->Release(); }
    }

    struct CB { size_t idx; };

    inline void BeginCB(const ImDrawList*, const ImDrawCmd* cmd)
    {
        auto* d = (CB*)cmd->UserCallbackData; if (!d) return;
        if (!gDev || !gCtx) { Log("BeginCB: no device/context"); delete d; return; }
        if (gTexSize.x <= 0 || gTexSize.y <= 0) { Log("BeginCB: bad tex size"); delete d; return; }
        if (d->idx >= gStack.size()) { Log("BeginCB: idx oob"); delete d; return; }

        auto& e = gStack[d->idx];
        Ensure(e.rt);
        Save(e.saved);

        ID3D11ShaderResourceView* nullsrv = nullptr;
        gCtx->PSSetShaderResources(0, 1, &nullsrv);

        D3D11_VIEWPORT vp{}; vp.TopLeftX = 0; vp.TopLeftY = 0; vp.Width = gTexSize.x; vp.Height = gTexSize.y; vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;

        const auto& p = e.props;
        D3D11_RECT scr{ (LONG)std::floor(p.min.x), (LONG)std::floor(p.min.y), (LONG)std::ceil(p.max.x), (LONG)std::ceil(p.max.y) };

        gCtx->OMSetRenderTargets(1, &e.rt.rtv, nullptr);
        gCtx->RSSetViewports(1, &vp);
        gCtx->RSSetScissorRects(1, &scr);
        gCtx->ClearRenderTargetView(e.rt.rtv, gClear);

        e.began = true;
        Log("BeginCB idx=%u rect=(%ld,%ld)-(%ld,%ld)", (unsigned)d->idx, scr.left, scr.top, scr.right, scr.bottom);
        delete d;
    }

    // ВАЖНО: reset сбивает RT; ребайндим наш RT обратно
    inline void RebindCB(const ImDrawList*, const ImDrawCmd* cmd)
    {
        auto* d = (CB*)cmd->UserCallbackData; if (!d || !gCtx) return;
        if (d->idx >= gStack.size()) { delete d; return; }
        auto& e = gStack[d->idx];

        D3D11_VIEWPORT vp{}; vp.TopLeftX = 0; vp.TopLeftY = 0; vp.Width = gTexSize.x; vp.Height = gTexSize.y; vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
        const auto& p = e.props;
        D3D11_RECT scr{ (LONG)std::floor(p.min.x), (LONG)std::floor(p.min.y), (LONG)std::ceil(p.max.x), (LONG)std::ceil(p.max.y) };

        gCtx->OMSetRenderTargets(1, &e.rt.rtv, nullptr);
        gCtx->RSSetViewports(1, &vp);
        gCtx->RSSetScissorRects(1, &scr);

        Log("RebindCB idx=%u", (unsigned)d->idx);
        delete d;
    }

    inline void EndCB(const ImDrawList*, const ImDrawCmd* cmd)
    {
        auto* d = (CB*)cmd->UserCallbackData; if (!d) return;
        if (!gDev || !gCtx) { Log("EndCB: no device/context"); delete d; return; }
        if (d->idx >= gStack.size()) { Log("EndCB: idx oob"); delete d; return; }

        auto& e = gStack[d->idx];
        if (!e.began) { Log("EndCB: not begun"); delete d; return; }

        Restore(e.saved);
        e.began = false;
        Log("EndCB idx=%u", (unsigned)d->idx);
        delete d;
    }

    inline void NewFrame(ID3D11Device* dev, ID3D11DeviceContext* ctx)
    {
        if (!gInited) { gDev = dev; gCtx = ctx; gInited = true; Log("Init"); }
        if (!gDev || !gCtx) return;

        ImVec2 ds = ImGui::GetMainViewport()->Size;
        if (ds.x <= 0 || ds.y <= 0) return;

        bool resized = (gTexSize.x != ds.x) || (gTexSize.y != ds.y);
        if (resized) {
            gTexSize = ds;
            for (auto& e : gStack) Release(e.rt);
            Log("Resize %.0fx%.0f", ds.x, ds.y);
        }
    }

    inline void Push(ImDrawList* dl, ImVec2 min, ImVec2 max, float rounding)
    {
        if (!gDev || !gCtx) { Log("Push: no device/context"); return; }
        if (gSP == gStack.size()) { Log("Push: overflow"); return; }
        if (min.x >= max.x || min.y >= max.y) { Log("Push: bad rect"); return; }

        gStack[gSP].props = { min,max,rounding };

        dl->AddCallback(BeginCB, new CB{ gSP });
        dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        dl->AddCallback(RebindCB, new CB{ gSP });

        ++gSP;
        Log("Push sp=%u rect=(%.1f,%.1f)-(%.1f,%.1f)", (unsigned)gSP, min.x, min.y, max.x, max.y);
    }

    inline void Pop(ImDrawList* dl)
    {
        if (gSP == 0) { Log("Pop: underflow"); return; }
        const size_t idx = gSP - 1;
        auto& s = gStack[idx];

        dl->AddCallback(EndCB, new CB{ idx });
        dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

        if (s.rt.srv) {
            ImVec2 uv0 = { s.props.min.x / gTexSize.x, s.props.min.y / gTexSize.y };
            ImVec2 uv1 = { s.props.max.x / gTexSize.x, s.props.max.y / gTexSize.y };
            if (gFlipUV) { uv0.y = 1.0f - uv0.y; uv1.y = 1.0f - uv1.y; }
            dl->AddImageRounded((ImTextureID)s.rt.srv, s.props.min, s.props.max, uv0, uv1, IM_COL32_WHITE, s.props.rounding);
            Log("Pop sp=%u draw srv=%p", (unsigned)idx, s.rt.srv);
        }
        else {
            Log("Pop: srv null");
        }

        gSP = idx;
    }

    inline void DebugOverlay(ImDrawList* dl)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "sp=%u size=%.0fx%.0f inited=%d flip=%d", (unsigned)gSP, gTexSize.x, gTexSize.y, gInited ? 1 : 0, gFlipUV ? 1 : 0);
        dl->AddRect(ImVec2(8, 8), ImVec2(360, 56), IM_COL32(0, 255, 0, 255));
        dl->AddText(ImVec2(12, 12), IM_COL32_WHITE, buf);
        for (size_t i = 0; i < gSP; i++) { auto& s = gStack[i]; dl->AddRect(s.props.min, s.props.max, IM_COL32(0, 128, 255, 200), s.props.rounding, 0, 2.0f); }
    }
}
